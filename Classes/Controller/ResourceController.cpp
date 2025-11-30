#include "ResourceController.h"
#include "../Model/PlayerData.h"

ResourceController::ResourceController() : _timeAccumulator(0.0f) {}

ResourceController::~ResourceController() {}

void ResourceController::update(float dt) {
  _timeAccumulator += dt;

  // 每 1 秒结算一次资源产出
  if (_timeAccumulator >= 1.0f) {
    // 获取单例并增加资源
    auto playerData = PlayerData::getInstance();
    playerData->addGold(_goldProductionRate);
    playerData->addElixir(_elixirProductionRate);

    // 扣除 1 秒，保留余数 (处理帧率波动)
    _timeAccumulator -= 1.0f;
  }
}