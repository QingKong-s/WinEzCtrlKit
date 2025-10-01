#pragma once
#include "MathHelper.h"

ECK_NAMESPACE_BEGIN
inline HRESULT MakePloyLinePath(const D2D1_POINT_2F* pPt, int cPt,
	ID2D1Factory* pFactory, ID2D1PathGeometry*& pPathGeo)
{
	ID2D1PathGeometry* pPathGeometry;
	HRESULT hr = pFactory->CreatePathGeometry(&pPathGeometry);
	if (!pPathGeometry)
		return hr;
	ID2D1GeometrySink* pSink;
	pPathGeometry->Open(&pSink);
	pSink->BeginFigure(*pPt, D2D1_FIGURE_BEGIN_HOLLOW);
	pSink->AddLines(pPt, cPt);
	pSink->EndFigure(D2D1_FIGURE_END_OPEN);
	hr = pSink->Close();
	if (FAILED(hr))
	{
		pSink->Release();
		pPathGeometry->Release();
		return hr;
	}
	pSink->Release();
	pPathGeo = pPathGeometry;
	return S_OK;
}

/// <summary>
/// ���㷱�����߸���
/// </summary>
/// <param name="vPt">�㼯��</param>
/// <param name="rOut">��Բ�뾶</param>
/// <param name="rInt">��Բ�뾶</param>
/// <param name="iOffsetPtPen">�������ԲԲ�ĵ�ƫ��</param>
/// <param name="fStep">����</param>
template<class TVal = int, class TPt>
inline void CalcSpirographPoint(std::vector<TPt>& vPt, TVal rOut, TVal rInt, TVal iOffsetPtPen, float fStep = 0.1f)
{
	vPt.clear();
	float t = 0.f;
	const float k = (float)rInt / (float)rOut;
	const float l = (float)iOffsetPtPen / (float)rInt;
	const float tEnd = 2.f * PiF * (float)rInt / (float)Gcd((UINT)rOut, (UINT)rInt);
	const float fOneMinusK = 1 - k;
	const float fOneMinusKDivK = fOneMinusK / k;
	while (t < tEnd)
	{
		vPt.emplace_back(
			(TVal)(rOut * (fOneMinusK * cosf(t) + l * k * cosf(fOneMinusKDivK * t))),
			(TVal)(rOut * (fOneMinusK * sinf(t) - l * k * sinf(fOneMinusKDivK * t))));
		t += fStep;
	}
	vPt.emplace_back(vPt.front());
}

/// <summary>
/// ����������
/// </summary>
/// <param name="hDC">�豸����</param>
/// <param name="xCenter">���ĵ�X</param>
/// <param name="yCenter">���ĵ�Y</param>
/// <param name="rOut">��Բ�뾶</param>
/// <param name="rInt">��Բ�뾶</param>
/// <param name="iOffsetPtPen">�������ԲԲ�ĵ�ƫ��</param>
/// <param name="fStep">����</param>
/// <returns>Polyline����ֵ</returns>
EckInline BOOL DrawSpirograph(HDC hDC, int xCenter, int yCenter, int rOut, int rInt, int iOffsetPtPen, float fStep = 0.1f)
{
	std::vector<POINT> vPt{};
	CalcSpirographPoint(vPt, rOut, rInt, iOffsetPtPen, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});
	return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// ����������
/// </summary>
/// <param name="pGraphics">ͼ�ζ���</param>
/// <param name="pPen">���ʶ���</param>
/// <param name="xCenter">���ĵ�X</param>
/// <param name="yCenter">���ĵ�Y</param>
/// <param name="rOut">��Բ�뾶</param>
/// <param name="rInt">��Բ�뾶</param>
/// <param name="iOffsetPtPen">�������ԲԲ�ĵ�ƫ��</param>
/// <param name="fStep">����</param>
/// <returns>GdipDrawLines����ֵ</returns>
EckInline Gdiplus::GpStatus DrawSpirograph(GpGraphics* pGraphics, GpPen* pPen, float xCenter, float yCenter, float rOut, float rInt,
	float fOffsetPtPen, float fStep = 0.1f)
{
	std::vector<GpPointF> vPt{};
	CalcSpirographPoint<float>(vPt, rOut, rInt, fOffsetPtPen, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.X + xCenter,yCenter - pt.Y };
		});
	return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_SPIROGRAPH_D2D_PARAM
{
	ID2D1Factory* pFactory = nullptr;// D2D����
	ID2D1RenderTarget* pRT = nullptr;// D2D��ȾĿ��
	ID2D1Brush* pBrush = nullptr;// D2D��ˢ
	float cxStroke = 1.f;// �ʻ����
	ID2D1StrokeStyle* pStrokeStyle = nullptr;// �ʻ���ʽ
	float xCenter = 0.f;// ���ĵ�X
	float yCenter = 0.f;// ���ĵ�Y
	float rOut = 0.f;// ��Բ�뾶
	float rInt = 0.f;// ��Բ�뾶
	float fOffsetPtPen = 0.f;// �������ԲԲ�ĵ�ƫ��
	float fStep = 0.1f;// ����
};

/// <summary>
/// ����������
/// </summary>
/// <param name="Info">����</param>
/// <param name="ppPathGeometry">����·�������α�����ָ��</param>
/// <returns>����·��������ʱ��ʧ����Ϣ���޷��жϻ��Ʋ����ɹ���񣬵��÷�Ӧ���EndDraw����ֵ</returns>
inline HRESULT DrawSpirograph(const DRAW_SPIROGRAPH_D2D_PARAM& Info, ID2D1PathGeometry** ppPathGeometry = nullptr)
{
	if (ppPathGeometry)
		*ppPathGeometry = nullptr;
	std::vector<D2D1_POINT_2F> vPt{};
	CalcSpirographPoint<float>(vPt, Info.rOut, Info.rInt, Info.fOffsetPtPen, Info.fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
		{
			return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
		});
	ID2D1PathGeometry* pPathGeometry;
	HRESULT hr;
	if (FAILED(hr = MakePloyLinePath(vPt.data(), (int)vPt.size(), Info.pFactory, pPathGeometry)))
		return hr;
	Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
	if (ppPathGeometry)
		*ppPathGeometry = pPathGeometry;
	else
		pPathGeometry->Release();
	return S_OK;
}

template<class TVal = int, class TPt>
/// <summary>
/// ����������߸���
/// </summary>
/// <param name="vPt">�㼯��</param>
/// <param name="fDeformationCoefficient">����ϵ��</param>
/// <param name="fScaleX">X��������</param>
/// <param name="fScaleY">Y��������</param>
/// <param name="fStep">����</param>
EckInline void CalcButterflyCurvePoint(std::vector<TPt>& vPt, float fDeformationCoefficient = 4.f,
	float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f)
{
	vPt.clear();
	float t = 0.f;
	while (t < PiF * 20.f)
	{
		const float f = (expf(cosf(t)) - 2.f * cosf(fDeformationCoefficient * t) - powf(sinf(t / 12.f), 5.f));
		vPt.emplace_back((TVal)(sinf(t) * f * fScaleX), (TVal)(cosf(t) * f * fScaleY));
		t += fStep;
	}
	vPt.emplace_back(vPt.front());
}

/// <summary>
/// ����������
/// </summary>
/// <param name="hDC">�豸����</param>
/// <param name="xCenter">���ĵ�X</param>
/// <param name="yCenter">���ĵ�Y</param>
/// <param name="fDeformationCoefficient">����ϵ��</param>
/// <param name="fScaleX">X��������</param>
/// <param name="fScaleY">Y��������</param>
/// <param name="fStep">����</param>
EckInline BOOL DrawButterflyCurve(HDC hDC, int xCenter, int yCenter, float fDeformationCoefficient = 4.f,
	float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f)
{
	std::vector<POINT> vPt{};
	CalcButterflyCurvePoint(vPt, fDeformationCoefficient, fScaleX, fScaleY, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});
	return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// ����������
/// </summary>
/// <param name="pGraphics">ͼ�ζ���</param>
/// <param name="pPen">����</param>
/// <param name="xCenter">���ĵ�X</param>
/// <param name="yCenter">���ĵ�Y</param>
/// <param name="fDeformationCoefficient">����ϵ��</param>
/// <param name="fScaleX">X��������</param>
/// <param name="fScaleY">Y��������</param>
/// <param name="fStep">����</param>
/// <returns>GdipDrawLines����ֵ</returns>
EckInline Gdiplus::GpStatus DrawButterflyCurve(GpGraphics* pGraphics, GpPen* pPen, int xCenter, int yCenter, float fDeformationCoefficient = 4.f,
	float fScaleX = 100.f, float fScaleY = 100.f, float fStep = 0.01f)
{
	std::vector<GpPointF> vPt{};
	CalcButterflyCurvePoint<float>(vPt, fDeformationCoefficient, fScaleX, fScaleY, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.X + xCenter,yCenter - pt.Y };
		});
	return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_BUTTERFLYCURVE_D2D_PARAM
{
	ID2D1Factory* pFactory = nullptr;// D2D����
	ID2D1RenderTarget* pRT = nullptr;// D2D��ȾĿ��
	ID2D1Brush* pBrush = nullptr;// D2D��ˢ
	float cxStroke = 1.f;// �ʻ����
	ID2D1StrokeStyle* pStrokeStyle = nullptr;// �ʻ���ʽ
	float xCenter = 0.f;// ���ĵ�X
	float yCenter = 0.f;// ���ĵ�Y
	float fDeformationCoefficient = 4.f;// ����ϵ��
	float fScaleX = 100.f;// X��������
	float fScaleY = 100.f;// Y��������
	float fStep = 0.01f;// ����
};

/// <summary>
/// ����������
/// </summary>
/// <param name="Info">����</param>
/// <param name="ppPathGeometry">����·�������α�����ָ��</param>
/// <returns>����·��������ʱ��ʧ����Ϣ���޷��жϻ��Ʋ����ɹ���񣬵��÷�Ӧ���EndDraw����ֵ</returns>
inline HRESULT DrawButterflyCurve(const DRAW_BUTTERFLYCURVE_D2D_PARAM& Info,
	ID2D1PathGeometry** ppPathGeometry = nullptr)
{
	if (ppPathGeometry)
		*ppPathGeometry = nullptr;
	std::vector<D2D1_POINT_2F> vPt{};
	CalcButterflyCurvePoint<float>(vPt, Info.fDeformationCoefficient, Info.fScaleX, Info.fScaleY, Info.fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
		{
			return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
		});
	ID2D1PathGeometry* pPathGeometry;
	HRESULT hr;
	if (FAILED(hr = MakePloyLinePath(vPt.data(), (int)vPt.size(), Info.pFactory, pPathGeometry)))
		return hr;
	Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
	if (ppPathGeometry)
		*ppPathGeometry = pPathGeometry;
	else
		pPathGeometry->Release();
	return S_OK;
}

template<class TVal = int, class TPt>
/// <summary>
/// ����õ�����߸���
/// </summary>
/// <param name="vPt">�㼯��</param>
/// <param name="a">���곤��</param>
/// <param name="n">������������</param>
/// <param name="fStep">����</param>
EckInline void CalcRoseCurvePoint(std::vector<TPt>& vPt, float a = 10.f, float n = 1.f, float fStep = 0.01f)
{
	vPt.clear();
	float t = 0.f;
	float x, y;
	while (t < PiF * 2.f)
	{
		Polar2Rect(a * sinf(n * t), t, x, y);
		vPt.emplace_back((TVal)x, (TVal)y);
		t += fStep;
	}
	vPt.emplace_back(vPt.front());
}

/// <summary>
/// ��õ������
/// </summary>
/// <param name="hDC">�豸����</param>
/// <param name="xCenter">���ĵ�X</param>
/// <param name="yCenter">���ĵ�Y</param>
/// <param name="a">���곤��</param>
/// <param name="n">������������</param>
/// <param name="fStep">����</param>
/// <returns>Polyline����ֵ</returns>
EckInline BOOL DrawRoseCurve(HDC hDC, int xCenter, int yCenter, float a = 300.f, float n = 10.f, float fStep = 0.01f)
{
	std::vector<POINT> vPt{};
	CalcRoseCurvePoint(vPt, a, n, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});
	return Polyline(hDC, vPt.data(), (int)vPt.size());
}

/// <summary>
/// ��õ������
/// </summary>
/// <param name="pGraphics">ͼ�ζ���</param>
/// <param name="pPen">���ʶ���</param>
/// <param name="xCenter">���ĵ�X</param>
/// <param name="yCenter">���ĵ�Y</param>
/// <param name="a">���곤��</param>
/// <param name="n">������������</param>
/// <param name="fStep">����</param>
/// <returns>GdipDrawLines����ֵ</returns>
EckInline Gdiplus::GpStatus DrawRoseCurve(GpGraphics* pGraphics, GpPen* pPen, float xCenter, float yCenter,
	float a = 300.f, float n = 10.f, float fStep = 0.01f)
{
	std::vector<GpPointF> vPt{};
	CalcRoseCurvePoint<float>(vPt, a, n, fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.X + xCenter,yCenter - pt.Y };
		});
	return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
}

struct DRAW_ROSECURVE_D2D_PARAM
{
	ID2D1Factory* pFactory = nullptr;// D2D����
	ID2D1RenderTarget* pRT = nullptr;// D2D��ȾĿ��
	ID2D1Brush* pBrush = nullptr;// D2D��ˢ
	float cxStroke = 1.f;// �ʻ����
	ID2D1StrokeStyle* pStrokeStyle = nullptr;// �ʻ���ʽ
	float xCenter = 0.f;// ���ĵ�X
	float yCenter = 0.f;// ���ĵ�Y
	float fDeformationCoefficient = 4.f;// ����ϵ��
	float a = 300.f;// ���곤��
	float n = 10.f;// ������������
	float fStep = 0.01f;// ����
};

/// <summary>
/// ��õ������
/// </summary>
/// <param name="Info">����</param>
/// <param name="ppPathGeometry">����·�������α�����ָ��</param>
/// <returns>����·��������ʱ��ʧ����Ϣ���޷��жϻ��Ʋ����ɹ���񣬵��÷�Ӧ���EndDraw����ֵ</returns>
inline HRESULT DrawRoseCurve(const DRAW_ROSECURVE_D2D_PARAM& Info,
	ID2D1PathGeometry** ppPathGeometry = nullptr)
{
	if (ppPathGeometry)
		*ppPathGeometry = nullptr;
	std::vector<D2D1_POINT_2F> vPt{};
	CalcRoseCurvePoint<float>(vPt, Info.a, Info.n, Info.fStep);
	std::transform(std::execution::par_unseq, vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
		{
			return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
		});
	ID2D1PathGeometry* pPathGeometry;
	HRESULT hr;
	if (FAILED(hr = MakePloyLinePath(vPt.data(), (int)vPt.size(), Info.pFactory, pPathGeometry)))
		return hr;
	Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
	if (ppPathGeometry)
		*ppPathGeometry = pPathGeometry;
	else
		pPathGeometry->Release();
	return S_OK;
}

/// <summary>
/// ����������/������θ���
/// </summary>
/// <param name="vPt">�㼯��</param>
/// <param name="r">���Բ�뾶</param>
/// <param name="n">���������</param>
/// <param name="fAngle">��ʼ�����X�����������ת�Ƕ�</param>
/// <param name="bLinkStar">�Ƿ�����Ϊ����</param>
template<class TVal = int, class TPt>
inline void CalcRegularStar(std::vector<TPt>& vPt, TVal r, int n, float fAngle = Deg2Rad(90.f), BOOL bLinkStar = TRUE)
{
	vPt.clear();
	const float fAngleUnit = PiF * 2.f / n;
	float fTheta = fAngle;
	TVal x, y;
	if (bLinkStar)
	{
		int i = 0;
		const int cLoop = (n % 2) ? n : (n / 2);
		EckCounterNV(cLoop)
		{
			CalcPointFromCircleAngle<TVal>(r, fTheta + fAngleUnit * i, x, y);
			vPt.emplace_back(x, y);
			i += 2;
			if (i >= n)
				i %= n;
		}
		vPt.emplace_back(vPt.front());

		if (n % 2 == 0)
		{
			i = 1;
			EckCounterNV(cLoop)
			{
				CalcPointFromCircleAngle<TVal>(r, fTheta + fAngleUnit * i, x, y);
				vPt.emplace_back(x, y);
				i += 2;
				if (i >= n)
					i %= n;
			}
			vPt.emplace_back(vPt[n / 2 + 1]);
		}
	}
	else
	{
		EckCounter(n, i)
		{
			CalcPointFromCircleAngle<TVal>(r, fTheta, x, y);
			vPt.emplace_back(x, y);
			fTheta += fAngleUnit;
		}
		vPt.emplace_back(vPt.front());
	}
}

/// <summary>
/// ��������/�������
/// </summary>
/// <param name="hDC">�豸����</param>
/// <param name="xCenter">���ĵ�X</param>
/// <param name="yCenter">���ĵ�Y</param>
/// <param name="r">���Բ�뾶</param>
/// <param name="n">���������</param>
/// <param name="fAngle">��ʼ�����X�����������ת�Ƕ�</param>
/// <param name="bLinkStar">�Ƿ�����Ϊ����</param>
/// <returns>Polyline����ֵ</returns>
inline BOOL DrawRegularStar(HDC hDC, int xCenter, int yCenter,
	int r, int n, float fAngle = Deg2Rad(90.f), BOOL bLinkStar = TRUE)
{
	std::vector<POINT> vPt{};
	CalcRegularStar(vPt, r, n, fAngle, bLinkStar);
	std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](POINT& pt)
		{
			return POINT{ pt.x + xCenter,yCenter - pt.y };
		});

	if (n % 2 || !bLinkStar)
		return Polyline(hDC, vPt.data(), (int)vPt.size());
	else
		return Polyline(hDC, vPt.data(), (int)vPt.size() / 2) &&
		Polyline(hDC, vPt.data() + n / 2 + 1, (int)vPt.size() / 2);
}

/// <summary>
/// ��������/�������
/// </summary>
/// <param name="pGraphics">ͼ�ζ���</param>
/// <param name="pPen">���ʾ��</param>
/// <param name="xCenter">���ĵ�X</param>
/// <param name="yCenter">���ĵ�Y</param>
/// <param name="r">���Բ�뾶</param>
/// <param name="n">���������</param>
/// <param name="fAngle">��ʼ�����X�����������ת�Ƕ�</param>
/// <param name="bLinkStar">�Ƿ�����Ϊ����</param>
/// <returns>GdipDrawLines����ֵ</returns>
inline Gdiplus::GpStatus DrawRegularStar(GpGraphics* pGraphics, GpPen* pPen, float xCenter, float yCenter,
	float r, int n, float fAngle = Deg2Rad(90.f), BOOL bLinkStar = TRUE)
{
	std::vector<GpPointF> vPt{};
	CalcRegularStar(vPt, r, n, fAngle, bLinkStar);
	std::transform(vPt.begin(), vPt.end(), vPt.begin(), [xCenter, yCenter](GpPointF& pt)
		{
			return GpPointF{ pt.X + xCenter,yCenter - pt.Y };
		});

	if (n % 2 || !bLinkStar)
		return GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size());
	else
	{
		GdipDrawLines(pGraphics, pPen, vPt.data(), (int)vPt.size() / 2);
		return GdipDrawLines(pGraphics, pPen, vPt.data() + n / 2 + 1, (int)vPt.size() / 2);
	}
}

struct DRAW_REGULARSTAR_D2D_PARAM
{
	ID2D1Factory* pFactory = nullptr;// D2D����
	ID2D1RenderTarget* pRT = nullptr;// D2D��ȾĿ��
	ID2D1Brush* pBrush = nullptr;// D2D��ˢ
	float cxStroke = 1.f;// �ʻ����
	ID2D1StrokeStyle* pStrokeStyle = nullptr;// �ʻ���ʽ
	float xCenter = 0.f;// ���ĵ�X
	float yCenter = 0.f;// ���ĵ�Y
	float r = 300;// ���Բ�뾶
	int n = 5;// ���������
	float fAngle = Deg2Rad(90.f);// ��ʼ�����X�����������ת�Ƕ�
	BOOL bLinkStar = TRUE;// �Ƿ�����Ϊ����
};

/// <summary>
/// ��������/�������
/// </summary>
/// <param name="Info">����</param>
/// <param name="ppPathGeometry">����·�������α�����ָ��</param>
/// <returns>����·��������ʱ��ʧ����Ϣ���޷��жϻ��Ʋ����ɹ���񣬵��÷�Ӧ���EndDraw����ֵ</returns>
inline HRESULT DrawRegularStar(const DRAW_REGULARSTAR_D2D_PARAM& Info,
	ID2D1PathGeometry** ppPathGeometry = nullptr)
{
	if (ppPathGeometry)
		*ppPathGeometry = nullptr;
	std::vector<D2D1_POINT_2F> vPt{};
	CalcRegularStar(vPt, Info.r, Info.n, Info.fAngle, Info.bLinkStar);
	std::transform(vPt.begin(), vPt.end(), vPt.begin(), [&Info](D2D1_POINT_2F& pt)
		{
			return D2D1_POINT_2F{ pt.x + Info.xCenter,Info.yCenter - pt.y };
		});

	ID2D1PathGeometry* pPathGeometry;
	HRESULT hr = Info.pFactory->CreatePathGeometry(&pPathGeometry);
	if (!pPathGeometry)
		return hr;
	ID2D1GeometrySink* pSink;
	pPathGeometry->Open(&pSink);
	if (Info.n % 2 || !Info.bLinkStar)
	{
		pSink->BeginFigure(vPt.front(), D2D1_FIGURE_BEGIN_HOLLOW);
		pSink->AddLines(vPt.data(), (UINT32)vPt.size());
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);
	}
	else
	{
		pSink->BeginFigure(vPt.front(), D2D1_FIGURE_BEGIN_HOLLOW);
		pSink->AddLines(vPt.data(), (UINT32)vPt.size() / 2);
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);

		pSink->BeginFigure(vPt[Info.n / 2 + 1], D2D1_FIGURE_BEGIN_HOLLOW);
		pSink->AddLines(vPt.data() + Info.n / 2 + 1, (UINT32)vPt.size() / 2);
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);
	}
	hr = pSink->Close();
	if (FAILED(hr))
	{
		pSink->Release();
		pPathGeometry->Release();
		return hr;
	}
	pSink->Release();
	Info.pRT->DrawGeometry(pPathGeometry, Info.pBrush, Info.cxStroke, Info.pStrokeStyle);
	if (ppPathGeometry)
		*ppPathGeometry = pPathGeometry;
	else
		pPathGeometry->Release();
	return S_OK;
}
ECK_NAMESPACE_END