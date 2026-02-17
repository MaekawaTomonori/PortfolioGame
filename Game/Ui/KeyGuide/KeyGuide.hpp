#ifndef KeyGuide_HPP_
#define KeyGuide_HPP_
#include <memory>
#include <string>

#include "Input.hpp"
#include "Sprite.hpp"
#include "Ui/UserInterface.hpp"

class KeyGuide {
    struct Key {
        Vector2 position;
        std::string texture;

        Vector4 defaultColor{};
        Vector4 pressColor{};

        std::unique_ptr<Sprite> key;
        std::unique_ptr<Sprite> frame;
    };

    const std::string JsonPath = "Assets/Data/KeyGuide/data.json";

    Input* input_ = nullptr;

    Vector2 position_ {};

    std::unique_ptr<Sprite> frame_;

    std::unordered_map<BYTE, Key> keys_;

    std::unique_ptr<Ui::UserInterface> ui_;

    // Debug用の入力バッファ
    int selectedKeyIndex_ = 0;
    char newKeyTexture_[256] = "white_x16.png";

    bool reload_ = false;

    // コピー用バッファ
    bool hasClipboard_ = false;
    struct ClipboardData {
        Vector2 position;
        std::string texture;
        Vector4 defaultColor;
        Vector4 pressColor;
        Vector2 frameSize;
        Vector2 labelSize;
    } clipboardKey_{};

public:
    void Initialize();
    void Update();
    void Draw() const;
    void Debug();

private:
    void Load();
    void Save();
    void Register(const BYTE& _code, Key _key);

}; // class KeyGuide

#endif // KeyGuide_HPP_
