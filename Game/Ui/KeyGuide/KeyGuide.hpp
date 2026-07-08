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

        // CTオーバーレイ用（Canvasエレメント名で指定）
        std::string cooldownElementName;
        float cooldownRatio = 0.f;
        float cooldownFullHeight = 0.f;
        Vector2 cooldownBasePos = {};
        float failFlashTimer = 0.f;
    };

    const std::string JsonPath = "Assets/Data/KeyGuide/data.json";

    Input* input_ = nullptr;

    Vector2 position_ {};

    std::unique_ptr<Sprite> frame_;

    std::unordered_map<BYTE, Key> keys_;

    std::unique_ptr<Ui::Canvas> ui_;

    // Debug用の入力バッファ
    int selectedKeyIndex_ = 0;
    char newKeyTexture_[256] = "white_x16.png";

    bool reload_ = false;
    bool active_ = true;

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
    void SetActive(bool _active);

    /** @brief 指定キーのCT比率を設定する (0=ready, 1=full cooldown) **/
    void SetCooldown(BYTE _key, float _ratio);

private:
    void Load();
    void Save();
    void Register(const BYTE& _code, Key _key);

}; // class KeyGuide

#endif // KeyGuide_HPP_
