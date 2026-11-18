# 万煞猎场（暂定名）

开源 UE5 PvPvE 超自然生存竞技灰盒原型。

## 当前目标

先实现一个可运行、可测试的 4 人灰盒原型：

- Server-authoritative 玩家与枪战
- 普通淘汰与最后存活胜利
- 皇祭坛可引导
- 皇印拾取、掉落、持有计时与胜负
- 临时结盟的创建与解除
- 结构化对局阶段配置
- 基础日志与测试清单

## 不承诺首版包含

- 完整美术与地图
- 异灵驯服系统
- 完整觉醒技能
- 商业匹配或账号服务
- 防作弊完整方案
- 完整移动端或主机支持

## 建议引擎

UE 5.5 或兼容版本。核心逻辑优先使用 C++，表现与配置使用 Blueprint/Data Assets。

## 源码方向

```text
CalamityCore       公共状态、配置、事件与日志
CalamityCombat     服务器权威射击、命中与死亡
CalamityCrown      皇祭坛、皇印与胜负
CalamityAlliance   临时结盟与解除
CalamityFlow       对局阶段与数据驱动流程
CalamityTests      核心系统与联机测试
```

详细内容请查看：

- `Docs/GAME_DESIGN.md`
- `Docs/TECH_DESIGN.md`
- `Docs/ACCEPTANCE_CHECKLIST.md`
- `Docs/LEGAL.md`

## 运行要求

首版仍在设计和实现中。提交完整 `.uproject` 前，需要在 UE5 环境中生成项目文件并验证：

- 4 名测试玩家能连接 Dedicated Server
- 基础射击能触发服务器权威伤害与淘汰
- 皇印可拾取、掉落、复位与计时
- 最后存活玩家立即胜利
- 对局超时或异常退出有明确处理

## 开源边界

MIT 许可证适用于仓库中由贡献者创建且有权发布的代码。仓库不会随意包含：

- Unreal Engine 源码或专有引擎组件
- 未获授权的模型、贴图、音效、动画或地图
- 受商标、版权或第三方许可限制的素材

另见 `Docs/LEGAL.md`。
