/*
* WinEzCtrlKit Library
*
* CRelativeLayout.h ： 相对布局
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CLayoutBase.h"
#include "Utility.h"

#include <stack>

ECK_NAMESPACE_BEGIN
enum :UINT
{
	RLF_FIXWIDTH = 1u << 0,	// 固定宽度
	RLF_FIXHEIGHT = 1u << 1,// 固定高度
	RLF_FIXSIZE = RLF_FIXWIDTH | RLF_FIXHEIGHT,// 固定大小

	// 以下标志指示四边确定性，一般仅供内部使用

	RLF_DTM_L = 1u << 31,
	RLF_DTM_R = 1u << 30,
	RLF_DTM_T = 1u << 29,
	RLF_DTM_B = 1u << 28,

	RLF_DTM_ALL = RLF_DTM_L | RLF_DTM_R | RLF_DTM_T | RLF_DTM_B,
};

#define ECK_RL_PARENT ((ILayout*)((UINT_PTR)-1))

class CRelativeLayout : public CLayoutBase
{
private:
	struct ITEM
	{
		ILayout* pCtrl;
		MARGINS Margin;
		UINT uFlags;
		int cx;
		int cy;
		RECT rcTemp;
		ILayout* pLeftOf;
		ILayout* pRightOf;
		ILayout* pTopOf;
		ILayout* pBottomOf;

		ILayout* pLeftAlign;
		ILayout* pRightAlign;
		ILayout* pTopAlign;
		ILayout* pBottomAlign;
	};

	std::unordered_map<ILayout*, ITEM> m_hmItem{};
public:
	EckInline ITEM& Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0)
	{
		const auto [cx, cy] = pCtrl->LoGetSize();
		auto& e = m_hmItem[pCtrl];
		e.pCtrl = pCtrl;
		e.Margin = Margin;
		e.uFlags = uFlags;
		e.cx = cx;
		e.cy = cy;
		return e;
	}

	void LoCommit() override
	{
		int t;

		size_t cNoDetermined{ m_hmItem.size() };
		for (auto& [_, e] : m_hmItem)
		{
			e.rcTemp = { .right = e.cx,.bottom = e.cy };
			e.uFlags &= ~RLF_DTM_ALL;

			if (e.uFlags & RLF_FIXWIDTH)
			{
				if (!e.pLeftOf && !e.pRightAlign && !e.pRightOf && !e.pLeftAlign)
				{
					e.uFlags |= RLF_DTM_R;
					//e.rcTemp.left = e.Margin.cxLeftWidth;
					e.uFlags |= RLF_DTM_L;
					//e.rcTemp.right = m_cx - e.Margin.cxRightWidth;
				}
			}
			else
			{
				if (!e.pLeftOf && !e.pRightAlign)
				{
					e.uFlags |= RLF_DTM_R;
					e.rcTemp.left = e.Margin.cxLeftWidth;
				}
				if (!e.pRightOf && !e.pLeftAlign)
				{
					e.uFlags |= RLF_DTM_L;
					e.rcTemp.right = m_cx - e.Margin.cxRightWidth;
				}
			}

			if (e.uFlags & RLF_FIXHEIGHT)
			{
				if (!e.pTopOf && !e.pBottomAlign && !e.pBottomOf && !e.pTopAlign)
				{
					e.uFlags |= RLF_DTM_B;
					//e.rcTemp.top = e.Margin.cyTopHeight;
					e.uFlags |= RLF_DTM_T;
					//e.rcTemp.bottom = m_cy - e.Margin.cyBottomHeight;
				}
			}
			else
			{
				if (!e.pTopOf && !e.pBottomAlign)
				{
					e.uFlags |= RLF_DTM_B;
					e.rcTemp.top = e.Margin.cyTopHeight;
				}
				if (!e.pBottomOf && !e.pTopAlign)
				{
					e.uFlags |= RLF_DTM_T;
					e.rcTemp.bottom = m_cy - e.Margin.cyBottomHeight;
				}
			}

			if (IsBitSet(e.uFlags, RLF_DTM_ALL))
				--cNoDetermined;
		}

		EckCounterNV(50)
		{
			if (!cNoDetermined)
				break;

			for (auto& [_, e] : m_hmItem)
			{
				if (IsBitSet(e.uFlags, RLF_DTM_ALL))
					continue;

				if (!(e.uFlags & RLF_DTM_R))
				{
					if (e.pLeftOf)
					{
						EckAssert(e.pLeftOf != ECK_RL_PARENT);
						const auto it = m_hmItem.find(e.pLeftOf);
						if (it == m_hmItem.end())
							t = e.pLeftOf->LoGetPos().first;
						else
						{
							if (it->second.uFlags & RLF_DTM_L)
								t = it->second.rcTemp.left;
							else
								goto EndR;
						}
						e.rcTemp.right = t + e.Margin.cxLeftWidth;
						if (e.uFlags & RLF_FIXWIDTH)
							e.rcTemp.left = e.rcTemp.right - e.cx;
						e.uFlags |= RLF_DTM_R;
					}
					else if (e.pRightAlign)
					{
						if (e.pRightAlign == ECK_RL_PARENT)
							e.rcTemp.right = m_cx - e.Margin.cxRightWidth;
						else
						{
							const auto it = m_hmItem.find(e.pRightAlign);
							if (it == m_hmItem.end())
								e.rcTemp.right = e.pRightAlign->LoGetPos().first +
								e.pRightAlign->LoGetSize().first - e.Margin.cxRightWidth;
							else
							{
								if (it->second.uFlags & RLF_DTM_R)
									e.rcTemp.right = it->second.rcTemp.right - e.Margin.cxRightWidth;
								else
									goto EndR;
							}
						}
						if (e.uFlags & RLF_FIXWIDTH)
							e.rcTemp.left = e.rcTemp.right - e.cx;
						e.uFlags |= RLF_DTM_R;
					}
					else
					{
						if (e.uFlags & RLF_FIXWIDTH)
						{
							if (e.uFlags & RLF_DTM_L)
								e.uFlags |= RLF_DTM_R;
						}
						else
							e.uFlags |= RLF_DTM_R;
					}
				EndR:;
				}

				if (!(e.uFlags & RLF_DTM_L))
				{
					if (e.pRightOf)
					{
						EckAssert(e.pLeftOf != ECK_RL_PARENT);
						const auto it = m_hmItem.find(e.pRightOf);
						if (it == m_hmItem.end())
							t = e.pRightOf->LoGetPos().first + e.pRightOf->LoGetSize().first;
						else
						{
							if (it->second.uFlags & RLF_DTM_R)
								t = it->second.rcTemp.right;
							else
								goto EndL;
						}
						e.rcTemp.left = t + e.Margin.cxLeftWidth;
						if (e.uFlags & RLF_FIXWIDTH)
							e.rcTemp.right = e.rcTemp.left + e.cx;
						e.uFlags |= RLF_DTM_L;
					}
					else if (e.pLeftAlign)
					{
						if (e.pLeftAlign == ECK_RL_PARENT)
							e.rcTemp.left = e.Margin.cxLeftWidth;
						else
						{
							const auto it = m_hmItem.find(e.pLeftAlign);
							if (it == m_hmItem.end())
								e.rcTemp.left = e.pLeftAlign->LoGetPos().first + e.Margin.cxLeftWidth;
							else
							{
								if (it->second.uFlags & RLF_DTM_L)
									e.rcTemp.left = it->second.rcTemp.left + e.Margin.cxLeftWidth;
								else
									goto EndL;
							}
						}
						if (e.uFlags & RLF_FIXWIDTH)
							e.rcTemp.right = e.rcTemp.left + e.cx;
						e.uFlags |= RLF_DTM_L;
					}
					else
					{
						if (e.uFlags & RLF_FIXWIDTH)
						{
							if (e.uFlags & RLF_DTM_R)
								e.uFlags |= RLF_DTM_L;
						}
						else
							e.uFlags |= RLF_DTM_L;
					}
				EndL:;
				}

				if (!(e.uFlags & RLF_DTM_B))
				{
					//if (wcscmp(((CWnd*)e.pCtrl)->m_pszDbgTag, L"按钮3") == 0)
					//{
					//	int a{};
					//}
					if (e.pTopOf)
					{
						EckAssert(e.pLeftOf != ECK_RL_PARENT);
						const auto it = m_hmItem.find(e.pTopOf);
						if (it == m_hmItem.end())
							t = e.pTopOf->LoGetPos().second;
						else
						{
							if (it->second.uFlags & RLF_DTM_T)
								t = it->second.rcTemp.top;
							else
								goto EndB;
						}
						e.rcTemp.bottom = t - e.Margin.cyBottomHeight;
						if (e.uFlags & RLF_FIXHEIGHT)
							e.rcTemp.top = e.rcTemp.bottom - e.cy;
						e.uFlags |= RLF_DTM_B;
					}
					else if (e.pBottomAlign)
					{
						if (e.pBottomAlign == ECK_RL_PARENT)
							e.rcTemp.bottom = m_cy - e.Margin.cyBottomHeight;
						else
						{
							const auto it = m_hmItem.find(e.pBottomAlign);
							if (it == m_hmItem.end())
								e.rcTemp.bottom = e.pBottomAlign->LoGetPos().second +
								e.pBottomAlign->LoGetSize().second - e.Margin.cyBottomHeight;
							else
							{
								if (it->second.uFlags & RLF_DTM_B)
									e.rcTemp.bottom = it->second.rcTemp.bottom - e.Margin.cyBottomHeight;
								else
									goto EndB;
							}
						}
						if (e.uFlags & RLF_FIXHEIGHT)
							e.rcTemp.top = e.rcTemp.bottom - e.cy;
						e.uFlags |= RLF_DTM_B;
					}
					else
					{
						if (e.uFlags & RLF_FIXHEIGHT)
						{
							if(e.uFlags & RLF_DTM_T)
								e.uFlags |= RLF_DTM_B;
						}
						else
							e.uFlags |= RLF_DTM_B;
					}
				EndB:;
				}

				if (!(e.uFlags & RLF_DTM_T))
				{
					if (e.pBottomOf)
					{
						EckAssert(e.pLeftOf != ECK_RL_PARENT);
						const auto it = m_hmItem.find(e.pBottomOf);
						if (it == m_hmItem.end())
							t = e.pBottomOf->LoGetPos().second + e.pBottomOf->LoGetSize().second;
						else
						{
							if (it->second.uFlags & RLF_DTM_B)
								t = it->second.rcTemp.bottom;
							else
								goto EndT;
						}
						e.rcTemp.top = t + e.Margin.cyTopHeight;
						if (e.uFlags & RLF_FIXHEIGHT)
							e.rcTemp.bottom = e.rcTemp.top + e.cy;
						e.uFlags |= RLF_DTM_T;
					}
					else if (e.pTopAlign)
					{
						if (e.pTopAlign == ECK_RL_PARENT)
							e.rcTemp.top = e.Margin.cyTopHeight;
						else
						{
							const auto it = m_hmItem.find(e.pTopAlign);
							if (it == m_hmItem.end())
								e.rcTemp.top = e.pTopAlign->LoGetPos().second + e.Margin.cyTopHeight;
							else
							{
								if (it->second.uFlags & RLF_DTM_T)
									e.rcTemp.top = it->second.rcTemp.top + e.Margin.cyTopHeight;
								else
									goto EndT;
							}
						}
						if (e.uFlags & RLF_FIXHEIGHT)
							e.rcTemp.bottom = e.rcTemp.top + e.cy;
						e.uFlags |= RLF_DTM_T;
					}
					else
					{
						if (e.uFlags & RLF_FIXHEIGHT)
						{
							if (e.uFlags & RLF_DTM_B)
								e.uFlags |= RLF_DTM_T;
						}
						else
							e.uFlags |= RLF_DTM_T;
					}
				EndT:;
				}
				if (IsBitSet(e.uFlags, RLF_DTM_ALL))
					--cNoDetermined;
			}
		}

		if (cNoDetermined)
		{
			EckDbgPrintWithPos(L"无法确定相对布局的位置，可能存在循环依赖");
			EckDbgBreak();
			return;
		}

		HDWP hDwp = PreArrange(m_hmItem.size());
		for (const auto& [pCtrl, e] : m_hmItem)
		{
			const auto hWnd = pCtrl->LoGetHWND();
			if (hWnd)
				hDwp = DeferWindowPos(hDwp, hWnd, NULL, e.rcTemp.left, e.rcTemp.top,
					e.rcTemp.right - e.rcTemp.left, e.rcTemp.bottom - e.rcTemp.top,
					SWP_NOZORDER | SWP_NOACTIVATE);
			else
				pCtrl->LoSetPosSize(e.rcTemp.left, e.rcTemp.top,
					e.rcTemp.right - e.rcTemp.left, e.rcTemp.bottom - e.rcTemp.top);
		}
		PostArrange(hDwp);
	}
};
ECK_NAMESPACE_END