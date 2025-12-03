@echo off
chcp 65001 >nul
echo ====================================
echo プロジェクト提出用クリーンアップ
echo ====================================
echo.
echo 以下のファイル・フォルダを削除します:
echo - .git フォルダ・ファイル
echo - .vs フォルダ
echo - .github フォルダ
echo - .gitignore, .gitattributes, .gitmodules ファイル
echo - *.md ファイル (Markdownメモ)
echo - *.cd ファイル (クラス図)
echo - *.vcxproj.user ファイル
echo - *.vcxproj.filters ファイル
echo - out フォルダ (ビルド出力)
echo - Compiled フォルダ (シェーダーコンパイル出力)
echo - Log フォルダ
echo - review フォルダ
echo - tools フォルダ
echo - mds フォルダ
echo.
echo 現在のディレクトリ: %CD%
echo.

pause

echo.
echo 削除を開始します...
echo.

REM .gitフォルダとファイルを削除
echo [1/16] .gitフォルダ・ファイルを削除中...
for /d /r . %%d in (.git) do (
    if exist "%%d" (
        echo   削除: %%d
        rd /s /q "%%d" 2>nul
    )
)
REM .gitファイルを削除（サブモジュール参照ファイル）
if exist "Engine\.git" (
    echo   削除: Engine\.git
    del /f /q "Engine\.git" 2>nul
)
if exist "Engine\vendor\spdlog\.git" (
    echo   削除: Engine\vendor\spdlog\.git
    del /f /q "Engine\vendor\spdlog\.git" 2>nul
)
if exist "Lib\Collision\.git" (
    echo   削除: Lib\Collision\.git
    del /f /q "Lib\Collision\.git" 2>nul
)
REM その他の.gitファイルを検索して削除
powershell -Command "Get-ChildItem -Path . -Filter .git -Recurse -File -Force -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue"

REM .vsフォルダを削除
echo [2/16] .vsフォルダを削除中...
for /d /r . %%d in (.vs) do (
    if exist "%%d" (
        echo   削除: %%d
        rd /s /q "%%d" 2>nul
    )
)

REM .githubフォルダを削除
echo [3/16] .githubフォルダを削除中...
for /d /r . %%d in (.github) do (
    if exist "%%d" (
        echo   削除: %%d
        rd /s /q "%%d" 2>nul
    )
)

REM outフォルダを削除
echo [4/16] outフォルダを削除中...
for /d /r . %%d in (out) do (
    if exist "%%d" (
        echo   削除: %%d
        rd /s /q "%%d" 2>nul
    )
)

REM Compiledフォルダを削除
echo [5/16] Compiledフォルダを削除中...
for /d /r . %%d in (Compiled) do (
    if exist "%%d" (
        echo   削除: %%d
        rd /s /q "%%d" 2>nul
    )
)

REM Logフォルダを削除
echo [6/16] Logフォルダを削除中...
for /d /r . %%d in (Log) do (
    if exist "%%d" (
        echo   削除: %%d
        rd /s /q "%%d" 2>nul
    )
)

REM reviewフォルダを削除
echo [7/16] reviewフォルダを削除中...
for /d /r . %%d in (review) do (
    if exist "%%d" (
        echo   削除: %%d
        rd /s /q "%%d" 2>nul
    )
)

REM toolsフォルダを削除
echo [8/16] toolsフォルダを削除中...
for /d /r . %%d in (tools) do (
    if exist "%%d" (
        echo   削除: %%d
        rd /s /q "%%d" 2>nul
    )
)

REM mdsフォルダを削除
echo [9/16] mdsフォルダを削除中...
for /d /r . %%d in (mds) do (
    if exist "%%d" (
        echo   削除: %%d
        rd /s /q "%%d" 2>nul
    )
)

REM .gitignoreファイルを削除
echo [10/16] .gitignoreファイルを削除中...
powershell -Command "Get-ChildItem -Path . -Filter .gitignore -Recurse -File -Force -ErrorAction SilentlyContinue | ForEach-Object { Write-Host \"  削除: $($_.FullName)\"; Remove-Item $_.FullName -Force -ErrorAction SilentlyContinue }"

REM .gitattributesファイルを削除
echo [11/16] .gitattributesファイルを削除中...
powershell -Command "Get-ChildItem -Path . -Filter .gitattributes -Recurse -File -Force -ErrorAction SilentlyContinue | ForEach-Object { Write-Host \"  削除: $($_.FullName)\"; Remove-Item $_.FullName -Force -ErrorAction SilentlyContinue }"

REM .gitmodulesファイルを削除
echo [12/16] .gitmodulesファイルを削除中...
powershell -Command "Get-ChildItem -Path . -Filter .gitmodules -Recurse -File -Force -ErrorAction SilentlyContinue | ForEach-Object { Write-Host \"  削除: $($_.FullName)\"; Remove-Item $_.FullName -Force -ErrorAction SilentlyContinue }"

REM .mdファイルを削除
echo [13/16] .mdファイルを削除中...
for /r . %%f in (*.md) do (
    echo   削除: %%f
    del /f /q "%%f" 2>nul
)

REM .cdファイルを削除
echo [14/16] .cdファイルを削除中...
for /r . %%f in (*.cd) do (
    echo   削除: %%f
    del /f /q "%%f" 2>nul
)

REM .vcxproj.userファイルを削除
echo [15/16] .vcxproj.userファイルを削除中...
for /r . %%f in (*.vcxproj.user) do (
    echo   削除: %%f
    del /f /q "%%f" 2>nul
)

REM .vcxproj.filtersファイルを削除
echo [16/16] .vcxproj.filtersファイルを削除中...
for /r . %%f in (*.vcxproj.filters) do (
    echo   削除: %%f
    del /f /q "%%f" 2>nul
)

REM 誤って生成されたnulファイルを削除
echo 誤生成されたnulファイルを検索中...
powershell -Command "Get-ChildItem -Path . -Filter nul -Recurse -File -Force -ErrorAction SilentlyContinue | ForEach-Object { Write-Host \"  削除: $($_.FullName)\"; Remove-Item \"\\?\$($_.FullName)\" -Force -ErrorAction SilentlyContinue }"
if exist ".\nul" (
    echo   削除: .\nul (カレントディレクトリ)
    powershell -Command "Remove-Item '\\?\%CD%\nul' -Force -ErrorAction SilentlyContinue"
)

echo.
echo ====================================
echo クリーンアップが完了しました
echo ====================================
echo.
echo このスクリプト自体を削除します...
timeout /t 3 /nobreak >nul

REM 自分自身を削除
(goto) 2>nul & del /f /q "%~f0"
