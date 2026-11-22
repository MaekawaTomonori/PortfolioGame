#include "ParticleScene.hpp"

#include "Effect/ParticleEffects.hpp"

void ParticleScene::Initialize() {
    // エンジンのパーティクル編集画面で、本番と同じ動きを確かめられるようにする
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
