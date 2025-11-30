#include "PlayerData.h"

static PlayerData* s_instance = nullptr;

PlayerData* PlayerData::getInstance() {
  if (!s_instance) {
    s_instance = new PlayerData();
    s_instance->initData();
  }
  return s_instance;
}

PlayerData::PlayerData() : _gold(0), _elixir(0) {}

void PlayerData::initData() {
  // ³õÊ¼×ÊÔ´
  _gold = 1000;
  _elixir = 1000;
}

void PlayerData::addGold(int amount) {
  _gold += amount;
  if (_gold > _maxGold) _gold = _maxGold;
}

void PlayerData::addElixir(int amount) {
  _elixir += amount;
  if (_elixir > _maxElixir) _elixir = _maxElixir;
}

bool PlayerData::consumeGold(int amount) {
  if (_gold >= amount) {
    _gold -= amount;
    return true;
  }
  return false;
}

bool PlayerData::consumeElixir(int amount) {
  if (_elixir >= amount) {
    _elixir -= amount;
    return true;
  }
  return false;
}