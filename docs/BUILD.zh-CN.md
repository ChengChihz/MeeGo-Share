# 构建 MeeShare

构建环境：Windows、Node.js，以及带 Harmattan ARM 目标的 Qt SDK。已使用 Qt 4.7.4、目标 `harmattan_10.2011.34-1_rt1.2` 和 SDK 随附的 ARM GCC 工具链构建。

仓库不分发 Nokia SDK、系统固件或设备凭据。请自行准备 SDK，并在 PowerShell 中执行：

```powershell
./build.ps1 -SdkRoot 'D:/QtSDK'
```

将 `D:/QtSDK` 替换成实际 SDK 根目录。脚本使用该目录下的 `bin/sh.exe`、Harmattan 目标、Qt 翻译工具与 ARM strip 工具；其他 SDK 布局需要调整对应路径。

脚本会依次生成翻译资源、编译 Mbed TLS、编译网络组件和界面程序，最后生成 `.deb` 与 SHA-256 校验文件。构建文件在 `build/`，安装包在 `packages/`。

## 目录

- `ui.cpp`、`Main.qml`、`SettingsRow.qml`：原生界面和入口
- `transfer.h`：文件选择、设置、传输控制与反馈
- `main.cpp`：网络组件，处理设备发现和 HTTPS 文件传输
- `localization.h`、`translations/`：五语言界面及运行时切换
- `libcontentaction0`：界面程序链接的设备原生文件打开库
- `third_party/qjson/`、`third_party/mbedtls-3.6.7/`：随附依赖源码和许可
- `package.cjs`：生成 Debian/Harmattan 安装包及重新链接材料

## 升级兼容

应用显示名为 MeeShare。为允许旧测试版升级，并保留现有设置和保存目录，包名 `localsend-meego`、安装目录 `/opt/localsend-meego` 和已有配置标识暂时保持兼容。这些内部标识不表示官方身份。

## QJson 重新链接

QJson 静态编入界面与网络组件。安装包随附 QJson 源码、应用源码、应用对象文件和 `libn9mbedtls.a`，位于 `/usr/share/doc/localsend-meego/`。`relink-kit.tar.gz` 中的 `RELINK.txt` 提供替换 QJson 后重新链接的命令。无需设备私钥即可重新构建包。

## 诊断入口

源码保留开发期的界面、语言、选择器和传输诊断入口。部分诊断需要额外的本地测试夹具，不是普通用户的启动参数。公开仓库不附带开发者的 SSH 连接脚本或设备配置。
