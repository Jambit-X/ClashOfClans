#ifndef __RESOURCE_CONTROLLER_H__
#define __RESOURCE_CONTROLLER_H__

class ResourceController {
public:
  ResourceController();
  ~ResourceController();

  // 每帧调用，计算资源产出
  void update(float dt);

private:
  float _timeAccumulator; // 时间累加器

  // 模拟产出率 (每秒产出多少资源)
  // 后期这将通过遍历所有金矿建筑来计算
  int _goldProductionRate = 10;
  int _elixirProductionRate = 5;
};

#endif // __RESOURCE_CONTROLLER_H__