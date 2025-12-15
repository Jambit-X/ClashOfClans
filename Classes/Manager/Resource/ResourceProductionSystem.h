#pragma once
#include "cocos2d.h"
#include <functional>

// 资源生产系统 - 负责计算和累积资源
class ResourceProductionSystem {
public:
  static ResourceProductionSystem* getInstance();
  static void destroyInstance();

  // 启动/停止生产
  void startProduction();
  void stopProduction();

  // 更新生产（每0.5秒调用）
  void update(float dt);

  // 计算生产速率
  int calculateGoldProductionRate() const;
  int calculateElixirProductionRate() const;

  // 获取待收集资源
  int getPendingGold() const { return _pendingGold; }
  int getPendingElixir() const { return _pendingElixir; }

  // 获取收集容量
  int getGoldStorageCapacity() const;
  int getElixirStorageCapacity() const;

  // 收集资源
  void collectGold();
  void collectElixir();

  // 设置回调
  using PendingResourceCallback = std::function<void(int gold, int elixir)>;
  void setPendingResourceCallback(PendingResourceCallback callback);

  // 处理离线时间
  void processOfflineTime(long long lastOnlineTime);

private:
  ResourceProductionSystem();
  ~ResourceProductionSystem();

  void notifyPendingResourceChanged();
  int calculateTotalGoldStorageCapacity() const;
  int calculateTotalElixirStorageCapacity() const;

  static ResourceProductionSystem* _instance;

  int _pendingGold;
  int _pendingElixir;
  bool _isProductionRunning;

  PendingResourceCallback _pendingResourceCallback;
};