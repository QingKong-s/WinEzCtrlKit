﻿/*
* WinEzCtrlKit Library
*
* CSignal.h ： 信号
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct Intercept_T {};
struct NoIntercept_T {};

/// <summary>
/// 信号。
/// 应自行管理槽所关联对象与信号的生命周期，每个槽带有一个ID，其中CSignal::Top=(UINT_PTR)-1保留
/// </summary>
/// <typeparam name="TIntercept">设为Intercept_T以指示信号可被拦截，在槽的尾部增加bProcessed参数指示是否拦截</typeparam>
/// <typeparam name="TRet">返回值类型</typeparam>
/// <typeparam name="...TArgs">参数类型列表</typeparam>
template<class TIntercept, class TRet, class ...TArgs>
class CSignal
{
public:
	using FProcPtr = std::conditional_t<
		std::is_same_v<TIntercept, Intercept_T>,
		TRet(*)(TArgs..., BOOL& bProcessed),
		TRet(*)(TArgs...)
	>;

	template<class TCls>
	using FProcMethodPtr = std::conditional_t<
		std::is_same_v<TIntercept, Intercept_T>,
		TRet(TCls::*)(TArgs..., BOOL& bProcessed),
		TRet(TCls::*)(TArgs...)
	>;

	using FProc = std::conditional_t<
		std::is_same_v<TIntercept, Intercept_T>,
		std::function<TRet(TArgs..., BOOL& bProcessed)>,
		std::function<TRet(TArgs...)>
	>;

	constexpr static UINT_PTR Top = (UINT_PTR)-1;
private:
	struct NODE
	{
		UINT_PTR uId;
		FProc fn;
		NODE* pNext;
		UINT uFlags;
		int cEnter;
	};

	enum :UINT
	{
		NF_DELETED = (1u << 0),
	};

	NODE* m_pHead{};
	int m_cDeleted{};
public:
	ECK_DISABLE_COPY_DEF_CONS(CSignal);

	/// <summary>
	/// 发射
	/// </summary>
	/// <param name="...args">参数</param>
	/// <returns>若某槽拦截信号，则返回该槽的返回值，否则返回返回值类型的默认构造结果</returns>
	TRet Emit(TArgs&& ...args)
	{
		if (m_pHead)
		{
			auto pNode = m_pHead;
			do
			{
				if (pNode->uFlags & NF_DELETED)
					continue;
				++pNode->cEnter;
				// CALL
				if constexpr (std::is_same_v<TIntercept, Intercept_T>)
				{
					if constexpr (std::is_same_v<TRet, void>)
					{
						BOOL bProcessed = FALSE;
						pNode->fn(std::forward<TArgs>(args)..., bProcessed);
						if (bProcessed)
							return;
					}
					else
					{
						BOOL bProcessed = FALSE;
						const auto lResult = pNode->fn(std::forward<TArgs>(args)..., bProcessed);
						if (bProcessed)
							return lResult;
					}
				}
				else
					pNode->fn(std::forward<TArgs>(args)...);
				--pNode->cEnter;
			} while (pNode = pNode->pNext);

			// 删除所有进入计数为0且标记为删除的节点
			EckAssert(m_cDeleted >= 0);
			if (m_cDeleted)
			{
				pNode = m_pHead;
				NODE* pPrev{};
				do
				{
					if (pNode->cEnter == 0 && pNode->uFlags & NF_DELETED)
					{
						if (pPrev)
							pPrev->pNext = pNode->pNext;
						else
							m_pHead = pNode->pNext;
						const auto pNext = pNode->pNext;
						delete pNode;
						pNode = pNext;
						--m_cDeleted;
					}
					else
					{
						pPrev = pNode;
						pNode = pNode->pNext;
					}
				} while (pNode);
			}
		}
		if constexpr (!std::is_same_v<TRet, void>)
			return {};
	}
private:
	NODE* FindEntry(UINT_PTR uId) const
	{
		auto p = m_pHead;
		while (p)
		{
			if (p->uId == uId)
				return p;
			p = p->pNext;
		}
		return NULL;
	}

	void IntConnect(FProc&& fn, UINT_PTR uId, UINT_PTR uIdAfter)
	{
		EckAssert(uId != Top);
		const auto pNew = new NODE{ uId,std::move(fn) };
		if (m_pHead)
		{
			if (uIdAfter == Top)
			{
				pNew->pNext = m_pHead;
				m_pHead = pNew;
			}
			else
			{
				const auto pAfter = FindEntry(uIdAfter);
				if (pAfter)
				{
					pNew->pNext = pAfter->pNext;
					pAfter->pNext = pNew;
				}
				else
				{
					pNew->pNext = m_pHead;
					m_pHead = pNew;
				}
			}
		}
		else
			m_pHead = pNew;
	}

	template <class TCls, size_t... Index>
	void IntConnect(TCls* pThis, FProcMethodPtr<TCls> pfnMethod, std::index_sequence<Index...>,
		UINT_PTR uId, UINT_PTR uIdAfter)
	{
		IntConnect(std::bind(pfnMethod, pThis, std::_Ph<Index + 1>{}...), uId, uIdAfter);
	}
public:
	template<class TProc>
	EckInline void Connect(TProc fn, UINT_PTR uId, UINT_PTR uIdAfter = Top)
	{
		IntConnect(fn, uId, uIdAfter);
	}

	template<class TCls>
	EckInline void Connect(TCls* pThis, FProcMethodPtr<TCls> pfnMethod, UINT_PTR uId, UINT_PTR uIdAfter = Top)
	{
		if constexpr (std::is_same_v<TIntercept, Intercept_T>)
			IntConnect(pThis, pfnMethod, std::make_index_sequence<sizeof...(TArgs) + 1>{}, uId, uIdAfter);
		else
			IntConnect(pThis, pfnMethod, std::make_index_sequence<sizeof...(TArgs)>{}, uId, uIdAfter);
	}

	EckInline void Connect(FProcPtr pfn, UINT_PTR uId, UINT_PTR uIdAfter = Top)
	{
		IntConnect(pfn, uId, uIdAfter);
	}

	EckInline void Connect(CCallbackTable& ct, UINT_PTR uId, UINT_PTR uIdAfter = Top)
	{
		EckAssert(&ct != this);
		if constexpr (std::is_same_v<TIntercept, Intercept_T>)
			IntConnect([&](TArgs&& ...args, BOOL& bProcessed)->TRet
				{
					return ct.Emit(std::forward<TArgs>(args)...);
				}, uId, uIdAfter);
		else
			IntConnect([&](TArgs&& ...args)->TRet
				{
					ct.Emit(std::forward<TArgs>(args)...);
				}, uId, uIdAfter);
	}

	BOOL Disconnect(UINT_PTR uId)
	{
		const auto p = FindEntry(uId);
		if (p)
		{
			p->uFlags |= NF_DELETED;
			++m_cDeleted;
			return TRUE;
		}
		return FALSE;
	}
};
ECK_NAMESPACE_END