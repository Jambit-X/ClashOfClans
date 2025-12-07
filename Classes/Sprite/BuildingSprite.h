#ifndef __BUILDING_SPRITE_H__
#define __BUILDING_SPRITE_H__

#include "cocos2d.h"
#include "Model/VillageData.h"

USING_NS_CC;

/**
 * @brief 建筑精灵类
 * 负责建筑的视觉表现和交互
 */
class BuildingSprite : public Sprite {
public:
	// ========== 创建和初始化 ==========

	/**
	 * @brief 根据建筑实例创建精灵
	 * @param building 建筑实例数据
	 * @return 建筑精灵指针
	 */
	static BuildingSprite* create(const BuildingInstance& building);

	/**
	 * @brief 初始化建筑精灵
	 * @param building 建筑实例数据
	 * @return 是否初始化成功
	 */
	virtual bool init(const BuildingInstance& building);

	// ========== 数据更新 ==========

	/**
	 * @brief 更新建筑数据（等级、状态等）
	 * @param building 新的建筑数据
	 */
	void updateBuilding(const BuildingInstance& building);

	/**
	 * @brief 更新建筑状态
	 * @param newState 新状态
	 */
	void updateState(BuildingInstance::State newState);

	/**
	 * @brief 更新建筑等级
	 * @param newLevel 新等级
	 */
	void updateLevel(int newLevel);

	// ========== 显示模式 ==========现在还没搞

	/**
	 * @brief 设置为拖拽模式（半透明，跟随鼠标）
	 * @param dragging 是否拖拽中
	 */
	void setDraggingMode(bool dragging);

	/**
	 * @brief 设置为放置预览模式（显示是否可放置）
	 * @param valid 位置是否有效
	 */
	void setPlacementPreview(bool valid);

	/**
	 * @brief 设置为选中状态（显示选中框）
	 * @param selected 是否选中
	 */
	void setSelected(bool selected);

	// ========== 进度显示 ==========

	/**
	 * @brief 显示建造/升级进度
	 * @param progress 进度值 (0.0 ~ 1.0)
	 */
	void showConstructionProgress(float progress);

	/**
	 * @brief 隐藏进度条
	 */
	void hideConstructionProgress();

	/**
	 * @brief 显示倒计时文本
	 * @param seconds 剩余秒数
	 */
	void showCountdown(int seconds);

	// ========== 访问器 ==========

	int getBuildingId() const { return _buildingId; }
	void setBuildingId(int id) { _buildingId = id; }

	int getBuildingType() const { return _buildingType; }
	int getLevel() const { return _level; }
	BuildingInstance::State getState() const { return _state; }

	const Vec2& getGridPos() const { return _gridPos; }
	void setGridPos(const Vec2& pos);

	Size getGridSize() const { return _gridSize; }
	
	/** 获取视觉偏移量（用于修正显示位置） */
	Vec2 getVisualOffset() const { return _visualOffset; }

	// ========== 碰撞检测 ==========

	/**
	 * @brief 检查点是否在建筑范围内
	 * @param point 世界坐标点
	 * @return 是否在范围内
	 */
	bool containsPoint(const Vec2& point) const;

	/**
	 * @brief 获取建筑的包围盒（用于碰撞检测）
	 * @return 包围盒
	 */
	Rect getBoundingBox() const override;

private:
	// ========== 内部属性 ==========
	int _buildingId;                    // 建筑唯一ID
	int _buildingType;                  // 建筑类型
	int _level;                         // 建筑等级
	BuildingInstance::State _state;     // 建筑状态
	Vec2 _gridPos;                      // 网格坐标
	Size _gridSize;                     // 占据网格大小
	float _normalScale;                 // 正常缩放比例（用于恢复）
	Vec2 _visualOffset;                 // 视觉偏移量（修正显示位置）

	// ========== UI 元素 ==========
	Sprite* _shadowSprite;              // 阴影
	ProgressTimer* _progressBar;        // 建造进度条
	Sprite* _progressBg;                // 进度条背景
	Label* _levelLabel;                 // 等级标签
	Label* _countdownLabel;             // 倒计时标签
	Sprite* _selectionBox;              // 选中框

	// ========== 内部方法 ==========
	void initVisualElements();          // 初始化视觉元素
	void updateSpriteTexture();         // 更新建筑纹理
	void createProgressBar();           // 创建进度条
	void createLevelLabel();            // 创建等级标签
	void createShadow();                // 创建阴影
	void createSelectionBox();          // 创建选中框
};

#endif // __BUILDING_SPRITE_H__
