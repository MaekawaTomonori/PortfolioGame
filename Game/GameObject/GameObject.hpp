#ifndef GameObject_HPP_
#define GameObject_HPP_
#include <memory>

#include "Model.hpp"
#include "Math/Vector3.hpp"

class GameObject {
protected:
    std::unique_ptr<Model> model_{ nullptr };
    Vector3 position_{0.f, 1.f, 0.f};
    Vector3 rotation_{};
    Vector3 scale_{1.f, 1.f, 1.f};
    Vector3 offset_ {};
    Vector3 velocity_{};
    bool active_{true};

public:
    virtual ~GameObject() = default;
    // 基本更新・描画
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void Debug() {}

    // Transform関連
    const Vector3& GetPosition() const { return position_; }
    const Vector3& GetRotation() const { return rotation_; }
    const Vector3& GetScale() const { return scale_; }

    void SetPosition(const Vector3& position);
    void SetRotation(const Vector3& rotation);
    void SetScale(const Vector3& scale);

    // Velocity関連
    const Vector3& GetVelocity() const { return velocity_; }
    void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
    void AddVelocity(const Vector3& velocity) { velocity_ += velocity; }

    bool IsActive() const { return active_; }
    void SetActive(bool active) { active_ = active; }

protected:
    void UpdateModel();

    /// <summary>
    /// velocityを位置に適用（継承先のUpdate内で呼び出す）
    /// </summary>
    void ApplyVelocity();

    void SetModel(const std::string& _name);
}; // class GameObject

#endif // GameObject_HPP_
