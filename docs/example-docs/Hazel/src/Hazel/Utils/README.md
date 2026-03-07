# Utils 模块

跨平台工具接口。

## 功能概览

| 组件 | 功能 |
|------|------|
| **PlatformUtils** | 平台相关工具：如剪贴板读写、系统对话框等；Windows 实现为 WindowsPlatformUtils |

## 架构要点

- 通过抽象接口屏蔽平台差异，应用与编辑器统一调用 Utils 接口。
