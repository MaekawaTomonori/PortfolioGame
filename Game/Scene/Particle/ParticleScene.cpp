#define NOMINMAX
#include "ParticleScene.hpp"

#include <cmath>
#include <algorithm>

#include "Math/MathUtils.hpp"

void ParticleScene::Initialize() {
    auto ps = Particle();

    // --- hit (Player弾ヒット: 青爆発) ---
    ps->RegisterUpdateFunc("hit_explosion", [](float t, const Vector3&, Vector3& pos, Vector3& velocity, Vector4& color) {
        (void)pos;
        if (t < 0.01f) {
            velocity = Vector3::Random() * 8.0f;
        }
        velocity = velocity * 0.92f;
        velocity.y -= 0.05f;
        color.w = 0.9f * (.7f - t);
    });

    // --- enemy_hit (敵ヒット: 赤爆発) ---
    ps->RegisterUpdateFunc("enemy_hit_explosion", [](float t, const Vector3&, Vector3& pos, Vector3& velocity, Vector4& color) {
        (void)pos;
        if (t < 0.01f) {
            velocity = Vector3::Random() * 6.0f;
        }
        velocity = velocity * 0.92f;
        velocity.y -= 0.05f;
        color.w = 0.9f * (.7f - t);
    });

    // --- field (フィールド霧: 環境エフェクト) ---
    ps->RegisterSpawnFunc("field_fog_spawn", [](const Vector3& center, Vector3& pos, Vector3& vel) {
        float angle  = MathUtils::Random(0.f, MathUtils::F_PI * 2.f);
        float radius = MathUtils::Random(0.5f, 10.5f);
        pos = {
            center.x + std::cos(angle) * radius,
            center.y + MathUtils::Random(-0.2f, 0.3f),
            center.z + std::sin(angle) * radius,
        };
        float horizAngle = MathUtils::Random(0.f, MathUtils::F_PI * 2.f);
        float horizSpeed = MathUtils::Random(0.f, 0.2f);
        vel = {
            std::cos(horizAngle) * horizSpeed,
            MathUtils::Random(0.35f, 0.65f),
            std::sin(horizAngle) * horizSpeed,
        };
    });
    ps->RegisterUpdateFunc("field_fog_update", [](float t, const Vector3&, Vector3&, Vector3& vel, Vector4& color) {
        vel.y *= 0.97f;
        vel.x *= 0.93f;
        vel.z *= 0.93f;
        float alpha = (t < 0.15f)
            ? (t / 0.15f) * 0.5f
            : 0.5f * (1.f - (t - 0.15f) / 0.85f);
        color.w = std::max(0.f, alpha);
        color.x = 0.32f;
        color.y = 0.28f;
        color.z = 0.42f;
    });

    // --- bh_suction (ブラックホール吸い込み) ---
    ps->RegisterSpawnFunc("bh_spawn", [](const Vector3& center, Vector3& pos, Vector3& vel) {
        float angle  = MathUtils::Random(0.f, MathUtils::F_PI * 2.f);
        float radius = MathUtils::Random(2.5f, 4.5f);
        pos = {
            center.x + std::cos(angle) * radius,
            center.y + MathUtils::Random(0.f, 0.8f),
            center.z + std::sin(angle) * radius,
        };
        vel = {};
    });
    ps->RegisterUpdateFunc("bh_suction", [](float t, const Vector3& origin, Vector3& pos, Vector3& vel, Vector4& color) {
        Vector3 toCenter = origin - pos;
        toCenter.y = 0.f;
        float dist = toCenter.Length();
        if (dist > 0.08f) {
            Vector3 inward  = toCenter.Normalize();
            Vector3 tangent = { -inward.z, 0.f, inward.x };
            float pull = 2.f + 6.f / (dist + 0.4f);
            vel.x = inward.x * pull + tangent.x * 1.8f;
            vel.z = inward.z * pull + tangent.z * 1.8f;
        } else {
            vel.x = 0.f;
            vel.z = 0.f;
        }
        vel.y *= 0.88f;
        color.x = 0.35f * (1.f - t * 0.9f);
        color.y = 0.f;
        color.z = 0.9f  * (1.f - t * 0.6f);
        color.w = std::max(0.f, 0.85f * (1.f - t * 1.1f));
    });
}

void ParticleScene::Update() {
}

void ParticleScene::Draw() {
}
