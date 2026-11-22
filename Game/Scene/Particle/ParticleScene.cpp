#include "ParticleScene.hpp"

#include "Effect/ParticleEffects.hpp"

void ParticleScene::Initialize() {
    // パーティクル編集画面で、本番と同じ動きを確かめる
    auto particle = Particle();
    if (!particle) return;

    ParticleEffects::RegisterHitExplosion(*particle);
    ParticleEffects::RegisterEnemyHitExplosion(*particle);
    ParticleEffects::RegisterFieldFog(*particle);
    ParticleEffects::RegisterBlackHoleSuction(*particle);
}

void ParticleScene::Update() {
}

void ParticleScene::Draw() {
}
