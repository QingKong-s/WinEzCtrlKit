/*
* WinEzCtrlKit Library
*
* CImgMat.h : ͼ�����
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CArray2D.h"

#include <stack>

ECK_NAMESPACE_BEGIN
inline constexpr float KernelX_Sobel[]
{
	-1,0,1,
	-2,0,2,
	-1,0,1
};
inline constexpr float KernelY_Sobel[]
{
	1,2,1,
	0,0,0,
	-1,-2,-1
};

inline constexpr float KernelX_Prewitt[]
{
	1,0,-1,
	1,0,-1,
	1,0,-1
};
inline constexpr float KernelY_Prewitt[]
{
	1,1,1,
	0,0,0,
	-1,-1,-1
};

inline constexpr BYTE StructElem_3x3_Cross[]
{
	0,1,0,
	1,1,1,
	0,1,0
};

inline constexpr BYTE StructElem_3x3_Rect[]
{
	1,1,1,
	1,1,1,
	1,1,1
};

inline constexpr BYTE StructElem_5x5_Cross[]
{
	0,1,0,
	0,1,0,
	1,1,1,
	0,1,0,
	0,1,0
};

inline constexpr BYTE StructElem_5x5_Rect[]
{
	1,1,1,1,1,
	1,1,1,1,1,
	1,1,1,1,1,
	1,1,1,1,1,
	1,1,1,1,1
};

inline constexpr BYTE StructElem_5x5_Ellipse[]
{
	0,0,1,0,0,
	1,1,1,1,1,
	1,1,1,1,1,
	1,1,1,1,1,
	0,0,1,0,0
};

inline constexpr float GaussianKernel_3x3_3Sigma[]
{
	0.077847, 0.123317, 0.077847,
	0.123317, 0.195346, 0.123317,
	0.077847, 0.123317, 0.077847
};

inline constexpr float GaussianKernel_5x5_3Sigma[]
{
	0.003765, 0.015019, 0.023792, 0.015019, 0.003765,
	0.015019, 0.059912, 0.094907, 0.059912, 0.015019,
	0.023792, 0.094907, 0.150342, 0.094907, 0.023792,
	0.015019, 0.059912, 0.094907, 0.059912, 0.015019,
	0.003765, 0.015019, 0.023792, 0.015019, 0.003765
};

inline constexpr float GaussianKernel_7x7_3Sigma[]
{
	0.000316, 0.001300, 0.002979, 0.004442, 0.005700, 0.006765, 0.000316,
	0.001300, 0.006033, 0.010755, 0.014676, 0.017896, 0.019517, 0.001300,
	0.002979, 0.010755, 0.024103, 0.034583, 0.042099, 0.046753, 0.002979,
	0.004442, 0.014676, 0.034583, 0.051721, 0.065693, 0.076399, 0.004442,
	0.005700, 0.017896, 0.042099, 0.065693, 0.086519, 0.094480, 0.005700,
	0.006765, 0.019517, 0.046753, 0.076399, 0.094480, 0.100000, 0.006765,
	0.000316, 0.001300, 0.002979, 0.004442, 0.005700, 0.006765, 0.000316
};

enum class BorderOpt
{
	Ingore,
	Zero,
	Replicate,
	Wrap,
	Mirror
};

enum class ImageChannel
{
	Red,
	Green,
	Blue,
	Alpha,
	Gray
};

inline CArray2D<float> GenerateGaussianKernel(int nRadius, float fSigma)
{
	const int cxy = 2 * nRadius + 1;
	CArray2D<float> Kernel(cxy, cxy);
	float fSum{};

	for (int x = -nRadius; x <= nRadius; ++x)
		for (int y = -nRadius; y <= nRadius; ++y)
		{
			const float t = (1 / (2 * PiF * fSigma * fSigma)) *
				exp(-(x * x + y * y) / (2 * fSigma * fSigma));
			fSum += t;
			Kernel[x + nRadius][y + nRadius] = t;
		}
	for (int i = 0; i < cxy; ++i)
		for (int j = 0; j < cxy; ++j)
			Kernel[i][j] /= fSum;
	return Kernel;
}


class CImageMat
{
protected:
	union TArgb
	{
		struct
		{
			BYTE b, g, r, a;
		};
		DWORD dw;
	};

	using TComp = decltype(TArgb::r);
	using TColor = decltype(TArgb::dw);

	void* m_pBits{};
	int m_cx{}, m_cy{};
	UINT m_cbStride{};

	void IntEdgeDetect(const CImageMat& Dst, const float* KernelX, const float* KernelY,
		int cxyKernel, BOOL bBinary, float fThreshold) const
	{
		const int cxyHalf = cxyKernel / 2;
		for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
			for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			{
				TArgb pix;
				float fSumX{};
				float fSumY{};
				for (int l = -cxyHalf; l <= cxyHalf; ++l)
					for (int k = -cxyHalf; k <= cxyHalf; ++k)
					{
						pix = Pixel(i + k, j + l);
						const auto fX = KernelX[(k + cxyHalf) + (l + cxyHalf) * 3];
						fSumX += (fX * pix.r);
						const auto fY = KernelY[(k + cxyHalf) + (l + cxyHalf) * 3];
						fSumY += (fY * pix.r);
					}
				const auto f = sqrt(fSumX * fSumX + fSumY * fSumY);
				auto& NewPix = Dst.Pixel(i, j);
				if (bBinary)
					NewPix.dw = ((f > fThreshold) ? 0xFFFFFFFF : 0);
				else
				{
					NewPix.r = NewPix.g = NewPix.b = (BYTE)f;
					NewPix.a = 0xFF;
				}
			}
	}
public:
	constexpr CImageMat() = default;
	constexpr CImageMat(void* pBits, int cx, int cy, UINT cbStride)
		: m_pBits{ pBits }, m_cx{ cx }, m_cy{ cy }, m_cbStride{ cbStride } {
	}

	virtual ~CImageMat() = default;

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="pBits">����ָ��</param>
	/// <param name="cx">���</param>
	/// <param name="cy">�߶�</param>
	/// <param name="cbStride">�粽</param>
	EckInline constexpr void SetBits(void* pBits, int cx, int cy, UINT cbStride)
	{
		m_pBits = pBits;
		m_cx = cx;
		m_cy = cy;
		m_cbStride = cbStride;
	}

	EckInline constexpr int GetWidth() const { return m_cx; }

	EckInline constexpr int GetHeight() const { return m_cy; }

	EckInline constexpr UINT GetStride() const { return m_cbStride; }

	EckInline constexpr void* GetBits() const { return m_pBits; }

	// ȡĳ������
	EckInline constexpr TArgb& Pixel(int x, int y) const
	{
		return *(TArgb*)((BYTE*)m_pBits + y * m_cbStride + x * sizeof(ARGB));
	}

	EckInline constexpr TArgb Pixel(int x, int y, BorderOpt eBorder) const
	{
		if (x < 0 || x >= m_cx || y < 0 || y >= m_cy)
			switch (eBorder)
			{
			case BorderOpt::Ingore:
			case BorderOpt::Zero:
				return TArgb{};
			case BorderOpt::Replicate:
				return Pixel(
					x < 0 ? 0 :
					x >= m_cx ? m_cx - 1 :
					x,
					y < 0 ? 0 :
					y >= m_cy ? m_cy - 1 :
					y);
			case BorderOpt::Wrap:
				return Pixel(x % m_cx, y % m_cy);
			case BorderOpt::Mirror:
				return Pixel(
					x < 0 ? -x :
					x >= m_cx ? 2 * m_cx - 1 - x :
					x,
					y < 0 ? -y :
					y >= m_cy ? 2 * m_cy - 1 - y :
					y);
			default: ECK_UNREACHABLE;
			}
		else ECKLIKELY
			return Pixel(x, y);
	}

	/// <summary>
	/// ���
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="Kernel">�����</param>
	/// <param name="cxyKernel">�˴�С������</param>
	/// <param name="eBorder">�߽�ѡ��</param>
	constexpr void Convolve(const CImageMat& Dst, const float* Kernel, int cxyKernel,
		BorderOpt eBorder = BorderOpt::Ingore) const
	{
		const int cxyHalf = cxyKernel / 2;
		const int iOffset = ((eBorder == BorderOpt::Ingore) ? cxyHalf : 0);
		for (int i = iOffset; i < m_cx - iOffset; ++i)
			for (int j = iOffset; j < m_cy - iOffset; ++j)
			{
				TArgb pix;
				float fSum[4]{};
				for (int l = -cxyHalf; l <= cxyHalf; ++l)
					for (int k = -cxyHalf; k <= cxyHalf; ++k)
					{
						const auto x = i + k;
						const auto y = j + l;
						if (eBorder == BorderOpt::Ingore)
							pix = Pixel(x, y);
						else
							pix = Pixel(x, y, eBorder);
						const auto f = Kernel[(k + cxyHalf) + (l + cxyHalf) * cxyKernel];
						fSum[0] += (f * pix.a);
						fSum[1] += (f * pix.r);
						fSum[2] += (f * pix.g);
						fSum[3] += (f * pix.b);
					}
				auto& pixNew = Dst.Pixel(i, j);
				pixNew.a = (BYTE)fSum[0];
				pixNew.r = (BYTE)fSum[1];
				pixNew.g = (BYTE)fSum[2];
				pixNew.b = (BYTE)fSum[3];
			}
		// �߽紦��
		if (eBorder == BorderOpt::Ingore)
			return;
	}

	/// <summary>
	/// Sobel��Ե���
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="bBinary">�Ƿ��ֵ�����</param>
	/// <param name="fThreshold">��bBinaryΪTRUE����ò���ָ����ֵ����ֵ</param>
	/// <returns></returns>
	EckInline void Sobel(const CImageMat& Dst, BOOL bBinary, float fThreshold) const
	{
		IntEdgeDetect(Dst, KernelX_Sobel, KernelY_Sobel, 3, bBinary, fThreshold);
	}

	/// <summary>
	/// Prewitt��Ե���
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="bBinary">�Ƿ��ֵ�����</param>
	/// <param name="fThreshold">��bBinaryΪTRUE����ò���ָ����ֵ����ֵ</param>
	/// <returns></returns>
	EckInline void Prewitt(const CImageMat& Dst, BOOL bBinary, float fThreshold) const
	{
		IntEdgeDetect(Dst, KernelX_Prewitt, KernelY_Prewitt, 3, bBinary, fThreshold);
	}

	/// <summary>
	/// Roberts��Ե���
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="bBinary">�Ƿ��ֵ�����</param>
	/// <param name="fThreshold">��bBinaryΪTRUE����ò���ָ����ֵ����ֵ</param>
	/// <returns></returns>
	EckInline void Roberts(const CImageMat& Dst, BOOL bBinary, float fThreshold) const
	{
		for (int i = 0; i < m_cx - 1; ++i)
			for (int j = 0; j < m_cy - 1; ++j)
			{
				const int G1 = Pixel(i, j).r - Pixel(i + 1, j + 1).r;
				const int G2 = Pixel(i + 1, j + 1).r - Pixel(i, j).r;
				const auto G = sqrt(G1 * G1 + G2 * G2);
				auto& NewPix = Dst.Pixel(i, j);
				if (bBinary)
					NewPix.dw = ((G > fThreshold) ? 0xFFFFFFFF : 0);
				else
				{
					NewPix.r = NewPix.g = NewPix.b = (BYTE)G;
					NewPix.a = 0xFF;
				}
			}
	}

	/// <summary>
	/// Canny��Ե���
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="fLowThresh">����ֵ</param>
	/// <param name="fHighThresh">����ֵ</param>
	void Canny(const CImageMat& Dst, float fLowThresh, float fHighThresh) const
	{
		struct GRADIENT
		{
			float g;
			float fAngle;
		};
		CArray2D<GRADIENT> G(m_cx, m_cy);
		// �����ݶ�
		constexpr int cxyHalf = 3 / 2;
		for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
			for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			{
				TArgb pix;
				float fSumX{};
				float fSumY{};
				for (int l = -cxyHalf; l <= cxyHalf; ++l)
					for (int k = -cxyHalf; k <= cxyHalf; ++k)
					{
						pix = Pixel(i + k, j + l);
						const auto fX = KernelX_Sobel[(k + cxyHalf) + (l + cxyHalf) * 3];
						fSumX += (fX * pix.r);
						const auto fY = KernelY_Sobel[(k + cxyHalf) + (l + cxyHalf) * 3];
						fSumY += (fY * pix.r);
					}
				auto& e = G[i][j];
				e.g = sqrt(fSumX * fSumX + fSumY * fSumY);
				e.fAngle = atan2(fSumY, fSumX) * 180.f / PiF;
			}
		// �Ǽ���ֵ����
		CArray2D<float> Suppressed(m_cx, m_cy);
		for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
			for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			{
				const auto& e = G[i][j];
				auto& f = Suppressed[i][j];
				float angle = (e.fAngle < 0.f ? (e.fAngle + 180.f) : e.fAngle);
				angle = fmod(angle, 180.f);

				float q, r;
				if (angle < 22.5f || angle >= 157.5f)// ˮƽ
				{
					q = G[i + 1][j].g;
					r = G[i - 1][j].g;
				}
				else if (angle >= 22.5f && angle < 67.5f)// 45��
				{
					q = G[i - 1][j + 1].g;
					r = G[i + 1][j - 1].g;
				}
				else if (angle >= 67.5f && angle < 112.5f)// ��ֱ
				{
					q = G[i][j + 1].g;
					r = G[i][j - 1].g;
				}
				else if (angle >= 112.5f && angle < 157.5f)// 135��
				{
					q = G[i - 1][j - 1].g;
					r = G[i + 1][j + 1].g;
				}
				else
					q = r = 0.f;

				if (e.g >= q && e.g >= r)
					f = e.g;
				else
					f = 0.f;
			}
		// ˫��ֵ
		constexpr TColor WeakColor{ 0xFFFF0000 };
		constexpr TColor WeakLabelColor{ 0xFF00FF00 };
		for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
			for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			{
				const auto f = Suppressed[i][j];
				auto& Pix = Dst.Pixel(i, j);
				if (f >= fHighThresh)
					Pix.dw = 0xFFFFFFFF;
				else if (f >= fLowThresh)
					Pix.dw = WeakColor;
				else
					Pix.dw = 0;
			}
		// ���ߴ���
		std::stack<POINT> s{};
		std::queue<POINT> q{};
		BOOL bConnected = FALSE;
		for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
			for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			{
				auto& Pix = Dst.Pixel(i, j);
				if (Pix.dw == WeakColor)
				{
					s.emplace(i, j);
					q.emplace(i, j);
					Pix.dw = WeakLabelColor;

					while (!s.empty())
					{
						const POINT pt = s.top();
						s.pop();
						auto& PixCurr = Dst.Pixel(pt.x, pt.y);
						for (int k = -1; k <= 1; ++k)
							for (int l = -1; l <= 1; ++l)
							{
								auto& PixNeighbor = Dst.Pixel(pt.x + k, pt.y + l);
								if (PixNeighbor.dw == WeakColor)// ����
								{
									s.emplace(pt.x + k, pt.y + l);
									q.emplace(pt.x + k, pt.y + l);
									PixNeighbor.dw = WeakLabelColor;
								}
								if (!bConnected && PixNeighbor.dw == 0xFFFFFFFF)// ǿ��
									bConnected = TRUE;
							}
					}

					if (bConnected)
					{
						while (!q.empty())
						{
							const POINT pt = q.front();
							q.pop();
							Dst.Pixel(pt.x, pt.y).dw = 0xFFFFFFFF;
						}
						bConnected = FALSE;
					}
					else
					{
						while (!q.empty())
						{
							const POINT pt = q.front();
							q.pop();
							Dst.Pixel(pt.x, pt.y).dw = 0;
						}
					}
				}
			}
	}

	// ���Ҷ�ͼ
	constexpr void Grayscale(const CImageMat& Dst) const
	{
		for (int i = 0; i < m_cx; ++i)
			for (int j = 0; j < m_cy; ++j)
			{
				auto& Pix = Dst.Pixel(i, j);
				Pix.r = Pix.g = Pix.b =
					BYTE(0.299f * Pix.r + 0.587f * Pix.g + 0.114f * Pix.b);
				Pix.a = 0xFF;
			}
	}

	// ���Ҷ�ͼ
	EckInline constexpr void Grayscale() const { Grayscale(*this); }

	/// <summary>
	/// ��ֵ��
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="byThreshold">��ֵ</param>
	/// <param name="eChannel">�ο�ͨ��</param>
	constexpr void Binary(const CImageMat& Dst, BYTE byThreshold,
		ImageChannel eChannel = ImageChannel::Gray) const
	{
		for (int i = 0; i < m_cx; ++i)
			for (int j = 0; j < m_cy; ++j)
			{
				auto& Pix = Dst.Pixel(i, j);
				switch (eChannel)
				{
				case ImageChannel::Gray:
					Pix.dw = (BYTE(0.299f * Pix.r + 0.587f * Pix.g + 0.114f * Pix.b) > byThreshold ?
						0xFFFFFFFF : 0);
					break;
				case ImageChannel::Red:
					Pix.dw = (Pix.r > byThreshold ? 0xFFFFFFFF : 0);
					break;
				case ImageChannel::Green:
					Pix.dw = (Pix.g > byThreshold ? 0xFFFFFFFF : 0);
					break;
				case ImageChannel::Blue:
					Pix.dw = (Pix.b > byThreshold ? 0xFFFFFFFF : 0);
					break;
				case ImageChannel::Alpha:
					Pix.dw = (Pix.a > byThreshold ? 0xFFFFFFFF : 0);
					break;
				default: ECK_UNREACHABLE;
				}
			}
	}

	/// <summary>
	/// ��ֵ��
	/// </summary>
	/// <param name="byThreshold">��ֵ</param>
	/// <param name="eChannel">�ο�ͨ��</param>
	EckInline constexpr void Binary(float fThreshold, ImageChannel eChannel = ImageChannel::Gray) const
	{
		Binary(*this, fThreshold, eChannel);
	}

	// ��ɫ
	constexpr void Invert() const
	{
		for (int i = 0; i < m_cx; ++i)
			for (int j = 0; j < m_cy; ++j)
			{
				auto& Pix = Pixel(i, j);
				Pix.r = 0xFF - Pix.r;
				Pix.g = 0xFF - Pix.g;
				Pix.b = 0xFF - Pix.b;
			}
	}

	/// <summary>
	/// ��ʴ
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="pbyStruct">�ṹԪ��</param>
	/// <param name="cxStruct">�ṹԪ�ؿ��</param>
	/// <param name="cyStruct">�ṹԪ�ظ߶�</param>
	void Erode(const CImageMat& Dst, const BYTE* pbyStruct, int cxStruct, int cyStruct) const
	{
		for (int i = 0; i < m_cx; ++i)
			for (int j = 0; j < m_cy; ++j)
			{
				auto& Pix = Dst.Pixel(i, j);
				Pix.dw = 0;
				for (int k = 0; k < cyStruct; ++k)
					for (int l = 0; l < cxStruct; ++l)
					{
						const int x = i - cxStruct / 2 + l;
						const int y = j - cyStruct / 2 + k;
						if (x >= 0 && x < m_cx && y >= 0 && y < m_cy &&
							pbyStruct[k * cxStruct + l] &&
							Pixel(x, y).dw)
						{
							Pix.dw = 0xFFFFFFFF;
							goto NextPixel;
						}
					}
			NextPixel:;
			}
	}

	/// <summary>
	/// ��ʴ��
	/// 3x3ʮ���νṹԪ��
	/// </summary>
	/// <param name="Dst">���</param>
	EckInline void Erode3x3Cross(const CImageMat& Dst) const { Erode(Dst, StructElem_3x3_Cross, 3, 3); }

	/// <summary>
	/// ����
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="pbyStruct">�ṹԪ��</param>
	/// <param name="cxStruct">�ṹԪ�ؿ��</param>
	/// <param name="cyStruct">�ṹԪ�ظ߶�</param>
	void Dilate(const CImageMat& Dst, const BYTE* pbyStruct, int cxStruct, int cyStruct) const
	{
		for (int i = 0; i < m_cx; ++i)
			for (int j = 0; j < m_cy; ++j)
			{
				auto& Pix = Dst.Pixel(i, j);
				Pix.dw = 0xFFFFFFFF;
				for (int k = 0; k < cyStruct; ++k)
					for (int l = 0; l < cxStruct; ++l)
					{
						const int x = i - cxStruct / 2 + l;
						const int y = j - cyStruct / 2 + k;
						if (x >= 0 && x < m_cx && y >= 0 && y < m_cy &&
							pbyStruct[k * cxStruct + l] &&
							Pixel(x, y).dw == 0)
						{
							Pix.dw = 0;
							goto NextPixel;
						}
					}
			NextPixel:;
			}
	}

	/// <summary>
	/// ���͡�
	/// 3x3ʮ���νṹԪ��
	/// </summary>
	/// <param name="Dst">���</param>
	EckInline void Dilate3x3Cross(const CImageMat& Dst) const { Dilate(Dst, StructElem_3x3_Cross, 3, 3); }

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="pbyStruct">�ṹԪ��</param>
	/// <param name="cxStruct">�ṹԪ�ؿ��</param>
	/// <param name="cyStruct">�ṹԪ�ظ߶�</param>
	void Open(const CImageMat& Dst, const BYTE* pbyStruct, int cxStruct, int cyStruct) const
	{
		Erode(Dst, pbyStruct, cxStruct, cyStruct);
		Dilate(Dst, pbyStruct, cxStruct, cyStruct);
	}

	/// <summary>
	/// �����㡣
	/// 3x3ʮ���νṹԪ��
	/// </summary>
	/// <param name="Dst">���</param>
	EckInline void Open3x3Cross(const CImageMat& Dst) const { Open(Dst, StructElem_3x3_Cross, 3, 3); }

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="pbyStruct">�ṹԪ��</param>
	/// <param name="cxStruct">�ṹԪ�ؿ��</param>
	/// <param name="cyStruct">�ṹԪ�ظ߶�</param>
	void Close(const CImageMat& Dst, const BYTE* pbyStruct, int cxStruct, int cyStruct) const
	{
		Dilate(Dst, pbyStruct, cxStruct, cyStruct);
		Erode(Dst, pbyStruct, cxStruct, cyStruct);
	}

	/// <summary>
	/// �����㡣
	/// 3x3ʮ���νṹԪ��
	/// </summary>
	/// <param name="Dst">���</param>
	EckInline void Close3x3Cross(const CImageMat& Dst) const { Close(Dst, StructElem_3x3_Cross, 3, 3); }

	/// <summary>
	/// ��˹ģ����
	/// �÷���ÿ�ε��ö����������ɺˣ���Ҫ���úˣ���ֱ��ʹ��Convolve������
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="nRadius">�뾶</param>
	/// <param name="eBorder">�߽�ѡ��</param>
	/// <param name="fSigma">��׼��</param>
	EckInline void GaussianBlur(const CImageMat& Dst, int nRadius,
		BorderOpt eBorder = BorderOpt::Zero, float fSigma = 3.f) const
	{
		const auto nSize = nRadius * 2 + 1;
		const auto Kernel = GenerateGaussianKernel(nSize, fSigma);
		Convolve(Dst, Kernel.Data(), nSize, eBorder);
	}

	/// <summary>
	/// ȡĳͨ���Ҷ�ͼ
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="eChannel">ͨ��</param>
	void GrayscaleChannel(const CImageMat& Dst, ImageChannel eChannel) const
	{
		for (int i = 0; i < m_cx; ++i)
			for (int j = 0; j < m_cy; ++j)
			{
				const auto Pix = Pixel(i, j);
				auto& NewPix = Dst.Pixel(i, j);
				const BYTE Val = (eChannel == ImageChannel::Red ? Pix.r :
					(eChannel == ImageChannel::Green ? Pix.g :
						(eChannel == ImageChannel::Blue ? Pix.b :
							(eChannel == ImageChannel::Alpha ? Pix.a :
								0))));
				NewPix.r = NewPix.g = NewPix.b = Val;
				NewPix.a = 0xFF;
			}
	}

	// ȡ��͸������������Ӿ���
	void GetContentRect(RECT& rc) const
	{
		for (int i = 0; i < m_cx; ++i)
			for (int j = 0; j < m_cy; ++j)
			{
				if (Pixel(i, j).a)
				{
					rc.left = i;
					goto EndL;
				}
			}
	EndL:
		for (int i = m_cx - 1; i >= rc.left; --i)
			for (int j = 0; j < m_cy; ++j)
			{
				if (Pixel(i, j).a)
				{
					rc.right = i + 1;
					goto EndR;
				}
			}
	EndR:
		for (int j = 0; j < m_cy; ++j)
			for (int i = rc.left; i < rc.right; ++i)
			{
				if (Pixel(i, j).a)
				{
					rc.top = j;
					goto EndT;
				}
			}
	EndT:
		for (int j = m_cy - 1; j >= rc.top; --j)
			for (int i = rc.left; i < rc.right; ++i)
			{
				if (Pixel(i, j).a)
				{
					rc.bottom = j + 1;
					goto EndB;
				}
			}
	EndB:;
	}

	/// <summary>
	/// ȥ����ɫ����
	/// </summary>
	/// <param name="Dst">���</param>
	/// <param name="byAlpha">Alpha�滻ֵ����Ϊ0���Զ�����</param>
	void RemoveBlackPixels(const CImageMat& Dst, BYTE byAlpha = 0) const
	{
		for (int i = 0; i < m_cx; ++i)
			for (int j = 0; j < m_cy; ++j)
			{
				const auto Pix = Pixel(i, j);
				auto& NewPix = Dst.Pixel(i, j);
				const BYTE byMax = std::max({ Pix.r, Pix.g, Pix.b });
				NewPix.r = std::min(Pix.r + 255 - byMax, 255);
				NewPix.g = std::min(Pix.g + 255 - byMax, 255);
				NewPix.b = std::min(Pix.b + 255 - byMax, 255);
				if (byAlpha)
					NewPix.a = byAlpha;
				else
					NewPix.a = std::min(Pix.a, byMax);
			}
	}
};

class CGpImageMat : public CImageMat
{
private:
	GpBitmap* m_pBitmap{};
	Gdiplus::BitmapData m_BitmapData{};
public:
	ECK_DISABLE_COPY_DEF_CONS(CGpImageMat);
	CGpImageMat(int cx, int cy)
	{
		GdipCreateBitmapFromScan0(cx, cy, 0, PixelFormat32bppARGB, nullptr, &m_pBitmap);
		Lock();
		SetBits(m_BitmapData.Scan0, m_BitmapData.Width, m_BitmapData.Height, m_BitmapData.Stride);
	}

	CGpImageMat(GpBitmap* pBitmap) : m_pBitmap{ pBitmap }
	{
		Lock();
		SetBits(m_BitmapData.Scan0, m_BitmapData.Width, m_BitmapData.Height, m_BitmapData.Stride);
	}

	CGpImageMat(CGpImageMat&& x) noexcept
		: m_pBitmap{ x.m_pBitmap }, m_BitmapData{ x.m_BitmapData }
	{
		x.m_pBitmap = nullptr;
		x.m_BitmapData = {};
		m_pBits = x.m_pBits;
		m_cx = x.m_cx;
		m_cy = x.m_cy;
		m_cbStride = x.m_cbStride;
	}

	CGpImageMat& operator=(CGpImageMat&& x) noexcept
	{
		if (this != &x)
		{
			Unlock();
			GdipDisposeImage(m_pBitmap);
			m_pBitmap = x.m_pBitmap;
			m_BitmapData = x.m_BitmapData;
			x.m_pBitmap = nullptr;
			x.m_BitmapData = {};
			m_pBits = x.m_pBits;
			m_cx = x.m_cx;
			m_cy = x.m_cy;
			m_cbStride = x.m_cbStride;
		}
		return *this;
	}

	~CGpImageMat()
	{
		Unlock();
		GdipDisposeImage(m_pBitmap);
	}

	EckInline [[nodiscard]] constexpr GpBitmap* GetGpBitmap() const { return m_pBitmap; }

	EckInline [[nodiscard]] constexpr const Gdiplus::BitmapData& GetBitmapData() { return m_BitmapData; }

	EckInline [[nodiscard]] constexpr BOOL IsValid() const { return m_pBitmap != nullptr; }

	EckInline GpStatus Lock()
	{
		if (!m_BitmapData.Scan0)
			return GdipBitmapLockBits(m_pBitmap, nullptr, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
				PixelFormat32bppARGB, &m_BitmapData);
		else
			return GpStatus::WrongState;
	}

	EckInline GpStatus Unlock()
	{
		if (m_BitmapData.Scan0)
		{
			const auto r = GdipBitmapUnlockBits(m_pBitmap, &m_BitmapData);
			m_BitmapData = {};
			return r;
		}
		else
			return GpStatus::WrongState;
	}

	EckInline GpBitmap* Attach(GpBitmap* pBitmap)
	{
		Unlock();
		const auto pOld = m_pBitmap;
		m_pBitmap = pBitmap;
		Lock();
		return pOld;
	}

	EckInline [[nodiscard]] GpBitmap* Detach()
	{
		Unlock();
		m_BitmapData = {};
		return std::exchange(m_pBitmap, nullptr);
	}

	EckInline [[nodiscard]] CGpImageMat Duplicate()
	{
		Unlock();
		GpBitmap* pClone = nullptr;
		GdipCloneBitmapAreaI(0, 0, m_BitmapData.Width, m_BitmapData.Height,
			PixelFormat32bppARGB, m_pBitmap, &pClone);
		CGpImageMat Ret{ pClone };
		Lock();
		return Ret;
	}
};

class CPureImageMat : public CImageMat
{
private:
	CArray2D<GPARGB> m_Data{};
public:
	CPureImageMat() = default;
	CPureImageMat(int cx, int cy) : m_Data{ (size_t)cx,(size_t)cy }
	{
		SetBits(m_Data.Data(), cx, cy, cx * 4);
	}

	CPureImageMat(const CImageMat& Src) : m_Data{ (size_t)Src.GetWidth(), (size_t)Src.GetHeight() }
	{
		if (Src.GetWidth() * 4 == Src.GetStride())
			memcpy(m_Data.Data(), Src.GetBits(), Src.GetWidth() * Src.GetHeight() * 4);
		else
			for (int i = 0; i < Src.GetHeight(); ++i)
			{
				memcpy(m_Data[i].AddrOf(),
					(BYTE*)Src.GetBits() + i * Src.GetWidth(),
					Src.GetWidth() * 4);
			}
		SetBits(m_Data.Data(), Src.GetWidth(), Src.GetHeight(), Src.GetWidth() * 4);
	}

	EckInline auto& GetArray2D() { return m_Data; }

	EckInline const auto& GetArray2D() const { return m_Data; }

	EckInline void ReSize(int cx, int cy)
	{
		m_Data.ReDim(cx, cy);
		SetBits(m_Data.Data(), cx, cy, cx * 4);
	}
};
ECK_NAMESPACE_END