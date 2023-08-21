/*
* WinEzCtrlKit Library
*
* CSubclassMgr.h ： 子类化管理器
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"
#include "WndHelper.h"

#include <CommCtrl.h>

#include <unordered_map>
#include <type_traits>
#include <algorithm>

ECK_NAMESPACE_BEGIN
template<class T>
concept CanCastToDwordPtr = requires{ std::is_convertible<T, DWORD_PTR>::value; };

template<class TId, class TObj>
class CObjRecorderOneToOne
{
public:
	using TRecord = std::unordered_map<TId, TObj>;
private:
	TRecord& m_Record;
public:
	CObjRecorderOneToOne() = delete;
	CObjRecorderOneToOne(TRecord& Record) :m_Record(Record)
	{

	}

	auto AddRecord(TId Id, TObj Obj, BOOL* pbCover = NULL)
	{
		auto ret = m_Record.insert(std::make_pair(Id, Obj));
#ifndef NDEBUG
		if (!ret.second)
			EckDbgPrint(L"对象记录器：添加对象记录时出现覆盖，位于：" __FUNCSIG__);
#endif // !NDEBUG
		if (pbCover)
			*pbCover = !ret.second;
		return ret.first;
	}

	BOOL DeleteRecord(TId Id)
	{
		auto it = m_Record.find(Id);
		if (it != m_Record.end())
		{
			m_Record.erase(it);
			return TRUE;
		}
		else
			return FALSE;
	}
};

template<class TObj>
void CALLBACK ObjRecordRefStdDeleter(TObj Obj)
{

}

template<class TId, class TObj>
class CObjRecorderRef
{
public:
	struct RefContext
	{
		int iRefCount = 1;
		TObj Obj{};
	};

	using TRecord = std::unordered_map<TId, RefContext>;
	using TDeleter = void(CALLBACK*)(TObj Obj);
private:
	TRecord& m_Record;
	TDeleter m_pfnDeleter = NULL;
public:
	CObjRecorderRef() = delete;
	CObjRecorderRef(TRecord& Record, TDeleter pfnDeleter) :m_Record(Record), m_pfnDeleter(pfnDeleter) { }

	auto AddRef(TId Id, TObj Obj, int* piRefCount = NULL)
	{
		auto it = m_Record.find(Id);
		if (it == m_Record.end())
		{
			it = m_Record.insert(std::make_pair(Id, RefContext{ 1,Obj })).first;
			if (piRefCount)
				*piRefCount = 1;
			return it;
		}
		else
		{
			auto& Ctx = it->second;
			++Ctx.iRefCount;
			if (piRefCount)
				*piRefCount = Ctx.iRefCount;
			return it;
		}
	}

	int Release(TId Id)
	{
		auto it = m_Record.find(Id);
		if (it != m_Record.end())
		{
			auto& Ctx = it->second;
			--Ctx.iRefCount;
			if (!Ctx.iRefCount)
			{
				m_pfnDeleter((*it).second.Obj);
				m_Record.erase(it);
				return 0;
			}
			else
				return Ctx.iRefCount;
		}
		else
		{
			EckDbgPrint(L"引用对象记录器：减少引用时未找到记录   " __FUNCSIG__);
			return -1;
		}
	}

	BOOL DeleteRecord(TId Id)
	{
		auto it = m_Record.find(Id);
		if (it != m_Record.end())
		{
			m_pfnDeleter((*it).second.Obj);
			m_Record.erase(it);
			return TRUE;
		}
		else
			return FALSE;
	}

	void SetDeleter(TDeleter pfnDeleter)
	{
		m_pfnDeleter = pfnDeleter;
	}
};

using ObjRecorderRefPlaceholder = int;
inline constexpr int ObjRecorderRefPlaceholderVal = 0;

template<class TObj>
using CWndRecorder = class CObjRecorderOneToOne<HWND, TObj>;

template<class TObj>
using CWndRecorderRef = class CObjRecorderRef<HWND, TObj>;

template<CanCastToDwordPtr TObj>
class CSubclassMgr
{
public:
	using TRecorder = CObjRecorderOneToOne<HWND, TObj>;
private:
	
	TRecorder m_Record;
	SUBCLASSPROC m_pfnSubclass;
	UINT_PTR m_uSubclassID;
public:
	CSubclassMgr() = delete;
	CSubclassMgr(TRecorder::TRecord& Record, SUBCLASSPROC pfnSubclass, UINT_PTR uSubclassID) :
		m_Record(Record), m_pfnSubclass(pfnSubclass), m_uSubclassID(uSubclassID) {}

	~CSubclassMgr() = default;

	auto AddSubclass(HWND hWnd, TObj Obj, BOOL* pbCover=NULL)
	{
		auto it = m_Record.AddRecord(hWnd, Obj, pbCover);
		SetWindowSubclass(hWnd, m_pfnSubclass, m_uSubclassID, (DWORD_PTR)Obj);
		return it;
	}

	BOOL RemoveSubclass(HWND hWnd)
	{
		if (m_Record.DeleteRecord(hWnd))
		{
			RemoveWindowSubclass(hWnd, m_pfnSubclass, m_uSubclassID);
			return TRUE;
		}
		else
			return FALSE;
	}

	CObjRecorderOneToOne<HWND, TObj>& GetRecorder()
	{
		return m_Record;
	}
};

template<CanCastToDwordPtr TObj>
class CSubclassMgrRef
{
public:
	using TRefRecorder = CObjRecorderRef<HWND, TObj>;
private:
	TRefRecorder m_Recorder;
	SUBCLASSPROC m_pfnSubclass;
	UINT_PTR m_uSubclassID;
public:
	CSubclassMgrRef() = delete;
	CSubclassMgrRef(TRefRecorder::TRecord& Record, SUBCLASSPROC pfnSubclass, UINT_PTR uSubclassID, TRefRecorder::TDeleter pfnDeleter) :
		m_Recorder(Record, pfnDeleter), m_pfnSubclass(pfnSubclass), m_uSubclassID(uSubclassID) {}

	auto AddRef(HWND hWnd, TObj Obj, int* piRefCount = NULL)
	{
		int iRefCount;
		auto it = m_Recorder.AddRef(hWnd, Obj, &iRefCount);
		if (iRefCount == 1)
			SetWindowSubclass(hWnd, m_pfnSubclass, m_uSubclassID, (DWORD_PTR)Obj);
		if (piRefCount)
			*piRefCount = iRefCount;
		return it;
	}

	int Release(HWND hWnd)
	{
		int iRefCount = m_Recorder.Release(hWnd);
		if(!iRefCount)
			RemoveWindowSubclass(hWnd, m_pfnSubclass, m_uSubclassID);
		return iRefCount;
	}

	BOOL DeleteRecord(HWND hWnd)
	{
		RemoveWindowSubclass(hWnd, m_pfnSubclass, m_uSubclassID);
		return m_Recorder.DeleteRecord(hWnd);
	}
};
ECK_NAMESPACE_END