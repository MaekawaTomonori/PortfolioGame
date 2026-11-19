#ifndef SKILL_PARAMS_JSON_HPP_
#define SKILL_PARAMS_JSON_HPP_

#include <variant>

#include "Json/JsonParams.hpp"
#include "Pattern/Singleton.hpp"

inline constexpr const char* SKILL_PARAMS_FILE = "SkillParams";

/** @brief SkillParams から値を読む。無い、または型が違うときは _out をそのままにする */
template <typename T>
void ReadSkillParam(const char* _group, const char* _key, T& _out) {
    const auto value = Singleton<JsonParams>::GetInstance()->GetValue(SKILL_PARAMS_FILE, _group, _key);
    if (const T* found = std::get_if<T>(&value)) _out = *found;
}

#endif // SKILL_PARAMS_JSON_HPP_
