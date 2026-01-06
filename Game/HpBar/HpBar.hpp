#ifndef HpBar_HPP_
#define HpBar_HPP_
#include <memory>
#include "Model.hpp"

class HpBar {
    Vector3* ownerPosition_ = nullptr;
    
    const Matrix4x4 BackToFront = MathUtils::Matrix::MakeRotateY(MathUtils::F_PI);

    std::unique_ptr<Model> front_;
    std::unique_ptr<Model> back_;

    Vector3 offset_{ 0.f, 2.f, 1.f };

    std::string texture_;
    Vector4 frontColor_{1.0f, 1.0f, 1.0f, 1.0f};
    Vector4 backColor_{0.3f, 0.3f, 0.3f, 1.0f};

    Vector3 size_{ 1.f, 0.15f, 1.f };

public:
    void Initialize(const std::string& _texture, Vector3* _ownerPos);
    void Update(float _hpRatio) const;
    void Draw() const;

    void SetFrontColor(const Vector4& _color) { frontColor_ = _color; }
    void SetBackColor(const Vector4& _color) { backColor_ = _color; }

private:

}; // class HpBar

#endif // HpBar_HPP_
