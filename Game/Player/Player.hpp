#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "GameObject/GameObject.hpp"
#include "Input.hpp"
#include "Model.hpp"
#include <memory>

class Player : public GameObject {
private:
    // 移動関連
    float moveSpeed_{5.0f};
    Vector3 velocity_{};
    
    // カメラ関連
    Vector3 cameraOffset_{};
    float maxCameraOffset_{10.0f};
    float cameraReturnSpeed_{2.0f};
    
    // レンダリング
    std::unique_ptr<Model> model_;
    
    // 入力
    Input* input_;
    CameraManager* cameraManager_;

public:
    Player();
    ~Player() override;
    
    void Initialize();
    
    // GameObject override
    void Update(float deltaTime) override;
    void Draw() override;
    
    // Player固有メソッド
    void UpdateMovement(float deltaTime);
    void UpdateCameraOffset(float deltaTime);
    
    // ゲッター
    const Vector3& GetVelocity() const { return velocity_; }
    const Vector3& GetCameraOffset() const { return cameraOffset_; }
    
    // セッター
    void SetMoveSpeed(float speed) { moveSpeed_ = speed; }
    void SetMaxCameraOffset(float offset) { maxCameraOffset_ = offset; }

private:
    Vector3 GetInputVector() const;
    Vector3 GetCameraOffsetInput() const;
};

#endif // PLAYER_HPP_