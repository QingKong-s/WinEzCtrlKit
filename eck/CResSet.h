#pragma once
#include "ECK.h"

#include <unordered_map>
#include <algorithm>

ECK_NAMESPACE_BEGIN
template<class TRes>
struct FStdResDeleter
{
	void operator()(TRes x)
	{

	}
};
template<class TRes, class TDeleter = FStdResDeleter<TRes>>
class CResSet
{
private:
	std::unordered_map<int, TRes> m_Record{};
	int m_cRes = 0;
public:
	~CResSet()
	{
		DestroyRes();
	}

	BOOL AddRes(int iID, TRes Res)
	{
		auto it = m_Record.find(iID);
		if (it == m_Record.end())
		{
			++m_cRes;
			it->second = Res;
			return TRUE;
		}
		else
		{
			TDeleter()(it->second);
			it->second = Res;
			return FALSE;
		}
	}

	EckInline TRes operator[](int iID)
	{
#ifdef _DEBUG
		if (!IsResExist(iID))
			EckDbgBreak();
#endif
		return m_Record[iID];
	}

	EckInline BOOL IsResExist(int iID)
	{
		return m_Record.find(iID) != m_Record.end();
	}

	BOOL DestroyRes(int iID)
	{
		auto it = m_Record.find(iID);
		if (it != m_Record.end())
		{
			--m_cRes;
			TDeleter()(it->second);
			m_Record.erase(it);
			return TRUE;
		}
		else
		{
			EckDbgPrintWithPos(L"资源不存在");
			return FALSE;
		}
	}

	void DestroyRes()
	{
		for (auto& x : m_Record)
		{
			TDeleter()(x.second);
		}
		m_Record.clear();
	}

	EckInline int GetResCount()
	{
		return m_cRes;
	}

	TRes Detach(int iID)
	{
		TRes Res{};
		auto it = m_Record.find(iID);
		--m_cRes;
#ifdef _DEBUG
		if (it == m_Record.end())
		{
			EckDbgPrintWithPos(L"未找到资源");
			EckDbgBreak();
		}
#endif
		Res = std::move(it->second);
		m_Record.erase(it);
		return Res;
	}
};

ECK_NAMESPACE_END