#ifndef ToTargetCommand_HPP_
#define ToTargetCommand_HPP_
#include "Command/ICommand.hpp"

/// <summary>
/// ターゲットに向かって移動するコマンド
/// </summary>
class ToTargetCommand : public ICommand {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="speed">移動速度</param>
    /// <param name="minDistance">これ以上近づかない距離</param>
    ToTargetCommand(float speed = 3.0f, float minDistance = 0.5f);

    void Execute(GameObject* executor, GameObject* target, float deltaTime) override;

private:
    float speed_;
    float minDistance_;
}; // class ToTargetCommand

#endif // ToTargetCommand_HPP_
