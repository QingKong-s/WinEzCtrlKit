#pragma once
#include "CResSet.h"
#include "ResStruct.h"
#include "ImageHelper.h"

ECK_NAMESPACE_BEGIN
#define ECK_RESTYPE_RESTABLE L"ECKRTRT"
struct CResTableEntry
{
	ResType uType;
	HANDLE hRes;
};

struct FResTableEntryDeleter
{
	void operator()(CResTableEntry& x)
	{
		switch (x.uType)
		{
		case ResType::Image:
		case ResType::Font:
			DeleteObject(x.hRes);
			break;
		case ResType::ImageList:
			ImageList_Destroy((HIMAGELIST)x.hRes);
			break;
		}
	}
};

using TTableResSet = CResSet<CResTableEntry, FResTableEntryDeleter>;

class CResTable
{
private:
	struct OFFSETENTRY
	{
		SIZE_T cbOffset;
		SIZE_T cbSize;
		ResType uType;
	};

	BYTE* m_pData = NULL;

	TTableResSet& m_ResSet;
	std::unordered_map<int, OFFSETENTRY> m_OffsetTable{};

	RESTABLEHEADER m_Header{};
public:
	CResTable() = delete;

	CResTable(TTableResSet& ResSet) :m_ResSet(ResSet)
	{

	}

	BOOL LoadTable(HINSTANCE hInstance, PCWSTR idRes, PCWSTR pszResType = ECK_RESTYPE_RESTABLE)
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
		RESTABLE_INDEXENTRY* pIndexEntry;
		EckCounter(m_Header.cResources, i)
		{
			r.SkipPointer(pIndexEntry);
			m_OffsetTable[pIndexEntry->iID] = { pIndexEntry->uOffset,pIndexEntry->cbSize,(ResType)pIndexEntry->eType };
		}
		return TRUE;
	}

	BOOL InstantiateRes(int iID)
	{
		auto it = m_OffsetTable.find(iID);
		if (it == m_OffsetTable.end())
			return FALSE;

		auto& ote = it->second;
		CResTableEntry e;
		e.uType = ote.uType;

		switch (ote.uType)
		{
		case ResType::Image:
			e.hRes = CreateHBITMAP(m_pData + ote.cbOffset, ote.cbSize);
			break;
		case ResType::ImageList:
		{
			IStream* pStream = SHCreateMemStream(m_pData + ote.cbOffset, ote.cbSize);
			e.hRes = ImageList_Read(pStream);
			pStream->Release();
		}
		break;
		case ResType::Font:
			assert(ote.cbSize == sizeof(LOGFONTW));// x86和x64下大小相同
			e.hRes = CreateFontIndirectW((const LOGFONTW*)(m_pData + ote.cbOffset));
			break;
		default:
			e.hRes = (HANDLE)(m_pData + ote.cbOffset);
			break;
		}

		m_ResSet.AddRes(iID, e);
		return TRUE;
	}

	BOOL InstantiateRes()
	{
		for (auto& x : m_OffsetTable)
		{
			InstantiateRes(x.first);
		}
		return TRUE;
	}

	PCBYTE GetResBin(int iID, SIZE_T* pcbRes)
	{
		auto it = m_OffsetTable.find(iID);
		if (it == m_OffsetTable.end())
			return NULL;
		*pcbRes = it->second.cbSize;
		return m_pData + it->second.cbOffset;
	}

	CRefBin GetResBin(int iID)
	{
		SIZE_T cb;
		auto p = GetResBin(iID, &cb);
		return CRefBin(p, cb);
	}
};
ECK_NAMESPACE_END