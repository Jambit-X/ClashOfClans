#include "UpgradeController.h"
#include "../Model/PlayerData.h"
#include "cocos2d.h"

bool UpgradeController::tryUpgrade(int cost, bool isGold) {
  auto playerData = PlayerData::getInstance();
  bool success = false;

  if (isGold) {
    success = playerData->consumeGold(cost);
  } else {
    success = playerData->consumeElixir(cost);
  }

  if (success) {
    cocos2d::log("Upgrade Success! Consumed %d resources.", cost);
    // 这里未来会添加升级倒计时逻辑
  } else {
    cocos2d::log("Upgrade Failed! Not enough resources.");
  }

  return success;
}