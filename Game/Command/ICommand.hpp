#ifndef ICommand_HPP_
#define ICommand_HPP_

class GameObject;

/// <summary>
/// コマンドの基底インターフェース
/// 移動、攻撃などのアクションを表現
///
/// 設計方針:
/// - Flyweightパターン: 複数のGameObjectで共有可能
/// - ステートフル: 各コマンドは内部状態を持てる（タイマー、フェーズなど）
/// - デバッグ可能: ImGuiでデバッグ情報表示
/// </summary>
class ICommand {
public:
    virtual ~ICommand() = default;

    /// <summary>
    /// コマンドを実行
    /// </summary>
    /// <param name="executor">実行者（敵またはプレイヤー）</param>
    /// <param name="target">ターゲット（NULLの場合もある）</param>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    virtual void Execute(GameObject* executor, GameObject* target, float deltaTime) = 0;

    /// <summary>
    /// コマンドがアクティブかどうか
    /// </summary>
    /// <returns>実行中の場合true</returns>
    virtual bool IsActive() const { return true; }

    /// <summary>
    /// デバッグ情報をImGuiで表示
    /// </summary>
    virtual void Debug() {}

    /// <summary>
    /// コマンドの内部状態をリセット
    /// コマンドを再利用する際に呼び出す
    /// </summary>
    virtual void Reset() {}
}; // class ICommand

#endif // ICommand_HPP_
