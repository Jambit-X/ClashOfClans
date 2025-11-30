#ifndef __PLAYER_DATA_H__
#define __PLAYER_DATA_H__

#include "cocos2d.h"

class PlayerData {
public:
  // 获取单例实例
  static PlayerData* getInstance();

  // 初始化默认数据
  void initData();

  // Getters
  int getGold() const { return _gold; }
  int getElixir() const { return _elixir; }

  // 增加资源 (生产)
  void addGold(int amount);
  void addElixir(int amount);

  // 消耗资源 (升级/建造)
  // 返回 true 表示扣除成功，false 表示余额不足
  bool consumeGold(int amount);
  bool consumeElixir(int amount);

private:
  PlayerData(); // 私有构造函数

  int _gold;
  int _elixir;

  // 限制最大值 (暂时硬编码，后期可由存储建筑决定)
  int _maxGold = 10000;
  int _maxElixir = 10000;
};

#endif // __PLAYER_DATA_H__