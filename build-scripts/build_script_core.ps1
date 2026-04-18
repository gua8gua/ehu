# 编译托管脚本程序集（Ehu-ScriptCore.dll、Game.dll），输出到各项目的 bin 目录。
# 需要已安装 .NET Framework 4.8 Developer Pack 与 dotnet SDK（用于 SDK 风格 net48 工程）。
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$sln = Join-Path $root "Scripting\CSharp\Ehu.Scripting.sln"
if (-not (Test-Path $sln)) {
    Write-Error "Solution not found: $sln"
}
dotnet build $sln -c Release
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "Output (copy into your project's Scripts/ folder as needed):"
Write-Host "  Ehu-ScriptCore -> $root\Scripting\CSharp\Ehu.ScriptCore\bin\Release\net48\Ehu-ScriptCore.dll"
Write-Host "  Game           -> $root\Scripting\CSharp\Game\bin\Release\net48\Game.dll"
