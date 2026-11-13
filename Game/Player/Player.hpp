#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include <memory>
#include <functional>
#include <vector>

#include "ParticleSystem/ParticleSystem.hpp"

#include "GameObject/GameObject.hpp"
#include "Movement/Movement.hpp"
#include "Movement/IMovementBehavior.hpp"
#include "Movement/MovementContext.hpp"
#include "Movement/WalkBehavior.hpp"
#include "Movement/DashBehavior.hpp"
#include "Movement/FlashBehavior.hpp"
#include "Module/Attack/Attack.hpp"

#include "InputHandler.hpp"
#include "FollowCamera.hpp"
#include "Status/PlayerStatus.hpp"
#include "Sprite.hpp"

class Player : public GameObject {
    ParticleSystem* particle_ = nullptr;
    FollowCamera* camera_ = nullptr;
    PostProcessExecutor* postEffect_ = nullptr;
    // プレイヤーの基本色
    static constexpr Vector4 BASE_COLOR = {0.3f, 0.3f, 1.f, 1.f};
    static constexpr Vector4 DAMAGE_FLASH_COLOR = {2.f, 2.f, 2.f, 1.f};

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

    WalkBehavior*  walk_  = nullptr;
    DashBehavior*  dash_  = nullptr;
    FlashBehavior* flash_ = nullptr;

    float baseWalkSpeed_ = 5.f;

    bool invulnerability_ = false;
    float invulnerabilityTimer_ = 1.f;
    static constexpr float INVULNERABILITY_DURATION = 1.f;

# ifdef _DEBUG  
    bool no_atk = false;
    bool no_move = false;
#endif

    std::unique_ptr<Sprite> reticle_;
    bool enable_ = false;

public:
    Player(ParticleSystem* _particle, PostProcessExecutor* _postEffect);
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Debug() override;

    void SetTargetPosition(const Vector3& _position);

    void OnCollision(const Collision::Collider* _collider);

    void SetCamera(FollowCamera* _camera);

    void SetStatus(const PlayerStatus& _status);

    void SetEnable(bool _enable) { enable_ = _enable; }

    /** @brief スキル発動時のコールバックを設定
     ** @param _callback (発射位置, 発射方向) を受け取るコールバック
     **/
    void SetOnSkillRequest(std::function<void(const Vector3&, const Vector3&)> _callback) { onSkillRequest_ = std::move(_callback); }

    /** @brief フラッシュのCT比率を返す (0=ready, 1=full) **/
    float GetFlashCooldownRatio() const {
        if (!flash_) return 0.f;
        const float max = flash_->GetCooldown();
        return max > 0.f ? flash_->GetCurrentCooldown() / max : 0.f;
    }

    bool IsDashing() const { return dash_ && dash_->IsDashing(); }

    void SetFieldBounds(float _halfExtent) { fieldHalfExtent_ = _halfExtent; }

    void UpdateWithoutInput();

    bool IsDead() const;

    /** @brief ライト追従用のアンカー位置(プレイヤー位置に補間された座標)を返す **/
    const Vector3& GetLightAnchor() const { return forLight_; }

private:
    std::function<void(const Vector3&, const Vector3&)> onSkillRequest_;

    float fieldHalfExtent_ = 0.f;
    bool isNearFieldEdge_ = false;
    static constexpr float FIELD_EDGE_WARNING_DISTANCE = 8.f;

    void LoadParams();
    void SaveParams() const;
    void UpdateInvulnerability(float _deltaTime);
    void UpdateAttack();
    void ClampToFieldBounds();
};
#endif // PLAYER_HPP_
