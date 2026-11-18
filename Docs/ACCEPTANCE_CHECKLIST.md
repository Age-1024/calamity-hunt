## 血月安全区验收

- [ ] BloodMoon 阶段开始前安全区不造成伤害，并显示等待状态。
- [ ] BloodMoon 阶段开始后 Zone 激活，客户端收到 bZoneActive 和 CurrentRadius。
- [ ] CurrentRadius 从 InitialRadius 平滑按阶段时间缩至 FinalRadius，且不会越界。
- [ ] 安全区外存活玩家在 DamageGraceSeconds 后按伤害间隔受伤。
- [ ] 安全区内玩家、已淘汰玩家不受缩圈伤害。
- [ ] 玩家进入安全区后停止受伤；安全区外伤害不会影响异灵。
- [ ] 对局结束后停止缩圈及伤害。
- [ ] Dedicated Server + 两客户端验证复制与阶段切换。
