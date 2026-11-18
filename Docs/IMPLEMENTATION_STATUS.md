# 灰盒原型实现状态

## 本次新增：弹药补给点

- 新增可放置、复制的 `ACalamityAmmoPickup`，角色进入触发范围后服务端发放备用弹药。
- 玩家备用弹药达到上限或已淘汰时不消耗补给点。
- 补给点支持配置数量和重生间隔；重生时恢复显示与碰撞。
- 同时统一 GameState、GameMode、Character 的接口与复制字段，补齐联盟、皇印、击杀计分、弹药、换弹和驯服代码间的声明/调用关系，避免近期变更中头文件与实现不匹配。

## 验证说明

这是静态整合，没有 UE 5.5 构建环境，尚未运行 UHT、C++ 编译或多人 PIE 测试。优先验证 RPC、USTRUCT 数组复制、Overlap 委托、Actor Timer、GameMode Logout 覆盖和 Dedicated Server 的拾取竞争。
