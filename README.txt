EasyAV - 工程框架说明
========================================

【本次交付范围】
仅搭建 Visual Studio C++ 工程框架，等同于复制
  ..\imgui-docking\examples\example_glfw_opengl3
的工程骨架。不含任何 EasyAV 业务代码（FFmpeg、转码、队列、UI 面板等均未实现）。

【技术栈】
C++17 + Dear ImGui (docking 分支) + GLFW + OpenGL3
工具集 v145，Windows SDK 10.0.26100.0，字符集 MultiByte，编译选项 /utf-8。

【打开与编译】
双击 EasyAV.sln，选择 Debug|x64（推荐）或 Release|x64，直接生成即可。
Win32 / x64、Debug / Release 共 4 个配置均已配置好。
生成的 exe 位于 Debug\ 或 Release\ 目录，运行时不弹出控制台窗口
（工程中设置 SubSystem=Windows + 入口 mainCRTStartup）。

【目录与依赖】
本项目与 imgui-docking 是同级目录，源码通过相对路径直接复用，不重复拷贝：
  头文件包含：..\imgui-docking
              ..\imgui-docking\backends
              ..\imgui-docking\examples\libs\glfw\include
  库目录：    ..\imgui-docking\examples\libs\glfw\lib-vc2010-32  (Win32)
              ..\imgui-docking\examples\libs\glfw\lib-vc2010-64  (x64)
  链接库：    opengl32.lib; glfw3.lib
因此请勿单独移动 imgui-docking 目录，否则需要同步修改包含/库路径。

文件清单：
  EasyAV.sln                解决方案（单项目，4 个配置）
  EasyAV.vcxproj            工程文件
  EasyAV.vcxproj.filters    源文件筛选器（imgui / sources）
  EasyAV.vcxproj.user       调试工作目录设为项目根目录
  main.cpp                  仅框架：GLFW 窗口 -> ImGui 上下文/后端 -> 主循环 -> 清理
  build_win32.bat           VS 命令行 32 位构建脚本（已适配路径）
  Makefile                  MinGW/Linux/macOS 构建脚本（已适配路径，非 VS 必需）
  .gitignore                忽略生成物

【与参考项目的差异】
1. 项目名 / ProjectGuid / RootNamespace / 解决方案 GUID 全部改为 EasyAV（新项目独立标识）。
2. 依赖路径从参考项目的 ..\.. 与 ..\libs 重定向为 ..\imgui-docking\...，
   因为 EasyAV 位于 TestNewImGUI 下、与 imgui-docking 同级。
3. 显式设置 LanguageStandard=stdcpp17（参考项目未设置，默认沿用工具集默认值）。
4. 4 个配置统一为 SubSystem=Windows + EntryPointSymbol=mainCRTStartup，
   替代参考项目里 x64=Windows / Win32=Console 及源码中的 linker pragma，行为一致且更规范。
5. main.cpp 保留完整框架流程（含 Docking / Viewports / DPI 缩放），
   把示例的 Demo 窗口、Hello world 窗口等示例 UI 替换为一个占位窗口，
   仅用于验证框架可运行；窗口标题改为 EasyAV。
6. 未复制 imgui.ini（运行时自动生成）、Makefile.emscripten（Emscripten 专用）。

【后续扩展提示（本次未实现）】
- 业务代码请从 main.cpp 主循环中 "Application UI placeholder" 处开始扩展，
  或新建 src\ 下的 .cpp/.h 并加入 vcxproj 的 ClCompile / ClInclude 项。
- 中文界面需先通过 io.Fonts->AddFontFromFileTTF + GetGlyphRangesChineseFull()
  加载中文字体，否则 ImGui 默认字体无法显示中文（占位窗口特意只用英文）。
- 若改用自行编译的新版 GLFW，可移除 main.cpp 中的 legacy_stdio_definitions pragma
  及工程里的 IgnoreSpecificDefaultLibraries=msvcrt.lib。
