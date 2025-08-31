#ifndef CAMERA_CONTROLLER_HPP_
#define CAMERA_CONTROLLER_HPP_

#include "Input.hpp"
#include "Math/Vector3.hpp"
#include "Camera/Manager/CameraManager.hpp"
#include "Pattern/Singleton.hpp"

class GameObject;

class CameraController {
private:
    GameObject* target_;
    Input* input_;
    CameraManager* cameraManager_;
    
    Vector3 cameraOffset_{0.0f, 10.0f, 0.0f};
    float maxCameraOffset_{10.0f};
    float cameraReturnSpeed_{2.0f};
    float cameraHeight_{50.0f};

public:
    CameraController();
    ~CameraController();
    
    void Initialize();
    void Update(float deltaTime);
    
    // Target設定
    void SetTarget(GameObject* target) { target_ = target; }
    
    // Camera設定
    void SetMaxOffset(float offset) { maxCameraOffset_ = offset; }
    void SetCameraHeight(float height) { cameraHeight_ = height; }
    void SetReturnSpeed(float speed) { cameraReturnSpeed_ = speed; }
    
    // Getters
    const Vector3& GetCameraOffset() const { return cameraOffset_; }

private:
    Vector3 GetCameraOffsetInput() const;
    void UpdateCameraPosition();
};

#endif // CAMERA_CONTROLLER_HPP_