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
static struct {SURoot texture,materials[4],stickers[128],gun;SMResources gun_resources;float visual_retry;unsigned count,design;int equipped;PTR hit_transform,texture_ctor,load_image,texture_set,scale,offset,has_property,layer;DWORD(__stdcall *module_path)(PTR,char *,DWORD);BOOL(__stdcall *file_size)(PTR,U64 *);} label;
static const char *designs[]={"Eat my dust","Certified Moon Sweeper","No dust left behind","Lunar Cleanup Crew"};
#include "resources.h"
#include "scene.h"
static void place_sticker(void){
    if(label.count>=128){tt.status="128 sticker limit reached; Backspace removes last";return;}if(!sticker_resources())return;
    PTR camera=tt_transform(su_get(tt.camera));if(!camera){tt_event("sticker-camera-unavailable");return;}SPVec3 origin=su_vector(su.tr_get_position,camera),forward=su_vector(su.tr_forward,camera);unsigned char hit[64];if(!tt_ray(origin,forward,8,hit)){tt.status="Aim at a surface within 8 m";tt_event("sticker-ray-missed");return;}
    int ok=1;PTR parent=call(label.hit_transform,hit,0,&ok);if(!ok||!parent||!su_alive(parent)){tt_event("sticker-hit-parent-unavailable");return;}SPVec3 point,normal;su_copy(&point,hit+su.hit_point,12);su_copy(&normal,hit+su.hit_normal,12);normal=su_normal(normal);
    SPVec3 up=su_vector(su.tr_up,camera);up=su_sub(up,su_mul(normal,su_dot(up,normal)));if(su_dot(up,up)<.01f)up=su_cross(normal,su_vector(su.tr_right,camera));
    SURoot sticker=sticker_quad(parent,su_get(label.materials[label.design]),su_add(point,su_mul(normal,.006f)),su_mul(normal,-1),su_normal(up));
    if(!sticker){tt_event("sticker-quad-create-failed");return;}
    label.stickers[label.count++]=sticker;tt.status="Sticker placed";tt_event("sticker-placed");
}
static void feature_tick(PTR walker,PTR world,int allow){
    (void)walker;(void)world;if(label.visual_retry>0)label.visual_retry-=tt.dt;label.equipped=allow&&tt.owned;int active=allow&&tt.owned&&label.equipped;label_gun(active);
    MoonInputState cycle=tt_action("cycle",113,active),place=tt_action("place",114,active),remove=tt_action("remove",8,active);
    if(active){if(cycle.pressed)label.design=(label.design+1)%4;if(place.pressed){tt_event("sticker-place-request");place_sticker();}if(remove.pressed&&label.count){SURoot *last=&label.stickers[--label.count];su_destroy(su_get(*last));su_drop(last);tt_event("sticker-removed");}}
    char b[256],*p=cat(b,label.equipped?"Q: Design  |  R: Place sticker  |  Backspace: Undo\n":"Select label gun in Equipment\n");p=cat(p,designs[label.design]);p=cat(p,"  |  ");p=num(p,label.count);p=cat(p," / 128 stickers");if(tt.status){p=cat(p,"\n");cat(p,tt.status);}tt_shop_text(b);
}
__declspec(dllexport) int MoonLab_ModStart(PTR domain,PTR game){
    if(!tt_start(domain,game))return 0;PTR k=GetModuleHandleA("kernel32.dll");label.module_path=(DWORD(__stdcall *)(PTR,char *,DWORD))GetProcAddress(k,"GetModuleFileNameA");label.file_size=(BOOL(__stdcall *)(PTR,U64 *))GetProcAddress(k,"GetFileSizeEx");
    label.layer=su_method("UnityEngine","GameObject","set_layer","System.Int32");label.hit_transform=su_method("UnityEngine","RaycastHit","get_transform","");label.texture_ctor=su_method("UnityEngine","Texture2D",".ctor","System.Int32;System.Int32;UnityEngine.TextureFormat;System.Boolean;System.Boolean");label.load_image=su_method("UnityEngine","ImageConversion","LoadImage","UnityEngine.Texture2D;System.Byte[]");label.texture_set=su_method("UnityEngine","Material","SetTexture","System.String;UnityEngine.Texture");label.scale=su_method("UnityEngine","Material","SetTextureScale","System.String;UnityEngine.Vector2");label.offset=su_method("UnityEngine","Material","SetTextureOffset","System.String;UnityEngine.Vector2");label.has_property=su_method("UnityEngine","Material","HasProperty","System.String");
    return label.layer&&label.module_path&&label.file_size&&label.hit_transform&&label.texture_ctor&&label.load_image&&label.texture_set&&label.scale&&label.offset&&label.has_property&&tt_install(game);
}
