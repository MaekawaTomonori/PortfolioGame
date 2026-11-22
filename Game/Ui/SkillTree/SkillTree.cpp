#include "SkillTree.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <fstream>
#include <utility>

#include "imgui.h"
#include "json/JsonParams.hpp"
#include "Log.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Time/Time.hpp"
#include "Ui/HudStyle.hpp"
#include "Ui/UiTheme.hpp"

namespace {
    // レベルアップ: 白フラッシュ + 上バウンス (0.4s)
    bool LevelUpAnim(float t, Vector2& pos, Vector4& col, float& /*scale*/) {
        constexpr float DURATION  = 0.4f;
        constexpr float PEAK_TIME = 0.08f;
        float k = (t <= PEAK_TIME) ? (t / PEAK_TIME) : std::max(0.f, 1.f - (t - PEAK_TIME) / (DURATION - PEAK_TIME));
        col.x = std::min(1.f, col.x + k * 0.7f);
        col.y = std::min(1.f, col.y + k * 0.7f);
        col.z = std::min(1.f, col.z + k * 0.7f);
        pos.y -= k * 10.f;
        return t >= DURATION;
    }

    // MAXレベル: 白フラッシュ + 大バウンス + 拡大 (0.6s)
    bool MaxedAnim(float t, Vector2& pos, Vector4& col, float& scale) {
        constexpr float DURATION  = 0.6f;
        constexpr float PEAK_TIME = 0.12f;
        float k = (t <= PEAK_TIME) ? (t / PEAK_TIME)
                               : std::max(0.f, 1.f - (t - PEAK_TIME) / (DURATION - PEAK_TIME));
        col.x = std::min(1.f, col.x + k);
        col.y = std::min(1.f, col.y + k);
        col.z = std::min(1.f, col.z + k);
        pos.y -= k * 18.f;
        scale += k * 0.15f;
        return t >= DURATION;
    }

    // LEVEL UP の文字: 大きく出て縮む → 保持 → フェードアウト + 上昇 (1.17s)
    bool PopupAnim(float t, Vector2& pos, Vector4& col, float& scale) {
        constexpr float FADE_IN_DURATION   = 0.12f;
        constexpr float HOLD_DURATION = 0.55f;
        constexpr float FADE_OUT_DURATION  = 0.5f;
        constexpr float DURATION  = FADE_IN_DURATION + HOLD_DURATION + FADE_OUT_DURATION;
        float alpha;
        if (t < FADE_IN_DURATION)               alpha = t / FADE_IN_DURATION;
        else if (t < FADE_IN_DURATION + HOLD_DURATION)  alpha = 1.f;
        else                       alpha = std::max(0.f, 1.f - (t - FADE_IN_DURATION - HOLD_DURATION) / FADE_OUT_DURATION);
        col.w = alpha;
        pos.y -= (t / DURATION) * 18.f;
        scale += std::max(0.f, 1.f - t / FADE_IN_DURATION) * 0.5f;
        return t >= DURATION;
    }


    // Assets/UI/SkillTree.json のノード（選択中は 1.1 倍）と SkillTreeTooltip.json の大きさ
    constexpr float   NODE_HALF    = 36.f * 1.1f;
    constexpr Vector2 TOOLTIP_SIZE = {300.f, 172.f};
    constexpr float   TOOLTIP_GAP  = 12.f;
    constexpr float   SCREEN_MARGIN = 16.f;
    constexpr float   BADGE_GAP    = 8.f;

    // ツールチップの行: 左の余白、1 行目（名前）の位置と、2 行目からの間隔(px)
    constexpr float TOOLTIP_PADDING   = 14.f;
    constexpr float TOOLTIP_TITLE_TOP  = 10.f;
    constexpr float TOOLTIP_FIRST_LINE = 40.f;
    constexpr float TOOLTIP_LINE_STEP  = 24.f;

    // ツールチップの出方: 少しずれた位置から、薄い状態で出てくる
    constexpr float TOOLTIP_FADE_TIME = 0.2f;
    constexpr float TOOLTIP_SLIDE    = 8.f;

    // スキル 1 つで変わる能力値。効果が 0 のものは表示しない
    struct StatChange {
        const char* label;
        float current;
        float delta;
        const char* unit;
        bool integer;
    };

    std::string FormatStat(float _value, const StatChange& _stat) {
        if (_stat.integer) {
            return std::format("{}{}", static_cast<int>(std::round(_value)), _stat.unit);
        }
        return std::format("{:.1f}{}", _value, _stat.unit);
    }

    Vector2 ToScreenPixels(const Vector2& _offset) {
        return {_offset.x * HudStyle::REFERENCE_SIZE.x, _offset.y * HudStyle::REFERENCE_SIZE.y};
    }

    Vector2 ToScreenRatio(const Vector2& _pixels) {
        return {_pixels.x / HudStyle::REFERENCE_SIZE.x, _pixels.y / HudStyle::REFERENCE_SIZE.y};
    }
}

SkillTree::~SkillTree() {
    tooltip_.canvas.Unload();
    ui_.Unload();
}

void SkillTree::Initialize(GameStatus* _status, const UiTheme& _theme) {
    status_ = _status;
    theme_ = &_theme;
    input_ = Singleton<Input>::GetInstance();

    LoadSkills("Assets/Data/SkillTree/data.json");

    ui_ = GEUI::Canvas::Load("Assets/UI/SkillTree.json");
    ui_.SetVisible(false);

    GEUI::Widget tree = ui_.Find("Tree");
    for (size_t i = 0; i < skills_.size(); ++i) {
        GEUI::Widget button = tree.Find(skills_[i].id);
        if (!button.IsValid()) {
            Log::Send(Log::Level::WARNING, "SkillTree: node not found in canvas - " + skills_[i].id);
            continue;
        }
        // 線は行き先のスキルIDで引く。前提の無い起点には線が無い
        GEUI::Widget line = skills_[i].required.empty() ? GEUI::Widget() : tree.Find("line_" + skills_[i].id);
        nodes_.push_back({button, button.Find("Icon"), button.Find("Level"), line, i});
        nodes_.back().icon.SetImage(skills_[i].iconTexture);
    }
    continue_ = tree.Find("Continue");
    points_ = ui_.Find("Points/Label");
    badge_ = ui_.Find("Badge");

    tooltip_.canvas = GEUI::Canvas::Load("Assets/UI/SkillTreeTooltip.json");
    tooltip_.canvas.SetVisible(false);
    tooltip_.root = tooltip_.canvas.Find("Tooltip");
    tooltip_.title = tooltip_.root.Find("Title");
    tooltip_.description = tooltip_.root.Find("Description");
    tooltip_.effects = {tooltip_.root.Find("Effect0"), tooltip_.root.Find("Effect1")};
    tooltip_.cost = tooltip_.root.Find("Cost");
    tooltip_.need = tooltip_.root.Find("Need");

    Refresh();
}

void SkillTree::Update() {
    if (!IsOpen()) return;

    HandleInput();
    UpdateEffects(Time().GetDeltaTime());
    UpdateTooltipFade(Time().GetDeltaTime());
}

void SkillTree::Open() {
    // 前回の取得演出が開幕に残らないよう、元の見た目へ戻す
    StopAllEffects();

    ui_.SetVisible(true);
    if (!nodes_.empty()) {
        nodes_.front().button.SetSelected(true);
    }
    focused_ = ui_.GetFocused();
    hovered_ = {};
    // 開いた直後は説明を出さない。キーを押すか、マウスを動かしてノードに乗ったときに初めて出す
    keyboardMode_ = false;
    openMousePosition_ = input_->GetMousePosition();
    mouseMoved_ = false;
    Refresh();
}

void SkillTree::Close() {
    ui_.SetVisible(false);
    tooltip_.canvas.SetVisible(false);
    tooltip_.target = nullptr;
}

bool SkillTree::IsOpen() const {
    return ui_.IsVisible();
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
}

void SkillTree::SetOnContinue(std::function<void()> _callback) {
    onContinue_ = std::move(_callback);
}

void SkillTree::HandleInput() {
    bool tooltipChanged = false;

    const auto navigate = [this, &tooltipChanged](GEUI::Direction _direction) {
        ui_.Navigate(_direction);
        if (!keyboardMode_) {
            keyboardMode_ = true;
            tooltipChanged = true;
        }
    };
    if (const auto direction = menuInput_.GetDirection()) navigate(*direction);
    const bool submitted = menuInput_.IsSubmit();
    if (submitted) ui_.Submit();

    // 開いたときにたまたまノードに重なっていたカーソルには反応しない。動かしてから受け付ける
    if (!mouseMoved_) {
        const Vector2 mouse = input_->GetMousePosition();
        mouseMoved_ = (mouse.x != openMousePosition_.x || mouse.y != openMousePosition_.y);
    }

    // カーソルが別のノードへ乗ったら、マウスの操作として扱う
    GEUI::Widget hovered;
    for (const Node& node : nodes_) {
        if (mouseMoved_ && node.button.IsHovered()) {
            hovered = node.button;
            break;
        }
    }
    if (hovered != hovered_) {
        hovered_ = hovered;
        if (hovered_.IsValid()) {
            keyboardMode_ = false;
        }
        tooltipChanged = true;
    }

    // 選択はキーでもマウスでも変わるので、変わったときにまとめて出し直す
    if (GEUI::Widget focused = ui_.GetFocused(); focused != focused_) {
        focused_ = focused;
        tooltipChanged = true;
    }
    if (tooltipChanged) {
        UpdateTooltip();
    }

    // マウスを動かす前のクリックは、開いた直後の押し間違いとして無視する（キーの決定は受け付ける）
    const bool acceptPress = submitted || mouseMoved_;
    if (!acceptPress) return;

    for (const Node& node : nodes_) {
        if (node.button.WasPressed()) {
            ApplySkill(skills_[node.skill].id);
            break;
        }
    }
    if (continue_.WasPressed() && onContinue_) {
        onContinue_();
    }
}

void SkillTree::Refresh() {
    if (!status_) return;

    for (Node& node : nodes_) {
        const SkillDef& skill = skills_[node.skill];

        uint32_t level = GetAcquiredLevel(skill.id);
        bool maxed = (level >= skill.maxLevel);
        bool acquired = (level > 0);

        bool requirementsMet = IsRequirementSatisfied(skill);
        bool affordable = (status_->point >= skill.cost);
        bool prereqHint = (!requirementsMet && affordable && HasAcquirableMissingRequirement(skill));

        // 取れないノードも説明を見られるよう、選べるまま押せなくする
        const bool pressable = !maxed && requirementsMet && affordable;
        node.button.SetEnabled(pressable);

        UiColor color = UiColor::Locked;
        if (maxed)           color = UiColor::Highlight;
        else if (acquired)   color = UiColor::Accent;
        else if (pressable)  color = UiColor::Text;
        else if (prereqHint) color = UiColor::TextDim;
        node.icon.SetColor(theme_->Get(color));

        node.level.SetText(maxed ? "MAX" : std::format("{}/{}", level, skill.maxLevel));
        node.level.SetColor(theme_->Get(color));

        if (node.line.IsValid()) {
            const bool reachable = acquired || pressable;
            node.line.SetColor(theme_->Get(reachable ? UiColor::Text : UiColor::Locked));
        }
    }

    UpdateTooltip();
    UpdatePointDisplay();
}

void SkillTree::LoadSkills(const std::string& _path) {
    std::ifstream file(_path);
    if (!file.is_open()) {
        Log::Send(Log::Level::WARNING, "SkillTree: skill data not found - " + _path);
        return;
    }

    // 例外を使わないので、壊れた JSON は discarded として受け取る
    const nlohmann::json root = nlohmann::json::parse(file, nullptr, false);
    if (root.is_discarded()) {
        Log::Send(Log::Level::ERR, "SkillTree: JSON parse error - " + _path);
        return;
    }

    if (!root.contains("Skills") || !root["Skills"].is_array()) return;

    for (const auto& skillData : root["Skills"]) {
        SkillDef skill;
        skill.id          = skillData.value("Id", "");
        skill.name        = skillData.value("Name", "");
        skill.description = skillData.value("Description", "");
        skill.iconTexture = skillData.value("Icon", "white_x16.png");
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

    for (const Node& node : nodes_) {
        SkillDef& skill = skills_[node.skill];
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
        // 演出は戻す先の見た目を読むので、新しい状態の色にしてから始める
        Refresh();
        PlayAcquireEffect(node, isMaxed);
        return;
    }
}

void SkillTree::UpdateTooltip() {
    const Node* node = GetTooltipTarget();
    if (!node || !status_) {
        tooltip_.canvas.SetVisible(false);
        tooltip_.target = nullptr;
        return;
    }

    const SkillDef& skill = skills_[node->skill];
    const uint32_t level = GetAcquiredLevel(skill.id);
    const bool maxed = (level >= skill.maxLevel);

    // 1 行目は名前とレベル。2 行目からは出す行だけを上から詰める
    tooltip_.title.SetText(maxed ? std::format("{}   MAX", skill.name)
                                 : std::format("{}   Lv {}/{}", skill.name, level, skill.maxLevel));
    tooltip_.title.SetOffset({TOOLTIP_PADDING / TOOLTIP_SIZE.x, TOOLTIP_TITLE_TOP / TOOLTIP_SIZE.y});

    float lineTop = TOOLTIP_FIRST_LINE;
    const auto showLine = [&lineTop](GEUI::Widget& _line, std::string_view _text, const Vector4& _color) {
        _line.SetText(_text);
        _line.SetColor(_color);
        _line.SetOffset({TOOLTIP_PADDING / TOOLTIP_SIZE.x, lineTop / TOOLTIP_SIZE.y});
        _line.SetVisible(true);
        lineTop += TOOLTIP_LINE_STEP;
    };

    showLine(tooltip_.description, skill.description, theme_->Get(UiColor::TextDim));

    // 効果は今の値と取得後の値を並べる。MAX なら今の値だけ
    const StatChange stats[] = {
        {"HP",          status_->playerStatus.hp,     skill.effect.hp,     "",  false},
        {"Damage",      status_->playerStatus.damage, skill.effect.damage, "",  false},
        {"Speed",       status_->playerStatus.ms,     skill.effect.ms,     "",  false},
        {"Attack Speed", status_->playerStatus.as,    skill.effect.as,     "",  false},
        {"Time",        status_->time,                skill.time,          "s", false},
        {"Max Enemies", static_cast<float>(status_->maxEnemyCount), static_cast<float>(skill.maxEnemyCount), "", true},
        {"Spawn Time",  status_->enemySpawnInterval,  skill.enemySpawnInterval, "s", false},
    };
    size_t effectCount = 0;
    for (const StatChange& stat : stats) {
        if (stat.delta == 0.f) continue;
        if (effectCount >= tooltip_.effects.size()) {
            Log::Send(Log::Level::WARNING, "SkillTree: too many effects to show - " + skill.id);
            break;
        }
        std::string text = std::format("{}: {}", stat.label, FormatStat(stat.current, stat));
        if (!maxed) {
            text += " -> " + FormatStat(stat.current + stat.delta, stat);
        }
        showLine(tooltip_.effects[effectCount++], text, theme_->Get(UiColor::Text));
    }
    for (size_t i = effectCount; i < tooltip_.effects.size(); ++i) {
        tooltip_.effects[i].SetVisible(false);
    }

    // MAX ならもう払わないので、コストは出さない
    if (maxed) {
        tooltip_.cost.SetVisible(false);
    } else {
        const bool affordable = (status_->point >= skill.cost);
        showLine(tooltip_.cost, std::format("Cost: {}pt", skill.cost), theme_->Get(affordable ? UiColor::Text : UiColor::Danger));
    }

    // 前提は MAX まで取る必要があるので、まだ足りない前提だけを出す
    std::string missing;
    for (const auto& required : skill.required) {
        const SkillDef* requiredSkill = FindSkillByName(required);
        if (!requiredSkill || GetAcquiredLevel(required) >= requiredSkill->maxLevel) continue;
        if (!missing.empty()) missing += ", ";
        missing += requiredSkill->name;
    }
    if (missing.empty()) {
        tooltip_.need.SetVisible(false);
    } else {
        showLine(tooltip_.need, std::format("(Need: {})", missing), theme_->Get(UiColor::Danger));
    }

    // ノードの右に出し、画面からはみ出すなら左に出す。上下は画面の内側に収める
    const Vector2 center = ToScreenPixels(node->button.GetOffset());
    Vector2 topLeft = {center.x + NODE_HALF + TOOLTIP_GAP, center.y - TOOLTIP_SIZE.y * 0.5f};
    float slide = -TOOLTIP_SLIDE;
    if (topLeft.x + TOOLTIP_SIZE.x > HudStyle::REFERENCE_SIZE.x - SCREEN_MARGIN) {
        topLeft.x = center.x - NODE_HALF - TOOLTIP_GAP - TOOLTIP_SIZE.x;
        slide = TOOLTIP_SLIDE;
    }
    topLeft.y = std::clamp(topLeft.y, SCREEN_MARGIN, HudStyle::REFERENCE_SIZE.y - SCREEN_MARGIN - TOOLTIP_SIZE.y);
    tooltip_.offset = ToScreenRatio(topLeft);
    tooltip_.slide = slide;

    // 出す相手が変わったときだけ出し直す。取得で中身が変わっただけなら、そのまま見せる
    if (tooltip_.target != node) {
        tooltip_.target = node;
        tooltip_.time = 0.f;
    }
    tooltip_.canvas.SetVisible(true);
    UpdateTooltipFade(0.f);
}

void SkillTree::UpdateTooltipFade(float _deltaTime) {
    if (!tooltip_.target) return;

    tooltip_.time = std::min(tooltip_.time + _deltaTime, TOOLTIP_FADE_TIME);
    const float t = tooltip_.time / TOOLTIP_FADE_TIME;
    // 出始めが速く、止まる直前でゆっくりになるように
    const float eased = 1.f - (1.f - t) * (1.f - t);

    tooltip_.canvas.SetAlpha(eased);
    const float slide = tooltip_.slide * (1.f - eased) / HudStyle::REFERENCE_SIZE.x;
    tooltip_.root.SetOffset({tooltip_.offset.x + slide, tooltip_.offset.y});
}

void SkillTree::UpdatePointDisplay() {
    if (!status_) return;

    points_.SetText(std::format("PT {}", status_->point));
}

void SkillTree::PlayAcquireEffect(const Node& _node, bool _isMaxed) {
    PlayEffect(_node.icon, _node.button, _isMaxed ? MaxedAnim : LevelUpAnim);

    // 文字はノードの真上から始める。戻す先の位置を読むので、前の演出を止めてから置く
    StopEffect(badge_);
    const Vector2 center = ToScreenPixels(_node.button.GetOffset());
    badge_.SetOffset(ToScreenRatio({center.x, center.y - NODE_HALF - BADGE_GAP}));
    badge_.SetText(_isMaxed ? "MAXED!" : "LEVEL UP!");
    PlayEffect(badge_, badge_, PopupAnim);
}

void SkillTree::PlayEffect(GEUI::Widget _colored, GEUI::Widget _moved, AnimFunc _func) {
    // 同じ相手の演出を重ねると戻す先がずれるので、前の演出を戻してから始める
    StopEffect(_colored);

    Effect effect;
    effect.colored = _colored;
    effect.moved = _moved;
    effect.func = _func;
    effect.baseColor = _colored.GetColor();
    if (_moved.IsValid()) {
        effect.baseOffset = _moved.GetOffset();
        effect.baseScale = _moved.GetScale();
    }
    effects_.push_back(effect);
}

void SkillTree::StopEffect(const GEUI::Widget& _colored) {
    std::erase_if(effects_, [this, &_colored](Effect& _effect) {
        if (_effect.colored != _colored) return false;
        RestoreEffect(_effect);
        return true;
    });
}

void SkillTree::StopAllEffects() {
    for (Effect& effect : effects_) {
        RestoreEffect(effect);
    }
    effects_.clear();
}

void SkillTree::UpdateEffects(float _deltaTime) {
    std::erase_if(effects_, [this, _deltaTime](Effect& _effect) {
        _effect.time += _deltaTime;
        Vector2 pos{};
        Vector4 col = _effect.baseColor;
        float scale = 1.f;
        const bool finished = _effect.func(_effect.time, pos, col, scale);
        if (finished) {
            RestoreEffect(_effect);
            return true;
        }

        _effect.colored.SetColor(col);
        if (_effect.moved.IsValid()) {
            const Vector2 move = ToScreenRatio(pos);
            _effect.moved.SetOffset({_effect.baseOffset.x + move.x, _effect.baseOffset.y + move.y});
            _effect.moved.SetScale(_effect.baseScale * scale);
        }
        return false;
    });
}

void SkillTree::RestoreEffect(Effect& _effect) {
    _effect.colored.SetColor(_effect.baseColor);
    if (_effect.moved.IsValid()) {
        _effect.moved.SetOffset(_effect.baseOffset);
        _effect.moved.SetScale(_effect.baseScale);
    }
}

const SkillTree::Node* SkillTree::GetTooltipTarget() const {
    // マウス操作中にどのノードにも乗っていなければ出さない
    return FindNodeByWidget(keyboardMode_ ? focused_ : hovered_);
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

const SkillTree::Node* SkillTree::FindNodeByWidget(const GEUI::Widget& _widget) const {
    if (!_widget.IsValid()) return nullptr;
    for (const Node& node : nodes_) {
        if (node.button == _widget) return &node;
    }
    return nullptr;
}

const SkillTree::SkillDef* SkillTree::FindSkillByName(const std::string& _name) const {
    for (auto& skill : skills_) {
        if (skill.id == _name) return &skill;
    }
    return nullptr;
}
