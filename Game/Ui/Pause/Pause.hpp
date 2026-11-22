#ifndef PAUSE_HPP_
#define PAUSE_HPP_
#include <functional>

#include "GEUI.hpp"
#include "Ui/MenuInput.hpp"

class Pause {
    GEUI::Canvas ui_;
    GEUI::Widget resume_;
    GEUI::Widget retry_;
    GEUI::Widget quit_;

    MenuInput menuInput_;
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

#endif // PAUSE_HPP_
