# Emoji Dungeon V3

> 基于 Qt 6.10 + C++17 的 Emoji 风格俯视角射击生存游戏  
> 南开大学 2026 年 C++ 大作业项目

## 简介

Emoji Dungeon 是一款俯视角射击生存游戏，使用 Microsoft Fluent Emoji 3D 风格图标作为游戏素材。玩家选择职业后，在 10 波怪物攻势中生存下来，通过升级特性和获取特殊子弹来强化自身。

## 游戏特性

- **3 大职业**: 战士 (Warrior) / 游侠 (Ranger) / 法师 (Caster)
- **10 种敌人**: 含 4 个 Boss，分属 3 个系列 (Bizarre / Undead / Alien)
- **6 种 AI 行为**: 追逐 / 射击移动 / 蓄力冲锋 / Boss 多阶段等
- **10 种特性**: 快手 / 厚皮 / 幸运射击 / 吸血光环 / 暴击专精等
- **5 种武器强化**: 弹道增幅 / 射程提升 / 穿透 / 子弹增幅 / 连击
- **12 种特殊子弹**: 飞刀 / 太阳法球 / 哈基米 / 雷霆之矛 / 回力镖等
- **15 级等级系统**: 按波次递增的经验倍率
- **动态难度**: HP ×1.05^wave, DMG ×1.03^wave
- **冲刺系统**: 空格键闪避，3 秒冷却
- **47 张 Fluent Emoji 3D 素材**

## 环境要求

| 依赖 | 版本 |
| --- | --- |
| Qt | 6.10+ |
| C++ 标准 | C++17 |
| CMake | 3.16+ |
| 编译器 | MinGW / MSVC / Clang |
| 构建工具 | Ninja / Make |

## 快速开始

```bash
# 克隆仓库
git clone https://github.com/WwPei/my-nankai26C-project.git
cd my-nankai26C-project

# 配置
cmake -B build -G Ninja

# 构建
cmake --build build --target emoji_dungeon

# 运行
./build/emoji_dungeon.exe
```

## 操作说明

| 按键 | 功能 |
| --- | --- |
| `W` `A` `S` `D` | 移动 |
| `鼠标左键` | 攻击 (自动瞄准最近敌人) |
| `空格键` | 冲刺 (3 秒冷却) |
| `ESC` | 暂停 |

## 项目结构

```
emoji_dungeon_v3/
├── CMakeLists.txt              # 构建配置 (147 行, Strategy A)
├── resources/
│   ├── resources.qrc           # 47 个 PNG 资源
│   ├── images/                 # 32 张游戏贴图
│   └── icons/                  # 15 张 UI 图标
├── src/
│   ├── main.cpp
│   ├── core/                   # 枚举、结构体、配置表、工具函数
│   ├── models/                 # 数据模型 (抽象接口 + 具体实现)
│   ├── views/                  # 视图层 (子弹/敌人渲染、战斗场景、UI 组件)
│   ├── pages/                  # 4 个游戏页面 (窗口/选职业/战斗/升级)
│   ├── systems/                # 5 个子系统 (工厂/波次/升级/战斗/导演)
│   └── bullets/                # 12 种特殊子弹系统
├── tests/
│   └── stage1_system_test.cpp  # 单元测试
└── docs/
    ├── CHANGELOG_V2.md         # V1→V2 变更记录
    ├── CHANGELOG_V3.md         # V1→V3 全记录
    ├── CODE_ANALYSIS.md        # 代码架构分析
    └── CODE_TYPE_ANALYSIS.md   # 类型系统分析
```

## 架构亮点

- **MV 分离**: 数据模型 (Model) 与视图渲染 (View) 完全分层
- **抽象基类体系**: 8 个抽象接口 + 具体实现，支持扩展
- **子系统拆分**: 战斗协调器 / 升级决策器 / 敌人导演独立模块
- **Strategy A CMake**: `target_include_directories` 零修改兼容

## 资源声明

本项目使用了 [Microsoft Fluent Emoji](https://github.com/microsoft/fluentui-emoji) 的 3D 风格图标资源，遵循 MIT 许可证。

**用途限制**: 本项目为学术大作业，不会用于任何商业用途。

## 许可证

MIT License