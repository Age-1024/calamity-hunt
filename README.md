# 万煞猎场（暂定名）

开源 UE5 PvPvE 超自然生存竞技原型，目标是逐步验证“枪战、淘汰、临时合作、争夺皇印”的核心循环。

> 当前仓库包含 UE 5.5 C++ 游戏模块骨架及第一批服务器权威玩法代码。它是早期原型源码，不是已经完整制作、编译验证或可直接上线的成品游戏。

## 当前源码

- UE 5.5 项目文件和 Game / Editor Target
- 服务端权威基础 hitscan 射击与生命值淘汰
- 对局阶段：搜刮、皇印争夺、血月、结束
- 皇印认领、掉落、持有计时与胜利判定
- 最后一名存活玩家获胜
- 超时结算的灰盒占位规则

## 文档

- `Docs/GAME_DESIGN.md` — 核心玩法
- `Docs/TECH_DESIGN.md` — 网络与模块设计
- `Docs/ACCEPTANCE_CHECKLIST.md` — 灰盒验收
- `Docs/LEGAL.md` — 开源和素材边界

## 在 UE 5.5 中构建

1. 安装 Unreal Engine 5.5 和对应 C++ 编译工具。
2. 克隆仓库，右键 `CalamityHunt.uproject` 生成 IDE 项目文件。
3. 用 IDE 编译 `CalamityHuntEditor` Development 配置。
4. 在编辑器中创建灰盒地图，放置 PlayerStart，并将地图设为启动地图。
5. 创建 `BP_CalamityCharacter` 子类，添加网格、相机和输入；将 Fire 输入绑定到 `RequestFire`，交互输入调用 GameMode 的 `TryClaimCrown` / `DropCrown`。
6. 设置合适的碰撞、武器射线目标和测试地图。服务器可使用 `-server -log` 启动，客户端通过 `open 127.0.0.1` 连接本机测试服。

目前没有提交二进制地图、模型、动画、音效或输入资产。角色 C++ 类提供玩法逻辑，实际可玩角色需在编辑器中配置相机、网格与输入。所有交互入口在正式多人版本中都应补充服务器端距离、视线、状态与权限校验。

## 当前限制

尚未实现或验证：Dedicated Server 多客户端完整测试、玩家移动输入资产、皇祭坛范围与引导进度、结盟、异灵 AI/驯服、觉醒、贡献分、反作弊和商业服务。对局配置目前由 C++ 默认值提供，后续再迁移到 Data Asset。

## 开源边界

仓库中的原创代码和文档采用 MIT License。许可证不覆盖 Unreal Engine、第三方插件或外部素材。不要提交未获授权的模型、音效、动画、地图和其他资产。
