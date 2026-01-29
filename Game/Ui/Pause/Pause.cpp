#include "Pause.hpp"

#include "imgui.h"
#include "Math/Easing.hpp"
#include "Pattern/Singleton.hpp"

void Pause::Initialize() {
    input_ = Singleton<Input>::GetInstance();

    ui_ = std::make_unique<Ui::UserInterface>();
    ui_->Setup("pause");
    ui_->SetActive(false);

    ui_->RegisterAction("Pause.Resume", [this]() { Close(); });
    ui_->RegisterAction("Pause.Retry", [this]() { if (onRetry_) onRetry_(); });
    ui_->RegisterAction("Pause.Quit", [this]() { if (onQuit_) onQuit_(); });

    actionIndices_ = ui_->GetIndicesWithEvent(Ui::EventKey::Execute);

    cursor_ = std::make_unique<Sprite>();
    cursor_->Initialize("white_x16.png");
    cursor_->SetSize({16.f, 16.f});

    if (actionIndices_.empty()) return;

    size_t elemIndex = actionIndices_[selectedIndex_];
    auto rect = ui_->GetElementRect(elemIndex);

    float cursorX = rect.position.x - rect.size.x / 2.f - 24.f;
    float cursorY = rect.position.y;
    cursor_->SetPosition({cursorX, cursorY});
}

void Pause::Update() {
    if (!ui_)return;

    CheckActivateState();

    if (!ui_->IsActive()) return;

    HandleInput();
    UpdateCursor();

    ui_->Update();
    cursor_->Update();
}

void Pause::Draw() {
    if (!ui_)return;
    if (!ui_->IsActive()) return;

    ui_->Draw();
    cursor_->Draw();
}

void Pause::HandleInput() {
    if (actionIndices_.empty()) return;

    int32_t count = static_cast<int32_t>(actionIndices_.size());
    
    bool up = input_->IsTrigger(DIK_UP) || input_->IsTrigger(DIK_W);
    bool down = input_->IsTrigger(DIK_DOWN) || input_->IsTrigger(DIK_S);
    if (up || down) {
        selectedIndex_ = up ? (selectedIndex_ - 1 + count) % count : (selectedIndex_ + 1) % count;
        timer_ = 0.f;
        start_ = cursor_->GetPosition();

        if (ui_->IsDirty()) {
            actionIndices_.clear();
            actionIndices_ = ui_->GetIndicesWithEvent(Ui::EventKey::Execute);
        }

        if (actionIndices_.empty()) return;

        size_t elemIndex = actionIndices_[selectedIndex_];
        auto rect = ui_->GetElementRect(elemIndex);

        float cursorX = rect.position.x - rect.size.x / 2.f - 24.f;
        float cursorY = rect.position.y;

        destination_ = {cursorX, cursorY};
    }
    if (input_->IsTrigger(DIK_RETURN) || input_->IsTrigger(DIK_SPACE)) {
        Execute();
    }
}

void Pause::UpdateCursor() {
    if (interpolationTime_<timer_)return;
    // TODO DeltaTime
    timer_ += 1.f/60.f;

    Vector2 pos = Ease::Out::Cubic(start_, destination_, timer_ / interpolationTime_);

    cursor_->SetPosition(pos);
}

void Pause::Execute() const {
    if (actionIndices_.empty()) return;

    size_t elemIndex = actionIndices_[selectedIndex_];
    ui_->ExecuteActionAt(elemIndex, Ui::EventKey::Execute);
}

void Pause::Open() {
    ui_->SetActive(true);
    selectedIndex_ = 0;
    UpdateCursor();
}

void Pause::Close() const {
    ui_->SetActive(false);
    if (onResume_) onResume_();
}

bool Pause::IsOpen() const {
    return ui_->IsActive();
}

void Pause::SetOnResume(std::function<void()> _callback) {
    onResume_ = std::move(_callback);
}

void Pause::SetOnRetry(std::function<void()> _callback) {
    onRetry_ = std::move(_callback);
}

void Pause::SetOnQuit(std::function<void()> _callback) {
    onQuit_ = std::move(_callback);
}

void Pause::CheckActivateState() {
    if (input_->IsTrigger(DIK_ESCAPE)) {
        IsOpen() ? Close() : Open();
    }
}

void Pause::Debug() {
    if (!ui_)return;

    ImGui::Begin("PauseMenu");
    ImGui::Text("Interpolation Time(sec)");
    ImGui::DragFloat("##interpolationtime", &interpolationTime_, 0.01f, 0.0f, 10.0f);
    ImGui::Separator();

    ui_->Debug();
    ImGui::End();
}
