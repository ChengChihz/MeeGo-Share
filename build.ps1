param(
    [Parameter(Mandatory=$true)][string]$SdkRoot,
    [string]$Target = 'harmattan_10.2011.34-1_rt1.2',
    [int]$Jobs = 4
)
$ErrorActionPreference = 'Stop'
$sdk = (Resolve-Path -LiteralPath $SdkRoot).Path
$project = $PSScriptRoot
$env:PATH = "$sdk/bin;" + $env:PATH
$sh = Join-Path $sdk 'bin/sh.exe'
if (!(Test-Path -LiteralPath $sh)) { throw 'Qt SDK sh.exe not found' }
function Build-Target([string]$Name, [string]$ProjectFile) {
    $dir = Join-Path $project "build/$Name"
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
    Push-Location $dir
    try {
        & $sh --login /bin/mad -t $Target qmake "../../$ProjectFile"
        if ($LASTEXITCODE -ne 0) { throw "qmake failed: $Name" }
        & $sh --login /bin/mad -t $Target make -s "-j$Jobs"
        if ($LASTEXITCODE -ne 0) { throw "make failed: $Name" }
    } finally { Pop-Location }
}
Push-Location $project
try {
    node ./build-translations.cjs
    if ($LASTEXITCODE -ne 0) { throw 'Translation generation failed' }
    $lrelease = Join-Path $sdk 'tools/qt-tools_win_2011-10-07_i686_qt4.7.4_mt0.22.12/bin/lrelease.exe'
    foreach ($lang in @('en','zh_TW','ja','ko')) {
        & $lrelease "translations/app_$lang.ts" -qm "translations/app_$lang.qm"
        if ($LASTEXITCODE -ne 0) { throw "lrelease failed: $lang" }
    }
    Build-Target 'mbedtls' 'mbedtls.pro'
    Build-Target 'core' 'network.pro'
    Build-Target 'ui' 'ui.pro'
    $strip = Join-Path $sdk 'toolchains/arm-2009q3-67-arm-none-linux-gnueabi-i586-mingw32msvc/arm-2009q3-67/bin/arm-none-linux-gnueabi-strip.exe'
    & $strip 'build/ui/n9-lanshare' 'build/core/probe'
    if ($LASTEXITCODE -ne 0) { throw 'ARM strip failed' }
    node ./package.cjs
    if ($LASTEXITCODE -ne 0) { throw 'Package generation failed' }
} finally { Pop-Location }
