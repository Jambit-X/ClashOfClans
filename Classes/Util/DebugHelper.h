#pragma once
#include "cocos2d.h"

/**
 * @brief 调试工具类（静态方法，完全隔离）
 * 封装所有调试操作，不修改现有类的接口
 */
class DebugHelper {
public:
    // ========== 资源操作 ==========
    static void setGold(int amount);
    static void setElixir(int amount);
    static void setGem(int amount);

    // ========== 建筑操作 ==========
    
    /**
     * @brief 设置建筑等级（会更新精灵显示）
     * @param buildingId 建筑ID
     * @param level 目标等级
     */
    static void setBuildingLevel(int buildingId, int level);
    
    /**
     * @brief 删除建筑（完整协调：数据层+精灵+选中状态+存档）
     * @param buildingId 要删除的建筑ID
     */
    static void deleteBuilding(int buildingId);
    
    /**
     * @brief 瞬间完成所有正在建造的建筑
     */
    static void completeAllConstructions();

    // ========== 存档操作 ==========
    
    /**
     * @brief 重置存档（删除存档文件，重启游戏后生效）
     */
    static void resetSaveData();
    
    /**
     * @brief 强制立即保存
     */
    static void forceSave();
};
