#ifndef ParticleScene_HPP_
#define ParticleScene_HPP_

#include "IScene.hpp"

class ParticleScene : public IScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;
};

#endif // ParticleScene_HPP_
