#include "GameObject.hpp"

GameObject::GameObject() {
    UpdateTransform();
}

GameObject::~GameObject() {
}

void GameObject::SetPosition(const Vector3& position) {
    position_ = position;
    UpdateTransform();
}

void GameObject::SetRotation(const Vector3& rotation) {
    rotation_ = rotation;
    UpdateTransform();
}

void GameObject::SetScale(const Vector3& scale) {
    scale_ = scale;
    UpdateTransform();
}

void GameObject::UpdateTransform() {
    transform_.translate = position_;
    transform_.rotate = rotation_;
    transform_.scale = scale_;
}
