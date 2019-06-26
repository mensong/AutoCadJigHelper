// (C) Copyright 2002-2012 by Autodesk, Inc. 
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to 
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//

//-----------------------------------------------------------------------------
//----- acrxEntryPoint.cpp
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "resource.h"
#include "../JigHelper/JigHelper.h"

//-----------------------------------------------------------------------------
#define szRDS _RXST("")

Acad::ErrorStatus AppendToDatabase(AcDbObjectId &objId, AcDbEntity* pEntity, AcDbDatabase *pDb = acdbCurDwg())
{
	AcDbBlockTable *pBlockTable = NULL;//块表
	AcDbBlockTableRecord *pSpaceRecord = NULL;//快记录
											  //打开块表为写入状态
	Acad::ErrorStatus es = pDb->getSymbolTable(pBlockTable, AcDb::kForRead);
	if (es != Acad::eOk)
		return Acad::eWrongDatabase;
	//获取符号表记录的对象指针
	es = pBlockTable->getAt(ACDB_MODEL_SPACE, pSpaceRecord, AcDb::kForWrite);
	pBlockTable->close();
	if (es != Acad::eOk)
	{
		return Acad::eNotInDatabase;
	}
	//添加实体到块表记录
	es = pSpaceRecord->appendAcDbEntity(objId, pEntity);
	//关闭块表，块表记录对象指针
	pSpaceRecord->close();
	return es;
}

//-----------------------------------------------------------------------------
//----- ObjectARX EntryPoint
class CSampleCadJigApp : public AcRxArxApp {

public:
	CSampleCadJigApp () : AcRxArxApp () {}

	virtual AcRx::AppRetCode On_kInitAppMsg (void *pkt) {
		// TODO: Load dependencies here

		// You *must* call On_kInitAppMsg here
		AcRx::AppRetCode retCode =AcRxArxApp::On_kInitAppMsg (pkt) ;
		
		// TODO: Add your initialization code here

		return (retCode) ;
	}

	virtual AcRx::AppRetCode On_kUnloadAppMsg (void *pkt) {
		// TODO: Add your code here

		// You *must* call On_kUnloadAppMsg here
		AcRx::AppRetCode retCode =AcRxArxApp::On_kUnloadAppMsg (pkt) ;

		// TODO: Unload dependencies here

		return (retCode) ;
	}

	virtual void RegisterServerComponents () {
	}
	
	/////////////移实体jig的简单应用，使用了内置的Update函数///////////////
	static void MyGroupJigMove() 
	{
		std::vector<AcDbEntity*> vctJigEnt;

		ads_name ssname;
		ads_name ent;
		//获取选择集
		acedPrompt(_T("\n选择实体："));
		acedSSGet(NULL, NULL, NULL, NULL, ssname);
		//获取选择集的长度
		Adesk::Int32 len = 0;
		int nRes = acedSSLength(ssname, &len);
		if (RTNORM == nRes)
		{
			//遍历选择集
			for (int i = 0; i < len; i++)
			{
				//获取实体名
				int nRes = acedSSName(ssname, i, ent);
				if (nRes != RTNORM)
					continue;
				//根据实体名得到ID，然后打开自定义实体
				AcDbObjectId id;
				acdbGetObjectId(id, ent);
				if (!id.isValid())
					continue;

				AcDbEntity *pEnt = NULL;
				acdbOpenObject(pEnt, id, AcDb::kForWrite);
				//判断自定义实体的类型
				if (pEnt == NULL)
					continue;

				vctJigEnt.push_back(pEnt);
			}
		}
		//释放选择集
		acedSSFree(ssname);
		if (vctJigEnt.size() < 1)
			return;

		//获得实体的范围的左下角点作为jig基点
		AcDbExtents extsAll;
		AcDbExtents exts;
		for (int i = 0; i < vctJigEnt.size(); ++i)
		{
			if (vctJigEnt[i]->getGeomExtents(exts) == Acad::eOk)
				extsAll.addExt(exts);
		}
		AcGePoint3d ptBase = extsAll.minPoint();

		JigHelper jig;
		jig.setDispPrompt(_T("请选择放置点："));
		jig.SetBasePoint(ptBase);
		jig.RegisterAsJigEntity(vctJigEnt);
		JigHelper::RESULT ec = jig.startJig();
		if (ec != JigHelper::RET_POINT)
		{
			//无效输入则撤销
			AcGeVector3d vec = jig.GetBasePoint() - jig.GetPosition();
			for (int i = 0; i < vctJigEnt.size(); ++i)
			{
				vctJigEnt[i]->transformBy(vec);
			}
		}

		//关闭图形实体
		for (int i = 0; i < vctJigEnt.size(); ++i)
		{
			vctJigEnt[i]->close();
		}
	}
	///////////////////////////////////////////////////////////////////////

	//////////////////////////演示一个箭头的Jig////////////////////////////
	static Adesk::Boolean UpdateJigArrow(class JigHelper *pJigHelper, const AcGePoint3d &posCur, const AcGePoint3d &posLast)
	{
		CSDBDumyJigEntity* pJigEntity = pJigHelper->GetJigEntity();
		AcDbLine* pLinePole = (AcDbLine*)pJigEntity->getEnity(0);
		AcDbLine* pLineLeft = (AcDbLine*)pJigEntity->getEnity(1);
		AcDbLine* pLineRight = (AcDbLine*)pJigEntity->getEnity(2);

		//设置杆
		pLinePole->setEndPoint(posCur);
		AcGeLineSeg3d geSeg(pLinePole->startPoint(), posCur);
		AcGePoint3d ptArrow = geSeg.evalPoint(0.7);

		//设置左箭头
		pLineLeft->setStartPoint(posCur);
		AcGePoint3d ptEnd = ptArrow;
		ptEnd.rotateBy(0.785375, AcGeVector3d(0, 0, 1), posCur);
		pLineLeft->setEndPoint(ptEnd);

		//设置右箭头
		pLineRight->setStartPoint(posCur);
		AcGePoint3d ptEnd2 = ptArrow;
		ptEnd2.rotateBy(-0.785375, AcGeVector3d(0, 0, 1), posCur);
		pLineRight->setEndPoint(ptEnd2);

		return Adesk::kTrue;
	}

	static void MyGroupJigArrow()
	{
		AcDbLine* pLinePole = new AcDbLine;
		AcDbLine* pLineLeft = new AcDbLine;
		AcDbLine* pLineRight = new AcDbLine;
		AcDbObjectId idPole;
		AppendToDatabase(idPole, pLinePole);
		AcDbObjectId idLeft;
		AppendToDatabase(idLeft, pLineLeft);
		AcDbObjectId idRight;
		AppendToDatabase(idRight, pLineRight);

		JigHelper jig;
		jig.setDispPrompt(_T("选择箭头终点："));
		AcGePoint3d ptBase(0, 0, 0);
		jig.SetBasePoint(ptBase);
		jig.RegisterAsJigEntity(pLinePole);
		jig.RegisterAsJigEntity(pLineLeft);
		jig.RegisterAsJigEntity(pLineRight);
		SetUpdateFunc(CSampleCadJigApp::UpdateJigArrow, &jig);

		JigHelper::RESULT res = jig.startJig();
		if (res != JigHelper::RET_POINT)
		{//不是以点结束则删除箭头
			pLinePole->erase();
			pLineLeft->erase();
			pLineRight->erase();
		}

		pLinePole->close();
		pLineLeft->close();
		pLineRight->close();
	}	
	///////////////////////////////////////////////////////////////////////

	////////////////////演示如何在Update函数中改变实体/////////////////////
	static AcDbLine* pLineLeft;
	static AcDbLine* pLineRight;
	static Adesk::Boolean UpdateJigHalfway(class JigHelper *pJigHelper, const AcGePoint3d &posCur, const AcGePoint3d &posLast)
	{
		//注册jig实体
		if (posLast.x * posCur.x < 0 || //跨区域则重新注册图形实体
			pJigHelper->GetJigEntity()->getEntityCount() < 1//第一次进入
			)
		{
			pJigHelper->UnregisterAllJigEntity();

			if (posCur.x < 0)
			{//左边
				pJigHelper->RegisterAsJigEntity(pLineLeft);
				pJigHelper->setDispPrompt(_T("在左边\n"));
			}
			else if (posCur.x > 0)
			{//右边
				pJigHelper->RegisterAsJigEntity(pLineRight);
				pJigHelper->setDispPrompt(_T("在右边\n"));
			}
		}
		
		if (posCur.x < 0)
		{
			AcDbLine* pLine = (AcDbLine*)pJigHelper->GetJigEntity()->getEnity(0);
			pLine->setStartPoint(AcGePoint3d(-1000, posCur.y, 0));
			pLine->setEndPoint(AcGePoint3d(0, posCur.y, 0));
			
		}
		else if (posCur.x > 0)
		{
			AcDbLine* pLine = (AcDbLine*)pJigHelper->GetJigEntity()->getEnity(0);
			pLine->setStartPoint(AcGePoint3d(1000, posCur.y, 0));
			pLine->setEndPoint(AcGePoint3d(0, posCur.y, 0));
		}

		return Adesk::kTrue;
	}

	static void MyGroupJigHalfway()
	{
		AcDbXline *pBreakLine = new AcDbXline;
		pBreakLine->setBasePoint(AcGePoint3d(0, 0, 0));
		pBreakLine->setUnitDir(AcGeVector3d(0, 1, 0));
		AcDbObjectId id1;
		AppendToDatabase(id1, pBreakLine);
		pBreakLine->close();
		
		pLineLeft = new AcDbLine;
		AcDbObjectId id2;
		AppendToDatabase(id2, pLineLeft);
		pLineRight = new AcDbLine;
		AcDbObjectId id3;
		AppendToDatabase(id3, pLineRight);
		
		JigHelper jig;
		jig.SetBasePoint(AcGePoint3d(0, 0, 0));
		SetUpdateFunc(CSampleCadJigApp::UpdateJigHalfway, &jig);
		jig.startJig();

		pLineLeft->close();
		pLineRight->close();
	}
	///////////////////////////////////////////////////////////////////////
} ;

AcDbLine* CSampleCadJigApp::pLineLeft = NULL;
AcDbLine* CSampleCadJigApp::pLineRight = NULL;

//-----------------------------------------------------------------------------
IMPLEMENT_ARX_ENTRYPOINT(CSampleCadJigApp)

ACED_ARXCOMMAND_ENTRY_AUTO(CSampleCadJigApp, MyGroup, JigMove, JigMove, ACRX_CMD_MODAL | ACRX_CMD_USEPICKSET, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CSampleCadJigApp, MyGroup, JigArrow, JigArrow, ACRX_CMD_MODAL | ACRX_CMD_USEPICKSET, NULL)
ACED_ARXCOMMAND_ENTRY_AUTO(CSampleCadJigApp, MyGroup, JigHalfway, JigHalfway, ACRX_CMD_MODAL | ACRX_CMD_USEPICKSET, NULL)