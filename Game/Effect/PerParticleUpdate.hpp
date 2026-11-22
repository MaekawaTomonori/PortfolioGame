#ifndef PER_PARTICLE_UPDATE_HPP_
#define PER_PARTICLE_UPDATE_HPP_
#include <span>
#include <utility>

#include "src/ParticleSystem/ParticleSystem.hpp"

/** @brief 粒子1つ単位で書いた更新関数を、ParticleSystemのバッチ更新関数へ変換する
 * @param _func (進行度[0-1], 発生位置, 位置, 速度, 色) を受け取り、位置・速度・色を書き換える関数
 * @return ParticleSystem::RegisterUpdateFuncへ渡せる更新関数
 */
template<typename Func>
ParticleSystem::UpdateFunc PerParticleUpdate(Func _func) {
    return [func = std::move(_func)](std::span<::Particle> _particles) {
        for (::Particle& particle : _particles) {
            Vector3 position = particle.GetPosition();
            Vector3 velocity = particle.GetVelocity();
            Vector4 color    = particle.GetColor();
            func(particle.GetProgress(), particle.GetOrigin(), position, velocity, color);
            particle.SetPosition(position).SetVelocity(velocity).SetColor(color);
        }
    };
}

#endif // PER_PARTICLE_UPDATE_HPP_
