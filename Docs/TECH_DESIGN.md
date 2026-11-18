# 技术设计摘要

## 引擎与架构

- Unreal Engine 5
- Dedicated Server 优先
- C++ 核心状态与服务器判定
- Blueprint 负责表现和快速迭代
- Gameplay Ability System 用于后续技能系统
- Gameplay Tags 用于状态、派系和能力分类
- Data Asset / Data Table 保存模式配置

## 模块

```text
CalamityCore       模式配置、公共状态、事件与日志
CalamityCombat     意图、射击、命中、伤害、死亡
CalamityCrown      皇祭坛、皇印、持印计时、胜负
CalamityAlliance   结盟关系、解除与友军伤害规则
CalamityFlow       阶段切换、事件触发与超时结算
CalamityTests      模拟联机与灰盒验收测试
```

## 网络权威规则

客户端只发送意图：

- 开火
- 换弹
- 拾取
- 引导祭坛
- 拾取或继承皇印
- 创建或解除结盟

服务端负责验证：

- 玩家存活状态
- 距离与视线
- 冷却与弹药
- 命中与伤害来源
- 皇印归属与仪式进度
- 联盟关系
- 胜利条件

## 对局流程配置示例

```yaml
mode:
  id: calamity_graybox
  max_players: 4
  duration_seconds: 900

phases:
  - id: scavenging
    start_seconds: 0
  - id: crown_contest
    start_seconds: 480
  - id: blood_moon
    start_seconds: 660

victory:
  crown_hold_seconds: 90
  last_survivor_wins: true
  timeout: highest_contribution
```

配置解析器首版可先使用 UE Data Asset。自然语言配置生成必须经过人工审核和 Schema 校验，远期再考虑。

## 安全与异常

- 保存关键事件日志：开火、死亡、皇印变化、胜负和断线
- 对重复拾取和继承请求去重
- 断线宽限与重连规则需由配置决定
- 皇印掉落到不可达区域时重置到祭坛
- 客户端预测不得决定最终伤害或胜负
