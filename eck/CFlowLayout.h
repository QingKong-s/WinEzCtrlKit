#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
enum
{
	// 固定宽度
	FLF_FIXWIDTH = 1u << 0,
	// 固定高度
	FLF_FIXHEIGHT = 1u << 1,
	// 在前方插入换行
	FLF_BREAKLINE = 1u << 2,
	// 对齐到所在行的顶边
	FLF_ALIGNTOP = 1u << 3,
	// 对齐到所在行的中间，这是默认值
	FLF_ALIGNCENTER = 0,
	// 对齐到所在行的底边
	FLF_ALIGNBOTTOM = 1u << 4,
};
class CFlowLayout
{
private:
	struct CTRL
	{
		CWnd* pWnd;
		UINT uFlags;
		RECT rcPos;
#if !ECKCXX20
		constexpr CTRL(CWnd* pWnd, UINT uFlags, RECT rcPos) noexcept
			: pWnd{ pWnd }, uFlags{ uFlags }, rcPos{ rcPos } {}
#endif
	};

	std::vector<CTRL> m_vCtrl{};

	MARGINS m_LineMargin{};
	int m_cxCtrlPadding = 0;
	int m_cyCtrlPadding = 0;
public:
	EckInline int Add(CWnd* pWnd, UINT uFlags = 0)
	{
		EckAssert(pWnd && pWnd->GetHWND());
		EckAssert(m_vCtrl.size() ?
			(GetParent(m_vCtrl.back().pWnd->GetHWND()) == GetParent(pWnd->GetHWND())) :
			TRUE);
		m_vCtrl.emplace_back(pWnd, uFlags, RECT{});
		return (int)m_vCtrl.size() - 1;
	}

	void Arrange(int cxClient, int cyClient)
	{
		const int cxMax = cxClient - m_LineMargin.cxRightWidth;
		HDWP hDwp = BeginDeferWindowPos((int)m_vCtrl.size());
		int y = m_LineMargin.cyTopHeight;
		int x = m_LineMargin.cxLeftWidth;
		int cxAppr, cyAppr;
		int cyLineMax = 0;
		int idxInLine = 0;
		EckCounter(m_vCtrl.size(), i)
		{
			auto& e = m_vCtrl[i];
			e.pWnd->LoGetAppropriateSize(cxAppr, cyAppr);
			if (IsBitSet(e.uFlags, FLF_FIXHEIGHT) || IsBitSet(e.uFlags, FLF_FIXWIDTH))
			{
				RECT rc;
				GetWindowRect(e.pWnd->GetHWND(), &rc);
				if (IsBitSet(e.uFlags, FLF_FIXHEIGHT))
					cyAppr = rc.bottom - rc.top;
				if (IsBitSet(e.uFlags, FLF_FIXWIDTH))
					cxAppr = rc.right - rc.left;
			}

			if (x + cxAppr <= cxMax || idxInLine == 0)
			{
				if (cyAppr > cyLineMax)
					cyLineMax = cyAppr;
				e.rcPos.left = x;
				e.rcPos.right = cxAppr;
				e.rcPos.bottom = cyAppr;
				++idxInLine;
				x += (cxAppr + m_cxCtrlPadding);
			}
			else// 开始新行
			{
				for (size_t j = i - idxInLine; j < i; ++j)
				{
					auto& f = m_vCtrl[j];
					f.rcPos.top = y + (cyLineMax - f.rcPos.bottom) / 2;
					hDwp = DeferWindowPos(hDwp, f.pWnd->GetHWND(), NULL,
						f.rcPos.left, f.rcPos.top, f.rcPos.right, f.rcPos.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
				}
				y += (m_LineMargin.cyTopHeight + cyLineMax);
				cyLineMax = 0;
				x = m_LineMargin.cxLeftWidth;
				idxInLine = 0;

				if (cyAppr > cyLineMax)
					cyLineMax = cyAppr;
				e.rcPos.left = x;
				e.rcPos.right = cxAppr;
				e.rcPos.bottom = cyAppr;
				++idxInLine;
				x += (cxAppr + m_cxCtrlPadding);
			}
		}
		if (idxInLine)
			for (size_t j = m_vCtrl.size() - idxInLine; j < m_vCtrl.size(); ++j)
			{
				auto& f = m_vCtrl[j];
				f.rcPos.top = y + (cyLineMax - f.rcPos.bottom) / 2;
				hDwp = DeferWindowPos(hDwp, f.pWnd->GetHWND(), NULL,
					f.rcPos.left, f.rcPos.top, f.rcPos.right, f.rcPos.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
			}
		EndDeferWindowPos(hDwp);
	}
};

ECK_NAMESPACE_END