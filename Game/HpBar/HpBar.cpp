#include "HpBar.hpp"

#include <algorithm>
#include <cmath>

#include "imgui.h"
#include "Camera/Controller/CameraController.hpp"
#include "Json/JsonParams.hpp"
#include "Math/MathUtils.hpp"
#include "Pattern/Singleton.hpp"
#include "src/Screen/Screen.hpp"
#include "src/Time/Time.hpp"

namespace {
    // Assets/Data/EnemyHpBar/EnemyHpBar.json の { "EnemyHpBar": { "Bar": { ... } } }
    constexpr const char* PARAMS_NAME  = "EnemyHpBar";
    constexpr const char* PARAMS_GROUP = "Bar";

    std::unique_ptr<Sprite> CreateBar() {
        auto sprite = std::make_unique<Sprite>();
        // 両端の丸いバー（Assets/Resources/UI/hp.png）
        sprite->Initialize("UI/hp.png");
        sprite->SetAnchorPoint({0.f, 0.5f});
        return sprite;
    }

    template <typename T>
    void ReadParam(const JsonParams& _json, const char* _key, T& _value) {
        _json.TryGetValue(PARAMS_NAME, PARAMS_GROUP, _key, _value);
    }
}

void HpBar::Initialize(const Vector3* _ownerPos) {
    ownerPosition_ = _ownerPos;

    back_  = CreateBar();
    trail_ = CreateBar();
    front_ = CreateBar();

    ratio_      = 1.f;
    trailRatio_ = 1.f;
    trailDelay_ = 0.f;
    visible_    = false;
}

void HpBar::Update(const float _hpRatio) {
    if (!ownerPosition_ || !back_) return;

    const float ratio = std::clamp(_hpRatio, 0.f, 1.f);
    if (ratio < ratio_) trailDelay_ = settings_.trailDelay;
    ratio_ = ratio;
    UpdateTrail(Time().GetDeltaTime());

    Vector2 screenPos{};
    visible_ = ratio_ < 1.f && ratio_ > 0.f && ProjectToScreen(screenPos);
    if (!visible_) return;

    // 位置はピクセル、大きさは基準解像度で指定するため、左端の計算にだけ画面の倍率を掛ける
    const float screenScale = Singleton<Screen>::GetInstance()->GetScale();
    const Vector2 size = settings_.size * settings_.scale;
    // 端数があると輪郭がにじむので、ピクセル境界にそろえる
    const Vector2 left = {
        std::round(screenPos.x - size.x * screenScale * 0.5f),
        std::round(screenPos.y - settings_.screenOffsetY * screenScale),
    };

    // 設定は実行中に調整されるので、毎回反映する
    back_->SetColor(settings_.backColor);
    trail_->SetColor(settings_.trailColor);
    front_->SetColor(settings_.frontColor);

    back_->SetPosition(left);
    trail_->SetPosition(left);
    front_->SetPosition(left);
    back_->SetSize(size);
    trail_->SetSize({size.x * trailRatio_, size.y});
    front_->SetSize({size.x * ratio_, size.y});

    back_->Update();
    trail_->Update();
    front_->Update();
}

void HpBar::Draw() const {
    if (!visible_) return;

    back_->Draw();
    trail_->Draw();
    front_->Draw();
}

void HpBar::LoadSettings() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    json->Load(PARAMS_NAME, PARAMS_NAME);

    ReadParam(*json, "Size", settings_.size);
    ReadParam(*json, "Scale", settings_.scale);
    ReadParam(*json, "WorldOffsetY", settings_.worldOffsetY);
    ReadParam(*json, "ScreenOffsetY", settings_.screenOffsetY);
    ReadParam(*json, "BackColor", settings_.backColor);
    ReadParam(*json, "TrailColor", settings_.trailColor);
    ReadParam(*json, "FrontColor", settings_.frontColor);
    ReadParam(*json, "TrailDelay", settings_.trailDelay);
    ReadParam(*json, "TrailSpeed", settings_.trailSpeed);
}

void HpBar::SaveSettings() {
    const auto& json = Singleton<JsonParams>::GetInstance();
    json->SetValue(PARAMS_NAME, PARAMS_GROUP, "Size", settings_.size);
    json->SetValue(PARAMS_NAME, PARAMS_GROUP, "Scale", settings_.scale);
    json->SetValue(PARAMS_NAME, PARAMS_GROUP, "WorldOffsetY", settings_.worldOffsetY);
    json->SetValue(PARAMS_NAME, PARAMS_GROUP, "ScreenOffsetY", settings_.screenOffsetY);
    json->SetValue(PARAMS_NAME, PARAMS_GROUP, "BackColor", settings_.backColor);
    json->SetValue(PARAMS_NAME, PARAMS_GROUP, "TrailColor", settings_.trailColor);
    json->SetValue(PARAMS_NAME, PARAMS_GROUP, "FrontColor", settings_.frontColor);
    json->SetValue(PARAMS_NAME, PARAMS_GROUP, "TrailDelay", settings_.trailDelay);
    json->SetValue(PARAMS_NAME, PARAMS_GROUP, "TrailSpeed", settings_.trailSpeed);
    json->Save(PARAMS_NAME, PARAMS_NAME);
}

void HpBar::DebugSettings() {
#ifdef _DEBUG
    ImGui::Begin("Enemy HpBar");
    ImGui::DragFloat2("Size (px)", &settings_.size.x, 0.5f, 1.f, 200.f);
    ImGui::SliderFloat("Scale", &settings_.scale, 0.5f, 3.f);
    ImGui::DragFloat("World Offset Y", &settings_.worldOffsetY, 0.01f);
    ImGui::DragFloat("Screen Offset Y (px)", &settings_.screenOffsetY, 0.5f);
    ImGui::ColorEdit4("Back", &settings_.backColor.x);
    ImGui::ColorEdit4("Trail", &settings_.trailColor.x);
    ImGui::ColorEdit4("Front", &settings_.frontColor.x);
    ImGui::DragFloat("Trail Delay", &settings_.trailDelay, 0.01f, 0.f, 5.f);
    ImGui::DragFloat("Trail Speed", &settings_.trailSpeed, 0.01f, 0.f, 10.f);
    if (ImGui::Button("Save to JSON")) SaveSettings();
    ImGui::SameLine();
    ImGui::TextDisabled("Assets/Data/EnemyHpBar/EnemyHpBar.json");
    ImGui::End();
#endif
}

void HpBar::UpdateTrail(const float _deltaTime) {
    // 回復したときは残像を残さない
    if (trailRatio_ <= ratio_) {
        trailRatio_ = ratio_;
        return;
    }

    if (trailDelay_ > 0.f) {
        trailDelay_ -= _deltaTime;
        return;
    }
    trailRatio_ = std::max(ratio_, trailRatio_ - settings_.trailSpeed * _deltaTime);
}

bool HpBar::ProjectToScreen(Vector2& _screenPos) const {
    const auto camera = Singleton<CameraController>::GetInstance()->GetActive();
    if (!camera) return false;

    const Vector3 world = {ownerPosition_->x, ownerPosition_->y + settings_.worldOffsetY, ownerPosition_->z};
    const Vector4 clip = MathUtils::Matrix::Transform(Vector4{world.x, world.y, world.z, 1.f}, camera->GetViewProjection());
    // カメラの後ろにあると w が負になり、上下左右が反転して映り込む
    if (clip.w <= 0.f) return false;

    const Screen* screen = Singleton<Screen>::GetInstance();
    const float ndcX = clip.x / clip.w;
    const float ndcY = clip.y / clip.w;
    _screenPos = {
        (ndcX * 0.5f + 0.5f) * screen->Width(),
        (0.5f - ndcY * 0.5f) * screen->Height(),
    };
    return true;
}
