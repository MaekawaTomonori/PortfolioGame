#ifndef HUD_LAYOUT_HPP_
#define HUD_LAYOUT_HPP_
#include <span>
#include <string>

#include "GEUI.hpp"

/** @brief HUD のまとまり（入れ物の Widget）の位置・倍率を調整し、Canvas の JSON へ書き戻す
 */
namespace HudLayout {
    /** @brief 調整対象のまとまり。name は Canvas 直下の Widget 名 */
    struct Group {
        const char* name = "";
        GEUI::Widget widget;
    };

    /** @brief 位置（基準解像度のピクセル）と倍率を ImGui で編集する
     * @param _group 対象
     */
    void DrawEditor(Group& _group);

    /** @brief まとまりの位置と倍率を Canvas の JSON に保存する（他の項目はそのまま残す）
     * @param _canvasPath Canvas の JSON のパス
     * @param _groups 保存するまとまり
     * @return 保存できたか
     */
    bool Save(const std::string& _canvasPath, std::span<const Group> _groups);

    /** @brief 保存ボタンと結果の表示をまとめて描く
     * @param _canvasPath Canvas の JSON のパス
     * @param _groups 保存するまとまり
     */
    void DrawSaveButton(const std::string& _canvasPath, std::span<const Group> _groups);
}

#endif // HUD_LAYOUT_HPP_
