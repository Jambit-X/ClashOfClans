// DebugHelper.h
// 调试辅助工具类，提供游戏开发和测试中常用的调试功能

#pragma once
#include "cocos2d.h"

/**
 * @brief 调试工具类（全部为静态方法）
 * 
 * 功能说明：
 * 1. 资源管理：直接设置金币、圣水、宝石数量
 * 2. 建筑操作：修改等级、删除建筑、瞬间完成建造
 * 3. 存档操作：强制保存、重置存档
 * 
 * 设计原则：
 * - 完全独立，不修改现有类接口
 * - 所有方法都是静态方法，方便调用
 * - 自动处理数据同步、UI更新、存档保存
 */
class DebugHelper {
public:
    // ========== 资源操作 ==========
    
    /**
     * @brief 设置金币数量
     * @param amount 目标金币数量
     * 
     * 实现方式：
     * 1. 计算当前值与目标值差值
     * 2. 根据差值正负调用addGold()或spendGold()
     * 3. 自动触发资源更新事件，更新HUD显示
     */
    static void setGold(int amount);
    
    /**
     * @brief 设置圣水数量
     * @param amount 目标圣水数量
     */
    static void setElixir(int amount);
    
    /**
     * @brief 设置宝石数量
     * @param amount 目标宝石数量
     */
    static void setGem(int amount);

    // ========== 建筑操作 ==========
    
    /**
     * @brief 设置建筑等级
     * @param buildingId 建筑ID
     * @param level 目标等级
     * 
     * 执行流程：
     * 1. 修改数据层：直接修改BuildingInstance的level字段
     * 2. 更新状态：设置为BUILT状态，清空finishTime
     * 3. 更新显示：调用VillageLayer::updateBuildingDisplay()刷新精灵
     * 4. 保存存档：调用saveToFile()
     * 5. 触发事件：发送资源更新事件（可能影响存储容量）
     */
    static void setBuildingLevel(int buildingId, int level);
    
    /**
     * @brief 删除建筑（完整流程）
     * @param buildingId 要删除的建筑ID
     * 
     * 执行流程（确保无残留）：
     * 1. 获取VillageLayer和HUDLayer引用
     * 2. 隐藏HUD操作菜单（hudLayer->hideBuildingActions()）
     * 3. 删除精灵（villageLayer->removeBuildingSprite()）
     *    - 内部会自动清除选中状态，避免光圈残留
     * 4. 从数据层删除（dataManager->removeBuilding()）
     *    - 同时更新网格占用状态
     * 5. 保存存档
     * 6. 触发资源更新事件
     * 
     * 注意事项：
     * - 必须先处理UI层（HUD、精灵），再处理数据层
     * - removeBuildingSprite()会自动调用hideSelectionEffect()
     */
    static void deleteBuilding(int buildingId);
    
    /**
     * @brief 瞬间完成所有正在建造的建筑
     * 
     * 实现方式：
     * 1. 遍历所有建筑，收集处于CONSTRUCTING状态的建筑ID
     * 2. 根据isInitialConstruction标志，调用对应的完成方法：
     *    - 新建：finishNewBuildingConstruction()
     *    - 升级：finishUpgradeBuilding()
     * 3. 自动处理：
     *    - 状态更新（CONSTRUCTING -> BUILT）
     *    - 资源扣除（升级费用）
     *    - 精灵更新（显示完成后的外观）
     */
    static void completeAllConstructions();

    // ========== 存档操作 ==========
    
    /**
     * @brief 重置存档（删除存档文件并重启游戏）
     * 
     * 执行流程：
     * 1. 检查village.json是否存在
     * 2. 删除存档文件（FileUtils::removeFile()）
     * 3. 销毁VillageDataManager单例（destroyInstance()）
     * 4. 重新加载VillageScene（使用默认数据初始化）
     * 5. 使用TransitionFade过渡到新场景
     * 
     * 注意：下次启动游戏时，将使用默认村庄数据
     */
    static void resetSaveData();
    
    /**
     * @brief 强制立即保存当前游戏状态
     * 
     * 应用场景：
     * - 调试时想保留某个特定状态
     * - 测试存档加载功能
     * - 避免数据丢失
     */
    static void forceSave();
};
