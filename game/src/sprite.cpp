/********************************************************************
    Written by James Moran
    Please see the file LICENSE.txt in the repository root directory.
*********************************************************************/

#include "sprite.h"
#include "hart/render/texture.h"
#include "hart/core/engine.h"
#include "hart/base/util.h"
#include "hart/base/time.h"
#include "hart/core/resourcemanager.h"

static hart::Freelist<Sprite> spriteFreelist;
#if HART_DEBUG_INFO
static hstd::vector<Sprite*> loadedSprites;
#endif

HART_OBJECT_TYPE_DECL_CUSTOM(Sprite,
[]() -> void* { return spriteFreelist.allocate(); },
[](void* ptr) { spriteFreelist.release(ptr); },
hobjfact::typehelper_t<Sprite>::constructType,
hobjfact::typehelper_t<Sprite>::destructType, 
nullptr);

void registierSpriteObject() {
    hobjfact::objectFactoryRegister(Sprite::getObjectDefinition(), nullptr);
#if HART_DEBUG_INFO
    hart::engine::addDebugMenu("Sprite Viewer", []() {
        static int current_anim = 0;
        static int current_sprite = (int)hutil::tmax(0ull, loadedSprites.size());
        static int current_frame = 0;
        static int preview_size = 64;
		static float preview_speed = 1.f;
        static float anim_time = 0.f;

        float delta_ms = (htime::deltaMS()*preview_speed)/1000.f;

        if (ImGui::Begin("Sprite Viewer", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_MenuBar)) {
            if(loadedSprites.size()) {
                ImGui::ListBox("Loaded Sprites",&current_sprite,[](void* data,int idx,const char** out_text) {
                    *out_text = ((Sprite**)data)[idx]->friendlyName;
                    return true;
                },loadedSprites.data(),(int)loadedSprites.size());
            }
            ImGui::Separator();
            ImGui::Columns(2, "sprites");
            ImGui::Text("Sprite View"); 
            if(current_sprite<loadedSprites.size()) {
                hstd::vector<Sprite::AnimFrame> const& anim_frames = loadedSprites[current_sprite]->anims[current_anim];                
                if (anim_frames.size()) {
                    current_frame = current_frame%anim_frames.size();
					Sprite::Frame frame = loadedSprites[current_sprite]->frames[anim_frames[current_frame].frame];
					ImTextureID tex_id = *((ImTextureID*)&loadedSprites[current_sprite]->texturePages[frame.page]);
                    ImGui::Image(tex_id, ImVec2((float)preview_size, (float)preview_size), ImVec2(frame.uvs[0], frame.uvs[1]), ImVec2(frame.uvs[2], frame.uvs[3]));
                    anim_time += delta_ms;
                    if (anim_time >= anim_frames[current_frame].lenght) {
					   current_frame = (current_frame+1)%anim_frames.size();
                       anim_time -= anim_frames[current_frame].lenght;
                    }
                } else {
                    anim_time = 0.f;
                    Sprite::Frame frame = loadedSprites[current_sprite]->frames[0];
                    ImTextureID tex_id = *((ImTextureID*)&loadedSprites[current_sprite]->texturePages[frame.page]);
                    ImGui::Image(tex_id, ImVec2((float)preview_size, (float)preview_size), ImVec2(frame.uvs[0], frame.uvs[1]), ImVec2(frame.uvs[2], frame.uvs[3]));
                }
                ImGui::SliderInt("Preview Size", &preview_size, 16, 256);
                ImGui::SliderFloat("Preview Speed", &preview_speed, 0.01f, 4.f);
                ImGui::Text("Frames in animation: %d",anim_frames.size());
            }
            ImGui::NextColumn();
            ImGui::ListBox("Sprite Anims", &current_anim, EnumNamesSpriteAnimType(), SpriteAnimType_MAX+1);
            ImGui::NextColumn();
            ImGui::End();
        }
    });
#endif
}

Sprite::~Sprite() {
#if HART_DEBUG_INFO
    loadedSprites.erase(std::find(loadedSprites.begin(), loadedSprites.end(), this));
#endif    
}

bool Sprite::deserialiseObject(MarshallType const* in_data, hobjfact::SerialiseParams const& params) {
    auto const* in_pages = in_data->pages();
    texturePages.resize(in_pages->size());
    for (uint32_t i=0, n=in_pages->size(); i<n; ++i) {
        texturePages[i] = hrnd::createTexture((*in_pages)[i]->data()->data(), (*in_pages)[i]->data()->size());
    }

    auto const* in_frames = in_data->frames();
    frames.resize(in_frames->size());
    for (uint32_t i=0, n=in_frames->size(); i<n; ++i) {
        resource::SpriteFrame const* in_f = (*in_frames)[i];
        Frame f = {hVec4(in_f->u1(), in_f->v1(), in_f->u2(), in_f->v2()), in_f->page()};
        frames[i] = f;
    }

    auto const* in_anims = in_data->anims();
    for (uint32_t i=0, n=in_anims->size(); i<n; ++i) {
        auto const* anim = (*in_anims)[i];
        SpriteAnimType at = anim->animType();
        anims[at].resize(anim->frames()->size());
        for (uint32_t f=0, fn=anim->frames()->size(); f<fn; ++f) {
            anims[at][f].frame = (*anim->frames())[f]->frame();
            anims[at][f].lenght = (*anim->frames())[f]->lenght();
        }
    }

#if HART_DEBUG_INFO
    friendlyName = params.resdata->friendlyName;
    loadedSprites.push_back(this);
    std::stable_sort(loadedSprites.begin(), loadedSprites.end(), [](Sprite* const& rhs, Sprite* const& lhs) {
        return hcrt::strcmp(rhs->friendlyName, lhs->friendlyName);
    });
#endif
    return true;
}


