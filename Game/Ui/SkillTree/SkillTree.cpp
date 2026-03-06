#include "SkillTree.hpp"

#include <cmath>
#include <fstream>

#include "imgui.h"
#include "json/JsonParams.hpp"
#include "Log.hpp"
#include "Math/Easing.hpp"
#include "Pattern/Singleton.hpp"

void SkillTree::Initialize(GameStatus* _status) {
    status_ = _status;
    input_ = Singleton<Input>::GetInstance();

    LoadSkills("Assets/Data/SkillTree/data.json");

    ui_ = std::make_unique<Ui::Canvas>();
    ui_->Setup("skilltree");
    ui_->SetActive(false);

    for (auto& skill : skills_) {
        std::string actionKey = "SkillTree.Upgrade." + skill.id;
        ui_->RegisterAction(actionKey, [this, id = skill.id]() { ApplySkill(id); });
    }

    ui_->RegisterAction("SkillTree.Continue", [this]() {
        if (onContinue_) onContinue_();
    });

    cursor_ = std::make_unique<Sprite>();
    cursor_->Initialize("white_x16.png");
    cursor_->SetSize({16.f, 16.f});

    Refresh();
    PositionCursorAtSelected();
}

void SkillTree::Update() {
    if (!ui_ || !ui_->IsActive()) return;

    HandleInput();
    UpdateCursor();

    ui_->Update();
    cursor_->Update();
}

void SkillTree::Draw() {
    if (!ui_ || !ui_->IsActive()) return;

    ui_->Draw();
    cursor_->Draw();
}

void SkillTree::Open() {
    if (!ui_) return;
    ui_->SetActive(true);
    Refresh();
    selectedIndex_ = 0;
    PositionCursorAtSelected();
}

void SkillTree::Close() const {
    if (!ui_) return;
    ui_->SetActive(false);
}

bool SkillTree::IsOpen() const {
    return ui_ && ui_->IsActive();
}

void SkillTree::Debug() {
    if (!ui_) return;

    ImGui::Begin("SkillTree");

    if (status_) {
        ImGui::Text("Points: %u", status_->point);
        if (ImGui::Button("AddPoint")){
            ++status_->point;
            Refresh();
        }
        ImGui::Separator();
        ImGui::Text("PlayerStatus");
        ImGui::Text("  HP: %.1f", status_->playerStatus.hp);
        ImGui::Text("  DMG: %.1f", status_->playerStatus.damage);
        ImGui::Text("  MS: %.1f", status_->playerStatus.ms);
        ImGui::Text("  AS: %.1f", status_->playerStatus.as);
        ImGui::Separator();
        ImGui::Text("GameStatus");
        ImGui::Text("  Time: %.1f", status_->time);
        ImGui::Text("  MaxEnemy: %u", status_->maxEnemyCount);
        ImGui::Text("  SpawnInterval: %.1f", status_->enemySpawnInterval);
        ImGui::Separator();
    }

    ImGui::DragFloat("Interpolation", &interpolationTime_, 0.01f, 0.0f, 10.0f);
    ImGui::Separator();

    ui_->Debug();
    ImGui::End();
}

void SkillTree::SetOnContinue(std::function<void()> _callback) {
    onContinue_ = std::move(_callback);
}

void SkillTree::HandleInput() {
    if (selectableIndices_.empty()) return;

    bool up    = input_->IsTrigger(DIK_UP)    || input_->IsTrigger(DIK_W);
    bool down  = input_->IsTrigger(DIK_DOWN)  || input_->IsTrigger(DIK_S);
    bool left  = input_->IsTrigger(DIK_LEFT)  || input_->IsTrigger(DIK_A);
    bool right = input_->IsTrigger(DIK_RIGHT) || input_->IsTrigger(DIK_D);

    if (up || down || left || right) {
        auto currentRect = ui_->GetElementRect(selectableIndices_[selectedIndex_]);
        Vector2 cur = currentRect.position;

        float dx = right ? 1.f : (left ? -1.f : 0.f);
        float dy = down  ? 1.f : (up   ? -1.f : 0.f);

        int32_t bestIndex = -1;
        float bestScore = 1e9f;

        for (int32_t i = 0; i < static_cast<int32_t>(selectableIndices_.size()); ++i) {
            if (i == selectedIndex_) continue;

            auto rect = ui_->GetElementRect(selectableIndices_[i]);
            float diffX = rect.position.x - cur.x;
            float diffY = rect.position.y - cur.y;

            float dot = diffX * dx + diffY * dy;
            if (dot <= 0.f) continue;

            float crossAbs = std::abs(diffX * dy - diffY * dx);
            float dist = std::sqrt(diffX * diffX + diffY * diffY);

            float score = dist + crossAbs * 2.f;
            if (score < bestScore) {
                bestScore = score;
                bestIndex = i;
            }
        }

        if (bestIndex >= 0) {
            selectedIndex_ = bestIndex;

            timer_ = 0.f;
            start_ = cursor_->GetPosition();

            auto rect = ui_->GetElementRect(selectableIndices_[selectedIndex_]);
            destination_ = {
                rect.position.x,
                rect.position.y - rect.size.y / 2.f - 16.f
            };

            UpdatePreview();
        }
    }

    if (input_->IsTrigger(DIK_RETURN) || input_->IsTrigger(DIK_SPACE)) {
        Execute();
    }
}

void SkillTree::UpdateCursor() {
    if (interpolationTime_ < timer_) return;
    timer_ += 1.f / 60.f;

    Vector2 pos = Ease::Out::Cubic(start_, destination_, timer_ / interpolationTime_);
    cursor_->SetPosition(pos);
}

void SkillTree::Execute() const {
    if (selectableIndices_.empty()) return;

    size_t elemIndex = selectableIndices_[selectedIndex_];
    ui_->ExecuteActionAt(elemIndex, Ui::EventKey::Execute);
}

void SkillTree::Refresh() {
    if (!status_) return;

    constexpr Vector4 colorAvailable = {1.f, 1.f, 1.f, 1.f};
    constexpr Vector4 colorDisabled  = {0.4f, 0.4f, 0.4f, 1.f};
    constexpr Vector4 colorAcquired  = {0.8f, 0.8f, 0.4f, 1.f};

    for (auto& skill : skills_) {
        auto* elem = ui_->FindElementByName(skill.id);
        if (!elem) continue;

        auto data = elem->GetData();

        if (acquired_.contains(skill.id)) {
            elem->SetEvent(Ui::EventKey::Execute, "");
            data.color = colorAcquired;
        } else if (status_->point >= skill.cost) {
            elem->SetEvent(Ui::EventKey::Execute, "SkillTree.Upgrade." + skill.id);
            data.color = colorAvailable;
        } else {
            elem->SetEvent(Ui::EventKey::Execute, "");
            data.color = colorDisabled;
        }

        elem->SetData(data);
    }

    CollectNavigableIndices();

    if (!selectableIndices_.empty()) {
        if (selectedIndex_ >= static_cast<int32_t>(selectableIndices_.size())) {
            selectedIndex_ = 0;
        }
    }

    UpdatePreview();
    UpdatePointDisplay();
}

void SkillTree::LoadSkills(const std::string& _path) {
    std::ifstream file(_path);
    if (!file.is_open()) {
        Log::Send(Log::Level::WARNING, "SkillTree: skill data not found - " + _path);
        return;
    }

    nlohmann::json root;
    try {
        file >> root;
        file.close();
    } catch (const nlohmann::json::parse_error& e) {
        Log::Send(Log::Level::ERR, "SkillTree: JSON parse error - " + std::string(e.what()));
        file.close();
        return;
    }

    if (!root.contains("Skills") || !root["Skills"].is_array()) return;

    for (const auto& skillData : root["Skills"]) {
        SkillDef skill;
        skill.id = skillData.value("Id", "");
        skill.name = skillData.value("Name", "");
        skill.iconTexture = skillData.value("Icon", "white_x16.png");
        skill.displayValue = skillData.value("DisplayValue", 0);
        skill.cost = skillData.value("Cost", 0u);

        if (skillData.contains("Effect") && skillData["Effect"].is_object()) {
            const auto& effect = skillData["Effect"];
            skill.effect.hp     = effect.value("hp", 0.f);
            skill.effect.damage = effect.value("damage", 0.f);
            skill.effect.ms     = effect.value("ms", 0.f);
            skill.effect.as     = effect.value("as", 0.f);
            skill.time               = effect.value("time", 0.f);
            skill.maxEnemyCount      = effect.value("maxEnemyCount", static_cast<int16_t>(0));
            skill.enemySpawnInterval = effect.value("enemySpawnInterval", 0.f);
        }

        skills_.push_back(std::move(skill));
    }

    Log::Send(Log::Level::INFO, "SkillTree: Loaded " + std::to_string(skills_.size()) + " skills");
}

void SkillTree::ApplySkill(const std::string& _id) {
    if (!status_) return;

    for (auto& skill : skills_) {
        if (skill.id != _id) continue;
        if (status_->point < skill.cost) return;

        status_->point -= skill.cost;
        status_->playerStatus.hp     += skill.effect.hp;
        status_->playerStatus.damage += skill.effect.damage;
        status_->playerStatus.ms     += skill.effect.ms;
        status_->playerStatus.as     += skill.effect.as;
        status_->time                += skill.time;
        status_->maxEnemyCount       += skill.maxEnemyCount;
        status_->enemySpawnInterval  += skill.enemySpawnInterval;

        acquired_.insert(skill.id);
        Refresh();
        return;
    }
}

void SkillTree::PositionCursorAtSelected() {
    if (selectableIndices_.empty()) return;

    size_t elemIndex = selectableIndices_[selectedIndex_];
    auto rect = ui_->GetElementRect(elemIndex);
    float cursorX = rect.position.x;
    float cursorY = rect.position.y - rect.size.y / 2.f - 16.f;

    cursor_->SetPosition({cursorX, cursorY});
    start_ = {cursorX, cursorY};
    destination_ = start_;
    timer_ = interpolationTime_ + 1.f;

    UpdatePreview();
}

void SkillTree::UpdatePreview() {
    if (selectableIndices_.empty()) return;

    size_t elemIndex = selectableIndices_[selectedIndex_];
    const SkillDef* skill = FindSkillByElementIndex(elemIndex);

    constexpr Vector2 DIGIT_TEX_SIZE = {64.f, 96.f};

    auto setDigit = [this, &DIGIT_TEX_SIZE](const std::string& _name, int32_t _digit, bool _visible = true) {
        auto* elem = ui_->FindElementByName(_name);
        if (!elem) return;
        auto d = elem->GetData();
        d.texture = "numbers.png";
        d.textureLeftTop = {static_cast<float>(std::abs(_digit) % 10) * DIGIT_TEX_SIZE.x, 0.f};
        d.textureSize = DIGIT_TEX_SIZE;
        d.visible = _visible;
        elem->SetData(d);
    };

    auto setIcon = [this](const std::string& _name, const std::string& _tex, bool _visible = true) {
        auto* elem = ui_->FindElementByName(_name);
        if (!elem) return;
        auto d = elem->GetData();
        d.texture = _tex;
        d.textureLeftTop = {};
        d.textureSize = {};
        d.visible = _visible;
        elem->SetData(d);
    };

    if (skill) {
        bool isAcquired = acquired_.contains(skill->id);

        int32_t absVal = std::abs(skill->displayValue);

        setIcon("pv_stat_icon", skill->iconTexture);
        setIcon("pv_sign", skill->displayValue >= 0 ? "sign_plus.png" : "sign_minus.png");
        setDigit("pv_val_d0", absVal / 10);
        setDigit("pv_val_d1", absVal % 10);

        if (isAcquired) {
            setIcon("pv_pt_icon", "white_x16.png", false);
            setDigit("pv_pt_d0", 0, false);
            setDigit("pv_pt_d1", 0, false);
            setIcon("pv_acquired", "acquired.png", true);
        } else {
            int32_t cost = static_cast<int32_t>(skill->cost);
            setIcon("pv_pt_icon", "pt_icon.png", true);
            setDigit("pv_pt_d0", cost / 10, true);
            setDigit("pv_pt_d1", cost % 10, true);
            setIcon("pv_acquired", "white_x16.png", false);
        }
    } else {
        setIcon("pv_stat_icon", "white_x16.png", false);
        setIcon("pv_sign", "white_x16.png", false);
        setDigit("pv_val_d0", 0, false);
        setDigit("pv_val_d1", 0, false);
        setIcon("pv_pt_icon", "white_x16.png", false);
        setDigit("pv_pt_d0", 0, false);
        setDigit("pv_pt_d1", 0, false);
        setIcon("pv_acquired", "white_x16.png", false);
    }
}

void SkillTree::UpdatePointDisplay() {
    if (!status_) return;

    constexpr Vector2 DIGIT_TEX_SIZE = {64.f, 96.f};

    auto setDigit = [this, &DIGIT_TEX_SIZE](const std::string& _name, int32_t _digit) {
        auto* elem = ui_->FindElementByName(_name);
        if (!elem) return;
        auto d = elem->GetData();
        d.texture = "numbers.png";
        d.textureLeftTop = {static_cast<float>(std::abs(_digit) % 10) * DIGIT_TEX_SIZE.x, 0.f};
        d.textureSize = DIGIT_TEX_SIZE;
        elem->SetData(d);
    };

    int32_t pt = static_cast<int32_t>(status_->point);
    setDigit("point_d0", pt / 10);
    setDigit("point_d1", pt % 10);
}

void SkillTree::CollectNavigableIndices() {
    selectableIndices_.clear();

    for (size_t i = 0; i < ui_->GetElementCount(); ++i) {
        auto* elem = ui_->GetElement(i);
        if (!elem) continue;

        const auto& name = elem->GetName();

        if (name == "Continue") {
            selectableIndices_.push_back(i);
            continue;
        }

        if (FindSkillByName(name)) {
            selectableIndices_.push_back(i);
        }
    }
}

const SkillTree::SkillDef* SkillTree::FindSkillByElementIndex(size_t _elemIndex) const {
    auto* elem = ui_->GetElement(_elemIndex);
    if (!elem) return nullptr;
    return FindSkillByName(elem->GetName());
}

const SkillTree::SkillDef* SkillTree::FindSkillByName(const std::string& _name) const {
    for (auto& skill : skills_) {
        if (skill.id == _name) return &skill;
    }
    return nullptr;
}
