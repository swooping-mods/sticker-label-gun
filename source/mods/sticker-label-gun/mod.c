#include "../../runtime/native/gameplay.h"
#define TOOL_ID "sticker-label-gun"
#define TOOL_DESCRIPTION "Handheld label gun with four sticker designs. Select in Equipment; Q cycles, R places and Backspace undoes."
#define TOOL_NAME "Sticker Label Gun"
#define TOOL_PRICE 60
#define TOOL_KEY 281
#define TOOL_HOTKEY "PageDown"
#define TOOL_ROW 1
#include "tool_runtime.h"
#include "../../runtime/sdk/scene/static_model.h"
#include "assets/generated/model_data.h"
int _fltused=0;
static struct {SURoot texture,materials[4],stickers[128],gun;SMResources gun_resources;float visual_retry;unsigned count,design;int equipped;PTR hit_transform,texture_ctor,load_image,texture_set,scale,offset;DWORD(__stdcall *module_path)(PTR,char *,DWORD);BOOL(__stdcall *file_size)(PTR,U64 *);} label;
static const char *designs[]={"Eat my dust","Certified Moon Sweeper","No dust left behind","Lunar Cleanup Crew"};
static int sticker_resources(void){
    if(label.texture)return 1;char path[1200];DWORD n=label.module_path(0,path,1000);if(!n||n>=1000)return 0;while(n&&path[n-1]!='\\'&&path[n-1]!='/')n--;path[n]=0;cat(path+n,"MoonLab\\mods\\sticker-label-gun\\assets\\sticker-atlas.png");
    PTR file=CreateFileA(path,0x80000000,1,0,3,0x80,0);if(file==(PTR)(U64)-1){tt.status="Sticker atlas missing from installed package";return 0;}
    U64 size=0;int ok=label.file_size(file,&size)&&size>8&&size<=8*1024*1024;SURoot bytes=ok?su_hold(su.new_array(su.byte_class,size)):0;DWORD got=0;
    ok=ok&&bytes&&tt.read(file,su.array_address(su_get(bytes),1,0),(DWORD)size,&got,0)&&got==size;tt.close(file);if(!ok){su_drop(&bytes);return 0;}
    int side=2,format=4;unsigned char mip=1,linear=0;SURoot texture=su_hold(su_new("UnityEngine","Texture2D"));void *ctor[]={&side,&side,&format,&mip,&linear};call(label.texture_ctor,su_get(texture),ctor,&ok);void *load[]={su_get(texture),su_get(bytes)};PTR loaded=call(label.load_image,0,load,&ok);su_drop(&bytes);
    if(!ok||!loaded||!*(unsigned char *)unbox(loaded)){su_destroy(su_get(texture));su_drop(&texture);return 0;}label.texture=texture;
    for(unsigned i=0;i<4;i++){
        label.materials[i]=su_hold(tt_material((SUColor){1,1,1,1}));PTR material=su_get(label.materials[i]);if(!material)return 0;
        T2 scale={.5f,.5f},offset={(float)(i%2)*.5f,i<2?.5f:0};const char *names[]={"_BaseMap","_MainTex"};
        for(unsigned j=0;j<2;j++){void *a[]={su.string(names[j]),su_get(label.texture)},*b[]={su.string(names[j]),&scale},*c[]={su.string(names[j]),&offset};call(label.texture_set,material,a,&ok);call(label.scale,material,b,&ok);call(label.offset,material,c,&ok);}
        PTR cull=su_method("UnityEngine","Material","SetInt","System.String;System.Int32");int off=0;void *c[]={su.string("_Cull"),&off};call(cull,material,c,&ok);
    }
    return ok;
}
static void label_gun(int visible){
    if(!visible){su_destroy(su_get(label.gun));su_drop(&label.gun);return;}
    if(label.gun||label.visual_retry>0)return;label.visual_retry=2;
    PTR parent=tt_transform(su_get(tt.camera));if(!parent)return;
    /* Immutable mesh/material resources survive holstering; only the camera
     * instance is replaced. Layer 2 and no Collider keep it out of sticker rays. */
    PTR go=sm_attach(&lg_model,&label.gun_resources,parent,2,"Handheld sticker label gun");
    label.gun=su_hold(go);if(!label.gun){su_destroy(go);return;}
    su_set_vector(su.tr_local_position,su_transform(go),lg_camera_position);
    label.visual_retry=0;tt_event("model-attached");
}
static void place_sticker(void){
    if(label.count>=128){tt.status="128 sticker limit reached; Backspace removes last";return;}if(!sticker_resources())return;
    PTR camera=tt_transform(su_get(tt.camera));SPVec3 origin=su_vector(su.tr_get_position,camera),forward=su_vector(su.tr_forward,camera);unsigned char hit[64];if(!tt_ray(origin,forward,8,hit)){tt.status="Aim at a surface within 8 m";return;}
    int ok=1;PTR parent=call(label.hit_transform,hit,0,&ok);if(!ok||!parent)return;SPVec3 point,normal;su_copy(&point,hit+su.hit_point,12);su_copy(&normal,hit+su.hit_normal,12);normal=su_normal(normal);
    PTR go=tt_primitive(parent,5,su_v(0,0,0),su_v(.58f,.58f,1),su_get(label.materials[label.design]));if(!go)return;PTR tr=su_transform(go);su_set_vector(su.tr_position,tr,su_add(point,su_mul(normal,.006f)));
    SPVec3 up=su_vector(su.tr_up,camera);up=su_sub(up,su_mul(normal,su_dot(up,normal)));if(su_dot(up,up)<.01f)up=su_cross(normal,su_vector(su.tr_right,camera));su_set_quat(su.tr_rotation,tr,su_rotation(su_mul(normal,-1),su_normal(up)));
    label.stickers[label.count++]=su_hold(go);tt.status="Sticker placed";tt_event("sticker-placed");
}
static void feature_drop(void){for(unsigned i=0;i<label.count;i++){su_destroy(su_get(label.stickers[i]));su_drop(&label.stickers[i]);}label.count=0;label_gun(0);sm_release(&label.gun_resources);for(unsigned i=0;i<4;i++){su_destroy(su_get(label.materials[i]));su_drop(&label.materials[i]);}su_destroy(su_get(label.texture));su_drop(&label.texture);label.equipped=0;label.visual_retry=0;}
static void feature_tick(PTR walker,PTR world,int allow){
    (void)walker;(void)world;if(label.visual_retry>0)label.visual_retry-=tt.dt;label.equipped=allow&&tt.owned;int active=allow&&tt.owned&&label.equipped;label_gun(active);
    MoonInputState cycle=tt_action("cycle",113,active),place=tt_action("place",114,active),remove=tt_action("remove",8,active);
    if(active){if(cycle.pressed)label.design=(label.design+1)%4;if(place.pressed)place_sticker();if(remove.pressed&&label.count){SURoot *last=&label.stickers[--label.count];su_destroy(su_get(*last));su_drop(last);tt_event("sticker-removed");}}
    char b[256],*p=cat(b,label.equipped?"Q: Design  |  R: Place sticker  |  Backspace: Undo\n":"Select label gun in Equipment\n");p=cat(p,designs[label.design]);p=cat(p,"  |  ");p=num(p,label.count);p=cat(p," / 128 stickers");if(tt.status){p=cat(p,"\n");cat(p,tt.status);}tt_shop_text(b);
}
__declspec(dllexport) int MoonLab_ModStart(PTR domain,PTR game){
    if(!tt_start(domain,game))return 0;PTR k=GetModuleHandleA("kernel32.dll");label.module_path=(DWORD(__stdcall *)(PTR,char *,DWORD))GetProcAddress(k,"GetModuleFileNameA");label.file_size=(BOOL(__stdcall *)(PTR,U64 *))GetProcAddress(k,"GetFileSizeEx");
    label.hit_transform=su_method("UnityEngine","RaycastHit","get_transform","");label.texture_ctor=su_method("UnityEngine","Texture2D",".ctor","System.Int32;System.Int32;UnityEngine.TextureFormat;System.Boolean;System.Boolean");label.load_image=su_method("UnityEngine","ImageConversion","LoadImage","UnityEngine.Texture2D;System.Byte[]");label.texture_set=su_method("UnityEngine","Material","SetTexture","System.String;UnityEngine.Texture");label.scale=su_method("UnityEngine","Material","SetTextureScale","System.String;UnityEngine.Vector2");label.offset=su_method("UnityEngine","Material","SetTextureOffset","System.String;UnityEngine.Vector2");
    return label.module_path&&label.file_size&&label.hit_transform&&label.texture_ctor&&label.load_image&&label.texture_set&&label.scale&&label.offset&&tt_install(game);
}
