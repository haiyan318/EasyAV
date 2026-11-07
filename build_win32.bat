@REM Build for Visual Studio compiler (32-bit). Run vcvars32.bat / vcvarsall.bat first.
@REM ImGui sources are reused from the sibling ..\imgui-docking folder.
@set OUT_DIR=Debug
@set OUT_EXE=EasyAV
@set INCLUDES=/Isrc /I..\imgui-docking /I..\imgui-docking\backends /I..\imgui-docking\examples\libs\glfw\include
@set SOURCES=main.cpp src\*.cpp src\core\*.cpp src\model\*.cpp src\platform\*.cpp src\ui\*.cpp src\ui\panels\*.cpp
@set SOURCES=%SOURCES% ..\imgui-docking\backends\imgui_impl_glfw.cpp ..\imgui-docking\backends\imgui_impl_opengl3.cpp ..\imgui-docking\imgui*.cpp
@set LIBS=/LIBPATH:..\imgui-docking\examples\libs\glfw\lib-vc2010-32 glfw3.lib opengl32.lib gdi32.lib shell32.lib comdlg32.lib ole32.lib
mkdir %OUT_DIR% 2>NUL
cl /nologo /Zi /MD /EHsc /std:c++17 /utf-8 %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS%
