#ifndef PARTICLE_EFFECTS_HPP_
#define PARTICLE_EFFECTS_HPP_

class ParticleSystem;

/** @brief ゲームで使うパーティクルの動きを登録する */
namespace ParticleEffects {
    /** @brief フィールドに漂う霧（field_fog_spawn / field_fog_update） */
    void RegisterFieldFog(ParticleSystem& _particle);

    /** @brief プレイヤーが被弾したときに弾ける光（hit_explosion） */
    void RegisterHitExplosion(ParticleSystem& _particle);

    /** @brief 敵が被弾したときに弾ける光（enemy_hit_explosion） */
    void RegisterEnemyHitExplosion(ParticleSystem& _particle);

    /** @brief ブラックホールに吸い込まれる粒（bh_spawn / bh_suction） */
    void RegisterBlackHoleSuction(ParticleSystem& _particle);
}

#endif // PARTICLE_EFFECTS_HPP_
