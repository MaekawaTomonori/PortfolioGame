# CLAUDE.md

このファイルは、このリポジトリでコードを操作する際のClaude Code (claude.ai/code) へのガイダンスを提供します。

## プロジェクト概要

ゲーム本体。

## ディレクトリ構造

- `Game/` - ゲームの子プロジェクト
- `Engine/` - ゲームエンジンの子プロジェクト(DirectX12を主としている)。CLAUDE.mdを持つ独立的プロジェクト。
- `Lib/` - ゲーム直接依存するゲームエンジン以外のライブラリ
- `Assets/` - ゲームのリソースやコンフィグのGit更新用。操作の必要なし。

- `Game/Assets` - アセット本体
  - `Config/` - ImGuiのコンフィグ
  - `Data/` - データ駆動用Json格納庫
  - `Fonts/` - TTFなどのフォント
  - `Resources` - 画像とモデル
  - `Shaders/` - 描画用シェーダー(.hlsl)

## ビルド/テスト

ビルドや起動テストはユーザーが行うためテストまで行う必要はない
編集・提案の終了をもって、ユーザーが実行確認を行うものとする

## 詳細ドキュメント

 - ゲーム企画 : `Docs/game.md`
 - 設計思想 : `Docs/design-philosophy.md`
 - コードルール : `Docs/Coderule.md`