#ifndef GameObject_HPP_
#define GameObject_HPP_
#include "Math/Vector3.hpp"
#include "Math/Transform.hpp"

class GameObject {
protected:
    Vector3 position_{};
    Vector3 rotation_{};
    Vector3 scale_{1.f, 1.f, 1.f};
    Transform transform_{};
    bool active_{true};

public:
    GameObject();
    virtual ~GameObject();

    // 基本更新・描画
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;

    // Transform関連
    const Vector3& GetPosition() const { return position_; }
    const Vector3& GetRotation() const { return rotation_; }
    const Vector3& GetScale() const { return scale_; }
    const Transform& GetTransform() const { return transform_; }
    
    void SetPosition(const Vector3& position);
    void SetRotation(const Vector3& rotation);
    void SetScale(const Vector3& scale);
    
    bool IsActive() const { return active_; }
    void SetActive(bool active) { active_ = active; }

protected:
    void UpdateTransform();
}; // class GameObject

#endif // GameObject_HPP_
