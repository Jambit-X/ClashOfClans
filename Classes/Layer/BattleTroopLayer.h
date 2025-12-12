#pragma once

// ===== 强制启用调试模式（临时测试用）=====
#ifndef COCOS2D_DEBUG
#define COCOS2D_DEBUG 1
#endif
// ========================================

#include "cocos2d.h"
#include "../Sprite/BattleUnitSprite.h"
#include <vector>
#include <memory>
#include <map>

USING_NS_CC;

// ===== 前向声明 =====
struct BuildingInstance;

/**
 * @brief 战斗单位管理层 - 负责所有军队相关功能
 * 
 * 职责：
 * 1. 管理所有战斗单位的生命周期
 * 2. 提供单位生成、移动、攻击接口
 * 3. 调试模式下的测试序列
 */
class BattleTroopLayer : public Layer {
public:
    static BattleTroopLayer* create();
    
    virtual bool init() override;
    
    /**
     * @brief 在指定网格位置生成单位
     * @param unitType 单位类型（如 "Barbarian"）
     * @param gridX 网格X坐标
     * @param gridY 网格Y坐标
     * @return 创建的单位精灵
     */
    BattleUnitSprite* spawnUnit(const std::string& unitType, int gridX, int gridY);
    
    /**
     * @brief 批量生成单位（用于测试/场景初始化）
     * @param unitType 单位类型
     * @param spacing 间隔（格子数）
     */
    void spawnUnitsGrid(const std::string& unitType, int spacing = 3);
    
    /**
     * @brief 移除所有单位
     */
    void removeAllUnits();
    
    /**
     * @brief 获取所有单位
     */
    const std::vector<BattleUnitSprite*>& getAllUnits() const { return _units; }
    
#if COCOS2D_DEBUG
    /**
     * @brief 调试模式：运行野蛮人测试序列
     */
    void runBarbarianTestSequence();
    
    /**
     * @brief 调试模式：验证寻路功能
     * 
     * 测试内容：
     * 1. 野蛮人从地图一角走到另一角（绕过障碍物）
     * 2. 验证 A* 算法正确性
     * 3. 测试路径平滑度和效率
     */
    void runPathfindingTest();
    
    /**
     * @brief 调试模式：交互式寻路测试
     * 
     * 测试内容：
     * 1. 生成1个野蛮人在地图中心
     * 2. 玩家放置建筑后，野蛮人自动寻路走到建筑旁边
     * 3. 验证实时寻路功能
     */
    void startInteractivePathfindingTest();
    
    /**
     * @brief 检查是否有新建筑放置或建筑移动（每秒调用）
     */
    void checkForNewBuildings();
    
    /**
     * @brief 让野蛮人走到指定的建筑旁边
     * @param building 目标建筑
     */
    void walkBarbarianToBuilding(const BuildingInstance& building);
#endif
    
private:
    // 所有单位列表
    std::vector<BattleUnitSprite*> _units;
    
    // 网格参数（从 GridMapUtils 获取）
    static const int GRID_WIDTH = 44;
    static const int GRID_HEIGHT = 44;
    
    // ===== 交互式测试用变量 =====
    BattleUnitSprite* _testBarbarian = nullptr;         // 测试用野蛮人
    size_t _lastBuildingCount = 0;                      // 上次建筑数量
    std::map<int, Vec2> _buildingPositions;             // 记录每个建筑的位置（用于检测移动）
};
