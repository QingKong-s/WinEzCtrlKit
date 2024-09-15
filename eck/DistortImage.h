#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CMifptpHBITMAP
{
private:
	HBITMAP m_hBitmap = nullptr;
	BITMAP m_Bmp{};
public:
	using TColor = COLORREF;
	using TColorComp = BYTE;
	using TCoord = POINT;

	CMifptpHBITMAP() = default;
	explicit CMifptpHBITMAP(HBITMAP hBitmap) :m_hBitmap{ hBitmap } { GetObjectW(hBitmap, sizeof(m_Bmp), &m_Bmp); }

	EckInline constexpr static TCoord MakeCoord(int x, int y) { return { x,y }; }

	EckInline constexpr static BYTE GetColorComp(COLORREF cr, int k) { return ((BYTE*)&cr)[k]; }

	EckInline constexpr static TColor MakeColor(const TColorComp(&Comp)[4])
	{
		return RGB(Comp[0], Comp[1], Comp[2]) | (Comp[3] << 24);
	}

	CMifptpHBITMAP New(TCoord Dimension) const
	{
		BITMAPINFO bmi{};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = Dimension.x;
		bmi.bmiHeader.biHeight = -Dimension.y;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
#pragma warning (suppress:6387)// ����ΪNULL
		const HBITMAP hbm = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, nullptr, nullptr, 0);
		return CMifptpHBITMAP(hbm);
	}

	EckInline TColor GetPixel(TCoord pt) const
	{
		return *(((TColor*)(((BYTE*)m_Bmp.bmBits) + pt.y * m_Bmp.bmWidthBytes)) + pt.x);
	}

	EckInline void SetPixel(TCoord pt, TColor cr)
	{
		*(((TColor*)(((BYTE*)m_Bmp.bmBits) + pt.y * m_Bmp.bmWidthBytes)) + pt.x) = cr;
	}

	EckInline int GetWidth() const { return m_Bmp.bmWidth; }

	EckInline int GetHeight() const { return m_Bmp.bmHeight; }

	EckInline HBITMAP GetHBITMAP() const { return m_hBitmap; }

	EckInline void Lock() const {}
	EckInline void UnLock() const {}

	EckInline constexpr static int GetX(TCoord c) { return c.x; }
	EckInline constexpr static int GetY(TCoord c) { return c.y; }
};

class CMifptpGpBitmap
{
private:
	GpBitmap* m_pBitmap = nullptr;
	Gdiplus::BitmapData m_Data{};
	int m_cx = 0,
		m_cy = 0;
public:
	using TColor = ARGB;
	using TColorComp = BYTE;
	using TCoord = GpPoint;

	CMifptpGpBitmap() = default;
	explicit CMifptpGpBitmap(GpBitmap* pBitmap) :m_pBitmap{ pBitmap }
	{
		GdipGetImageWidth(pBitmap, (UINT*)&m_cx);
		GdipGetImageHeight(pBitmap, (UINT*)&m_cy);
	}

	EckInline static TCoord MakeCoord(int x, int y) { return { x,y }; }

	EckInline constexpr static TColorComp GetColorComp(TColor cr, int k) { return ((BYTE*)&cr)[k]; }

	EckInline constexpr static TColor MakeColor(const TColorComp(&Comp)[4])
	{
		return Comp[0] | (Comp[1] << 8) | (Comp[2] << 16) | (Comp[3] << 24);
	}

	CMifptpGpBitmap New(TCoord Dimension) const
	{
		GpBitmap* pBitmap;
		GdipCreateBitmapFromScan0(Dimension.X, Dimension.Y, 0, PixelFormat32bppARGB, nullptr, &pBitmap);
		return CMifptpGpBitmap(pBitmap);
	}

	EckInline TColor GetPixel(TCoord pt) const
	{
		return *(((TColor*)(((BYTE*)m_Data.Scan0) + pt.Y * m_Data.Stride)) + pt.X);
	}

	EckInline void SetPixel(TCoord pt, TColor cr)
	{
		*(((TColor*)(((BYTE*)m_Data.Scan0) + pt.Y * m_Data.Stride)) + pt.X) = cr;
	}

	EckInline int GetWidth() const { return m_cx; }

	EckInline int GetHeight() const { return m_cy; }

	EckInline GpBitmap* GetGpBitmap() const { return m_pBitmap; }

	EckInline void Lock()
	{
		const GpRect rc{ 0,0,m_cx,m_cy };
		GdipBitmapLockBits(m_pBitmap, &rc, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
			PixelFormat32bppPARGB, &m_Data);
	}

	EckInline void UnLock()
	{
		GdipBitmapUnlockBits(m_pBitmap, &m_Data);
	}

	EckInline static int GetX(TCoord c) { return c.X; }
	EckInline static int GetY(TCoord c) { return c.Y; }
};

/// <summary>
/// ����Ť��ͼ��
/// ���ɴ�ԭͼ���ϵĶ��������ӳ�䵽Ŀ�����������Ť��ͼ�񣬺���������ε���Ӿ�������λͼ��(0,0)����
/// </summary>
/// <param name="Bmp">����λͼ������</param>
/// <param name="NewBmp">���λͼ��������Ӧ�����ͷ������Դ</param>
/// <param name="pptSrc">Դ���������</param>
/// <param name="pptDst">Ŀ����������</param>
/// <param name="cPt">������</param>
/// <returns></returns>
template<class TBmpHandler, class TCoord>
inline BOOL MakeImageFromPolygonToPolygon(TBmpHandler& Bmp, TBmpHandler& NewBmp,
	const TCoord* pptSrc, const TCoord* pptDst, int cPt)
{
	static_assert(std::is_same_v<TCoord, typename TBmpHandler::TCoord>);
	if (cPt < 3)
		return FALSE;
	const auto [itMinY, itMaxY] = std::minmax_element(pptDst, pptDst + cPt, [](const TCoord& pt1, const TCoord& pt2)->bool
		{
			return TBmpHandler::GetY(pt1) < TBmpHandler::GetY(pt2);
		});
	const auto [itMinX, itMaxX] = std::minmax_element(pptDst, pptDst + cPt, [](const TCoord& pt1, const TCoord& pt2)->bool
		{
			return TBmpHandler::GetX(pt1) < TBmpHandler::GetX(pt2);
		});
	const int cyPolygon = TBmpHandler::GetY(*itMaxY) - TBmpHandler::GetY(*itMinY) + 1;
	const int cxPolygon = TBmpHandler::GetX(*itMaxX) - TBmpHandler::GetX(*itMinX) + 1;
	if (cxPolygon <= 0 || cyPolygon <= 0)
		return FALSE;

	NewBmp = Bmp.New(TBmpHandler::MakeCoord(cxPolygon, cyPolygon));
	std::vector<TCoord> vPtDst(cPt);
	EckCounter(cPt, i)
	{
		vPtDst[i] = TCoord
		{
			TBmpHandler::GetX(pptDst[i]) - TBmpHandler::GetX(*itMinX),
			TBmpHandler::GetY(pptDst[i]) - TBmpHandler::GetY(*itMinY)
		};
	}
	const int yMax = TBmpHandler::GetY(vPtDst[std::distance(pptDst, itMaxY)]);

	struct EDGE
	{
		float x;
		float dx;
		float Rx;
		float Ry;
		float dRx;
		float dRy;
		int yMax;
		int xPtYMax;// y���ĵ��x����
	};

	std::unordered_map<int, std::vector<EDGE*>> ET{};
	using TETIt = typename std::unordered_map<int, std::vector<EDGE*>>::iterator;
	EckCounter(cPt, i)
	{
		const auto& pt1 = vPtDst[i];
		const auto& pt2 = vPtDst[(i + 1) % cPt];
		const auto& ptSrc1 = pptSrc[i];
		const auto& ptSrc2 = pptSrc[(i + 1) % cPt];
		if (TBmpHandler::GetY(pt1) == TBmpHandler::GetY(pt2))
			continue;
		auto p = new EDGE;
		int yMax, yMin;
		if (TBmpHandler::GetY(pt1) < TBmpHandler::GetY(pt2))
		{
			p->x = (float)TBmpHandler::GetX(pt1);
			p->Rx = (float)TBmpHandler::GetX(ptSrc1);
			p->Ry = (float)TBmpHandler::GetY(ptSrc1);
			p->xPtYMax = (int)TBmpHandler::GetX(pt2);
			yMax = (int)TBmpHandler::GetY(pt2);
			yMin = (int)TBmpHandler::GetY(pt1);
		}
		else
		{
			p->x = (float)TBmpHandler::GetX(pt2);
			p->Rx = (float)TBmpHandler::GetX(ptSrc2);
			p->Ry = (float)TBmpHandler::GetY(ptSrc2);
			p->xPtYMax = (int)TBmpHandler::GetX(pt1);
			yMax = (int)TBmpHandler::GetY(pt1);
			yMin = (int)TBmpHandler::GetY(pt2);
		}
		p->dx = (float)(TBmpHandler::GetX(pt1) - TBmpHandler::GetX(pt2)) / (float)(TBmpHandler::GetY(pt1) - TBmpHandler::GetY(pt2));
		p->yMax = yMax;
		p->dRx = (float)(TBmpHandler::GetX(ptSrc1) - TBmpHandler::GetX(ptSrc2)) / (float)(TBmpHandler::GetY(pt1) - TBmpHandler::GetY(pt2));
		p->dRy = (float)(TBmpHandler::GetY(ptSrc1) - TBmpHandler::GetY(ptSrc2)) / (float)(TBmpHandler::GetY(pt1) - TBmpHandler::GetY(pt2));
		ET[yMin].push_back(p);
	}

	for (auto& x : ET)
	{
		std::sort(x.second.begin(), x.second.end(), [](const EDGE* p1, const EDGE* p2)->bool
			{
				if (p1->x == p2->x)
					return p1->dx < p2->dx;
				else
					return p1->x < p2->x;
			});
	}

	std::vector<EDGE*> AEL{};
	std::vector<size_t> vNeedDel{};

	Bmp.Lock();
	NewBmp.Lock();
	struct YAAINFO
	{
		float k;
		float b;
		float x0;
		float x1;
	};

	std::vector<YAAINFO> vPrevYAA[2]{};// ��Ϊ���²���
	auto fnDoYAA = [&](int y, float k, float b, float x0, float x1, bool bSampleDirection)
		{
			for (int x = (int)x0; x <= (int)x1; ++x)
			{
				const float yReal = k * x + b;
				const float e = (bSampleDirection ? yReal - y : y - yReal);// cr[0]��Ȩֵ
				if (e > 1.f)
					continue;
				const int y2 = (bSampleDirection ? y - 1 : y + 1);
				if (y2 >= 0 && y2 < NewBmp.GetHeight())
				{
					const typename TBmpHandler::TColor cr[2]
					{
						NewBmp.GetPixel(TBmpHandler::MakeCoord(x,y)),
						NewBmp.GetPixel(TBmpHandler::MakeCoord(x,y2))
					};
					typename TBmpHandler::TColorComp crNew[4];
					EckCounter(4, k)
					{
						crNew[k] = (typename TBmpHandler::TColorComp)(
							TBmpHandler::GetColorComp(cr[0], k) * (1.f - e) +
							TBmpHandler::GetColorComp(cr[1], k) * e);
					}
					NewBmp.SetPixel(TBmpHandler::MakeCoord(x, y), TBmpHandler::MakeColor(crNew));
					//NewBmp.SetPixel(TBmpHandler::MakeCoord(x, y), 0xFFFF0000);
				}
			}
		};
	size_t idxCurrPrevYAA{};
	for (int y = 0; y <= yMax; ++y)
	{
		if (TETIt it; (it = ET.find(y)) != ET.end())
		{
			AEL.insert(AEL.end(), it->second.begin(), it->second.end());
			std::sort(AEL.begin(), AEL.end(), [](const EDGE* p1, const EDGE* p2)->bool
				{
					if (p1->x == p2->x)
						return p1->dx < p2->dx;
					else
						return p1->x < p2->x;
				});
		}
		if (!AEL.empty())
		{
			vNeedDel.clear();
			EckCounter(AEL.size(), i)
			{
				if (y == AEL[i]->yMax)
					vNeedDel.emplace_back(i);
			}
			for (auto it = vNeedDel.rbegin(); it < vNeedDel.rend(); ++it)
				AEL.erase(AEL.begin() + *it);
			EckCounter(AEL.size() / 2, i)
			{
				const auto pL = AEL[i * 2];
				const auto pR = AEL[i * 2 + 1];
				const float dRxx = (pL->Rx - pR->Rx) / (float)(pL->x - pR->x);
				const float dRyy = (pL->Ry - pR->Ry) / (float)(pL->x - pR->x);
				float Rxx = pL->Rx;
				float Ryy = pL->Ry;

				const float kL = 1.f / pL->dx;
				const float kR = 1.f / pR->dx;

				const float bL = pL->yMax - kL * pL->xPtYMax;
				const float bR = pR->yMax - kR * pR->xPtYMax;

				float xL, xL1, xR, xR1;
				bool bYAAL, bYAAR, bSampleDirectionL, bSampleDirectionR;// TRUE = ����Ϊ�ⲿ
				if (kL == 0.f || kL == INFINITY ||
					(kL <= -1.f || kL >= 1.f) ||
					y == 0)
				{
					bYAAL = false;
				}
				else
				{
					bYAAL = true;
					if (kL < 0.f)
					{
						bSampleDirectionL = FALSE;
						xL = (y - bL) / kL;
						xL1 = (y - 1 - bL) / kL;
					}
					else
					{
						bSampleDirectionL = TRUE;
						xL = (y - bL) / kL;
						xL1 = (y + 1 - bL) / kL;
					}
				}

				if (kR == 0.f || kR == INFINITY ||
					(kR <= -1.f || kR >= 1.f) ||
					y == 0)
				{
					bYAAR = false;
				}
				else
				{
					bYAAR = true;
					if (kR < 0.f)
					{
						bSampleDirectionR = TRUE;
						xR = (y + 1 - bR) / kR;
						xR1 = (y - bR) / kR;
					}
					else
					{
						bSampleDirectionR = FALSE;
						xR = (y - 1 - bR) / kR;
						xR1 = (y - bR) / kR;
					}
				}

				const int xBegin = (int)ceilf(pL->x);
				const int xEnd = (int)floorf(pR->x);
				int x = xBegin;
				BOOL bAddL{}, bAddR{}, bSmpL{}, bSmpR{};
				for (; x <= xEnd; ++x)
				{
					int x0 = (int)floorf(Rxx);
					float fRateX = Rxx - x0;
					if (x0 < 0)
					{
						x0 = 0;
						fRateX = 0.f;
					}
					else if (x0 >= Bmp.GetWidth() - 1)
					{
						x0 = Bmp.GetWidth() - 2;
						fRateX = 1.f;
					}
					int y0 = (int)floorf(Ryy);
					float fRateY = Ryy - y0;
					if (y0 < 0)
					{
						y0 = 0;
						fRateY = 0.f;
					}
					else if (y0 >= Bmp.GetHeight() - 1)
					{
						y0 = Bmp.GetHeight() - 2;
						fRateY = 1.f;
					}
					fRateX = 1.f - fRateX;
					fRateY = 1.f - fRateY;
					const typename TBmpHandler::TColor cr[4]
					{
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0,y0)),
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0 + 1,y0)),
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0,y0 + 1)),
						Bmp.GetPixel(TBmpHandler::MakeCoord(x0 + 1,y0 + 1)),
					};

					typename TBmpHandler::TColorComp crNew[4];
					EckCounter(4, k)
					{
						crNew[k] = (typename TBmpHandler::TColorComp)(
							TBmpHandler::GetColorComp(cr[0], k) * fRateX * fRateY +
							TBmpHandler::GetColorComp(cr[1], k) * (1 - fRateX) * fRateY +
							TBmpHandler::GetColorComp(cr[2], k) * fRateX * (1 - fRateY) +
							TBmpHandler::GetColorComp(cr[3], k) * (1 - fRateX) * (1 - fRateY));
					}
					if (bYAAL && (x >= xL && x <= xL1))
					{
						if (bSampleDirectionL)
						{
							if (!bSmpL)
							{
								bSmpL = TRUE;
								fnDoYAA(y, kL, bL, xL, xL1, true);
							}
						}
						else if (!bAddL)
						{
							bAddL = TRUE;
							vPrevYAA[idxCurrPrevYAA].emplace_back(kL, bL, xL, xL1);
						}
						goto SkipNormalSetPixel;
					}

					if (bYAAR && (x >= xR && x <= xR1))
					{
						if (bSampleDirectionR)
						{
							if (!bSmpR)
							{
								bSmpR = TRUE;
								fnDoYAA(y, kR, bR, xR, xR1, true);
							}
						}
						else if (!bAddR)
						{
							bAddR = TRUE;
							vPrevYAA[idxCurrPrevYAA].emplace_back(kR, bR, xR, xR1);
						}
						goto SkipNormalSetPixel;
					}
					NewBmp.SetPixel(TBmpHandler::MakeCoord(x, y), TBmpHandler::MakeColor(crNew));
				SkipNormalSetPixel:
					Rxx += dRxx;
					Ryy += dRyy;

					if (x == xBegin && (kL <= -1.f || kL >= 1.f))
					{
						const float e = 1.f - (x - pL->x);// cr[0]��Ȩֵ
						const int x2 = x - 1;
						if (x2 >= 0)
						{
							const typename TBmpHandler::TColor cr[2]
							{
								NewBmp.GetPixel(TBmpHandler::MakeCoord(x,y)),
								NewBmp.GetPixel(TBmpHandler::MakeCoord(x2,y))
							};
							typename TBmpHandler::TColorComp crNew[4];
							EckCounter(4, k)
							{
								crNew[k] = (typename TBmpHandler::TColorComp)(
									TBmpHandler::GetColorComp(cr[0], k) * (1.f - e) +
									TBmpHandler::GetColorComp(cr[1], k) * e);
							}
							NewBmp.SetPixel(TBmpHandler::MakeCoord(x2, y), TBmpHandler::MakeColor(crNew));
						}
					}
				}

				if (--x >= 0 && (kR <= -1.f || kR >= 1.f))
				{
					const float e = 1.f - (pR->x - x);// cr[0]��Ȩֵ
					const int x2 = x + 1;
					if (x2 < NewBmp.GetWidth())
					{
						const typename TBmpHandler::TColor cr[2]
						{
							NewBmp.GetPixel(TBmpHandler::MakeCoord(x,y)),
							NewBmp.GetPixel(TBmpHandler::MakeCoord(x2,y))
						};
						typename TBmpHandler::TColorComp crNew[4];
						EckCounter(4, k)
						{
							crNew[k] = (typename TBmpHandler::TColorComp)(
								TBmpHandler::GetColorComp(cr[0], k) * (1.f - e) +
								TBmpHandler::GetColorComp(cr[1], k) * e);
						}
						NewBmp.SetPixel(TBmpHandler::MakeCoord(x2, y), TBmpHandler::MakeColor(crNew));
					}
				}
			}

			for (auto e : AEL)
			{
				e->x += e->dx;
				e->Rx += e->dRx;
				e->Ry += e->dRy;
			}
		}

		idxCurrPrevYAA = (idxCurrPrevYAA + 1) % 2;
		for (const auto& e : vPrevYAA[idxCurrPrevYAA])
			fnDoYAA(y - 1, e.k, e.b, e.x0, e.x1, false);
		vPrevYAA[idxCurrPrevYAA].clear();
	}
	NewBmp.UnLock();
	Bmp.UnLock();
	return TRUE;
}

ECK_NAMESPACE_END