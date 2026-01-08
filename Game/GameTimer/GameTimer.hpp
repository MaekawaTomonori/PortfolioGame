#ifndef GameTimer_HPP_
#define GameTimer_HPP_
#include <array>
#include <memory>

#include "Sprite.hpp"

class GameTimer {
    enum CountStyle {
        UP,
        DOWN
    };

    const Vector2 TEXTURE_SIZE = { 64.f, 96.f };


    CountStyle style_ = DOWN;
    float time_ = 0.f;
    std::array<std::unique_ptr<Sprite>, 2> sprites_;

    Vector2 position_ = { 50.f, 50.f };
    Vector2 size_ = TEXTURE_SIZE;
    Vector4 color_ = { 1.f, 1.f, 1.f, 1.f };

    bool done_ = false;

public:
    void Initialize();
    void Update(float _deltaTime);
    void Draw() const;

    void Debug();

    bool IsDone() const { return done_; }

    void SetPosition(const Vector2& _position) { position_ = _position; }
    void SetColor(const Vector4& _color) { color_ = _color; }
    void SetDuration(float _sec);

private:

}; // class GameTimer

#endif // GameTimer_HPP_
