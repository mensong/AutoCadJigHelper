#include "StdAfx.h"
#include "JigHelper.h"
#include <AcString.h>

JigHelper::JigHelper()
	:m_bPat(false)
	, m_bCanecl(false)
{
	SetUpdateClassFunc(JigHelper::UpdateJig, this, this);
}

JigHelper::~JigHelper(void)
{
}

Adesk::Boolean CALLBACK JigHelper::UpdateJig(class JigHelper *pJigHelper, const AcGePoint3d &posCur, const AcGePoint3d &posLast)
{
	//1 确定矩阵
	AcGeVector3d vecMove = posCur - posLast;
	//2 更新位置
	for (int i = 0; i < m_DumyJigEnt.m_entArray.length(); i++)
	{
		AcDbEntity* pEnt = m_DumyJigEnt.getEnity(i);
		if (NULL == pEnt) 
			continue;
		pEnt->transformBy(vecMove);
	}

	return Adesk::kTrue;
}

AcEdJig::DragStatus JigHelper::sampler()
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

	if (m_bPat)
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

Adesk::Boolean JigHelper::update()
{
	Adesk::Boolean bRes = Adesk::kFalse;
	if (m_funcUpdateJig)
	{
		bRes = m_funcUpdateJig(this, m_posCur, m_posLast);
	}
	m_posLast = m_posCur;

	return bRes;
}

AcDbEntity *JigHelper::entity() const
{
	return (AcDbEntity *)&m_DumyJigEnt;
}

void JigHelper::RegisterAsJigEntity(AcDbEntity *pEnt)
{
	m_DumyJigEnt.addEntity(pEnt);
}

void JigHelper::RegisterAsJigEntity(std::vector<AcDbEntity *> &vctEnts)
{
	int nSize = vctEnts.size();
	for (int i = 0; i < nSize; ++i)
	{
		this->RegisterAsJigEntity(vctEnts[i]);
	}
}

JigHelper::RESULT JigHelper::startJig()
{
	acedGetAcadDwgView()->SetFocus();

	m_bCanecl = false;

	AcDbDatabase *pDbOld = acdbCurDwg();

	AcEdJig::DragStatus status = this->drag();

	if (acdbCurDwg() != pDbOld)
	{//在jig的过程中切换了DWG
		return JigHelper::RET_STOP;
	}
	else if (kNormal == status)
	{//选取点
		if (m_str[0] == '\0')
			return JigHelper::RET_POINT;

		//把字符串转为点
		ACHAR szAxis[3][30];		
		int nDim = 0;
		int nIdx = 0;
		int nLen = _tcslen(m_str);
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
				return JigHelper::RET_STRING;
			}
		}
		szAxis[nDim++][nIdx] = '\0';

		if (nDim < 2 || nDim > 3)
			return JigHelper::RET_STRING;

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

		return JigHelper::RET_POINT;
	}
	else if (kOther == status)
	{//输入字符串
		return JigHelper::RET_STRING;
	}

	return JigHelper::RET_CANCEL;
}

void JigHelper::UnregisterJigEntity(AcDbEntity *pEnt)
{
	m_DumyJigEnt.remove(pEnt);
}

void JigHelper::UnregisterJigEntity(std::vector<AcDbEntity *> &vctEnts)
{
	int nSize = vctEnts.size();
	for (int i = 0; i < nSize; ++i)
	{
		this->UnregisterJigEntity(vctEnts[i]);
	}
}

void JigHelper::UnregisterAllJigEntity()
{
	m_DumyJigEnt.removeAll();
}

void JigHelper::SetUpdateJigFunc(FUNC_UPDATE_JIG pFunc)
{
	m_funcUpdateJig = pFunc;
}

void JigHelper::SetBasePoint(const AcGePoint3d &ptOrigin)
{
	m_posLast = ptOrigin;
	m_posBase = ptOrigin;
}

//设置是否正交极轴
void JigHelper::SetPat(bool bPat)
{
	m_bPat = bPat;
}
