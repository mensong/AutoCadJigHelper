/**
 * @file    JigHelper.h
 * @brief   简易通用jig类
 * @author  Gergul
 * @date    2018/10/29
 * Note:    
 */
#pragma once
#include <dbjig.h>
#include <functional>
#include <vector>
#include "SDBDumyJigEntity.h"

 //更新jig实体函数原型
typedef Adesk::Boolean (*pfnUpdateJig)(class CJigHelper *pJigHelper, const AcGePoint3d &posCur, const AcGePoint3d &posLast);

//函数存储结构
#define FUNC_UPDATE_JIG std::function<Adesk::Boolean(class CJigHelper *pJigHelper, const AcGePoint3d &posCur, const AcGePoint3d &posLast)>

/**
 * @class   CJigHelper
 * @brief   简易通用jig帮助类
 * @author  Gergul
 * @date    2018/10/29
 * Note:    
 */
class CJigHelper :
	public AcEdJig
{
public:
	CJigHelper(void);
	virtual ~CJigHelper(void);

public:
	/**
	 * @brief   设置更新jig实体函数
	 * @param   pFunc - 更新函数
	 * @author  Gergul
	 * @date    2018/10/29
	 * Note:    
	 */
	void SetUpdateJigFunc(FUNC_UPDATE_JIG pFunc);

	/**
	 * @brief   把类成员函数设置为jig的Update函数
	 * @param   ClassFunc - 类成员函数。例如：CClass::UpdateJig
	 * @param   pClassObj - Update函数所在的类对象。类型为CClass，一般可为this
	 * @param   pJigHelper - jig对象指针
	 * @author  Gergul
	 * @date    2018/10/29
	 * Note:    此宏为辅助SetUpdateJigFunc函数使用
	 */
#define SetUpdateClassFunc(ClassFunc, pClassObj, pJigHelper) \
	(pJigHelper)->SetUpdateJigFunc(std::bind(&ClassFunc, (pClassObj), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	 /**
	  * @brief   把非类成员函数或类静态成员函数设置为jig的Update函数
	  * @param   GeneralFunc - 全局或类成员函数名。例如：UpdateJig
	  * @param   pJigHelper - jigHelper对象指针
	  * @author  Gergul
	  * @date    2018/10/29
	  * Note:    此宏为辅助SetUpdateJigFunc函数使用
	  */
#define SetUpdateFunc(GeneralFunc, pJigHelper) \
	(pJigHelper)->SetUpdateJigFunc(std::bind(&GeneralFunc, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	/**
	 * @brief   设置jig基点
	 * @param   ptOrigin - 基点
	 * @author  Gergul
	 * @date    2018/10/29
	 * Note:    
	 */
	void SetBasePoint(const AcGePoint3d &ptOrigin);

	/**
	 * @brief   取消设置jig基点
	 * @author  Gergul
	 * @date    2018/10/29
	 * Note:    配合pat使用
	 */
	void UnSetBasePoint();
	
	/**
	* @brief   获得基点
	* @author  Gergul
	* @date    2018/10/29
	* @return  返回基点
	* Note:
	*/
	const AcGePoint3d& GetBasePoint() const { return m_posBase; }

	/**
	 * @brief   设置是否正交极轴
	 * @param   bPat - 是否设置为正交模式
	 * @author  Gergul
	 * @date    2018/10/29
	 * Note:    
	 */
	void SetPat(bool bPat);
	
	/**
	 * @brief   添加图形实体
	 * @param   pEnt - 图形实体(实体需要以AcDb::kForWrite的方式打开)
	 * @author  Gergul
	 * @date    2018/10/29
	 * Note:    
	 */
	void RegisterAsJigEntity(AcDbEntity *pEnt);

	/**
	* @brief   批量添加图形实体
	* @param   vctEnts - 图形实体集合(实体需要以AcDb::kForWrite的方式打开)
	* @author  Gergul
	* @date    2018/10/29
	* Note:
	*/
	void RegisterAsJigEntity(std::vector<AcDbEntity *> &vctEnts);
	
	/**
	 * @brief   撤销注册的图形实体
	 * @param   pEnt - 要撤销的实体
	 * @author  Gergul
	 * @date    2018/10/29
	 * Note:    不会帮关闭，需要自行关闭
	 */
	void UnregisterJigEntity(AcDbEntity *pEnt);

	/**
	* @brief   批量撤销注册的图形实体
	* @param   vctEnts - 要撤销的实体集合
	* @author  Gergul
	* @date    2018/10/29
	* Note:    不会帮关闭，需要自行关闭
	*/
	void UnregisterJigEntity(std::vector<AcDbEntity *> &vctEnts);
	
	/**
	* @brief   撤销注册全部JIG实体
	* @author  Gergul
	* @date    2018/10/29
	* Note:    不会帮关闭，需要自行关闭
	*/
	void UnregisterAllJigEntity();

	//startJig返回值
	typedef enum
	{
		RET_POINT,	//点
		RET_STRING,	//字符串
		RET_CANCEL,	//取消
		RET_STOP,	//停止，在切换DWG的时候
	} RESULT;

	/**
	 * @brief   开始jig效果
	 * @author  Gergul
	 * @date    2018/10/29
	 * @return  在jig过程中所操作的结果
	 * Note:    
	 */
	CJigHelper::RESULT startJig();

	/**
	 * @brief   获得字符串
	 * @author  Gergul
	 * @date    2018/10/29
	 * @return  如果startJig返回RET_POINT时可使用此函数返回输出的字符串
	 * Note:    
	 */
	const ACHAR *GetStringResult() const { return (ACHAR *)m_str; }
	
	/**
	* @brief   获得当前点
	* @author  Gergul
	* @date    2018/10/29
	* @return  如果startJig返回RET_STRING时可使用此函数返回输出的点
	* Note:
	*/
	const AcGePoint3d &GetPosition() const { return m_posCur; }

	/**
	 * @brief   强制终止jig
	 * @author  Gergul
	 * @date    2018/10/29
	 * @return  
	 * Note:    退出jig效果
	 */
	void cancel() { m_bCanecl = true; }

	/**
	 * @brief   获得图形实体容器实体
	 * @author  Gergul
	 * @date    2018/10/30
	 * @return  
	 * Note:    
	 */
	CSDBDumyJigEntity* GetJigEntity() { return &m_DumyJigEnt; }

protected:
	/**
	 * @brief   缺省的Update函数
	 * @param   posCur - 当前点
	 * @param   posLast - 上一次进行UpdateJig的点
	 * @author  Gergul
	 * @date    2018/10/29
	 * @return  
	 * Note:    
	 */
	Adesk::Boolean CALLBACK UpdateJig(class CJigHelper *pJigHelper, const AcGePoint3d &posCur, const AcGePoint3d &posLast);

protected:
	//- AcEdJig overrides
	//- input sampler
	virtual AcEdJig::DragStatus sampler();
	//- jigged entity update
	virtual Adesk::Boolean update();
	//- jigged entity pointer return
	virtual AcDbEntity *entity() const;

protected:
	//- Entity being jigged
	CSDBDumyJigEntity m_DumyJigEnt;

	AcGePoint3d m_posLast;//前一个位置

	AcGePoint3d m_posCur;//当前位置

	bool m_bHasSetBasePos;//是否已经设置了基点
	AcGePoint3d m_posBase;//基点

	ACHAR m_str[2049];	//接受结果的字符串

	FUNC_UPDATE_JIG m_funcUpdateJig;//更新jig实体函数

	bool m_bPat;	//是否正交极轴
	bool m_bCanecl;	//用户是否终止
};
