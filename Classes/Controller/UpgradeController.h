#ifndef __UPGRADE_CONTROLLER_H__
#define __UPGRADE_CONTROLLER_H__

class UpgradeController {
public:
  // 尝试执行一个模拟升级
  // cost: 升级花费
  // isGold: true 为消耗金币，false 为消耗圣水
  bool tryUpgrade(int cost, bool isGold);
};

#endif // __UPGRADE_CONTROLLER_H__