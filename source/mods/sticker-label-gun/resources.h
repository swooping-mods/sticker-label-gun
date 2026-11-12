/* Publish only a complete atlas/material set. A failed native property call
 * must release temporary resources so the next placement can actually retry. */
static void sticker_release_resources(SURoot *texture,SURoot materials[4]) {
    for(unsigned i=0;i<4;i++){su_destroy(su_get(materials[i]));su_drop(materials+i);}
    su_destroy(su_get(*texture));su_drop(texture);
}
static SURoot sticker_new_material(void) {
    const char *names[]={"Universal Render Pipeline/Unlit","Sprites/Default"};
    int ok=1;PTR shader=0;
    for(unsigned i=0;i<2&&!shader;i++) {
        SURoot name=su_hold(su.string(names[i]));if(!name)return 0;
        void *args[]={su_get(name)};shader=call(su.shader_find,0,args,&ok);su_drop(&name);
        if(!ok)return 0;
    }
    if(!shader)return 0;
    SURoot shader_root=su_hold(shader);if(!shader_root)return 0;
    PTR object=su_new("UnityEngine","Material");SURoot material=su_hold(object);
    if(!material){su_destroy(object);su_drop(&shader_root);return 0;}
    /* Keep ownership before invoking constructors/setters: their exception
     * cannot be allowed to hide an already-created native object. */
    void *ctor[]={su_get(shader_root)};call(su.material_ctor,su_get(material),ctor,&ok);
    SUColor white={1,1,1,1};void *color[]={&white};
    if(ok)call(su.material_color,su_get(material),color,&ok);
    su_drop(&shader_root);
    if(!ok){su_destroy(su_get(material));su_drop(&material);return 0;}
    return material;
}
static int sticker_texture_slot(PTR material,PTR texture,const char *name,T2 scale,T2 offset) {
    int ok=1;SURoot property=su_hold(su.string(name));if(!property)return -1;
    void *query[]={su_get(property)};PTR found=call(label.has_property,material,query,&ok);
    if(!ok||!found){su_drop(&property);return -1;}
    if(!*(unsigned char *)unbox(found)){su_drop(&property);return 0;}
    void *image[]={su_get(property),texture},*scaling[]={su_get(property),&scale},*placement[]={su_get(property),&offset};
    call(label.texture_set,material,image,&ok);
    if(!ok)tt_event("sticker-texture-set-failed");
    if(ok){call(label.scale,material,scaling,&ok);if(!ok)tt_event("sticker-texture-scale-failed");}
    if(ok){call(label.offset,material,placement,&ok);if(!ok)tt_event("sticker-texture-offset-failed");}
    su_drop(&property);return ok?1:-1;
}
static int sticker_material_culling(PTR material) {
    int ok=1;SURoot property=su_hold(su.string("_Cull"));if(!property)return 0;
    void *query[]={su_get(property)};PTR found=call(label.has_property,material,query,&ok);
    if(!ok||!found){su_drop(&property);return 0;}
    if(*(unsigned char *)unbox(found)) {
        /* SetInt is stripped from the pinned game; its float shader property
         * uses the same already-bound SetFloat path as the model materials. */
        float off=0;void *args[]={su_get(property),&off};
        if(su.material_float)call(su.material_float,material,args,&ok);else ok=0;
    }
    su_drop(&property);return ok;
}
static int sticker_resources(void) {
    if(label.texture)return 1;
    char path[1200];DWORD n=label.module_path(0,path,1000);
    if(!n||n>=1000){tt_event("sticker-module-path-failed");return 0;}
    while(n&&path[n-1]!='\\'&&path[n-1]!='/')n--;
    path[n]=0;cat(path+n,"MoonLab\\mods\\sticker-label-gun\\assets\\sticker-atlas.png");
    PTR file=CreateFileA(path,0x80000000,1,0,3,0x80,0);
    if(file==(PTR)(U64)-1){tt.status="Sticker atlas missing from installed package";tt_event("sticker-atlas-open-failed");return 0;}
    U64 size=0;int ok=label.file_size(file,&size)&&size>8&&size<=8*1024*1024;
    SURoot bytes=ok?su_hold(su.new_array(su.byte_class,size)):0;DWORD got=0;
    ok=ok&&bytes&&tt.read(file,su.array_address(su_get(bytes),1,0),(DWORD)size,&got,0)&&got==size;
    tt.close(file);
    if(!ok){tt_event("sticker-atlas-read-failed");su_drop(&bytes);return 0;}
    int side=2,format=4;unsigned char mip=1,linear=0;
    PTR texture_object=su_new("UnityEngine","Texture2D");
    SURoot texture=su_hold(texture_object),materials[4]={0};
    if(!texture){su_destroy(texture_object);su_drop(&bytes);tt_event("sticker-atlas-root-failed");goto failed;}
    void *ctor[]={&side,&side,&format,&mip,&linear};call(label.texture_ctor,su_get(texture),ctor,&ok);
    void *load[]={su_get(texture),su_get(bytes)};PTR loaded=ok?call(label.load_image,0,load,&ok):0;su_drop(&bytes);
    if(!ok||!loaded||!*(unsigned char *)unbox(loaded)){tt_event("sticker-atlas-decode-failed");goto failed;}
    for(unsigned i=0;i<4;i++) {
        materials[i]=sticker_new_material();PTR material=su_get(materials[i]);
        if(!material){tt_event("sticker-material-create-failed");goto failed;}
        T2 scale={.5f,.5f},offset={(float)(i%2)*.5f,i<2?.5f:0};
        const char *names[]={"_BaseMap","_MainTex"};int mapped=0;
        for(unsigned j=0;j<2;j++) {
            int supported=sticker_texture_slot(material,su_get(texture),names[j],scale,offset);
            if(supported<0){tt_event("sticker-material-properties-failed");goto failed;}
            mapped+=supported;
        }
        if(!mapped){tt_event("sticker-shader-no-texture-slot");goto failed;}
        if(!sticker_material_culling(material)){tt_event("sticker-material-culling-failed");goto failed;}
    }
    /* The atlas pointer is the cache-ready marker, published after all four. */
    for(unsigned i=0;i<4;i++)label.materials[i]=materials[i];
    label.texture=texture;return 1;
failed:
    sticker_release_resources(&texture,materials);return 0;
}
