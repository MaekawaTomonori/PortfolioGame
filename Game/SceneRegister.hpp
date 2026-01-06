#ifndef SceneRegister_HPP_
#define SceneRegister_HPP_
#include <functional>

#include "IScene.hpp"

namespace SceneRegister {
    void Register(const std::function<void(std::string, std::function<std::unique_ptr<IScene>()>)>& _func);
}

#endif
