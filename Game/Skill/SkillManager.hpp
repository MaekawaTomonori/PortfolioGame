#ifndef SKILL_MANAGER_HPP_
#define SKILL_MANAGER_HPP_

#include <vector>
#include <memory>

#include "ISkillEntity.hpp"
#include "BlackHole/BlackHole.hpp"

class Enemies;
class ParticleSystem;

/** @brief スキルエンティティの管理クラス
 ** スキルの生成・更新・描画・破棄を一括管理する
 ** 各スキルの共通パラメータを所有し、常時ImGuiで調整可能
 **/
class SkillManager {
    std::vector<std::unique_ptr<ISkillEntity>> entities_;
    Enemies* enemies_ = nullptr;
    ParticleSystem* particle_ = nullptr;

    BlackHole::Params blackHoleParams_;

public:
    void Initialize();
    void Update();
    void Draw() const;
    void Debug();

    /** @brief ブラックホールスキルを生成
     ** @param _position 発生位置
     ** @param _direction 発射方向
     **/
    void SpawnBlackHole(const Vector3& _position, const Vector3& _direction);

    void SetEnemies(Enemies* _enemies) { enemies_ = _enemies; }
    void SetParticle(ParticleSystem* _particle) { particle_ = _particle; }

private:
    void LoadParams();
    void SaveParams();
};

#endif // SKILL_MANAGER_HPP_
