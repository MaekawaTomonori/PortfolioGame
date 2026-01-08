#ifndef Stage_HPP_
#define Stage_HPP_
#include <memory>

#include "Model.hpp"
#include "Enemy/Enemies.hpp"
#include "Sky/Skybox.hpp"
#include "Player/Player.hpp"
#include "Status/GameStatus.hpp"

class Stage {
    const uint16_t RequirementKills = 3;

    ParticleSystem* particle_ = nullptr;
    PostProcessExecutor* postEffect_ = nullptr;

    std::unique_ptr<Skybox> skybox_;
    std::unique_ptr<Model> terrain_;
    std::unique_ptr<Player> player_;
    std::unique_ptr<Enemies> enemies_;

public:
    void Setup(ParticleSystem* _particle, PostProcessExecutor* _postEffect);
    void Initialize(const GameStatus& _status);
    void Update();
    void Draw() const;
    void Debug() const;

    //Getter
    [[nodiscard]] Player* GetPlayer() const;
    [[nodiscard]] Enemies* GetEnemies() const;

    void SetCamera(FollowCamera* _camera) const;

    bool IsClear() const;
}; // class Stage

#endif // Stage_HPP_
