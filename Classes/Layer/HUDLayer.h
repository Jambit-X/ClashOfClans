#ifndef __HUDLAYER_H__
#define __HUDLAYER_H__

#include "cocos2d.h"

class HUDLayer : public cocos2d::Layer {
public:
  virtual bool init();
  CREATE_FUNC(HUDLayer);

  void updateResourceDisplay(int gold, int elixir);

private:
  cocos2d::Label* _goldLabel;
  cocos2d::Label* _elixirLabel;
};

#endif // __HUDLAYER_H__