#include "UserInterface.hpp"

#include <filesystem>

#include "imgui_internal.h"
#include "Scene/Title/TitleScene.hpp"

namespace Ui {
    void UserInterface::Update() const {
        for (auto& element : elements_) {
            element->Update();
        }
    }

    void UserInterface::Draw() const {
        for (auto& element : elements_) {
            if (!element->IsVisible()) continue;
            element->Draw();
        }
    }

    void UserInterface::Debug() {
        ImGui::Begin("UI");

        if (ImGui::Button("Open UI Editor")) {
            editing_ = true;
        }

        ImGui::End();

        if (editing_) {
            Editor();
        }
    }

    void UserInterface::RegisterAction(const std::string& _actionId, const std::function<void()>& _action) {
        if (!actions_.actions.contains(_actionId)) {
            actions_.actions[_actionId] = _action;
        }
    }

    void UserInterface::Editor() {
        ImGui::Begin("Editor", &editing_);

        ImGui::Text("Editing UI Element");

        ImGui::Checkbox("Default Open", &defaultOpen_);

        if (ImGui::Button("Add Element", ImVec2(100.f, 0.f))) {
            auto element = std::make_unique<Element>();
            element->Initialize();
            element->IsOpen() = defaultOpen_;
            elements_.emplace_back(std::move(element));
        }
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(100.f, 0.f))) {
            //Save to Json
        }

        // TODO: Delete
        
        ImGui::BeginChild("Elements", ImVec2(0.f, 0.f), true, ImGuiWindowFlags_NoMove);
        
        // Buffer for duplicated elements
        std::vector<Element::Data> duplicated;

        for (auto itr = elements_.begin(); itr != elements_.end(); ++itr) {
            bool& isOpen = (*itr)->IsOpen();

            ImGui::PushID((*itr)->GetUUID().c_str());

            ImGui::BeginGroup();

            if (ImGui::SmallButton(isOpen ? "-" : "+")) {
                isOpen = !isOpen;
            }

            ImGui::SameLine();

            ImGui::Text((*itr)->GetName().c_str());

            ImGui::SameLine(ImGui::GetWindowWidth() - 120.f);

            ImGui::BeginDisabled(itr == elements_.begin());
            if (ImGui::SmallButton("^")) {
                // Move Up
                std::iter_swap(itr, std::prev(itr));
            }
            ImGui::EndDisabled();
            ImGui::SameLine();

            ImGui::BeginDisabled(itr == std::prev(elements_.end()));
            if (ImGui::SmallButton("v")) {
                // Move Down
                std::iter_swap(itr, std::next(itr));
            }
            ImGui::EndDisabled();

            ImGui::SameLine();
            if (ImGui::SmallButton("[+]")) {
                // Duplicate
                duplicated.push_back((*itr)->GetData());
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Duplicate");
            }

            ImGui::EndGroup();

            // Body
            (*itr)->Debug();

            ImGui::PopID();
        }

        ImGui::Separator();

        ImGui::EndChild();

        ImGui::End();

        // Add duplicated elements after iteration is complete
        for (auto& data : duplicated) {
            auto element = std::make_unique<Element>();
            element->Initialize();
            element->SetData(data);
            element->IsOpen() = defaultOpen_;
            elements_.push_back(std::move(element));
        }
    }
}
