<#
.SYNOPSIS
    Builds Jolt Physics (x64) and installs Jolt.lib where the project expects it.

.DESCRIPTION
    Produces:
        dependencies\JoltPhysics\lib\x64\Release\Jolt.lib
        dependencies\JoltPhysics\lib\x64\Debug\Jolt.lib

    These .lib files are intentionally NOT committed to git (see .gitignore) -
    they are ~92 MB combined. Run this once after cloning, or whenever you
    update the Jolt source.

    Players do NOT need this. The Release exe links Jolt statically, so a
    distributed build already contains everything it needs.

.PARAMETER Config
    Which configuration(s) to build. Default builds both, because the project
    links a different Jolt.lib for Debug and Release (CRT mismatch otherwise).

.EXAMPLE
    .\build_jolt.ps1

.EXAMPLE
    .\build_jolt.ps1 -Config Release
#>
param(
    [ValidateSet('Release', 'Debug', 'Both')]
    [string]$Config = 'Both'
)

$ErrorActionPreference = 'Stop'

function Find-CMake {
    # 1. Whatever is on PATH
    $cmd = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    # 2. The CMake bundled with Visual Studio (found via vswhere)
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -property installationPath 2>$null
        if ($vsPath) {
            $p = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
            if (Test-Path $p) { return $p }
        }
    }

    # 3. Known VS 2022 install locations
    foreach ($edition in @('Community', 'Professional', 'Enterprise', 'BuildTools')) {
        $p = "C:\Program Files\Microsoft Visual Studio\2022\$edition\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        if (Test-Path $p) { return $p }
    }

    throw "CMake not found. Install CMake, or the 'Desktop development with C++' workload in Visual Studio 2022."
}

$root   = $PSScriptRoot
$src    = Join-Path $root 'dependencies\JoltPhysics\Build'
$build  = Join-Path $env:TEMP 'rpg-jolt-build'
$libOut = Join-Path $root 'dependencies\JoltPhysics\lib\x64'

if (-not (Test-Path $src)) {
    throw "Jolt source not found at: $src"
}

$cmake = Find-CMake
Write-Host "CMake: $cmake"

Write-Host "Configuring Jolt..."
# USE_STATIC_MSVC_RUNTIME_LIBRARY defaults to ON, which builds Jolt against the
# static CRT (/MT). RPG-I and SFML both use the dynamic CRT (/MD and /MDd), and
# mixing the two fails at link time with LNK2038. Force the dynamic CRT so the
# Jolt libs match the application.
& $cmake -S $src -B $build -G "Visual Studio 17 2022" -A x64 `
    -DTARGET_UNIT_TESTS=OFF -DTARGET_HELLO_WORLD=OFF -DTARGET_PERFORMANCE_TEST=OFF `
    -DTARGET_SAMPLES=OFF -DTARGET_VIEWER=OFF `
    -DENABLE_ALL_WARNINGS=OFF -DINTERPROCEDURAL_OPTIMIZATION=OFF `
    -DUSE_STATIC_MSVC_RUNTIME_LIBRARY=OFF
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed." }

$configs = if ($Config -eq 'Both') { @('Release', 'Debug') } else { @($Config) }

foreach ($cfg in $configs) {
    Write-Host "Building Jolt ($cfg)..."
    & $cmake --build $build --config $cfg --parallel
    if ($LASTEXITCODE -ne 0) { throw "Jolt $cfg build failed." }

    $built = Join-Path $build "$cfg\Jolt.lib"
    if (-not (Test-Path $built)) {
        throw "Build succeeded but $built was not produced."
    }

    $dst = Join-Path $libOut $cfg
    New-Item -ItemType Directory -Force -Path $dst | Out-Null
    Copy-Item $built (Join-Path $dst 'Jolt.lib') -Force
    Write-Host "  -> $dst\Jolt.lib"
}

Write-Host "Done. You can now build RPG-I.vcxproj."
