#include "HpBar.hpp"

#include "Camera/Controller/CameraController.hpp"

void HpBar::Initialize(const std::string& _texture, Vector3* _ownerPos) {
    texture_ = _texture;
    ownerPosition_ = _ownerPos;

    front_ = std::make_unique<Model>();
    back_ = std::make_unique<Model>();

    front_->Initialize("plane");
    back_->Initialize("plane");

    front_->SetTexture(texture_);
    back_->SetTexture(texture_);

    front_->SetColor(frontColor_);
    back_->SetColor(backColor_);

    Vector3 barPos = *ownerPosition_ + offset_;
    front_->SetTranslate(barPos);
    back_->SetTranslate(barPos);

    front_->SetScale(size_);
    back_->SetScale(size_);

    front_->SetRotate({ -(MathUtils::F_PI / 2.f), 0.f, 0.f });
    back_->SetRotate({ -(MathUtils::F_PI / 2.f), 0.f, 0.f });

    front_->GetMesh()->EnableLighting(false);
}

void HpBar::Update(const float _hpRatio) const {
    if (!ownerPosition_) return;

    front_->SetColor(frontColor_);
    back_->SetColor(backColor_);

    front_->SetScale({ size_.x * _hpRatio, size_.y, size_.z });
    back_->SetScale({ size_.x, size_.y, size_.z });

    Vector3 barPos = *ownerPosition_ + offset_;
    front_->SetTranslate(barPos);
    back_->SetTranslate(barPos);

    back_->Update();
    front_->Update();
}

void HpBar::Draw() const {
    if (!ownerPosition_) return;

    back_->Draw();
    front_->Draw();
}
