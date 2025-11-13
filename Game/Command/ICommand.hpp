#ifndef ICommand_HPP_
#define ICommand_HPP_

class GameObject;

/// <summary>
/// コマンドの基底インターフェース
/// 移動、攻撃などのアクションを表現
/// </summary>
class ICommand {
public:
    virtual ~ICommand() = default;

    /// <summary>
    /// コマンドを実行
    /// </summary>
    /// <param name="executor">実行者（敵またはプレイヤー）</param>
    /// <param name="target">ターゲット（NULLの場合もある）</param>
    virtual void Execute(GameObject* executor, GameObject* target) = 0;
}; // class ICommand

#endif // ICommand_HPP_
