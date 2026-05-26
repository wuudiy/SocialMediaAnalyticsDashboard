@echo off
setlocal

:: 设置环境变量，确保 Qt MinGW 13.1.0 在最前面
set PATH=D:\qt\Tools\mingw1310_64\bin;D:\qt\6.10.2\mingw_64\bin;D:\qt\Tools\CMake_64\bin;%PATH%

:: 进入项目目录
cd /d D:\qt\test\SocialMediaAnalyticsDashboard

:: 清理旧构建目录
if exist build rmdir /s /q build
mkdir build
cd build

:: 运行 CMake 配置
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

:: 编译项目
mingw32-make -j4

endlocal
pause