#pragma once
#include "ECK.h"
#include "ResStruct.h"
#include "DesignerDef.h"

#include <unordered_map>
#include <stack>

ECK_NAMESPACE_BEGIN
#define ECK_RESTYPE_FORMTABLE L"ECKRTFT"
class CFormTable
{
private:
	struct OFFSETENTRY
	{
		SIZE_T cbOffset;
		SIZE_T cbSize;
	};

	BYTE* m_pData = NULL;

	std::unordered_map<int, OFFSETENTRY> m_OffsetTable{};

	FORMTABLEHEADER m_Header{};
public:
	BOOL LoadTable(HINSTANCE hInstance, PCWSTR idRes, PCWSTR pszResType = ECK_RESTYPE_FORMTABLE)
	{
		HRSRC hResInfo = FindResourceW(hInstance, idRes, pszResType);
		if (!hResInfo)
			return FALSE;
		HGLOBAL hRes = LoadResource(hInstance, hResInfo);
		if (!hRes)
			return FALSE;
		void* p = LockResource(hRes);
		if (!LoadTable(p))
			return FALSE;
	}

	BOOL LoadTable(PCVOID pData)
	{
		m_pData = (BYTE*)pData;
		CMemReader r(pData);

		r >> m_Header;
		if (m_Header.iVer != DATAVER_RESTABLE_1)
			return FALSE;
		FORMTABLE_INDEXENTRY* pIndexEntry;
		EckCounter(m_Header.cForms, i)
		{
			r.SkipPointer(pIndexEntry);
			m_OffsetTable[pIndexEntry->iID] = { pIndexEntry->uOffset,pIndexEntry->cbSize };
		}
		return TRUE;
	}

	PCBYTE GetFormBin(int iID, SIZE_T* pcbRes)
	{
		auto it = m_OffsetTable.find(iID);
		if (it == m_OffsetTable.end())
			return NULL;
		*pcbRes = it->second.cbSize;
		return m_pData + it->second.cbOffset;
	}

	CRefBin GetFormBin(int iID)
	{
		CRefBin rb;
		SIZE_T cb;
		auto p = GetFormBin(iID, &cb);
		if (!p || !cb)
			return rb;
		rb.DupStream(p, cb);
		return rb;
	}
};

class CFormTableSerializer
{
private:
	struct DATAENTRY
	{
		int iID;
		PCVOID pData;
		SIZE_T cbSize;
	};

	std::vector<DATAENTRY> m_aData{};
	SIZE_T m_cbTotal = sizeof(FORMTABLEHEADER);
public:
	void AddFormData(int iID, PCVOID pData, SIZE_T cbSize)
	{
		m_aData.emplace_back(iID, pData, cbSize);
		m_cbTotal += (cbSize + sizeof(FORMTABLE_INDEXENTRY));
	}

	CRefBin Serialize()
	{
		CRefBin rb(m_cbTotal);
		CMemWriter w(rb, m_cbTotal);
		int cForms = (int)m_aData.size();

		FORMTABLEHEADER* pHeader;
		w.SkipPointer(pHeader);
		pHeader->iVer = DATAVER_FORMTABLE_1;
		pHeader->cForms = cForms;

		FORMTABLE_INDEXENTRY* pEntry;
		SIZE_T ocb = sizeof(FORMTABLEHEADER) + sizeof(FORMTABLE_INDEXENTRY) * cForms;

		EckCounter(cForms, i)
		{
			auto& Info = m_aData[i];
			w.SkipPointer(pEntry);
			pEntry->cbSize = (UINT)Info.cbSize;
			pEntry->iID = Info.iID;
			pEntry->uOffset = ocb;

			EckDbgCheckMemRange(rb.Data(), m_cbTotal, rb.Data() + ocb);
			memcpy(rb.Data() + ocb, Info.pData, Info.cbSize);

			ocb += m_aData[i].cbSize;
		}
		return rb;
	}
};

class CCtrlsDeserializer
{
private:
	PCBYTE m_pData = NULL;
	const FTCTRLDATAHEADER* m_pHeader = NULL;
	CMemReader m_r{ NULL };

	struct PARENTINFO
	{
		int cChildren;
		int j;
		HWND hParent;
	};
	std::stack<PARENTINFO> m_stackParentInfo{};

	HWND m_hParent = NULL;
public:
	CCtrlsDeserializer() = default;
	CCtrlsDeserializer(PCVOID pData, SIZE_T cbMax = 0u)
	{
		SetOrgData(pData, cbMax);
	}

	const FTCTRLDATAHEADER* SetOrgData(PCVOID pData, SIZE_T cbMax = 0u)
	{
		m_pData = (PCBYTE)pData;
		m_r.SetPtr(m_pData, cbMax);
		m_r.SkipPointer(m_pHeader);
		return m_pHeader;
	}

	/// <summary>
	/// �����ؼ�
	/// </summary>
	/// <param name="Processor">��������HWND Processor(const FTCTRLDATA* p, PCBYTE pCtrlData, HWND hParent)��
	/// ����������֤�����´����ĵ�ǰ�ؼ��������hParentΪ��ǰ�ؼ�Ӧ�����ĸ����ڣ�ΪNULL���������Ϊ�״��ڣ�
	/// �������������κ���Ч��ֵ����Ӧ����hParent����</param>
	template<class TProcessor>
	void For(TProcessor Processor)
	{
		const FTCTRLDATA* p;
		PCBYTE pCtrlData;
		HWND hWnd;

		EckCounter(m_pHeader->cCtrls, i)
		{
			m_r.SkipPointer(p);
			pCtrlData = (PCBYTE)m_r;

			if (m_stackParentInfo.size())
			{
				auto& Top = m_stackParentInfo.top();
				m_hParent = Top.hParent;
				++Top.j;
				if (Top.j == Top.cChildren)
					m_stackParentInfo.pop();
			}
			else
				m_hParent = NULL;

			hWnd = Processor(p, pCtrlData, m_hParent);
			if (p->cChildren)
				m_stackParentInfo.push({ p->cChildren,0,hWnd });
			m_r += p->cbData;
		}
		assert(m_stackParentInfo.size() == 0u);
	}

	const FTCTRLDATAHEADER* GetHeader()
	{
		return m_pHeader;
	}
};

/// <summary>
/// �������������ؼ�
/// </summary>
/// <param name="idxClass">������</param>
/// <param name="pszText">���⣬��pData��ΪNULL������Դ˲���</param>
/// <param name="dwStyle">��ʽ����pData��ΪNULL������Դ˲���</param>
/// <param name="dwExStyle">��չ��ʽ����pData��ΪNULL������Դ˲���</param>
/// <param name="x">x</param>
/// <param name="y">y</param>
/// <param name="cx">���</param>
/// <param name="cy">�߶�</param>
/// <param name="hParent">������</param>
/// <param name="nID">�ؼ�ID</param>
/// <param name="pData">����</param>
/// <returns>�ɹ�����CWndָ�룬�����߸���ʹ��delete�ͷţ�ʧ�ܷ���NULL</returns>
EckInline CWnd* CommCreateCtrl(int idxClass, PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
{
	if (idxClass < 0 || idxClass >= ARRAYSIZE(s_EckDesignAllCtrl))
	{
		EckDbgPrintWithPos(L"������������Χ");
		EckDbgBreak();
		return NULL;
	}

	auto pWnd = s_EckDesignAllCtrl[idxClass].pfnCreate((PCBYTE)pData, pszText, dwStyle, dwExStyle,
		x, y, cx, cy, hParent, nID);
	return pWnd;
}

/// <summary>
/// ���봰�塣
/// ������ָ���״����ϴ������������м�¼�����пؼ�
/// </summary>
/// <param name="hBK">Ҫ�����ϴ����ؼ��Ĵ���</param>
/// <param name="pFormData">�������ݣ���FTCTRLDATAHEADER��ͷ</param>
/// <param name="pWnds">ָ��vector������ָ�룬������������ָ�밴����װ�ص������У���ΪNULL�򲻱���ָ��</param>
/// <returns>�ɹ�����TRUE��ʧ�ܷ���FALSE</returns>
EckInline BOOL LoadForm(HWND hBK, PCVOID pFormData, std::vector<CWnd*>* pWnds = NULL)
{
	int iDpi = GetDpi(hBK);
	CCtrlsDeserializer Deserializer(pFormData);
	if (pWnds)
	{
		pWnds->clear();
		pWnds->reserve(Deserializer.GetHeader()->cCtrls);
	}

	Deserializer.For([=](const FTCTRLDATA* p, PCBYTE pCtrlData, HWND hParent)->HWND
		{
			if (!hParent)
				hParent = hBK;
			RECT rc = p->rc;
			DpiScale(rc, iDpi);
			auto pWnd = CommCreateCtrl(p->idxInfo, NULL, 0, 0,
				rc.left, rc.top, rc.right, rc.bottom, hParent, 0, pCtrlData);
			HWND hWnd;
			if (pWnds)
			{
				hWnd = pWnd->GetHWND();
				pWnds->push_back(pWnd);
			}
			else
			{
				hWnd = pWnd->Manage(CWnd::ManageOp::Detach, NULL);
				delete pWnd;
			}
			return hWnd;
		});
	return TRUE;
}
ECK_NAMESPACE_END