# Generate compile_commands.json for C++ IntelliSense (run from repo root)
# Usage: .\scripts\gen_compile_commands.ps1
# 在已加载 VS 环境的同一进程中执行 cmake，避免 Launch-VsDevShell 启动新壳导致后续命令无 cl/标准库

$root = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $root 'build-ninja'

# 查找 vcvarsall.bat（与 VS 版本无关的常见路径）
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

if (!(Test-Path $buildDir)) { New-Item -ItemType Directory -Path $buildDir | Out-Null }

$buildDirArg = $buildDir -replace '/', '\'
$rootArg = $root -replace '/', '\'
$cmd = "call `"$vcvars`" amd64 && cd /d `"$buildDirArg`" && cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug `"$rootArg`""
cmd /c $cmd
$code = $LASTEXITCODE
if ($code -eq 0) {
    $cc = Join-Path $buildDir 'compile_commands.json'
    Write-Host ('Done: ' + $cc) -ForegroundColor Green
    Write-Host 'If IntelliSense still shows errors, run: C/C++ Reset IntelliSense Database' -ForegroundColor Gray
}
exit $code
