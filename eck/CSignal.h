#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct Intercept_T {};
struct NoIntercept_T {};

/// <summary>
/// 信号。
/// 应自行管理槽所关联对象与信号的生命周期
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
private:
	struct NODE
	{
#ifdef _DEBUG
		CSignal* pThis;
#endif
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
	using HSlot = const NODE*;
#define ECK_SIG_TOP		nullptr
#define ECK_SIG_BOTTOM	(HSlot(UINT_PTR(-1)))

	ECK_DISABLE_COPY_MOVE_DEF_CONS(CSignal);

	/// <summary>
	/// 发射
	/// </summary>
	/// <param name="...args">参数</param>
	/// <param name="bProcessed">是否被拦截</param>
	/// <returns>若某槽拦截信号，则返回该槽的返回值，否则返回返回值类型的默认构造结果</returns>
	TRet Emit2([[maybe_unused]] BOOL& bProcessed, TArgs ...args)
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
						pNode->fn(args..., bProcessed);
						if (bProcessed)
							return;
					}
					else
					{
						const auto r = pNode->fn(args..., bProcessed);
						if (bProcessed)
							return r;
					}
				}
				else
					pNode->fn(args...);
				--pNode->cEnter;
			} while (pNode = pNode->pNext);
			CleanupNow();
		}
		if constexpr (!std::is_same_v<TRet, void>)
			return {};
	}

	/// <summary>
	/// 发射
	/// </summary>
	/// <param name="...args">参数</param>
	/// <returns>若某槽拦截信号，则返回该槽的返回值，否则返回返回值类型的默认构造结果</returns>
	EckInline TRet Emit(TArgs ...args)
	{
		BOOL bProcessed{};
		return Emit2(bProcessed, args...);
	}
private:
	NODE* IntConnect(FProc&& fn, UINT_PTR uId, NODE* pAfter)
	{
#ifdef _DEBUG
		const auto pNew = new NODE{ this,uId,std::move(fn) };
#else
		const auto pNew = new NODE{ uId,std::move(fn) };
#endif
		if (m_pHead)
		{
			if (pAfter == ECK_SIG_TOP)
			{
				pNew->pNext = m_pHead;
				m_pHead = pNew;
			}
			else if (pAfter == ECK_SIG_BOTTOM)
			{
				auto p = m_pHead;
				while (p->pNext)
					p = p->pNext;
				p->pNext = pNew;
			}
			else
			{
				EckAssert(pAfter->pThis == this && !(pAfter->uFlags & NF_DELETED));
				pNew->pNext = pAfter->pNext;
				pAfter->pNext = pNew;
			}
		}
		else
		{
			EckAssert(pAfter == ECK_SIG_TOP || pAfter == ECK_SIG_BOTTOM);
			m_pHead = pNew;
		}
		return pNew;
	}

	template <class TCls, size_t... Index>
	NODE* IntConnect(TCls* pThis, FProcMethodPtr<TCls> pfnMethod,
		std::index_sequence<Index...>, UINT_PTR uId, NODE* pAfter)
	{
		return IntConnect(std::bind(pfnMethod, pThis, std::_Ph<Index + 1>{}...), uId, pAfter);
	}
public:
	template<class TProc>
	EckInline HSlot Connect(TProc&& fn, UINT_PTR uId = 0u, HSlot pAfter = ECK_SIG_TOP)
	{
		return IntConnect(fn, uId, (NODE*)pAfter);
	}

	template<class TCls>
	EckInline HSlot Connect(TCls* pThis, FProcMethodPtr<TCls> pfnMethod,
		UINT_PTR uId = 0u, HSlot pAfter = ECK_SIG_TOP)
	{
		if constexpr (std::is_same_v<TIntercept, Intercept_T>)
			return IntConnect(pThis, pfnMethod,
				std::make_index_sequence<sizeof...(TArgs) + 1>{}, uId, (NODE*)pAfter);
		else
			return IntConnect(pThis, pfnMethod,
				std::make_index_sequence<sizeof...(TArgs)>{}, uId, (NODE*)pAfter);
	}

	EckInline HSlot Connect(FProcPtr pfn, UINT_PTR uId = 0u, HSlot pAfter = ECK_SIG_TOP)
	{
		return IntConnect(pfn, uId, (NODE*)pAfter);
	}

	EckInline HSlot Connect(CSignal& sig, UINT_PTR uId = 0u, HSlot pAfter = ECK_SIG_TOP)
	{
		EckAssert(&sig != this);
		if constexpr (std::is_same_v<TIntercept, Intercept_T>)
			return IntConnect([&](TArgs ...args, BOOL& bProcessed)->TRet
				{
					if constexpr (std::is_same_v<TRet, void>)
						sig.Emit2(bProcessed, args...);
					else
					{
						const auto r = sig.Emit2(bProcessed, args...);
						if (bProcessed)
							return r;
					}
				}, uId, (NODE*)pAfter);
		else
			return IntConnect([&](TArgs ...args)->TRet
				{
					sig.Emit(args...);
				}, uId, (NODE*)pAfter);
	}

	HSlot FindSlot(UINT_PTR uId, HSlot pBegin = ECK_SIG_TOP) const
	{
		auto p = (NODE*)(pBegin ? pBegin : m_pHead);
		while (p)
		{
			if (p->uId == uId && !(p->uFlags & NF_DELETED))
				return p;
			p = p->pNext;
		}
		return nullptr;
	}

	void Disconnect(HSlot hSlot)
	{
		EckAssert(hSlot && hSlot->pThis == this && !(hSlot->uFlags & NF_DELETED));
		((NODE*)hSlot)->uFlags |= NF_DELETED;
		++m_cDeleted;
	}

	BOOL Disconnect(UINT_PTR uId)
	{
		const auto p = FindSlot(uId);
		if (p)
		{
			Disconnect(p);
			return TRUE;
		}
		return FALSE;
	}

	// 删除所有进入计数为0且标记为删除的节点
	void CleanupNow()
	{
		EckAssert(m_cDeleted >= 0);
		if (m_cDeleted)
		{
			NODE* pNode = m_pHead;
			NODE* pPrev{};
			do
			{
				if (pNode->cEnter == 0 && (pNode->uFlags & NF_DELETED))
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

	// 可能返回已标记为删除的节点
	HSlot GetHead() const { return m_pHead; }

	void Clear()
	{
		auto pNode = m_pHead;
		while (pNode)
		{
			const auto pNext = pNode->pNext;
			Disconnect(pNode);
			pNode = pNext;
		}
		CleanupNow();
	}
};
ECK_NAMESPACE_END