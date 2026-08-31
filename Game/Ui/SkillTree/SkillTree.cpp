#include "SkillTree.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <utility>

#include "imgui.h"
#include "json/JsonParams.hpp"
#include "Log.hpp"
#include "Math/Easing.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Time/Time.hpp"

#undef min
#undef max

namespace {
    // レベルアップ: 白フラッシュ + 上バウンス (0.4s)
    const Ui::AnimFunc kLevelUpAnim = [](float t, Vector2& pos, Vector4& col) -> bool {
        constexpr float kDur  = 0.4f;
        constexpr float kPeak = 0.08f;
        float k = (t <= kPeak) ? (t / kPeak) : std::max(0.f, 1.f - (t - kPeak) / (kDur - kPeak));
        col.x = std::min(1.f, col.x + k * 0.7f);
        col.y = std::min(1.f, col.y + k * 0.7f);
        col.z = std::min(1.f, col.z + k * 0.7f);
        pos.y -= k * 10.f;
        return t >= kDur;
    };

    // MAXレベル: ゴールドフラッシュ + 大バウンス (0.6s)
    const Ui::AnimFunc kMaxedAnim = [](float t, Vector2& pos, Vector4& col) -> bool {
        constexpr float kDur  = 0.6f;
        constexpr float kPeak = 0.12f;
        float k = (t <= kPeak) ? (t / kPeak)
                               : std::max(0.f, 1.f - (t - kPeak) / (kDur - kPeak));
        col.x = std::min(1.f, col.x + k);
        col.y = std::min(1.f, col.y + k * 0.75f);
        col.z = col.z * (1.f - k * 0.7f);
        pos.y -= k * 18.f;
        return t >= kDur;
    };

    // アニメなし (開幕リセット用)
    const Ui::AnimFunc kNoop = [](float, Vector2&, Vector4&) -> bool { return true; };

    // LEVEL UP バッジ: フェードイン → 保持 → フェードアウト + 上昇 (1.17s)
    const Ui::AnimFunc kPopupAnim = [](float t, Vector2& pos, Vector4& col) -> bool {
        constexpr float kIn   = 0.12f;
        constexpr float kHold = 0.55f;
        constexpr float kOut  = 0.5f;
        constexpr float kDur  = kIn + kHold + kOut;
        float alpha;
        if (t < kIn)               alpha = t / kIn;
        else if (t < kIn + kHold)  alpha = 1.f;
        else                       alpha = std::max(0.f, 1.f - (t - kIn - kHold) / kOut);
        col.w = alpha;
        pos.y -= (t / kDur) * 18.f;
        return t >= kDur;
    };

    // パネル: 明転フラッシュ (0.3s)
    const Ui::AnimFunc kPanelFlashAnim = [](float t, Vector2& /*pos*/, Vector4& col) -> bool {
        constexpr float kDur  = 0.3f;
        constexpr float kPeak = 0.05f;
        float k = (t <= kPeak) ? (t / kPeak)
                               : std::max(0.f, 1.f - (t - kPeak) / (kDur - kPeak));
        col.x = std::min(1.f, col.x + k * 0.5f);
        col.y = std::min(1.f, col.y + k * 0.5f);
        col.z = std::min(1.f, col.z + k * 0.5f);
        col.w = std::min(1.f, col.w + k * 0.2f);
        return t >= kDur;
    };

    // 接続線: 今すぐ取得可能/取得済みへの線は白、まだ届かない線はグレーのまま
    constexpr Vector4 kLineColorReachable = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr Vector4 kLineColorLocked    = {0.6f, 0.6f, 0.6f, 1.0f};

    // 接続線スプライト名 -> その線が指す先(子)スキルID
    const std::vector<std::pair<std::string, std::string>> kLineTargets = {
        {"hline_root", "root"}, {"stem_main", "root"},
        {"stem_to_r1", "atk_up"}, {"stem_to_r2", "hp_up"}, {"stem_to_r3", "spd_up"},
        {"stem_to_r4", "as_up"}, {"stem_to_r5", "time_up"},
        {"r1_t1_to_t2", "atk_up_2"}, {"r2_t1_to_t2", "hp_up_2"}, {"r3_t1_to_t2", "spd_up_2"},
        {"r4_t1_to_t2", "as_up_2"}, {"r5_t1_to_t2", "enemy_up"},
        {"r1_t2_to_t3", "atk_up_3"}, {"r2_t2_to_t3", "hp_up_3"}, {"r3_t2_to_t3", "spd_up_3"},
        {"r4_t2_to_t3", "as_up_3"}, {"r5_t2_to_t3", "spawn_down"},
        {"r1_t3_to_t4", "atk_max"}, {"r2_t3_to_t4", "hp_max"}, {"r5_t3_to_t4", "time_up_2"},
    };
}

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

    ui_->SetCustomDebug([this]{ Debug(); });

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

    cursor_->Update();
}

void SkillTree::Draw() {
    if (!ui_ || !ui_->IsActive()) return;

    cursor_->Draw();
}

void SkillTree::Open() {
    if (!ui_) return;

    // 前回の取得演出が開幕に再生されないようリセット
    if (auto* lv = ui_->FindElementByName("pv_level"))
        lv->SetShowFunc(kNoop);
    if (auto* badge = ui_->FindElementByName("pv_levelup_badge"))
        badge->SetShowFunc(kNoop);

    ui_->SetActive(true);
    ui_->SetFocused(true);
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
        ImGui::Text("Acquired Levels");
        for (auto& skill : skills_) {
            uint32_t lv = GetAcquiredLevel(skill.id);
            ImGui::Text("  %s: %u / %u", skill.id.c_str(), lv, skill.maxLevel);
        }
        ImGui::Separator();
    }

    ImGui::DragFloat("Interpolation", &interpolationTime_, 0.01f, 0.0f, 10.0f);
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

    int canvasCursor = ui_->GetCursorIndex();
    if (canvasCursor >= 0) {
        for (int32_t i = 0; i < static_cast<int32_t>(selectableIndices_.size()); ++i) {
            if (static_cast<int>(selectableIndices_[i]) == canvasCursor && i != selectedIndex_) {
                selectedIndex_ = i;
                timer_ = 0.f;
                start_ = cursor_->GetPosition();
                auto rect = ui_->GetElementRect(selectableIndices_[selectedIndex_]);
                destination_ = {rect.position.x, rect.position.y - rect.size.y / 2.f - 16.f};
                UpdatePreview();
                break;
            }
        }
    }

    if (input_->IsTrigger(DIK_RETURN) || input_->IsTrigger(DIK_SPACE)) {
        Execute();
    }
}

void SkillTree::UpdateCursor() {
    if (interpolationTime_ < timer_) return;
    timer_ += Time::GetDeltaTime();

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

    // lime = max level, aqua = acquired, orange = currently acquirable,
    // muted amber = prerequisite can be taken now, gray = locked
    constexpr Vector4 colorMaxed      = {0.2f, 1.0f, 0.2f, 1.f};
    constexpr Vector4 colorAcquired   = {0.4f, 0.85f, 0.85f, 1.f};
    constexpr Vector4 colorAvailable  = {1.0f, 0.65f, 0.15f, 1.f};
    constexpr Vector4 colorPrereqHint = {0.7f, 0.62f, 0.45f, 1.f};
    constexpr Vector4 colorDisabled   = {0.4f, 0.4f, 0.4f, 1.f};

    std::unordered_map<std::string, bool> reachableSkills;

    for (auto& skill : skills_) {
        auto* elem = ui_->FindElementByName(skill.id);
        if (!elem) continue;

        // ノードのアイコンをステータスアイコンに設定
        elem->SetTexture(skill.iconTexture);
        elem->SetTextureRegion({}, {});

        uint32_t level = GetAcquiredLevel(skill.id);
        bool maxed = (level >= skill.maxLevel);
        bool acquired = (level > 0);

        bool requirementsMet = IsRequirementSatisfied(skill);
        bool affordable = (status_->point >= skill.cost);
        bool prereqHint = (!requirementsMet && affordable && HasAcquirableMissingRequirement(skill));

        // 線の色分け用: 「既に取得済み/今すぐ取得可能」かどうか
        reachableSkills[skill.id] = maxed || acquired || (requirementsMet && affordable);

        if (maxed) {
            ui_->SetElementEvent(skill.id, Ui::EventKey::Execute, "");
            elem->SetColor(colorMaxed);
        } else if (requirementsMet && affordable) {
            ui_->SetElementEvent(skill.id, Ui::EventKey::Execute, "SkillTree.Upgrade." + skill.id);
            elem->SetColor(acquired ? colorAcquired : colorAvailable);
        } else {
            ui_->SetElementEvent(skill.id, Ui::EventKey::Execute, "");
            if (acquired) {
                elem->SetColor(colorAcquired);
            } else if (prereqHint) {
                elem->SetColor(colorPrereqHint);
            } else {
                elem->SetColor(colorDisabled);
            }
        }
    }

    UpdateConnectionLines(reachableSkills);

    CollectNavigableIndices();

    if (!selectableIndices_.empty()) {
        if (selectedIndex_ >= static_cast<int32_t>(selectableIndices_.size())) {
            selectedIndex_ = 0;
        }
    }

    UpdatePreview();
    UpdatePointDisplay();
}

void SkillTree::UpdateConnectionLines(const std::unordered_map<std::string, bool>& _reachableSkills) const {
    for (const auto& [lineName, skillId] : kLineTargets) {
        auto* lineElem = ui_->FindElementByName(lineName);
        if (!lineElem) continue;

        auto it = _reachableSkills.find(skillId);
        bool reachable = (it != _reachableSkills.end()) && it->second;
        lineElem->SetColor(reachable ? kLineColorReachable : kLineColorLocked);
    }
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
        skill.id          = skillData.value("Id", "");
        skill.name        = skillData.value("Name", "");
        skill.description = skillData.value("Description", "");
        skill.iconTexture = skillData.value("Icon", "white_x16.png");
        skill.displayValue = skillData.value("DisplayValue", 0);
        skill.cost        = skillData.value("Cost", 0u);
        skill.maxLevel    = skillData.value("MaxLevel", 1u);

        if (skillData.contains("Required") && skillData["Required"].is_array()) {
            for (const auto& required : skillData["Required"]) {
                if (!required.is_string()) continue;
                skill.required.push_back(required.get<std::string>());
            }
        }

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

    for (const auto& skill : skills_) {
        for (const auto& required : skill.required) {
            if (FindSkillByName(required)) continue;
            Log::Send(Log::Level::WARNING, "SkillTree: Unknown required skill id - " + required + " (skill: " + skill.id + ")");
        }
    }

    Log::Send(Log::Level::INFO, "SkillTree: Loaded " + std::to_string(skills_.size()) + " skills");
}

void SkillTree::ApplySkill(const std::string& _id) {
    if (!status_) return;

    for (auto& skill : skills_) {
        if (skill.id != _id) continue;

        uint32_t level = GetAcquiredLevel(skill.id);
        if (level >= skill.maxLevel) return;
        if (!IsRequirementSatisfied(skill)) return;
        if (status_->point < skill.cost) return;

        status_->point -= skill.cost;
        status_->playerStatus.hp     += skill.effect.hp;
        status_->playerStatus.damage += skill.effect.damage;
        status_->playerStatus.ms     += skill.effect.ms;
        status_->playerStatus.as     += skill.effect.as;
        status_->time                += skill.time;
        status_->maxEnemyCount       += skill.maxEnemyCount;
        status_->enemySpawnInterval  += skill.enemySpawnInterval;

        acquiredLevels_[skill.id]++;
        bool isMaxed = (acquiredLevels_[skill.id] >= skill.maxLevel);
        PlayAcquireEffect(skill.id, isMaxed);
        PlayPanelEffect();
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
        elem->SetTexture("numbers.png");
        elem->SetTextureRegion({static_cast<float>(std::abs(_digit) % 10) * DIGIT_TEX_SIZE.x, 0.f}, DIGIT_TEX_SIZE);
        elem->SetVisible(_visible);
    };

    auto setIcon = [this](const std::string& _name, const std::string& _tex, bool _visible = true) {
        auto* elem = ui_->FindElementByName(_name);
        if (!elem) return;
        elem->SetTexture(_tex);
        elem->SetTextureRegion({}, {});
        elem->SetVisible(_visible);
    };

    auto setText = [this](const std::string& _name, const std::string& _text, bool _visible = true) {
        auto* elem = ui_->FindElementByName(_name);
        if (!elem) return;
        elem->SetText(_text);
        elem->SetVisible(_visible);
    };

    if (skill) {
        uint32_t level  = GetAcquiredLevel(skill->id);
        bool     maxed  = (level >= skill->maxLevel);

        int32_t absVal = std::abs(skill->displayValue);

        // スキル名テキスト
        setText("pv_name", skill->name);

        // 説明テキスト
        setText("pv_description", skill->description);

        // レベル表示 (MAX は金色、通常はシアン)
        {
            auto* lvElem = ui_->FindElementByName("pv_level");
            if (lvElem) {
                lvElem->SetVisible(true);
                if (maxed) {
                    lvElem->SetText("MAX");
                    lvElem->SetColor({1.f, 0.82f, 0.1f, 1.f});
                } else {
                    lvElem->SetText("Lv " + std::to_string(level) + " / " + std::to_string(skill->maxLevel));
                    lvElem->SetColor({0.5f, 0.9f, 0.9f, 1.f});
                }
            }
        }

        setIcon("pv_stat_icon", skill->iconTexture);
        setIcon("pv_sign", skill->displayValue >= 0 ? "sign_plus.png" : "sign_minus.png");
        setDigit("pv_val_d0", absVal / 10);
        setDigit("pv_val_d1", absVal % 10);

        if (maxed) {
            setText("pv_cost_label", "COST", false);
            setIcon("pv_pt_icon", "white_x16.png", false);
            setDigit("pv_pt_d0", 0, false);
            setDigit("pv_pt_d1", 0, false);
        } else {
            int32_t cost = static_cast<int32_t>(skill->cost);
            setText("pv_cost_label", "COST", true);
            setIcon("pv_pt_icon", "pt_icon.png", true);
            setDigit("pv_pt_d0", cost / 10, true);
            setDigit("pv_pt_d1", cost % 10, true);
        }

        // バッジをリセット (選択変更時に前回の残像を消す)
        auto* badge = ui_->FindElementByName("pv_levelup_badge");
        if (badge) {
            Vector4 c = badge->GetColor();
            c.w = 0.f;
            badge->SetColor(c);
        }
    } else {
        setText("pv_name", "", false);
        setText("pv_description", "", false);
        setText("pv_level", "", false);
        setText("pv_cost_label", "COST", false);
        setIcon("pv_stat_icon", "white_x16.png", false);
        setIcon("pv_sign", "white_x16.png", false);
        setDigit("pv_val_d0", 0, false);
        setDigit("pv_val_d1", 0, false);
        setIcon("pv_pt_icon", "white_x16.png", false);
        setDigit("pv_pt_d0", 0, false);
        setDigit("pv_pt_d1", 0, false);
        auto* badge = ui_->FindElementByName("pv_levelup_badge");
        if (badge) {
            Vector4 c = badge->GetColor();
            c.w = 0.f;
            badge->SetColor(c);
        }
    }
}

void SkillTree::UpdatePointDisplay() {
    if (!status_) return;

    constexpr Vector2 DIGIT_TEX_SIZE = {64.f, 96.f};

    auto setDigit = [this, &DIGIT_TEX_SIZE](const std::string& _name, int32_t _digit) {
        auto* elem = ui_->FindElementByName(_name);
        if (!elem) return;
        elem->SetTexture("numbers.png");
        elem->SetTextureRegion({static_cast<float>(std::abs(_digit) % 10) * DIGIT_TEX_SIZE.x, 0.f}, DIGIT_TEX_SIZE);
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

uint32_t SkillTree::GetAcquiredLevel(const std::string& _id) const {
    auto it = acquiredLevels_.find(_id);
    return it != acquiredLevels_.end() ? it->second : 0u;
}

bool SkillTree::IsRequirementSatisfied(const SkillDef& _skill) const {
    for (const auto& required : _skill.required) {
        const SkillDef* requiredSkill = FindSkillByName(required);
        if (!requiredSkill) return false;
        if (GetAcquiredLevel(required) < requiredSkill->maxLevel) return false;
    }

    return true;
}

bool SkillTree::IsSkillAcquirable(const SkillDef& _skill) const {
    if (!status_) return false;

    uint32_t level = GetAcquiredLevel(_skill.id);
    if (level >= _skill.maxLevel) return false;
    if (status_->point < _skill.cost) return false;

    return IsRequirementSatisfied(_skill);
}

bool SkillTree::HasAcquirableMissingRequirement(const SkillDef& _skill) const {
    bool hasMissingRequirement = false;

    for (const auto& required : _skill.required) {
        const SkillDef* requiredSkill = FindSkillByName(required);
        if (!requiredSkill) return false;

        if (GetAcquiredLevel(required) >= requiredSkill->maxLevel) continue;

        hasMissingRequirement = true;
        if (!IsSkillAcquirable(*requiredSkill)) return false;
    }

    return hasMissingRequirement;
}

void SkillTree::PlayAcquireEffect(const std::string& _id, bool _isMaxed) {
    auto* elem = ui_->FindElementByName(_id);
    if (!elem) return;
    elem->SetShowFunc(_isMaxed ? kMaxedAnim : kLevelUpAnim);
    elem->PlayShow();

    auto* levelElem = ui_->FindElementByName("pv_level");
    if (levelElem) {
        levelElem->SetShowFunc(_isMaxed ? kMaxedAnim : kLevelUpAnim);
        levelElem->PlayShow();
    }

    auto* badge = ui_->FindElementByName("pv_levelup_badge");
    if (badge) {
        badge->SetText(_isMaxed ? "MAXED!" : "LEVEL UP!");
        badge->SetShowFunc(kPopupAnim);
        badge->PlayShow();
    }
}

void SkillTree::PlayPanelEffect() {
    auto* panel = ui_->FindElementByName("SelectedContent");
    if (!panel) return;
    panel->SetShowFunc(kPanelFlashAnim);
    panel->PlayShow();
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
