#include "ParticleEffects.hpp"

#include <algorithm>
#include <cmath>

#include "Effect/PerParticleUpdate.hpp"
#include "Math/MathUtils.hpp"

namespace {
    // 霧
    constexpr float FOG_SPAWN_RADIUS_MIN     = 0.5f;
    constexpr float FOG_SPAWN_RADIUS_MAX     = 10.5f;
    constexpr float FOG_SPAWN_HEIGHT_MIN     = -0.2f;
    constexpr float FOG_SPAWN_HEIGHT_MAX     = 0.3f;
    constexpr float FOG_HORIZONTAL_SPEED_MAX = 0.2f;
    constexpr float FOG_RISE_SPEED_MIN       = 0.35f;
    constexpr float FOG_RISE_SPEED_MAX       = 0.65f;

    constexpr float FOG_VERTICAL_DRAG   = 0.97f;
    constexpr float FOG_HORIZONTAL_DRAG = 0.93f;
    constexpr float FOG_FADE_IN_DURATION = 0.15f;
    constexpr float FOG_MAX_ALPHA        = 0.5f;
    constexpr Vector3 FOG_COLOR = {0.32f, 0.28f, 0.42f};

    // 被弾の光
    constexpr float HIT_EXPLOSION_INITIAL_BURST_THRESHOLD = 0.01f;
    constexpr float HIT_EXPLOSION_BURST_SPEED = 8.0f;
    constexpr float ENEMY_HIT_EXPLOSION_BURST_SPEED = 6.0f;
    constexpr float HIT_EXPLOSION_DRAG        = 0.92f;
    constexpr float HIT_EXPLOSION_GRAVITY     = 0.05f;
    constexpr float HIT_EXPLOSION_LIFETIME    = 0.7f;
    constexpr float HIT_EXPLOSION_ALPHA_SCALE = 0.9f;

    // ブラックホールの吸い込み
    constexpr float SUCTION_SPAWN_RADIUS_MIN = 2.5f;
    constexpr float SUCTION_SPAWN_RADIUS_MAX = 4.5f;
    constexpr float SUCTION_SPAWN_HEIGHT_MAX = 0.8f;
    constexpr float SUCTION_STOP_DISTANCE    = 0.08f;
    constexpr float SUCTION_BASE_PULL        = 2.f;
    constexpr float SUCTION_NEAR_PULL        = 6.f;
    constexpr float SUCTION_PULL_SOFTENING   = 0.4f;
    constexpr float SUCTION_SWIRL_SPEED      = 1.8f;
    constexpr float SUCTION_VERTICAL_DRAG    = 0.88f;

    /** @brief 出た瞬間にばらばらの向きへ弾け、減速しながら落ちて消える動き */
    ParticleSystem::UpdateFunc MakeExplosionUpdate(const float _burstSpeed) {
        return PerParticleUpdate([_burstSpeed](float _t, const Vector3&, Vector3&, Vector3& _velocity, Vector4& _color) {
            if (_t < HIT_EXPLOSION_INITIAL_BURST_THRESHOLD) {
                _velocity = Vector3::Random() * _burstSpeed;
            }
            _velocity = _velocity * HIT_EXPLOSION_DRAG;
            _velocity.y -= HIT_EXPLOSION_GRAVITY;
            _color.w = HIT_EXPLOSION_ALPHA_SCALE * (HIT_EXPLOSION_LIFETIME - _t);
        });
    }
}

void ParticleEffects::RegisterFieldFog(ParticleSystem& _particle) {
    // 発生の中心のまわりに散らばって出る
    _particle.RegisterSpawnFunc("field_fog_spawn", [](const Vector3& _center, Vector3& _position, Vector3& _velocity) {
        const float angle = MathUtils::Random(0.f, MathUtils::F_PI * 2.f);
        const float radius = MathUtils::Random(FOG_SPAWN_RADIUS_MIN, FOG_SPAWN_RADIUS_MAX);
        _position = {
            _center.x + std::cos(angle) * radius,
            _center.y + MathUtils::Random(FOG_SPAWN_HEIGHT_MIN, FOG_SPAWN_HEIGHT_MAX),
            _center.z + std::sin(angle) * radius,
        };
        const float horizontalAngle = MathUtils::Random(0.f, MathUtils::F_PI * 2.f);
        const float horizontalSpeed = MathUtils::Random(0.f, FOG_HORIZONTAL_SPEED_MAX);
        _velocity = {
            std::cos(horizontalAngle) * horizontalSpeed,
            MathUtils::Random(FOG_RISE_SPEED_MIN, FOG_RISE_SPEED_MAX),
            std::sin(horizontalAngle) * horizontalSpeed,
        };
    });

    // ゆっくり昇りながら、少しだけ濃くなってから薄れていく
    _particle.RegisterUpdateFunc("field_fog_update", PerParticleUpdate([](float _t, const Vector3&, Vector3&, Vector3& _velocity, Vector4& _color) {
        _velocity.y *= FOG_VERTICAL_DRAG;
        _velocity.x *= FOG_HORIZONTAL_DRAG;
        _velocity.z *= FOG_HORIZONTAL_DRAG;
        const float alpha = (_t < FOG_FADE_IN_DURATION)
            ? (_t / FOG_FADE_IN_DURATION) * FOG_MAX_ALPHA
            : FOG_MAX_ALPHA * (1.f - (_t - FOG_FADE_IN_DURATION) / (1.f - FOG_FADE_IN_DURATION));
        _color = {FOG_COLOR.x, FOG_COLOR.y, FOG_COLOR.z, std::max(0.f, alpha)};
    }));
}

void ParticleEffects::RegisterHitExplosion(ParticleSystem& _particle) {
    _particle.RegisterUpdateFunc("hit_explosion", MakeExplosionUpdate(HIT_EXPLOSION_BURST_SPEED));
}

void ParticleEffects::RegisterEnemyHitExplosion(ParticleSystem& _particle) {
    _particle.RegisterUpdateFunc("enemy_hit_explosion", MakeExplosionUpdate(ENEMY_HIT_EXPLOSION_BURST_SPEED));
}

void ParticleEffects::RegisterBlackHoleSuction(ParticleSystem& _particle) {
    // 外側のリングに出る
    _particle.RegisterSpawnFunc("bh_spawn", [](const Vector3& _center, Vector3& _position, Vector3& _velocity) {
        const float angle = MathUtils::Random(0.f, MathUtils::F_PI * 2.f);
        const float radius = MathUtils::Random(SUCTION_SPAWN_RADIUS_MIN, SUCTION_SPAWN_RADIUS_MAX);
        _position = {
            _center.x + std::cos(angle) * radius,
            _center.y + MathUtils::Random(0.f, SUCTION_SPAWN_HEIGHT_MAX),
            _center.z + std::sin(angle) * radius,
        };
        _velocity = {};
    });

    // 中心に近いほど強く引かれ、横向きの速さを足して渦を巻く
    _particle.RegisterUpdateFunc("bh_suction", PerParticleUpdate([](float _t, const Vector3& _origin, Vector3& _position, Vector3& _velocity, Vector4& _color) {
        Vector3 toCenter = _origin - _position;
        toCenter.y = 0.f;
        const float distance = toCenter.Length();

        if (distance > SUCTION_STOP_DISTANCE) {
            const Vector3 inward = toCenter.Normalize();
            const Vector3 tangent = {-inward.z, 0.f, inward.x};
            const float pull = SUCTION_BASE_PULL + SUCTION_NEAR_PULL / (distance + SUCTION_PULL_SOFTENING);
            _velocity.x = inward.x * pull + tangent.x * SUCTION_SWIRL_SPEED;
            _velocity.z = inward.z * pull + tangent.z * SUCTION_SWIRL_SPEED;
        } else {
            _velocity.x = 0.f;
            _velocity.z = 0.f;
        }
        _velocity.y *= SUCTION_VERTICAL_DRAG;

        // 時間とともに暗く、透明にする
        _color.x = 0.35f * (1.f - _t * 0.9f);
        _color.y = 0.f;
        _color.z = 0.9f * (1.f - _t * 0.6f);
        _color.w = std::max(0.f, 0.85f * (1.f - _t * 1.1f));
    }));
}
