# 觉醒原型接入说明

`UCalamityAwakeningComponent` 是一个可复制的 ActorComponent。请将它添加到角色 Blueprint（或在 C++ Character 构造函数中创建子对象）。单独把组件文件放进仓库并不会自动附加到角色。

把 `ACalamityAwakeningSource` 放到关卡，分别配置其 `MarkToGrant` 和交互半径。玩家输入应在本地调用组件的 `RequestClaimSource` 或 `RequestCastAbility`；服务器 RPC 会完成最终验证。当前输入资产、HUD、交互提示和练习地图未包含在仓库中。

建议先在编辑器 PIE 中开启一个 Dedicated Server 和两个客户端测试竞争领取、网络拥有权、冲刺复制及能量冷却，再制作正式技能表现。
