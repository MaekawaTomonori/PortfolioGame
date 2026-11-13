#ifndef Pause_HPP_
#define Pause_HPP_
#include <functional>

#include "GEUI.hpp"
#include "Input.hpp"

class Pause {
    GEUI::Canvas ui_;
    GEUI::Widget resume_;
    GEUI::Widget retry_;
    GEUI::Widget quit_;

    Input* input_ = nullptr;
    float fadeTime_ = 0.f;

    std::function<void()> onResume_;
    std::function<void()> onRetry_;
    std::function<void()> onQuit_;

public:
    ~Pause();

    void Initialize();
    void Update();

    void Open();
    void Close();
    bool IsOpen() const;

    void SetOnResume(std::function<void()> _callback);
    void SetOnRetry(std::function<void()> _callback);
    void SetOnQuit(std::function<void()> _callback);

private:
    void CheckActivateState();
    void HandleInput();
    void UpdateFade(float _deltaTime);

}; // class Pause

#endif // Pause_HPP_
