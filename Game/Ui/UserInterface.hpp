#ifndef UserInterface_HPP_
#define UserInterface_HPP_
#include <unordered_map>

#include "Element/Element.hpp"

namespace Ui {
    /*** @brief UIのルートクラス
     * このクラスを1単位として使用する
     */
    class UserInterface {
        struct UiActions {
            // key : ActionID, value : UiActions
            std::unordered_map<std::string, std::function<void()>> actions;
        };
        
        UiActions actions_;

        std::vector<std::unique_ptr<Element>> elements_;

        bool editing_ = false;
        bool defaultOpen_ = true;

    public:
        void Update() const;
        void Draw() const;

        void Debug();

        void RegisterAction(const std::string& _actionId, const std::function<void()>& _action);

    private:
        void Editor();

    }; // class UserInterface
} // namespace Ui
#endif // UserInterface_HPP_
