#include "GameObject.hpp"

#include "Time/Time.hpp"

void GameObject::SetPosition(const Vector3& position) {
    position_ = position;
}

void GameObject::SetRotation(const Vector3& rotation) {
    rotation_ = rotation;
}

void GameObject::SetScale(const Vector3& scale) {
    scale_ = scale;
}

void GameObject::ApplyVelocity() {
    position_ += velocity_ * Time().GetDeltaTime();
}

void GameObject::SetModel(const std::string& _name) {
    model_ = std::make_unique<Model>();
    model_->Initialize(_name);
}

void GameObject::UpdateModel() {
    if (model_){
        model_->SetScale(scale_);
        model_->SetRotate(rotation_);
        model_->SetTranslate(position_ + offset_);
        model_->Update();
    }

    offset_ = {};
}
