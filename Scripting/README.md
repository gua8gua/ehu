# 仓库根目录 `Scripting/` 说明

本目录存放 **托管侧（C#）** 脚本工程，与引擎源码里的 **原生侧（C++）** 脚本模块是两套东西，不要混用路径。

| 位置 | 语言 | 作用 |
|------|------|------|
| **本目录** `Scripting/` | C# | `Ehu.ScriptCore`（引擎暴露给脚本的 API）与 `Game`（游戏/示例脚本），由 `dotnet` 编译成 DLL |
| `Ehu/src/Ehu/Scripting/` | C++ | Mono 嵌入、`ScriptEngine` / `ScriptRegistry` / `ScriptGlue`，由 CMake 编入 `EhuLib` |

更完整的运行时架构、调用链与排错见 [docs/technical/05-ScriptingModule.md](../docs/technical/05-ScriptingModule.md)。

---

## 子目录一览

```
Scripting/
├── README.md                 # 本文件
└── CSharp/
    ├── Ehu.Scripting.sln     # 解决方案（含下面两个项目）
    ├── Ehu.ScriptCore/       # 程序集名：Ehu-ScriptCore.dll；InternalCalls 与封装（Log、Transform、Input 等）
    └── Game/                 # 程序集名：Game.dll；示例脚本（如 PlaceholderScript）
```

- **Ehu.ScriptCore**：与 C++ 中 `ScriptGlue` 注册的 InternalCall **符号名一致**；改 C++ 绑定后需同步改这里的 `InternalCalls.cs` 等，并重新编译。
- **Game**：引用 ScriptCore，写实际游戏逻辑；也可在自有解决方案中另建程序集，只要引擎项目配置里指向对应 DLL 即可。

---

## 如何编译

在项目根目录（推荐，与仓库约定一致）：

```powershell
.\build-scripts\build_script_core.ps1
```

或在 `Scripting/CSharp` 下：

```powershell
dotnet build .\Ehu.Scripting.sln -c Release
```

**环境**：需安装 **.NET SDK**；目标框架为 **net48**，通常还需 **.NET Framework 4.8 Developer Pack**。

**典型输出路径**：

- `Scripting/CSharp/Ehu.ScriptCore/bin/Release/net48/Ehu-ScriptCore.dll`
- `Scripting/CSharp/Game/bin/Release/net48/Game.dll`

- **`bin/`**：最终可部署的程序集（及 pdb 等）。
- **`obj/`**：还原与编译中间文件（NuGet 生成项、缓存等），**不是手写源码**，可整体视为构建产物。

---

## 部署到 Ehu 项目

引擎通过项目配置加载两个程序集路径（见 `.ehuproject` 中的 `ScriptCoreAssembly` / `ScriptAppAssembly`）。新建项目的默认值一般为资产目录下的 `Scripts/Ehu-ScriptCore.dll` 与 `Scripts/Game.dll`（实现见 `Ehu/src/Ehu/Project/Project.cpp`）。

将上述 Release 输出的两个 DLL 复制到**你的项目**资产目录的 `Scripts/`（或配置中指定的相对路径），再打开项目运行即可。

---

## 与版本库、忽略规则

仓库根 [.gitignore](../.gitignore) 已忽略常见二进制产物（如 `*.dll`、`*.pdb` 等），本地编译生成的 `bin/`、`obj/` 下的文件通常**不会**被提交。若你希望仅忽略 C# 工程目录下的构建目录，也可在 `.gitignore` 中增加例如：

```gitignore
Scripting/CSharp/**/bin/
Scripting/CSharp/**/obj/
```

（与全局规则二选一或并存均可，按团队习惯即可。）
