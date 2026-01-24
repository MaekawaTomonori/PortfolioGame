#define NOMINMAX
#include "Element.hpp"

#include <filesystem>

#include "imgui_internal.h"
#include "Utils.hpp"

namespace Ui {
    void Element::Initialize() {
        uuid_ = Utils::GenerateUniqueId();
        name_ = "NoName";
        visible_ = true;

        data_.texture = "white_x16.png";
        data_.position = {640.f, 360.f};
        data_.size = {64.f, 64.f};
        data_.color = {1.f, 0.5859f, 0.7421f, 1.f};

        sprite_ = std::make_unique<Sprite>();
        sprite_->Initialize(data_.texture);
        sprite_->SetPosition(data_.position);
        sprite_->SetSize(data_.size);
        sprite_->SetColor(data_.color);

        auto len = std::min(name_.size(), nameBuff_.size() - 1);
        std::copy_n(name_.begin(), len, nameBuff_.begin());
        nameBuff_[len] = '\0';
    }

    void Element::Update() {
        if (!visible_)return;

        if (textureBuff_[0] != '\0') {
            data_.texture = textureBuff_.data();
            textureBuff_.fill('\0');
            sprite_->SetTexture(data_.texture);
        }

        sprite_->SetPosition(data_.position);
        sprite_->SetSize(data_.size);
        sprite_->SetColor(data_.color);
        sprite_->Update();
    }

    void Element::Draw() const {
        if (!visible_) return;

        sprite_->Draw();
    }

    void Element::Debug() {
        if (isOpen_){
            ImGui::BeginChild("Body", ImVec2(0.f, 240.f), true, ImGuiWindowFlags_NoMove);

            ImGui::Text("Name : ");
            ImGui::SameLine();
            if (changeName_){
                ImGui::SetNextItemWidth(80.f);
                ImGui::InputText("##name", nameBuff_.data(), nameBuff_.size());
                ImGui::SameLine();
                if (nameBuff_[0] == '\0') { ImGui::BeginDisabled(); }
                if (ImGui::Button("Apply")) {
                    name_ = nameBuff_.data();
                    changeName_ = false;
                }
                if (nameBuff_[0] == '\0') { ImGui::EndDisabled(); }
            }else {
                ImGui::SetNextItemWidth(80.f);
                ImGui::TextColored(ImVec4(0.f, 1.f, 0.6f, 1.f), name_.c_str());
                ImGui::SameLine();
                if (ImGui::Button("Change")){
                    auto len = std::min(name_.size(), nameBuff_.size() - 1);
                    std::copy_n(name_.begin(), len, nameBuff_.begin());
                    nameBuff_[len] = '\0';
                    changeName_ = true;
                }
            }

            ImGui::Separator();

            ImGui::Text("Visible : ");
            ImGui::SameLine();
            ImGui::Checkbox("##visible", &visible_);

            ImGui::Separator();

            ImGui::Text("Params");
            
            // テクスチャ選択
            ImGui::Text("Texture: ");
            ImGui::SameLine();
            // 現在のテクスチャ名を表示
            if (!data_.texture.empty()) {
                ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), data_.texture.c_str());
            }
            ImGui::SameLine();
            if (ImGui::Button("...##BrowseTexture")) {
                std::string selectedFile = Utils::OpenFileDialog("PNG");
                if (!selectedFile.empty()) {
                    // Assets/Resources/からの相対パスに変換
                    std::filesystem::path fullPath(selectedFile);
                    std::string filename = fullPath.filename().string();
                    strcpy_s(textureBuff_.data(), textureBuff_.size(), filename.c_str());
                }
            }

            ImGui::Text("Position");
            ImGui::SetNextItemWidth(120.f);
            ImGui::DragFloat2("##pos", &data_.position.x);

            ImGui::Text("Size : AspectLock");
            ImGui::SameLine();
            ImGui::Checkbox("##asplock", &aspectLock_);
            ImGui::SetNextItemWidth(120.f);
            if (aspectLock_) {
                if (ImGui::DragFloat("##size", &data_.size.x)){
                    data_.size.y = data_.size.x;
                }
            } else {
                ImGui::DragFloat2("##size", &data_.size.x);
            }

            ImGui::Text("Color");
            ImGui::ColorEdit4("##color", &data_.color.x);

            ImGui::EndChild();
        }
    }

    bool Element::IsVisible() const {
        return visible_;
    }

    std::string Element::GetName() const {
        return name_;
    }

    std::string Element::GetUUID() const {
        return uuid_;
    }

    bool& Element::IsOpen() {
        return isOpen_;
    }

    Element::Data Element::GetData() const {
        return data_;
    }

    void Element::SetData(const Data& _data) {
        data_ = _data;
    }
}
