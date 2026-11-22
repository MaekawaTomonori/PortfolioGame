#ifndef SKILL_TREE_HPP_
#define SKILL_TREE_HPP_
#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "GEUI.hpp"
#include "Input.hpp"
#include "Status/GameStatus.hpp"
#include "Ui/MenuInput.hpp"

class UiTheme;

class SkillTree {
    struct SkillDef {
        std::string id;
        std::string name;
        std::string description;
        std::vector<std::string> required;
        std::string iconTexture;
        uint32_t cost = 0;
        uint32_t maxLevel = 1;
        PlayerStatus effect{};
        float time = 0.f;
        int16_t maxEnemyCount = 0;
        float enemySpawnInterval = 0.f;
    };

    // t: 経過秒, pos: 動かす量(px), col: 色, scale: 倍率。終わったら true
    using AnimFunc = bool (*)(float _t, Vector2& _pos, Vector4& _col, float& _scale);

    struct Node {
        GEUI::Widget button;
        GEUI::Widget icon;
        GEUI::Widget level;
        GEUI::Widget line;
        size_t skill = 0;
    };

    // 全体を薄くして出すため、ツールチップは別の Canvas にする
    struct Tooltip {
        GEUI::Canvas canvas;
        GEUI::Widget root;
        GEUI::Widget title;
        GEUI::Widget description;
        std::array<GEUI::Widget, 2> effects;
        GEUI::Widget cost;
        GEUI::Widget need;
        const Node* target = nullptr;
        Vector2 offset{};   // 出し終わった位置（画面に対する割合）
        float slide = 0.f;  // 出始めのずれ(px)。ノードの側からずれて出てくる
        float time = 0.f;
    };

    struct Effect {
        GEUI::Widget colored;
        GEUI::Widget moved;
        AnimFunc func = nullptr;
        Vector4 baseColor{};
        Vector2 baseOffset{};
        float baseScale = 1.f;
        float time = 0.f;
    };

    GEUI::Canvas ui_;
    GEUI::Widget continue_;
    GEUI::Widget focused_;
    GEUI::Widget hovered_;
    GEUI::Widget points_;
    GEUI::Widget badge_;
    Tooltip tooltip_;
    std::vector<Node> nodes_;
    std::vector<Effect> effects_;
    Input* input_ = nullptr;
    MenuInput menuInput_;
    const UiTheme* theme_ = nullptr;
    // 最後の操作がキーなら選択中のノード、マウスならカーソル下のノードの説明を出す
    bool keyboardMode_ = false;
    // 開いたときのカーソル位置。ここから動くまでは、カーソル下のノードとクリックを無視する
    Vector2 openMousePosition_{};
    bool mouseMoved_ = false;

    std::vector<SkillDef> skills_;
    std::unordered_map<std::string, uint32_t> acquiredLevels_;
    GameStatus* status_ = nullptr;

    std::function<void()> onContinue_;

public:
    ~SkillTree();

    void Initialize(GameStatus* _status, const UiTheme& _theme);
    void Update();

    void Open();
    void Close();
    bool IsOpen() const;

    void Debug();

    void SetOnContinue(std::function<void()> _callback);

private:
    void HandleInput();
    void Refresh();
    void LoadSkills(const std::string& _path);
    void ApplySkill(const std::string& _id);
    void UpdateTooltip();
    void UpdateTooltipFade(float _deltaTime);
    void UpdatePointDisplay();
    void PlayAcquireEffect(const Node& _node, bool _isMaxed);
    void PlayEffect(GEUI::Widget _colored, GEUI::Widget _moved, AnimFunc _func);
    void StopEffect(const GEUI::Widget& _colored);
    void StopAllEffects();
    void UpdateEffects(float _deltaTime);
    void RestoreEffect(Effect& _effect);
    const Node* GetTooltipTarget() const;
    uint32_t GetAcquiredLevel(const std::string& _id) const;
    bool IsRequirementSatisfied(const SkillDef& _skill) const;
    bool IsSkillAcquirable(const SkillDef& _skill) const;
    bool HasAcquirableMissingRequirement(const SkillDef& _skill) const;
    const Node* FindNodeByWidget(const GEUI::Widget& _widget) const;
    const SkillDef* FindSkillByName(const std::string& _name) const;

}; // class SkillTree

#endif // SKILL_TREE_HPP_
