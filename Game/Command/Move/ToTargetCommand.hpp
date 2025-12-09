#ifndef ToTargetCommand_HPP_
#define ToTargetCommand_HPP_
#include "Command/ICommand.hpp"

/**
 * @brief ターゲットに向かって直線的に移動するコマンド
 */
class ToTargetCommand : public ICommand {
public:
    /**
     * @brief コンストラクタ
     * @param speed 移動速度（未使用、Behaviorで管理）
     * @param minDistance これ以上近づかない距離
     */
    ToTargetCommand(float speed = 3.0f, float minDistance = 0.5f);

    void Execute(MovementContext& context) override;
    void Debug() override;

private:
    float speed_;
    float minDistance_;
};

#endif // ToTargetCommand_HPP_
