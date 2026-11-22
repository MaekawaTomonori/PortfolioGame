#ifndef ITEM_SHOWCASE_HPP_
#define ITEM_SHOWCASE_HPP_
#include <memory>

#include "Model.hpp"
#include "Math/MathUtils.hpp"

/** @brief 指定したモデルを、回転させながら上下にゆらして展示する */
class ItemShowcase {
    struct Transform {
        Vector3 scale;
        Vector3 rotate;
        Vector3 translate;
    };

    std::unique_ptr<Model> model_;
    Transform transform_{ {1,1,1}, {0,0,0}, {0,0,0} };
    Vector3 originalP = { 0,0,0 };

    float angle_ = MathUtils::F_PI / 180.f;

    float t_ = 0.f;

    float rotateSpd_ = 1.f;
    float waveAmp_ = 1.f;

public:
    ItemShowcase(std::unique_ptr<Model> _model) : model_(std::move(_model)) {}
    void Update();
    void Draw();

    ItemShowcase& SetPosition(const Vector3& _position);
    ItemShowcase& SetRotation(const Vector3& _rotation);
    ItemShowcase& SetScale(const Vector3& _scale);
    ItemShowcase& SetRotateSpeed(float _speed);
    ItemShowcase& SetWaveAmplitude(float _amplitude);

private:
    void Debug();
}; // class ItemShowcase

#endif // ITEM_SHOWCASE_HPP_
