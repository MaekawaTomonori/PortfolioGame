#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "GameObject/GameObject.hpp"
#include "Movement/Movement.hpp"
#include "Movement/IMovementBehavior.hpp"
#include "Movement/MovementContext.hpp"
#include "Module/Attack/Attack.hpp"
#include <memory>
#include <vector>

#include "FollowCamera.hpp"
#include "Status/PlayerStatus.hpp"
#include "InputHandler.hpp"
#include "ParticleSystem/ParticleSystem.hpp"
#include <functional>

class Player : public GameObject {
    ParticleSystem* particle_ = nullptr;
    FollowCamera* camera_ = nullptr;
    PostProcessExecutor* postEffect_ = nullptr;
    // プレイヤーの基本色
    const Vector4 BaseColor = {0.3f, 0.3f, 1.f, 1.f};
    const Vector4 DamageFlashColor = {2.f, 2.f, 2.f, 1.f};

    std::unique_ptr<Movement> movement_;
    std::unique_ptr<Attack> attack_;

    std::unique_ptr<InputHandler> inputHandler_;

    std::vector<std::unique_ptr<IMovementBehavior>> behaviors_;

    std::unique_ptr<Collision::Collider> collider_;

    MovementContext movementContext_;

    bool targetExist_ = false;
    Vector3 targetPosition_ = {};

    Vector3 forLight_ = {};

    PlayerStatus status_{};

    bool invulnerability_ = false;
    float invulnerabilityTimer_ = 1.f;
    const float InvulnerabilityDuration = 1.f;

# ifdef _DEBUG  
    bool no_atk = false;
    bool no_move = false;

#endif
    std::unique_ptr<Model> reticle_;


public:
    Player(ParticleSystem* _particle, PostProcessExecutor* _postEffect);
    void Initialize() override;
    void Update(float _deltaTime) override;
    void Draw() override;
    void Debug() override;

    void SetTargetPosition(Vector3 _position);

    void OnCollision(const Collision::Collider* _collider);

    void SetCamera(FollowCamera* _camera);

    void SetStatus(const PlayerStatus& _status);

    /** @brief スキル発動時のコールバックを設定
     ** @param _callback (発射位置, 発射方向) を受け取るコールバック
     **/
    void SetOnSkillRequest(std::function<void(const Vector3&, const Vector3&)> _callback) { onSkillRequest_ = std::move(_callback); }

    void UpdateWithoutInput();

    bool IsDead() const;

private:
    std::function<void(const Vector3&, const Vector3&)> onSkillRequest_;

    void UpdateInvulnerability(float _deltaTime);
    void UpdateAttack();
};
#endif // PLAYER_HPP_
