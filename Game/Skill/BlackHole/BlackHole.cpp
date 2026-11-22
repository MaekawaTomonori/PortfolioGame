#include "BlackHole.hpp"

#include "CollisionAttribute.hpp"
#include "Effect/ParticleEffects.hpp"
#include "Enemy/Enemies.hpp"
#include "Math/MathUtils.hpp"
#include "ParticleSystem/ParticleSystem.hpp"
#include "Random/RandomEngine.hpp"
#include "Time/Time.hpp"

BlackHole::BlackHole(const Params& _params, const Vector3& _position, const Vector3& _direction)
    : params_(&_params) {
    position_ = _position;
    speed_ = params_->initialSpeed;
    velocity_ = _direction.Normalize() * speed_;
    state_ = State::Flying;
    suckTimer_ = params_->suckDuration;

    scale_ = params_->flyingScale;
    SetModel("sphere");
    model_->SetTexture("white_x16.png");
    model_->SetColor(params_->flyingColor);
    model_->SetName("Skill_BlackHole");
    UpdateModel();

    collider_ = std::make_unique<Collision::Collider>();
    collider_->SetEvent(Collision::EventType::Trigger, [this](const Collision::Collider* _col) { OnCollision(_col); })
        ->SetType(Collision::Type::Sphere)
        ->SetSize(0.25f)
        ->SetTranslate(position_)
        ->AddAttribute(static_cast<uint32_t>(CollisionAttribute::Skill))
        ->AddIgnore(static_cast<uint32_t>(CollisionAttribute::Player))
        ->AddIgnore(static_cast<uint32_t>(CollisionAttribute::PlayerBullet))
        ->AddIgnore(static_cast<uint32_t>(CollisionAttribute::Skill))
        ->Enable();
}

void BlackHole::SetParticle(ParticleSystem* _particle) {
    particle_ = _particle;
    if (!particle_) return;

    particleGroupName_ = "bh_suction";
    ParticleEffects::RegisterBlackHoleSuction(*particle_);
    particle_->Register(particleGroupName_);
}

void BlackHole::Update() {
    if (!active_) return;

    float deltaTime = Time().GetDeltaTime();

    switch (state_) {
    case State::Flying:
        UpdateFlying(deltaTime);
        break;
    case State::Sucking:
        UpdateSucking(deltaTime);
        break;
    }

    collider_->SetTranslate(position_);
    UpdateModel();
}

void BlackHole::Draw() {
    if (!active_ || !model_) return;
    model_->Draw();
    if (vortexPlane_) {
        vortexPlane_->Draw();
    }
}

void BlackHole::UpdateFlying(float _deltaTime) {
    speed_ -= params_->deceleration * _deltaTime;
    if (speed_ <= params_->stopThreshold) {
        TransitionToSucking();
        return;
    }

    Vector3 dir = velocity_.Normalize();
    velocity_ = dir * speed_;
    position_ += velocity_ * _deltaTime;
}

void BlackHole::UpdateSucking(float _deltaTime) {
    suckTimer_ -= _deltaTime;
    if (suckTimer_ <= 0.f) {
        active_ = false;
        collider_->Disable();
        vortexPlane_.reset();
        return;
    }

    // パーティクル発射
    if (particle_ && !particleGroupName_.empty()) {
        particleTimer_ += _deltaTime;
        if (particleTimer_ >= 0.15f) {
            particleTimer_ = 0.f;
            particle_->Emit(particleGroupName_, position_);
        }
    }

    // vortex plane の回転更新
    if (vortexPlane_) {
        vortexAngle_ += params_->vortexRotateSpeed * _deltaTime;
        vortexPlane_->SetRotate({-(MathUtils::F_PI / 2.f), vortexAngle_, 0.f});
        vortexPlane_->SetScale(params_->vortexScale);
        vortexPlane_->SetColor(params_->vortexColor);
        vortexPlane_->Update();
    }

    if (!enemies_) return;

    for (const auto& enemy : enemies_->GetAll()) {
        if (!enemy->IsActive() || enemy->IsDead()) continue;

        Vector3 toCenter = position_ - enemy->GetPosition();
        toCenter.y = 0.f;
        float distance = toCenter.Length();

        if (distance < params_->suckRadius && distance > 0.1f) {
            Vector3 pullDir = toCenter.Normalize();
            float strength = params_->suckForce * (1.f - distance / params_->suckRadius);
            enemy->AddExternalForce(pullDir * strength);
        }
    }
}

void BlackHole::TransitionToSucking() {
    state_ = State::Sucking;
    speed_ = 0.f;
    velocity_ = {};
    suckTimer_ = params_->suckDuration;
    vortexAngle_ = 0.f;
    particleTimer_ = 0.f;

    scale_ = params_->suckingScale;
    model_->SetColor(params_->suckingColor);

    vortexPlane_ = std::make_unique<Model>();
    vortexPlane_->Initialize("plane");
    vortexPlane_->SetTexture("blackhole.png");
    vortexPlane_->SetColor(params_->vortexColor);
    vortexPlane_->SetScale(params_->vortexScale);
    vortexPlane_->SetRotate({-(MathUtils::F_PI / 2.f), 0.f, 0.f});
    vortexPlane_->SetTranslate({position_.x, 0.1f, position_.z});
    vortexPlane_->Update();
}

void BlackHole::OnCollision(const Collision::Collider* _collider) {
    if (state_ != State::Flying) return;

    if (_collider->GetAttribute() & static_cast<uint32_t>(CollisionAttribute::Enemy)) {
        TransitionToSucking();
    }
}
