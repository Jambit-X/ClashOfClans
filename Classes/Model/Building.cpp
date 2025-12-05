#pragma once // 比 #ifndef 更现代，防止重复包含

// 纯净的 Model，不引入 cocos2d.h，除非你为了方便用 Vec2
// 这里我们只用基础类型，保持 Model 轻量化

class Building {
public:
    // 建筑类型
    enum class Type {
        TOWN_HALL,
        GOLD_MINE,
        BARRACKS,
        CANNON  // 加回加农炮类型
    };

    // 建筑状态（为了拖拽功能，这个非常重要！）
    enum class State {
        PREVIEW,   // 预览/拖拽中（还没真正放下）
        PLACED,    // 已放置（正常工作）
        MOVING     // 放置后被重新移动（如果你要做编辑模式）
    };

    // 构造函数 - 修复签名以匹配cpp文件
    Building(int id, Type type, int width, int height);

    // Getters
    int getId() const { return _id; }
    Type getType() const { return _type; }
    int getGridX() const { return _gridX; }
    int getGridY() const { return _gridY; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    State getState() const { return _state; }

    // Setters
    void setGridPosition(int x, int y) { _gridX = x; _gridY = y; }
    void setState(State state) { _state = state; }

    // 静态工厂方法声明
    static Building* createTownHall(int id, int gridX, int gridY);
    static Building* createCannon(int id, int gridX, int gridY);
    static Building* createGoldMine(int id, int gridX, int gridY);
    static Building* createBarracks(int id, int gridX, int gridY);

private:
    int _id;
    Type _type;
    int _gridX;
    int _gridY;
    int _width;
    int _height;
    State _state;   // 核心状态
};