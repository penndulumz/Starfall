param(
    [switch]$Test,
    [switch]$Package,
    [ValidateSet('Auto', 'GCC', 'MSVC')]
    [string]$Toolchain = 'Auto',
    [string]$BuildDirectory = '',
    [string]$StageDirectory = 'dist',
    [string]$PackageOutputDirectory = 'releases'
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot

function Find-Executable([string]$name) {
    $command = Get-Command $name -ErrorAction SilentlyContinue
    if ($command) { return $command.Source }
    return $null
}

function Invoke-CMake([string[]]$CMakeArguments) {
    & $script:cmakePath @CMakeArguments
    if ($LASTEXITCODE -ne 0) {
        throw "CMake failed (exit $LASTEXITCODE). See the message above; the build was not reported ready."
    }
}

Push-Location -LiteralPath $repoRoot
try {
    $vswherePath = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    $cmakePath = Find-Executable 'cmake.exe'
    $ninjaPath = Find-Executable 'ninja.exe'
    if (Test-Path -LiteralPath $vswherePath) {
        if (-not $cmakePath) {
            $cmakePath = & $vswherePath -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' | Select-Object -First 1
        }
        if (-not $ninjaPath) {
            $ninjaPath = & $vswherePath -latest -products '*' -find 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja/ninja.exe' | Select-Object -First 1
        }
    }
    if (-not $cmakePath) {
        throw 'CMake is required to rebuild. Install CMake or Visual Studio with Desktop development with C++ and its CMake tools. See docs/DEVELOPER.md. Playing the supplied game does not need build tools.'
    }

    $gccPath = Find-Executable 'g++.exe'
    if (-not $gccPath) {
        foreach ($candidate in @("$env:USERPROFILE/scoop/apps/gcc/current/bin/g++.exe", 'C:/msys64/ucrt64/bin/g++.exe', 'C:/msys64/mingw64/bin/g++.exe')) {
            if (Test-Path -LiteralPath $candidate) { $gccPath = $candidate; break }
        }
    }
    $selectedToolchain = $Toolchain
    if ($selectedToolchain -eq 'Auto') {
        $selectedToolchain = if ($gccPath) { 'GCC' } else { 'MSVC' }
    }
    if (-not $BuildDirectory) { $BuildDirectory = 'build/' + $selectedToolchain.ToLowerInvariant() }

    $configureArguments = @('-S', $repoRoot, '-B', $BuildDirectory, '-DBUILD_TESTING=ON')
    if ($Package) {
        $configureArguments += @("-DSTARFALL_PACKAGE_STAGE=$([IO.Path]::GetFullPath($StageDirectory))", "-DSTARFALL_PACKAGE_OUTPUT=$([IO.Path]::GetFullPath($PackageOutputDirectory))")
    }
    if ($selectedToolchain -eq 'GCC') {
        if (-not $gccPath) { throw 'GCC was selected but g++.exe was not found. Select -Toolchain MSVC or install a Windows x64 GCC toolchain.' }
        if ($ninjaPath) {
            $configureArguments += @('-G', 'Ninja', "-DCMAKE_MAKE_PROGRAM=$ninjaPath")
        } else {
            $makePath = Find-Executable 'mingw32-make.exe'
            if (-not $makePath) { throw 'The GCC build needs Ninja or mingw32-make. Install one, or use Visual Studio CMake tools. See docs/DEVELOPER.md.' }
            $configureArguments += @('-G', 'MinGW Makefiles', "-DCMAKE_MAKE_PROGRAM=$makePath")
        }
        $configureArguments += @("-DCMAKE_CXX_COMPILER=$gccPath", '-DCMAKE_BUILD_TYPE=Release')
    } else {
        # Let CMake select the installed Visual Studio version; request x64 explicitly.
        $configureArguments += @('-A', 'x64')
    }

    Write-Host "Building Starfall with $selectedToolchain through CMake..."
    Invoke-CMake $configureArguments
    Invoke-CMake @('--build', $BuildDirectory, '--config', 'Release', '--parallel')
    if ($Package) {
        # The CMake target runs both suites, installs into a fresh stage, writes
        # exact file hashes and archives the same declared installation rules.
        Invoke-CMake @('--build', $BuildDirectory, '--config', 'Release', '--target', 'starfall_package')
        Write-Host "Portable ZIP and checksum ready in $PackageOutputDirectory." -ForegroundColor Cyan
    } else {
        if ($Test) {
            $ctestPath = Join-Path (Split-Path -Parent $cmakePath) 'ctest.exe'
            & $ctestPath --test-dir $BuildDirectory -C Release --output-on-failure
            if ($LASTEXITCODE -ne 0) { throw 'Automated checks failed. No new game was staged.' }
        }
        # Installation stages only CMake-declared runtime files, never player save data.
        Invoke-CMake @('--install', $BuildDirectory, '--config', 'Release', '--prefix', [IO.Path]::GetFullPath($StageDirectory))
        Write-Host "Ready! Game staged in $StageDirectory. Open PLAY.bat inside that folder to play." -ForegroundColor Cyan
    }
} catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
} finally {
    Pop-Location
}
