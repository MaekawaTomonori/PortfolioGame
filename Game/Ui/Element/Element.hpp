#ifndef UiElement_HPP_
#define UiElement_HPP_
#include <array>
#include <memory>

#include "Sprite.hpp"


namespace Ui{
    /**
     * @brief UIの最小単位要素
     */
    class Element {
    public:
        struct Data{
            std::string texture = "white_x16.png";
            Vector2 position {};
            Vector2 size{64.f, 64.f};
            Vector4 color{1.f, 1.f, 1.f, 1.f};
        };

    private:
        std::string uuid_;
        std::string name_;

        std::unique_ptr<Sprite> sprite_ = nullptr;

        bool visible_ = true;

        Data data_{};

        bool isOpen_ = true;
        bool aspectLock_ = false;
        bool changeName_ = false;
        std::array<char, 256> nameBuff_ = {};
        std::array<char, 256> textureBuff_ = {};

    public:
        void Initialize();
        void Update();
        void Draw() const;

        void Debug();

        bool IsVisible() const;
        std::string GetName() const;
        std::string GetUUID() const;
        bool& IsOpen();

        Data GetData() const;
        void SetData(const Data& _data);
    }; // class Element
} // namespace Ui

#endif // UiElement_HPP_
