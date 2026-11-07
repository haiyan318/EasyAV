EasyAV 内置 FFmpeg 放置说明
========================================

EasyAV 本身不包含 FFmpeg 二进制。运行时会按以下顺序查找（找到后执行 -version 校验）：

1. 环境变量 EASYAV_FFMPEG_DIR（目录，或直接指向 ffmpeg.exe）
2. 系统 PATH（优先使用用户已安装的 FFmpeg）
3. 本目录：assets/ffmpeg/ffmpeg.exe 或 assets/ffmpeg/bin/ffmpeg.exe
4. exe 同目录及其 bin 子目录
5. 当前工作目录（从 IDE / 仓库根目录运行时）

需要两个可执行文件：
  ffmpeg.exe   处理/转码
  ffprobe.exe  读取时长、编码、分辨率

放置方式（任选其一）：
  assets/ffmpeg/ffmpeg.exe + assets/ffmpeg/ffprobe.exe
  assets/ffmpeg/bin/ffmpeg.exe + assets/ffmpeg/bin/ffprobe.exe

构建时如果本目录存在，会整体拷贝到输出目录（Debug/ 或 Release/）的 assets/ffmpeg。
若使用动态链接版 FFmpeg，请把依赖的 DLL 一并放在同目录。

注意：
- 本目录下的 *.exe / *.dll 默认不纳入 Git（见 .gitignore）；如需提交请使用 git add -f。
- 建议使用支持 Unicode 路径的 Windows 构建（官方 MSVC 或 BtbN 构建），
  以便正确处理中文与空格路径。
- 分发前请确认 FFmpeg 的许可证（GPL/LGPL）与裁剪范围。
