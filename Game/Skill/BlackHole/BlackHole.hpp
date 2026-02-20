#ifndef BLACK_HOLE_HPP_
#define BLACK_HOLE_HPP_

#include "Skill/ISkillEntity.hpp"
#include "Math/Vector4.hpp"
#include "Model.hpp"

class Enemies;
class ParticleSystem;

/** @brief ブラックホールスキル
 ** マウス方向に飛び、減速停止後に周囲の敵を吸い込む
 ** 状態遷移: Flying → Sucking → 消滅
 **/
class BlackHole : public ISkillEntity {
public:
    /** @brief ブラックホールの共通パラメータ
     ** SkillManagerが所有し、全インスタンスが参照する
     **/
    struct Params {
        // Flying
        float initialSpeed = 15.f;
        float deceleration = 8.f;
        float stopThreshold = 0.5f;
        Vector3 flyingScale = {0.5f, 0.5f, 0.5f};
        Vector4 flyingColor = {0.2f, 0.f, 0.5f, 1.f};

        // Sucking
        float suckRadius = 8.f;
        float suckForce = 1.2f;
        float suckDuration = 3.f;
        Vector3 suckingScale = {0.8f, 0.8f, 0.8f};
        Vector4 suckingColor = {0.5f, 0.f, 1.f, 1.f};

        // Vortex (ground plane effect)
        float vortexRotateSpeed = 3.f;
        Vector3 vortexScale = {5.f, 5.f, 1.f};
        Vector4 vortexColor = {1.f, 1.f, 1.f, 1.f};
    };

private:
    enum class State { Flying, Sucking };
    State state_ = State::Flying;

    float speed_ = 0.f;
    float suckTimer_ = 0.f;
    float vortexAngle_ = 0.f;

    std::unique_ptr<Model> vortexPlane_;
    float particleTimer_ = 0.f;
    std::string particleGroupName_;

    const Params* params_ = nullptr;
    Enemies* enemies_ = nullptr;
    ParticleSystem* particle_ = nullptr;

public:
    void Initialize(const Vector3& _position, const Vector3& _direction) override;
    void Update(float _deltaTime) override;
    void Draw() override;

    void SetParams(const Params* _params) { params_ = _params; }
    void SetEnemies(Enemies* _enemies) { enemies_ = _enemies; }
    void SetParticle(ParticleSystem* _particle) { particle_ = _particle; }

private:
    void UpdateFlying(float _deltaTime);
    void UpdateSucking(float _deltaTime);
    void TransitionToSucking();
    void OnCollision(const Collision::Collider* _collider);
};

#endif // BLACK_HOLE_HPP_
