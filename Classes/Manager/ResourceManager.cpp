#include "ResourceManager.h"

ResourceManager* ResourceManager::_instance = nullptr;

ResourceManager* ResourceManager::getInstance() {
  if (!_instance) {
    _instance = new ResourceManager();
  }
  return _instance;
}

void ResourceManager::destroyInstance() {
  if (_instance) {
    delete _instance;
    _instance = nullptr;
  }
}

ResourceManager::ResourceManager()
  : _gold(0), _elixir(0), _darkElixir(0), _gems(0) {}

ResourceManager::~ResourceManager() {}

void ResourceManager::addGold(int amount) {
  _gold += amount;
  notifyResourceChanged();
}

void ResourceManager::addElixir(int amount) {
  _elixir += amount;
  notifyResourceChanged();
}

bool ResourceManager::consumeGold(int amount) {
  if (_gold < amount) return false;
  _gold -= amount;
  notifyResourceChanged();
  return true;
}

bool ResourceManager::consumeElixir(int amount) {
  if (_elixir < amount) return false;
  _elixir -= amount;
  notifyResourceChanged();
  return true;
}

void ResourceManager::setGold(int amount) {
  _gold = amount;
  notifyResourceChanged();
}

void ResourceManager::setElixir(int amount) {
  _elixir = amount;
  notifyResourceChanged();
}

void ResourceManager::loadResources() {
  // 通常由 SaveManager 加载后调用 setGold/setElixir
}

void ResourceManager::notifyResourceChanged() {
  // 通知UI或其他系统资源变化
  // EventDispatcher::getInstance()->dispatchCustomEvent("RESOURCE_CHANGED");
}