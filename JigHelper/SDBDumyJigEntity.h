/**
 * @file    SDBDumyJigEntity.h
 * @brief   Jig辅助实体类
 * @author  Gergul
 * @date    2018/10/29
 * Note:    强制调用图形实体的worldDraw和viewportDraw进行重绘图形实体
 */
#pragma once
#include "dbents.h"

/**
 * @class   CSDBDumyJigEntity
 * @brief   Jig辅助实体类
 * @author  Gergul
 * @date    2018/10/29
 * Note:    
 */
class CSDBDumyJigEntity : public AcDbEntity
{
public:
	ACRX_DECLARE_MEMBERS(CSDBDumyJigEntity);

	CSDBDumyJigEntity(void);

	virtual ~CSDBDumyJigEntity(void);

public:
	//通过指针添加实体
	void addEntity(AcDbEntity* pEnt);
	//获取实体
	AcDbEntity* getEnity(int pos) const;
	//获得实体个数
	int getEntityCount();
	//删除实体
	bool remove(AcDbEntity *pEntRemove);
	bool remove(int nIdex);
	//清空实体
	bool removeAll();

public:
	/** 世界坐标绘制接口
	 *	@param   mode - 接口输入输出指针
	 */
	virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw *mode);
	/** 单一视口绘制接口
	 *	@param   mode - 接口输入输出指针
	 */
	virtual void subViewportDraw(AcGiViewportDraw* pVd);

public:
	//外部open的实体
	AcDbVoidPtrArray m_entArray;
	//用于viewportDraw的对象
	AcDbVoidPtrArray m_entArrayViewPointDraw;
};

//#ifdef SDENTITY_MODULE
ACDB_REGISTER_OBJECT_ENTRY_AUTO(CSDBDumyJigEntity)
	//acrxClassDictionary
//#endif