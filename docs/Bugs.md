# EasyAV 缺陷与问题记录

**是否已解决：否**

> 说明：BUG-001 的源码修复已完成，但尚未完成重新构建、导入回归与重新打包，
> 因此整体状态仍标记为“否”。修复验证完成后请把本行改为“是”，并勾选下方验证清单。

- 记录日期：2026-11-07
- 影响版本：V0.1（Debug|x64）
- 测试环境：Windows 11、Visual Studio 2026 18.9、v145（14.51）工具集、
  Dear ImGui docking 分支 1.92
- 关联代码：`src/ui/panels/QueuePanel.cpp`、`../imgui-docking/imgui.cpp:11530`

---

## BUG-001 导入文件后弹出 Debug Error：abort() has been called

**是否已解决：否（源码已修复，待重新构建与回归验证）**

### 现象

在 VS 中以 Debug|x64 运行 EasyAV，点击“添加文件”或拖入文件后，队列刚刚要显示第一条记录时，
弹出：

```
Microsoft Visual C++ Runtime Library
Debug Error!
Program: ...\Debug\EasyAV.exe
abort() has been called
```

此时队列仍显示 0，界面被错误框阻塞，程序随后退出（VS 调试输出退出码 0xFFFFFFFF）。

### 复现条件

1. 使用 Debug|x64 构建；
2. 队列为空时程序正常，不崩溃；
3. 导入任意受支持文件（1 个即可），队列需要绘制文件行时立即触发；
4. 与文件格式、FFmpeg 是否可用、显卡加速无关。

补充：通过启动时直接调用添加文件（绕过文件对话框和 GLFW 拖拽）同样可以复现，
说明不是文件对话框或 WM_DROPFILES 的问题。

### 根因

ImGui 1.92（2025/06/25 之后）新增了一条布局检查
`ErrorCheckUsingSetCursorPosToExtendParentBoundaries()`（`imgui.cpp` 约 11530 行）：

> 如果窗口/子窗口的最后一次操作是 `SetCursorPos()/SetCursorScreenPos()`，
> 并且该位置会扩展父级边界，但没有再提交任何 item，则触发断言。

`QueuePanel.cpp` 的队列行循环中，每一行结尾都执行：

```cpp
ImGui::SetCursorScreenPos(ImVec2(rowOrigin.x, rowOrigin.y + rowHeight + rowGap));
ImGui::PopID();
```

最后一行也执行了该调用，随后循环结束并直接 `ImGui::EndChild()`，没有提交 item。
空队列不会进入行循环，所以只有“至少 1 个文件”时才崩溃。

ASan 构建捕获到的真实断言原文：

```
Assertion failed: (0) && "Code uses SetCursorPos()/SetCursorScreenPos() to extend
window/parent boundaries.
Please submit an item e.g. Dummy() afterwards in order to grow window/parent boundaries.",
file ../imgui-docking/imgui.cpp, line 11530
```

### 修复

已修改 `src/ui/panels/QueuePanel.cpp`：只在确实还有下一行时推进游标。

```cpp
// ImGui 1.92 起，SetCursorScreenPos 后必须提交 item；
// 最后一行不再手动推进，避免 EndChild 时触发边界断言。
if (i + 1 < static_cast<int>(items.size()))
    ImGui::SetCursorScreenPos(ImVec2(rowOrigin.x, rowOrigin.y + rowHeight + rowGap));
ImGui::PopID();
```

也可采用 ImGui 官方建议的等价写法：在 `SetCursorScreenPos` 之后补一个
`ImGui::Dummy(ImVec2(0.0f, 0.0f));`。本项目采用“最后一行不推进”的方式，
避免额外影响行间距。

### 已排除的方向

- 不是 FFmpeg / ffprobe / 转码引擎问题：无 UI 的 Debug 测试程序可正常完成
  文件添加、探测、MP3 转码。
- 不是内存越界：ASan 未报告堆/栈错误，唯一报告就是上述 ImGui 断言。
- 不是文件对话框或拖拽回调：通过环境变量在启动时直接添加文件同样复现。

### 验证清单

- [x] 定位根因
- [x] 修改源码
- [ ] Debug|x64 重新构建
- [ ] 导入 1 个文件，确认不再弹出错误框、队列显示该文件
- [ ] 探测到时长后开始压缩 / 提取 MP3，确认任务可执行
- [ ] Release|x64 重新构建并做导入冒烟
- [ ] 重新生成 `dist/EasyAV_V0.1_win64` 与 zip

---

## BUG-002 当前 Release 产物与 dist 包过期

**是否已解决：否**

- 当前工作区中 `Release/EasyAV.exe` 不存在；
- `dist/EasyAV_V0.1_win64/` 内仍是修复前的 Release 程序；
- 在 BUG-001 修复并完成 Release 构建前，不应把现有 dist 包当作可用版本分发。

处理方式：修复验证通过后重新构建 Release|x64，重新拷贝到 dist 并重新生成 zip。

---

## BUG-003 诊断残留文件

**是否已解决：否**

诊断期间生成了以下临时内容，确认修复后应删除：

- `DebugAsan/`：AddressSanitizer 诊断构建（含 `asan.err` 断言证据）
- 该目录不属于正式工程，也不应提交到 Git

建议在完成回归后执行清理，保留 `docs/Bugs.md` 作为记录即可。

---

## BUG-004 顶栏仍显示“界面预览”

**是否已解决：否（低优先级）**

`src/ui/panels/TopBar.cpp:40` 的副标题仍为“极简音视频工具 · 界面预览”，
与实际 V0.1 功能状态不一致。建议改为“极简音视频工具 · v0.1”。

---

## 其他结论与开发建议

1. **ImGui 1.92 规则已全量排查**：其余 `SetCursorPos/SetCursorScreenPos` 调用
   （App / ActionBar / FilterPanel / RecordPanel / TopBar / SettingsDialog）后面
   都跟有 item，未发现同类违规；后续新增代码须遵守同一规则。
2. **不要用跨进程手工 WM_DROPFILES 做自动化测试**：诊断中曾用自建 HDROP 注入，
   该方式不属于真实拖拽，可能引入额外问题；应使用真实界面操作或应用内测试入口。
3. **建议补一个导入冒烟测试**：当前单元测试不覆盖 UI，建议增加
   “Debug 启动 → 添加文件 → 断言队列出现 1 行 → 无断言/崩溃”的最小检查，
   避免同类问题再次漏过。
4. 该断言主要影响 Debug 构建；Release 中 `IM_ASSERT` 可能被编译掉，
   但代码仍违反 ImGui 布局规则，必须在源码层修复，不能只靠 Release 规避。
