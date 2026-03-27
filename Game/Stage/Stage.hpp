#ifndef Stage_HPP_
#define Stage_HPP_
#include <memory>

#include "Model.hpp"
#include "Enemy/Enemies.hpp"
#include "Sky/Skybox.hpp"
#include "Player/Player.hpp"
#include "Skill/SkillManager.hpp"
#include "Status/GameStatus.hpp"

class Stage {
    ParticleSystem* particle_ = nullptr;
    PostProcessExecutor* postEffect_ = nullptr;

    GameStatus& status_;

    std::unique_ptr<Skybox> skybox_;
    std::unique_ptr<Model> terrain_;
    std::unique_ptr<Player> player_;
    std::unique_ptr<Enemies> enemies_;
    std::unique_ptr<SkillManager> skillManager_;

    float fogTimer_ = 0.f;
    bool active_ = true;
    bool pending_ = false;

public:
    Stage(GameStatus& _status) : status_(_status) {}
    void Setup(ParticleSystem* _particle, PostProcessExecutor* _postEffect);
    void Initialize();
    void Update();
    void Draw() const;
    void Debug();

    void SetActive(bool _active) { pending_ = _active; }
    bool IsActive() const { return active_; }

    //Getter
    [[nodiscard]] Player* GetPlayer() const;
    [[nodiscard]] Enemies* GetEnemies() const;

    bool IsClear() const;
    bool IsGameOver() const;

    void SetCamera(FollowCamera* _camera) const;
}; // class Stage

#endif // Stage_HPP_
