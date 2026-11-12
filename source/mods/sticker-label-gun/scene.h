/* Provider-owned scene transactions: no registry entry until the actual quad
 * is rooted, collider-free, parented, posed and assigned its atlas material. */
static PTR sticker_component(PTR go,const char *name,int *ok){
    PTR type=su_type("UnityEngine",name);if(!type){*ok=0;return 0;}
    void *args[]={type};return call(su.go_component,go,args,ok);
}
static SURoot sticker_quad(PTR parent,PTR material,SPVec3 position,SPVec3 forward,SPVec3 up){
    int ok=1,shape=5,layer=2;unsigned char disabled=0,keep_world=0;
    SURoot root=0,array=0;void *create[]={&shape};
    PTR go=call(su.primitive,0,create,&ok);
    if(!ok||!go){su_destroy(go);return 0;}
    root=su_hold(go);if(!root){su_destroy(go);return 0;}
    PTR collider=sticker_component(go,"Collider",&ok);if(!ok)goto failed;
    PTR renderer=sticker_component(go,"Renderer",&ok);if(!ok)goto failed;
    PTR tr=call(su.go_transform,go,0,&ok);
    if(!ok||!collider||!renderer||!tr||!parent||!material)goto failed;
    void *off[]={&disabled},*layers[]={&layer},*parenting[]={parent,&keep_world};
    call(su.collider_enabled,collider,off,&ok);if(!ok)goto failed;
    call(label.layer,go,layers,&ok);if(!ok)goto failed;
    call(su.tr_parent,tr,parenting,&ok);if(!ok)goto failed;
    SPVec3 scale={.58f,.58f,1};void *size[]={&scale},*point[]={&position};
    call(su.tr_scale,tr,size,&ok);if(!ok)goto failed;
    call(su.tr_position,tr,point,&ok);if(!ok)goto failed;
    void *orientation[]={&forward,&up};PTR boxed=call(su.look_rotation,0,orientation,&ok);
    if(!ok||!boxed)goto failed;
    SUQuat rotation;su_copy(&rotation,unbox(boxed),sizeof(rotation));void *pose[]={&rotation};
    call(su.tr_rotation,tr,pose,&ok);if(!ok)goto failed;
    array=su_hold(su.new_array(su.material_class,1));if(!array)goto failed;
    PTR *items=su.array_address(su_get(array),sizeof(PTR),0);if(!items)goto failed;
    tt.write_barrier(su_get(array),items,material);void *materials[]={su_get(array)};
    call(su.renderer_materials,renderer,materials,&ok);if(!ok)goto failed;
    su_drop(&array);return root;
failed:
    su_drop(&array);su_destroy(su_get(root));su_drop(&root);return 0;
}
static void label_gun(int visible){
    if(!visible){su_destroy(su_get(label.gun));su_drop(&label.gun);return;}
    if(label.gun){
        /* A native camera child may die before its managed handle. A failed
         * liveness query is unknown; only confirmed destruction permits retry. */
        int ok=1;void *args[]={su_get(label.gun)};PTR live=call(su.obj_alive,0,args,&ok);
        if(!ok||!live||*(unsigned char *)unbox(live))return;
        su_drop(&label.gun);
    }
    if(label.visual_retry>0)return;
    label.visual_retry=2;
    PTR parent=tt_transform(su_get(tt.camera));if(!parent)return;
    /* Immutable upload resources survive holstering. The camera instance has
     * no Collider and uses layer 2, so sticker rays ignore it. */
    PTR go=sm_attach(&lg_model,&label.gun_resources,parent,2,"Handheld sticker label gun");
    label.gun=su_hold(go);if(!label.gun){su_destroy(go);return;}
    int ok=1;PTR tr=call(su.go_transform,go,0,&ok);if(!tr)ok=0;
    void *position[]={(void *)&lg_camera_position};if(ok)call(su.tr_local_position,tr,position,&ok);
    if(!ok){su_destroy(go);su_drop(&label.gun);return;}
    label.visual_retry=0;tt_event("model-attached");
}
static void feature_drop(void){
    for(unsigned i=0;i<label.count;i++){su_destroy(su_get(label.stickers[i]));su_drop(&label.stickers[i]);}
    label.count=0;label_gun(0);sm_release(&label.gun_resources);
    sticker_release_resources(&label.texture,label.materials);label.equipped=0;label.visual_retry=0;
}
