#ifndef Pause_HPP_
#define Pause_HPP_
#include <memory>
#include <functional>
#include <vector>

#include "Input.hpp"
#include "Ui/UserInterface.hpp"

class Pause {
    std::unique_ptr<Ui::Canvas> ui_;
    std::unique_ptr<Sprite> cursor_;

    Input* input_ = nullptr;

    std::vector<size_t> actionIndices_;
    int32_t selectedIndex_ = 0;

    std::function<void()> onResume_;
    std::function<void()> onRetry_;
    std::function<void()> onQuit_;

    float interpolationTime_ = 0.4f;
    Vector2 start_ {};
    Vector2 destination_{};
    float timer_ = 1.f;

public:
    void Initialize();
    void Update();
    void Draw();

    void Open();
    void Close() const;
    bool IsOpen() const;

    void Debug();

    void SetOnResume(std::function<void()> _callback);
    void SetOnRetry(std::function<void()> _callback);
    void SetOnQuit(std::function<void()> _callback);

private:
    void CheckActivateState();
    void HandleInput();
    void UpdateCursor();
    void Execute() const;

}; // class Pause

#endif // Pause_HPP_
