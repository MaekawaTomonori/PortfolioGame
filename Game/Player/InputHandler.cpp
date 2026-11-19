#include "InputHandler.hpp"

#include "Camera/Controller/CameraController.hpp"
#include "Math/MathUtils.hpp"
#include "Pattern/Singleton.hpp"
#include "Screen/Screen.hpp"
#include "src/Camera/Camera.hpp"

void InputHandler::Initialize(const KeyBinds& _binds) {
    input_ = Singleton<Input>::GetInstance();
    binds_ = _binds;
}

void InputHandler::UpdateContext(MovementContext& _context, const Vector3& _currentPos) const {
    if (!input_) return;

    _context.Reset();
    _context.position = _currentPos;

    // 移動入力
    if (IsPress(InputAction::MoveForward)) _context.moveDirection.z += 1.0f;
    if (IsPress(InputAction::MoveBack)) _context.moveDirection.z -= 1.0f;
    if (IsPress(InputAction::MoveLeft)) _context.moveDirection.x -= 1.0f;
    if (IsPress(InputAction::MoveRight)) _context.moveDirection.x += 1.0f;

    // 正規化
    if (_context.moveDirection.Length() > 0.0f) {
        _context.moveDirection = _context.moveDirection.Normalize();
    }

    _context.isDashRequested = IsPress(InputAction::Dash);
}

bool InputHandler::IsSkillRequested(SkillSlot _slot) const {
    if (!input_) return false;

    switch (_slot) {
    case SkillSlot::Ability1: return IsTrigger(InputAction::Ability1);
    case SkillSlot::Ability2: return IsTrigger(InputAction::Ability2);
    default: return false;
    }
}

Vector3 InputHandler::GetMouseInWorld() const {
    if (!input_) return {};
    if (auto camera = Singleton<CameraController>::GetInstance()->GetActive()) {
        // スクリーン座標を取得
        Vector2 mousePos = input_->GetMousePosition();
        
        // スクリーン座標 → NDC に手動変換
        float ndcX = (2.0f * mousePos.x / Singleton<Screen>::GetInstance()->Width()) - 1.0f;
        float ndcY = 1.0f - (2.0f * mousePos.y / Singleton<Screen>::GetInstance()->Height());

        // ViewProjectionの逆行列のみで逆変換
        Matrix4x4 mInv = camera->GetViewProjection().Inverse();

        Vector3 nPos = {ndcX, ndcY, 0.f};
        Vector3 fPos = {ndcX, ndcY, 1.f};

        nPos = MathUtils::Matrix::Transform(nPos, mInv);
        fPos = MathUtils::Matrix::Transform(fPos, mInv);

        Vector3 mouseDir = (fPos - nPos).Normalize();

        Vector3 res = camera->transform_.translate;

        // レイとY=0平面の交点を求める
        if (std::abs(mouseDir.y) > 1e-6f) {
            float t = -nPos.y / mouseDir.y;
            if (t >= 0.f) {
                res = nPos + mouseDir * t;
            }
        }
        return res;
    }
    return {};
}

bool InputHandler::IsPress(InputAction _action) const {
    return input_->IsPress(binds_.GetKey(_action));
}

bool InputHandler::IsTrigger(InputAction _action) const {
    return input_->IsTrigger(binds_.GetKey(_action));
}
