#include "BlackHole.hpp"

#include "ColliderType.hpp"
#include "Enemy/Enemies.hpp"
#include "Math/MathUtils.hpp"
#include "ParticleSystem/ParticleSystem.hpp"
#include "Random/RandomEngine.hpp"

#undef max

void BlackHole::Initialize(const Vector3& _position, const Vector3& _direction) {
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
        ->SetOwner(this)
        ->SetType(Collision::Type::Sphere)
        ->SetSize(0.25f)
        ->SetTranslate({position_.x, position_.y, position_.z})
        ->AddAttribute(static_cast<uint32_t>(CollisionType::Skill))
        ->AddIgnore(static_cast<uint32_t>(CollisionType::Player))
        ->AddIgnore(static_cast<uint32_t>(CollisionType::P_Bullet))
        ->AddIgnore(static_cast<uint32_t>(CollisionType::Skill))
        ->Enable();

    // 吸い込みパーティクル登録
    if (particle_) {
        particleGroupName_ = "bh_suction";

        // 外周リングにランダムスポーン
        particle_->RegisterSpawnFunc("bh_spawn", [](const Vector3& center, Vector3& pos, Vector3& vel) {
            float angle = MathUtils::Random(0.f, MathUtils::F_PI * 2.f);
            float radius = MathUtils::Random(2.5f, 4.5f);
            pos = {
                center.x + std::cos(angle) * radius,
                center.y + MathUtils::Random(0.f, 0.8f),
                center.z + std::sin(angle) * radius,
            };
            vel = {};
        });

        // 中心に向かって加速しながらスパイラル
        particle_->RegisterUpdateFunc("bh_suction", [](float t, const Vector3& origin, Vector3& pos, Vector3& vel, Vector4& color) {
            Vector3 toCenter = origin - pos;
            toCenter.y = 0.f;
            float dist = toCenter.Length();

            if (dist > 0.08f) {
                Vector3 inward = toCenter.Normalize();
                Vector3 tangent = {-inward.z, 0.f, inward.x};
                // 距離が近いほど強く引き寄せる
                float pull = 2.f + 6.f / (dist + 0.4f);
                vel.x = inward.x * pull + tangent.x * 1.8f;
                vel.z = inward.z * pull + tangent.z * 1.8f;
            } else {
                vel.x = 0.f;
                vel.z = 0.f;
            }
            vel.y *= 0.88f;

            // 時間経過とともに暗く透明に
            color.x = 0.35f * (1.f - t * 0.9f);
            color.y = 0.f;
            color.z = 0.9f * (1.f - t * 0.6f);
            color.w = std::max(0.f, 0.85f * (1.f - t * 1.1f));
        });

        particle_->Register(particleGroupName_);
    }
}

void BlackHole::Update(float _deltaTime) {
    if (!active_) return;

    switch (state_) {
    case State::Flying:
        UpdateFlying(_deltaTime);
        break;
    case State::Sucking:
        UpdateSucking(_deltaTime);
        break;
    }

    collider_->SetTranslate({position_.x, position_.y, position_.z});
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

    if (_collider->GetAttribute() & static_cast<uint32_t>(CollisionType::Enemy)) {
        TransitionToSucking();
    }
}
