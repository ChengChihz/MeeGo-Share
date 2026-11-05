# Building MeeShare

Requirements: Windows, Node.js and a Qt SDK with the Harmattan ARM target. The project has been built with Qt 4.7.4, target `harmattan_10.2011.34-1_rt1.2` and the SDK's ARM GCC toolchain.

The repository does not distribute the Nokia SDK, system firmware or device credentials. Install the SDK separately, then run in PowerShell:

```powershell
./build.ps1 -SdkRoot 'D:/QtSDK'
```

Replace `D:/QtSDK` with the actual SDK root. The script uses its `bin/sh.exe`, Harmattan target, Qt translation tools and ARM strip tool. Other SDK layouts require the relevant paths to be adjusted.

The script generates translation catalogs, builds Mbed TLS, builds the network helper and interface, then produces a `.deb` and SHA-256 checksum file. Build artifacts go in `build/`; packages go in `packages/`.

## Layout

- `ui.cpp`, `Main.qml`, `SettingsRow.qml`: native interface and entry point
- `transfer.h`: file selection, settings, transfer control and feedback
- `main.cpp`: network helper for discovery and HTTPS file transfers
- `localization.h`, `translations/`: five-language interface and runtime switching
- `third_party/qjson/`, `third_party/mbedtls-3.6.7/`: bundled dependency sources and licenses
- `package.cjs`: Debian/Harmattan package generation and relinking materials

## Upgrade compatibility

The display name is MeeShare. To allow upgrades from the earlier test app while preserving settings and the receive folder, the package ID `localsend-meego`, installation path `/opt/localsend-meego` and existing configuration identifiers are retained for compatibility. These internal identifiers do not imply official status.

## Relinking QJson

QJson is statically linked into both executables. The package includes QJson sources, application sources, application object files and `libn9mbedtls.a` under `/usr/share/doc/localsend-meego/`. The `RELINK.txt` in `relink-kit.tar.gz` provides commands to relink after replacing QJson. No device private key is needed to rebuild a package.

## Diagnostic entry points

The source retains development diagnostics for the interface, translations, pickers and transfers. Some require separate local test fixtures and are not normal launch options. The public repository does not include the developer's SSH connection scripts or device configuration.
