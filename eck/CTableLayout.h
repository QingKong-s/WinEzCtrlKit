/*
* WinEzCtrlKit Library
*
* CTableLayout.h ： 表格布局
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CLayoutBase.h"
#include "CArray.h"

ECK_NAMESPACE_BEGIN
enum class TlCellMode
{
	Fixed,
	Weight,
	Auto
};

class CTableLayout final :public CLayoutBase
{
public:
	ECK_RTTI(CTableLayout);
private:
	static constexpr int InvalidCellSize = INT_MIN;

	struct ITEM :public ITEMBASE
	{
		int idxRow;
		int idxCol;
		int cRowSpan;
		int cColSpan;

		ITEM() = default;
		constexpr ITEM(ILayout* pCtrl, const MARGINS& Margin, UINT uFlags, short cx, short cy,
			int idxRow, int idxCol, int nRowSpan, int nColSpan)
			: ITEMBASE{ pCtrl, Margin, uFlags, cx, cy }, idxRow{ idxRow }, idxCol{ idxCol },
			cRowSpan{ nRowSpan }, cColSpan{ nColSpan } {}
	};

	struct CELL
	{
		int x{};
		int y{};
		int cx{ InvalidCellSize };
		int cy{ InvalidCellSize };
		UINT nWeightH{};
		UINT nWeightV{};
	};

	std::vector<ITEM> m_vItem{};
	CArray<CELL> m_Table{};
	int m_cRow{}, m_cCol{};
	TlCellMode m_eCellSizeMode{ TlCellMode::Auto };

	void UpdateCellSize(const ITEM& e)
	{
		EckAssert(m_eCellSizeMode == TlCellMode::Auto && L"单元格尺寸必须活动");
		const int cx = e.cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth;
		const int cy = e.cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight;
		if (e.cRowSpan)
		{
			const auto d = cy / (e.cRowSpan + 1);
			for (int i = 0; i < e.cRowSpan; ++i)
			{
				auto& t = m_Table[e.idxRow + i][e.idxCol].Data().cy;
				t = std::max(t, d);
			}
		}
		else
		{
			auto& t = m_Table[e.idxRow][e.idxCol].Data().cy;
			t = std::max(t, cy);
		}

		if (e.cColSpan)
		{
			const auto d = cx / (e.cColSpan + 1);
			for (int i = 0; i < e.cColSpan; ++i)
			{
				auto& t = m_Table[e.idxRow][e.idxCol + i].Data().cx;
				t = std::max(t, d);
			}
		}
		else
		{
			auto& t = m_Table[e.idxRow][e.idxCol].Data().cx;
			t = std::max(t, (int)cx);
		}
	}
public:
	size_t Add(ILayout* pCtrl, int idxRow, int idxCol, const MARGINS& Mar = {}, UINT uFlags = 0,
		int nRowSpan = 0, int nColSpan = 0)
	{
		EckAssert(idxRow + nRowSpan < m_cRow && idxCol + nColSpan < m_cCol && L"超出表格范围");
		const auto s = pCtrl->LoGetSize();
		auto& e = m_vItem.emplace_back(pCtrl, Mar, uFlags, (short)s.first, (short)s.second,
			idxRow, idxCol, nRowSpan, nColSpan);
		if (m_eCellSizeMode == TlCellMode::Auto)
			UpdateCellSize(e);
		return m_vItem.size() - 1;
	}

	void LoOnDpiChanged(int iDpi) override
	{
		Refresh();
		for (auto& e : m_vItem)
		{
			ReCalcDpiSize(e, iDpi);
			e.pCtrl->LoOnDpiChanged(iDpi);
		}
		m_iDpi = iDpi;
	}

	void LoInitDpi(int iDpi) override
	{
		m_iDpi = iDpi;
		for (auto& e : m_vItem)
			e.pCtrl->LoInitDpi(iDpi);
	}

	void SetRowCol(int cRow, int cCol)
	{
		m_cRow = cRow;
		m_cCol = cCol;
		m_Table.ReDim(cRow, cCol);
	}

	CELL& GetCell(int idxRow, int idxCol)
	{
		return m_Table[idxRow][idxCol].Data();
	}

	void Clear() override
	{
		CLayoutBase::Clear();
		m_vItem.clear();
		m_Table.Reset();
		m_cRow = m_cCol = 0;
	}

	// 若单元格权重变化，或固定控件大小变化，则需要调用此函数刷新布局
	void Refresh() override
	{
		if (m_eCellSizeMode == TlCellMode::Weight)
		{
			UINT n;
			EckCounter(m_cRow, i)
			{
				n = 0u;
				EckCounter(m_cCol, j)
					n += m_Table[i][j].Data().nWeightH;
				EckCounter(m_cCol, j)
					m_Table[i][j].Data().cx = (int)(m_cx * m_Table[i][j].Data().nWeightH / n);
			}
			EckCounter(m_cCol, j)
			{
				n = 0u;
				EckCounter(m_cRow, i)
					n += m_Table[i][j].Data().nWeightV;
				EckCounter(m_cRow, i)
					m_Table[i][j].Data().cy = (int)(m_cy * m_Table[i][j].Data().nWeightV / n);
			}
		}
		else if (m_eCellSizeMode == TlCellMode::Auto)
		{
			for (auto& e : m_Table)
				e.cx = e.cy = InvalidCellSize;
			for (const auto& e : m_vItem)
				UpdateCellSize(e);
		}
	}

	// 单元格的尺寸修改后，需要调用此函数刷新
	void CommitTableMetrics()
	{
		EckCounter(m_cRow, i)
		{
			m_Table[i][0].Data().x = 0;
			if (i)
				m_Table[i][0].Data().y = m_Table[i - 1][0].Data().y + m_Table[i - 1][0].Data().cy;
		}
		EckCounter(m_cCol, i)
		{
			m_Table[0][i].Data().y = 0;
			if (i)
				m_Table[0][i].Data().x = m_Table[0][i - 1].Data().x + m_Table[0][i - 1].Data().cx;
		}
		for (int i = 1; i < m_cRow; ++i)
			for (int j = 1; j < m_cCol; ++j)
			{
				auto& t = m_Table[i][j].Data();
				t.y = m_Table[i - 1][j].Data().y + m_Table[i - 1][j].Data().cy;
				t.x = m_Table[i][j - 1].Data().x + m_Table[i][j - 1].Data().cx;
			}
		LoGetAppropriateSize(m_cx, m_cy);
	}

	void LoGetAppropriateSize(int& cx, int& cy) override
	{
		const auto& e = m_Table[m_cRow - 1][m_cCol - 1].Data();
		cx = e.x + e.cx;
		cy = e.y + e.cy;
	}

	void LoCommit() override
	{
		int x, y, cx, cy;
		if (m_eCellSizeMode == TlCellMode::Weight)
		{
			Refresh();
			CommitTableMetrics();
		}
		HDWP hDwp = PreArrange(m_vItem.size());
		for (const auto& e : m_vItem)
		{
			const auto& cell = m_Table[e.idxRow][e.idxCol].Data();
			CalcCtrlPosSize(e, { cell.x,cell.y,cell.cx,cell.cy },
				x, y, cx, cy);
			MoveCtrlPosSize(e, hDwp, x, y, cx, cy);
		}
		PostArrange(hDwp);
	}

	EckInline constexpr void SetCellMode(TlCellMode e) { m_eCellSizeMode = e; }

	EckInline constexpr TlCellMode GetCellMode() const { return m_eCellSizeMode; }

	// 取底层列表
	EckInline constexpr auto& GetList() { return m_vItem; }

	// 取底层表格
	EckInline constexpr auto& GetTable() { return m_Table; }
};
ECK_RTTI_IMPL_BASE_INLINE(CTableLayout, CLayoutBase);
ECK_NAMESPACE_END