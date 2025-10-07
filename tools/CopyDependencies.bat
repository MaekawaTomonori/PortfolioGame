@echo off
setlocal enabledelayedexpansion

:: 引数: %1=TargetDir, %2=SolutionDir, %3=ProjectDir
set TARGET_DIR=%1
set SOLUTION_DIR=%2
set PROJECT_DIR=%3

echo Copying DirectX dependencies...

:: Windows SDK パスの候補を設定
set SDK_PATHS[0]="%WindowsSdkDir%bin\%TargetPlatformVersion%\x64"
set SDK_PATHS[1]="%WindowsSdkDir%bin\%WindowsTargetPlatformVersion%.0\x64"
set SDK_PATHS[2]="%WindowsSdkDir%bin\10.0.22621.0\x64"
set SDK_PATHS[3]="%WindowsSdkDir%bin\10.0.26100.0\x64"

:: dxcompiler.dll のコピー
set FOUND_DXC=0
for /L %%i in (0,1,3) do (
    if exist !SDK_PATHS[%%i]!\dxcompiler.dll (
        echo Found dxcompiler.dll at !SDK_PATHS[%%i]!
        copy !SDK_PATHS[%%i]!\dxcompiler.dll "%TARGET_DIR%dxcompiler.dll" > nul
        set FOUND_DXC=1
        goto :dxil_search
    )
)
:dxil_search

:: dxil.dll のコピー
set FOUND_DXIL=0
for /L %%i in (0,1,3) do (
    if exist !SDK_PATHS[%%i]!\dxil.dll (
        echo Found dxil.dll at !SDK_PATHS[%%i]!
        copy !SDK_PATHS[%%i]!\dxil.dll "%TARGET_DIR%dxil.dll" > nul
        set FOUND_DXIL=1
        goto :copy_assets
    )
)
:copy_assets

:: アセットのコピー
echo Copying assets...
if exist "%SOLUTION_DIR%Assets" (
    xcopy "%SOLUTION_DIR%Assets" "%PROJECT_DIR%Assets" /E /Y /I > nul
    echo Assets copied successfully
) else (
    echo Warning: Assets directory not found at %SOLUTION_DIR%Assets
)

:: 結果報告
if %FOUND_DXC%==0 echo Warning: dxcompiler.dll not found in any SDK path
if %FOUND_DXIL%==0 echo Warning: dxil.dll not found in any SDK path

echo Dependency copy completed.