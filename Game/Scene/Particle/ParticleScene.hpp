#ifndef PARTICLE_SCENE_HPP_
#define PARTICLE_SCENE_HPP_

#include "IScene.hpp"

class ParticleScene : public IScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
};

#endif // PARTICLE_SCENE_HPP_
