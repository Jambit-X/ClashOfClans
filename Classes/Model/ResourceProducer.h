#ifndef __RESOURCE_PRODUCER_H__
#define __RESOURCE_PRODUCER_H__

#include "../Model/Building.h"

// 资源生产建筑基类（金矿、圣水采集器）
class ResourceProducer : public Building {
public:
  enum ResourceType {
    GOLD,
    ELIXIR,
    DARK_ELIXIR
  };

  ResourceProducer();
  virtual ~ResourceProducer();

  // 生产相关
  void startProduction();
  void stopProduction();
  virtual void update(float dt) override;

  // 资源计算
  int calculateProducedResource();
  void collectResource();
  void calculateOfflineProduction(long long offlineSeconds);

  // Getter
  int getCurrentStored() const { return _currentStored; }
  int getStorageCapacity() const { return _storageCapacity; }
  int getProductionRate() const { return _productionRate; }
  long long getLastUpdateTime() const { return _lastUpdateTime; }
  ResourceType getResourceType() const { return _resourceType; }

protected:
  ResourceType _resourceType;
  int _productionRate;      // 每小时生产速率
  int _storageCapacity;     // 存储容量
  int _currentStored;       // 当前存储量
  long long _lastUpdateTime;
  bool _isProducing;

  void updateVisualStorage();  // 更新UI显示
};

#endif