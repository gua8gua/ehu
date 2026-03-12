# scripts — 项目编译脚本

本目录为项目的**编译脚本**，**默认使用此脚本进行配置与编译**。

## 脚本说明

| 脚本 | 作用 |
|------|------|
| **gen_compile_commands.ps1** | 配置：在 VS 环境下运行 CMake（Ninja 生成器），生成 `build-ninja/` 与 `compile_commands.json`。首次或 CMake 变更后执行。 |
| **build_ninja.ps1** | 编译：在 VS 环境下运行 Ninja，编译 EhuLib + SandBox。 |

## 使用方式（在项目根目录执行）

```powershell
# 1. 配置（首次或修改 CMake 后）
.\scripts\gen_compile_commands.ps1

# 2. 编译
.\scripts\build_ninja.ps1
```

输出：`build-ninja/bin/Debug/SandBox.exe`，库在 `build-ninja/bin-int/Debug/`。

更详细的构建说明见 [docs/README.md](../docs/README.md)。
