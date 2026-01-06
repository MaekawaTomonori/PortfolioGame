#ifndef ICommand_HPP_
#define ICommand_HPP_

#include "Player/Movement/MovementContext.hpp"

/**
 * @brief コマンドの基底インターフェース
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    /**
     * @brief コマンドを実行
     * @param context 移動コンテキスト
     */
    virtual void Execute(MovementContext& context) = 0;

    /**
     * @brief コマンドがアクティブかどうか
     * @return 実行中の場合true
     */
    virtual bool IsActive() const { return true; }

    /**
     * @brief デバッグ情報をImGuiで表示
     */
    virtual void Debug() {}

    /**
     * @brief コマンドの内部状態をリセット
     */
    virtual void Reset() {}
};

#endif // ICommand_HPP_
