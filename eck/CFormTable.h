#pragma once
#include "ECK.h"
#include "ResStruct.h"

#include <unordered_map>

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

HWND LoadForm(PCBYTE pData)
{
	CMemReader r(pData);
	FTCTRLDATA* pCtrlData;
	r.SkipPointer(pCtrlData);
	HWND hWnd = CreateWindowExW(0, MAKEINTATOMW(32770), NULL, WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, SW_SHOW, NULL, NULL, NULL, NULL);

}
ECK_NAMESPACE_END