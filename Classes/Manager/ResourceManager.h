#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__

#include "cocos2d.h"

class ResourceManager {
public:
  static ResourceManager* getInstance();
  static void destroyInstance();

  // 资源操作
  void addGold(int amount);
  void addElixir(int amount);
  bool consumeGold(int amount);
  bool consumeElixir(int amount);

  // Getter/Setter
  int getGold() const { return _gold; }
  int getElixir() const { return _elixir; }
  int getGems() const { return _gems; }

  void setGold(int amount);
  void setElixir(int amount);
  // 保存/加载
  void saveResources();
  void loadResources();

private:
  ResourceManager();
  ~ResourceManager();

  static ResourceManager* _instance;

  int _gold;
  int _elixir;
  int _darkElixir;
  int _gems;

  void notifyResourceChanged();  // 通知UI更新
};

#endif