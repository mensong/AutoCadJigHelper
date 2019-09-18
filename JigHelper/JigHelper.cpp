#include "StdAfx.h"
#include "JigHelper.h"
#include <AcString.h>
#include <tchar.h>

#ifndef _ttof
#ifdef UNICODE
#define _ttof _wtof
#else
#define _ttof atof
#endif
#endif

CJigHelper::CJigHelper()
	:m_bPat(false)
	, m_bCanecl(false)
	, m_bHasSetBasePos(false)
	, m_pUserData(NULL)
{
	SetUpdateFunc(CJigHelper::UpdateJig, this, this);
}

CJigHelper::~CJigHelper(void)
{
}

Adesk::Boolean CALLBACK CJigHelper::UpdateJig(class CJigHelper *pJigHelper, const AcGePoint3d &posCur, const AcGePoint3d &posLast, void* pUserData)
{
	CJigHelper* pThis = (CJigHelper*)pUserData;
	if (!pThis)
		return Adesk::kFalse;

	//1 确定矩阵
	AcGeVector3d vecMove = posCur - posLast;
	//2 更新位置
	for (int i = 0; i < pThis->m_DumyJigEnt.m_entArray.length(); i++)
	{
		AcDbEntity* pEnt = pThis->m_DumyJigEnt.getEnity(i);
		if (NULL == pEnt) 
			continue;
		pEnt->transformBy(vecMove);
	}

	return Adesk::kTrue;
}

AcEdJig::DragStatus CJigHelper::sampler()
{
	if (m_bCanecl)
	{
		return AcEdJig::kCancel;
	}

	setUserInputControls((UserInputControls)(
		AcEdJig::kAccept3dCoordinates
		| AcEdJig::kNullResponseAccepted
		| AcEdJig::kNoNegativeResponseAccepted
		| AcEdJig::kNoZeroResponseAccepted
		| AcEdJig::kAcceptOtherInputString
		));

	AcEdJig::DragStatus status = AcEdJig::kCancel;
	static AcGePoint3d ptTemp;

	if (m_bPat && m_bHasSetBasePos)
	{
		status = this->acquirePoint(m_posCur, m_posBase);
	}
	else
	{
		status = this->acquirePoint(m_posCur);
	}

	this->acquireString(m_str);
	AcString str = m_str;
	if (str == _T("\n") || str == _T(" ") || str == _T("\r\n"))
		m_str[0] = '\0';
	else
		m_str[_tcslen(m_str)] = '\0';

	if (ptTemp != m_posCur)
	{
		ptTemp = m_posCur;
	}
	else if (AcEdJig::kNormal == status)
	{
		return AcEdJig::kNoChange;
	}

	return (status);
}

Adesk::Boolean CJigHelper::update()
{
	Adesk::Boolean bRes = Adesk::kFalse;
	if (m_funcUpdateJig)
	{
		bRes = m_funcUpdateJig(this, m_posCur, m_posLast, m_pUserData);
	}
	m_posLast = m_posCur;

	return bRes;
}

AcDbEntity *CJigHelper::entity() const
{
	return (AcDbEntity *)&m_DumyJigEnt;
}

void CJigHelper::RegisterAsJigEntity(AcDbEntity *pEnt)
{
	m_DumyJigEnt.addEntity(pEnt);
}

void CJigHelper::RegisterAsJigEntity(std::vector<AcDbEntity *> &vctEnts)
{
	int nSize = (int)vctEnts.size();
	for (int i = 0; i < nSize; ++i)
	{
		this->RegisterAsJigEntity(vctEnts[i]);
	}
}

CJigHelper::RESULT CJigHelper::startJig()
{
	acedGetAcadDwgView()->SetFocus();

	m_bCanecl = false;

	AcDbDatabase *pDbOld = acdbCurDwg();

	AcEdJig::DragStatus status = this->drag();

	if (acdbCurDwg() != pDbOld)
	{//在jig的过程中切换了DWG
		return CJigHelper::RET_STOP;
	}
	else if (kNormal == status)
	{//选取点
		if (m_str[0] == '\0')
			return CJigHelper::RET_POINT;

		//把字符串转为点
		ACHAR szAxis[3][30];		
		int nDim = 0;
		int nIdx = 0;
		int nLen = (int)_tcslen(m_str);
		for (int i = 0; i < nLen; ++i)
		{
			if (('0' <= m_str[i] && '9' >= m_str[i]) || m_str[i] == '.')
			{
				szAxis[nDim][nIdx++] = m_str[i];
			}
			else if ((m_str[i] == ',' || m_str[i] == '\0') && i != 0)
			{
				szAxis[nDim++][nIdx] = '\0';
				nIdx = 0;
			}
			else
			{
				return CJigHelper::RET_STRING;
			}
		}
		szAxis[nDim++][nIdx] = '\0';

		if (nDim < 2 || nDim > 3)
			return CJigHelper::RET_STRING;

		if (nDim == 2)
			m_posCur.z = 0;
		for (int i = 0; i < nDim; ++i)
		{
			if (i == 0)
				m_posCur.x = _ttof(szAxis[i]);
			else if (i == 1)
				m_posCur.y = _ttof(szAxis[i]);
			else if (i == 2)
				m_posCur.z = _ttof(szAxis[i]);
		}

		return CJigHelper::RET_POINT;
	}
	else if (kOther == status)
	{//输入字符串
		return CJigHelper::RET_STRING;
	}

	return CJigHelper::RET_CANCEL;
}

void CJigHelper::UnregisterJigEntity(AcDbEntity *pEnt)
{
	m_DumyJigEnt.remove(pEnt);
}

void CJigHelper::UnregisterJigEntity(std::vector<AcDbEntity *> &vctEnts)
{
	int nSize = (int)vctEnts.size();
	for (int i = 0; i < nSize; ++i)
	{
		this->UnregisterJigEntity(vctEnts[i]);
	}
}

void CJigHelper::UnregisterAllJigEntity()
{
	m_DumyJigEnt.removeAll();
}

void CJigHelper::SetUpdateJigFunc(FUNC_UPDATE_JIG pFunc, void* pUserData)
{
	m_funcUpdateJig = pFunc;
	m_pUserData = pUserData;
}

void CJigHelper::SetBasePoint(const AcGePoint3d &ptOrigin)
{
	m_posLast = ptOrigin;
	m_posBase = ptOrigin;
	m_bHasSetBasePos = true;
}


void CJigHelper::UnSetBasePoint()
{
	m_bHasSetBasePos = false;
}

//设置是否正交极轴
void CJigHelper::SetPat(bool bPat)
{
	m_bPat = bPat;
}
