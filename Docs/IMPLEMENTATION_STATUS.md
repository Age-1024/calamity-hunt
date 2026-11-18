# 灰盒原型实现状态

## 新增：稀有觉醒源与觉醒冲刺

- 新增可放置的单次使用觉醒源 `ACalamityAwakeningSource`，可配置人、鬼、僵尸、怪物四类印记。
- 觉醒组件 `UCalamityAwakeningComponent` 可添加到玩家角色或蓝图子类，负责服务器权威授予印记、能量回复、冷却及冲刺技能。
- 客户端仅通过所属角色组件的 Server RPC 请求使用觉醒源或释放技能；源在服务器检查距离、是否已消耗和玩家是否已有印记。
- 技能原型为直线冲刺，不造成伤害；能量消耗、回复、冲刺速度、垂直速度和冷却均可编辑。
- 印记、能量、冷却和觉醒源消耗状态复制给客户端。

## 使用步骤

1. 将 `UCalamityAwakeningComponent` 添加到玩家角色 Blueprint。
2. 在关卡放置 `ACalamityAwakeningSource` 或 Blueprint 子类，设置 `MarkToGrant` 和 `UseRadius`。
3. 输入映射调用组件 `RequestClaimSource(SourceActor)`；释放技能调用 `RequestCastAbility()`。
4. 运行服务器与客户端，验证只有一个玩家能消费单次觉醒源。

## 静态检查/限制

代码采用独立 Actor 和组件实现，不依赖联盟接口。未在 Unreal Engine 5.5 环境执行 UHT 或 C++ 编译；本地需要重点验证组件 Server RPC 所属 Actor 的网络拥有权、源 Actor 参数解析、复制字段和多人竞态。觉醒事件目前是单步互动，没有试炼、打断、反噬、角色差异技能或 UI。
