#include "CTestWnd.h"

void CTestWnd::Test()
{
	using namespace eck;
	using namespace eck::Literals;

	GpRegion* gpr{},* gpr2{};
	CRegion r{};
	RECT rc[]
	{
		{100,100,200,200},
		{300,300,400,400},
	};
	r.SetRect(rc, ARRAYSIZE(rc));
	GpRect rcgp[]
	{
		{100,100,100,100},
		{300,300,100,100},
	};
	GdipCreateRegionRectI(rcgp, &gpr);
	auto gps = GdipCombineRegionRectI(gpr, rcgp + 1, Gdiplus::CombineModeUnion);

	std::vector<RECT> rcg{};
	r.GetRect(rcg);

	CRegion r2{};
	RECT rc2[]
	{
		{150,150,350,350},
		{360,360,450,450}
	};
	r2.SetRect(rc2, ARRAYSIZE(rc2));

	GpRect rcgp2[]
	{
		{150,150,200,200},
		{360,360,450 - 360,450 - 360},
	};
	GdipCreateRegionRectI(rcgp2, &gpr2);
	//gps = GdipCombineRegionRectI(gpr, rcgp2, Gdiplus::CombineModeUnion);
	gps = GdipCombineRegionRectI(gpr2, rcgp2 + 1, Gdiplus::CombineModeUnion);


	gps = GdipCombineRegionRegion(gpr, gpr2, Gdiplus::CombineModeXor);

	auto r3 = r.SymmetricDifference(r2);
	rcg.clear();
	r3.GetRect(rcg);

	auto hDC = m_DP.GetHDC();
	RandSeed(GetTickCount());
	for (auto& rc : rcg)
	{
		SetDCBrushColor(hDC, RGB(Rand(0, 255), Rand(0, 255), Rand(0, 255)));
		FrameRect(hDC, &rc, (HBRUSH)GetStockObject(DC_BRUSH));
		FillRect(hDC, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}
	GpMatrix* pMatrix{};
	GdipCreateMatrix(&pMatrix);
	UINT nCount{};
	gps = GdipGetRegionScansCount(gpr, &nCount, pMatrix);
	std::vector<GpRect> rgRects(nCount);
	GdipGetRegionScansI(gpr, rgRects.data(), (int*)&nCount, pMatrix);

	RandSeed(GetTickCount());
	for (auto& rc : rgRects)
	{
		SetDCBrushColor(hDC, RGB(Rand(0, 255), Rand(0, 255), Rand(0, 255)));
		RECT rc2 = { rc.X, rc.Y, rc.X + rc.Width, rc.Y + rc.Height };
		//FrameRect(hDC, &rc2, (HBRUSH)GetStockObject(DC_BRUSH));
		//FillRect(hDC, &rc2, (HBRUSH)GetStockObject(WHITE_BRUSH));
	}
	m_DP.Redraw();

	int aaaaaaaaa{};
}

LRESULT CTestWnd::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		m_DP.Create(nullptr, WS_CHILD | WS_VISIBLE, 0,
			0, 0, 900, 900, hWnd, 0);
		Test();
		break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}
