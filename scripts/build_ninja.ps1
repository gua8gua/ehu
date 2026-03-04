# Build with Ninja (requires build-ninja already configured; run gen_compile_commands.ps1 first)
# Usage: .\scripts\build_ninja.ps1
# 在已加载 VS 环境的同一进程中执行 ninja，避免 Launch-VsDevShell 启动新壳导致 cl/标准库不可用

$root = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $root 'build-ninja'
if (!(Test-Path $buildDir)) {
    Write-Error 'build-ninja not found. Run .\scripts\gen_compile_commands.ps1 first.'
    exit 1
}

$vcvarsCandidates = @(
    'D:\ProgramFiles\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat',
    (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat'),
    (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat')
)
$vcvars = $null
foreach ($p in $vcvarsCandidates) {
    if ($p -and (Test-Path $p)) { $vcvars = $p; break }
}
if (!$vcvars) {
    Write-Error 'vcvarsall.bat not found. Install VS 2022 with C++ workload or set path in script.'
    exit 1
}

$buildDirArg = $buildDir -replace '/', '\'
$cmd = "call `"$vcvars`" amd64 && cd /d `"$buildDirArg`" && ninja"
cmd /c $cmd
$code = $LASTEXITCODE
if ($code -eq 0) {
    $exe = Join-Path $buildDir 'bin\Debug\SandBox.exe'
    if (Test-Path $exe) { Write-Host ('Exe: ' + $exe) -ForegroundColor Green }
}
exit $code
