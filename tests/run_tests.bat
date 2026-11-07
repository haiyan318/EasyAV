@echo off
REM 轻量单元测试：只链接纯逻辑源码，不依赖 ImGui 与正式 UI。
REM 需先运行 vcvars32.bat / vcvars64.bat，使 cl 在 PATH 中。
setlocal
cd /d "%~dp0"
if not exist bin mkdir bin

cl /nologo /std:c++17 /utf-8 /EHsc /W4 /I..\src /I. /Fo:bin\ /Fe:bin\EasyAVTests.exe ^
  TestMain.cpp TestCommandLine.cpp TestProbeParser.cpp TestOutputNamer.cpp ^
  TestCommandBuilder.cpp TestErrorFormatter.cpp TestProgressParser.cpp ^
  TestTaskController.cpp TestConvertMapping.cpp ^
  ..\src\services\CommandLine.cpp ..\src\services\ProbeOutputParser.cpp ^
  ..\src\services\OutputNamer.cpp ..\src\services\FfmpegCommandBuilder.cpp ^
  ..\src\services\ErrorFormatter.cpp ..\src\services\ProgressParser.cpp ^
  ..\src\model\TaskController.cpp ..\src\model\ConvertMapping.cpp ^
  ..\src\core\Text.cpp ..\src\core\Format.cpp ^
  /link shell32.lib
if errorlevel 1 exit /b 1

bin\EasyAVTests.exe
exit /b %errorlevel%
