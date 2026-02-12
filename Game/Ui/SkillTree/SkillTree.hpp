#ifndef SkillTree_HPP_
#define SkillTree_HPP_
#include <memory>
#include <functional>
#include <unordered_set>
#include <vector>

#include "Input.hpp"
#include "Ui/UserInterface.hpp"
#include "Status/GameStatus.hpp"

class SkillTree {
    struct SkillDef {
        std::string id;
        std::string name;
        std::string iconTexture;
        int32_t displayValue = 0;
        uint32_t cost = 0;
        PlayerStatus effect{};
        float time = 0.f;
        int16_t maxEnemyCount = 0;
        float enemySpawnInterval = 0.f;
    };

    std::unique_ptr<Ui::UserInterface> ui_;
    std::unique_ptr<Sprite> cursor_;
    Input* input_ = nullptr;

    std::vector<SkillDef> skills_;
    std::unordered_set<std::string> acquired_;
    GameStatus* status_ = nullptr;

    std::vector<size_t> selectableIndices_;
    int32_t selectedIndex_ = 0;

    std::function<void()> onContinue_;

    float interpolationTime_ = 0.2f;
    Vector2 start_ {};
    Vector2 destination_ {};
    float timer_ = 1.f;

public:
    void Initialize(GameStatus* _status);
    void Update();
    void Draw();

    void Open();
    void Close() const;
    bool IsOpen() const;

    void Debug();

    void SetOnContinue(std::function<void()> _callback);

private:
    void HandleInput();
    void UpdateCursor();
    void Execute() const;
    void Refresh();
    void LoadSkills(const std::string& _path);
    void ApplySkill(const std::string& _id);
    void PositionCursorAtSelected();
    void UpdatePreview();
    void UpdatePointDisplay();
    void CollectNavigableIndices();
    const SkillDef* FindSkillByElementIndex(size_t _elemIndex) const;
    const SkillDef* FindSkillByName(const std::string& _name) const;

}; // class SkillTree

#endif // SkillTree_HPP_
