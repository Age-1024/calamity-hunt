# 修复与功能更新记录

## 本次修复：觉醒源 RepNotify ��范围配置

- 将 `bConsumed` 改为 `ReplicatedUsing=OnRep_Consumed`，客户端收到状态变化后隐藏已消耗觉醒源并关闭碰撞。
- 服务端成功授予后显式调用同一状态应用函数，因为服务端本地属性赋值不会自动执行 RepNotify。
- `OnConstruction` 根据 `UseRadius` 更新球形组件半径，并按配置印记显示不同的中文灰盒标签。
- 授予前服务端额外确认请求者是存活的 `ACalamityCharacter`，并再次按配置半径校验。
- 成功消耗后调用 `ForceNetUpdate`，请求尽快同步变化。

## 未验证事项

这是代码级修正；没有 Unreal Engine 5.5 环境，未执行 UHT、C++ 构建或 PIE 多客户端测试。需要验证 RepNotify、关卡实例属性和碰撞组件行为。此前发现的其他跨文件接口/构建问题仍需以本地完整编译结果为准。
