#pragma once
#include "CArray2D.h"
#include "MathHelper.h"
#include "CRegion.h"

#include <stack>
#include <set>

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
	0,0,1,0,0,
	0,0,1,0,0,
	1,1,1,1,1,
	0,0,1,0,0,
	0,0,1,0,0
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
	0.077847f, 0.123317f, 0.077847f,
	0.123317f, 0.195346f, 0.123317f,
	0.077847f, 0.123317f, 0.077847f
};

inline constexpr float GaussianKernel_5x5_3Sigma[]
{
	0.003765f, 0.015019f, 0.023792f, 0.015019f, 0.003765f,
	0.015019f, 0.059912f, 0.094907f, 0.059912f, 0.015019f,
	0.023792f, 0.094907f, 0.150342f, 0.094907f, 0.023792f,
	0.015019f, 0.059912f, 0.094907f, 0.059912f, 0.015019f,
	0.003765f, 0.015019f, 0.023792f, 0.015019f, 0.003765f
};

inline constexpr float GaussianKernel_7x7_3Sigma[]
{
	0.000316f, 0.001300f, 0.002979f, 0.004442f, 0.005700f, 0.006765f, 0.000316f,
	0.001300f, 0.006033f, 0.010755f, 0.014676f, 0.017896f, 0.019517f, 0.001300f,
	0.002979f, 0.010755f, 0.024103f, 0.034583f, 0.042099f, 0.046753f, 0.002979f,
	0.004442f, 0.014676f, 0.034583f, 0.051721f, 0.065693f, 0.076399f, 0.004442f,
	0.005700f, 0.017896f, 0.042099f, 0.065693f, 0.086519f, 0.094480f, 0.005700f,
	0.006765f, 0.019517f, 0.046753f, 0.076399f, 0.094480f, 0.100000f, 0.006765f,
	0.000316f, 0.001300f, 0.002979f, 0.004442f, 0.005700f, 0.006765f, 0.000316f
};

enum class BorderOpt
{
	Ignore,
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
		for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
			{
				TArgb Pix;
				float fSumX{};
				float fSumY{};
				for (int l = -cxyHalf; l <= cxyHalf; ++l)
					for (int k = -cxyHalf; k <= cxyHalf; ++k)
					{
						Pix = Pixel(i + k, j + l);
						const auto fX = KernelX[(k + cxyHalf) + (l + cxyHalf) * 3];
						fSumX += (fX * Pix.r);
						const auto fY = KernelY[(k + cxyHalf) + (l + cxyHalf) * 3];
						fSumY += (fY * Pix.r);
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
	/// 置数据
	/// </summary>
	/// <param name="pBits">数据指针</param>
	/// <param name="cx">宽度</param>
	/// <param name="cy">高度</param>
	/// <param name="cbStride">跨步</param>
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

	// 取某点像素
	EckInline constexpr TArgb& Pixel(int x, int y) const
	{
		return *(TArgb*)((BYTE*)m_pBits + y * m_cbStride + x * sizeof(ARGB));
	}

	// 取某点像素。函数根据边界选项自动处理越界情况
	EckInline constexpr TArgb Pixel(int x, int y, BorderOpt eBorder) const
	{
		if (x < 0 || x >= m_cx || y < 0 || y >= m_cy)
			switch (eBorder)
			{
			case BorderOpt::Ignore:
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
	/// 卷积
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="Kernel">卷积核</param>
	/// <param name="cxyKernel">核大小，奇数</param>
	/// <param name="eBorder">边界选项</param>
	constexpr void Convolve(const CImageMat& Dst, const float* Kernel, int cxyKernel,
		BorderOpt eBorder = BorderOpt::Ignore) const
	{
		EckAssert(this != &Dst && L"不支持原地操作");
		const int cxyHalf = cxyKernel / 2;
		const int iOffset = ((eBorder == BorderOpt::Ignore) ? cxyHalf : 0);
		for (int j = iOffset; j < m_cy - iOffset; ++j)
			for (int i = iOffset; i < m_cx - iOffset; ++i)
			{
				TArgb Pix;
				float fSum[4]{};
				for (int l = -cxyHalf; l <= cxyHalf; ++l)
					for (int k = -cxyHalf; k <= cxyHalf; ++k)
					{
						const auto x = i + k;
						const auto y = j + l;
						if (eBorder == BorderOpt::Ignore)
							Pix = Pixel(x, y);
						else
							Pix = Pixel(x, y, eBorder);
						const auto f = Kernel[(k + cxyHalf) + (l + cxyHalf) * cxyKernel];
						fSum[0] += (f * Pix.a);
						fSum[1] += (f * Pix.r);
						fSum[2] += (f * Pix.g);
						fSum[3] += (f * Pix.b);
					}
				auto& NewPix = Dst.Pixel(i, j);
				NewPix.a = (BYTE)fSum[0];
				NewPix.r = (BYTE)fSum[1];
				NewPix.g = (BYTE)fSum[2];
				NewPix.b = (BYTE)fSum[3];
			}
		// 边界处理
		if (eBorder == BorderOpt::Ignore)
			return;
	}

	/// <summary>
	/// Sobel边缘检测
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="bBinary">是否二值化结果</param>
	/// <param name="fThreshold">若bBinary为TRUE，则该参数指定二值化阈值</param>
	/// <returns></returns>
	EckInline void Sobel(const CImageMat& Dst, BOOL bBinary, float fThreshold) const
	{
		EckAssert(this != &Dst && L"不支持原地操作");
		IntEdgeDetect(Dst, KernelX_Sobel, KernelY_Sobel, 3, bBinary, fThreshold);
	}

	/// <summary>
	/// Prewitt边缘检测
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="bBinary">是否二值化结果</param>
	/// <param name="fThreshold">若bBinary为TRUE，则该参数指定二值化阈值</param>
	/// <returns></returns>
	EckInline void Prewitt(const CImageMat& Dst, BOOL bBinary, float fThreshold) const
	{
		EckAssert(this != &Dst && L"不支持原地操作");
		IntEdgeDetect(Dst, KernelX_Prewitt, KernelY_Prewitt, 3, bBinary, fThreshold);
	}

	/// <summary>
	/// Roberts边缘检测
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="bBinary">是否二值化结果</param>
	/// <param name="fThreshold">若bBinary为TRUE，则该参数指定二值化阈值</param>
	/// <returns></returns>
	EckInline void Roberts(const CImageMat& Dst, BOOL bBinary, float fThreshold) const
	{
		EckAssert(this != &Dst && L"不支持原地操作");
		for (int j = 0; j < m_cy - 1; ++j)
			for (int i = 0; i < m_cx - 1; ++i)
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
	/// Canny边缘检测
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="fLowThresh">低阈值</param>
	/// <param name="fHighThresh">高阈值</param>
	void Canny(const CImageMat& Dst, float fLowThresh, float fHighThresh) const
	{
		EckAssert(this != &Dst && L"不支持原地操作");
		struct GRADIENT
		{
			float g;
			float fAngle;
		};
		CArray2D<GRADIENT> G(m_cx, m_cy);
		// 计算梯度
		constexpr int cxyHalf = 3 / 2;
		for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
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
		// 非极大值抑制
		CArray2D<float> Suppressed(m_cx, m_cy);
		for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
			{
				const auto& e = G[i][j];
				auto& f = Suppressed[i][j];
				float angle = (e.fAngle < 0.f ? (e.fAngle + 180.f) : e.fAngle);
				angle = fmod(angle, 180.f);

				float q, r;
				if (angle < 22.5f || angle >= 157.5f)// 水平
				{
					q = G[i + 1][j].g;
					r = G[i - 1][j].g;
				}
				else if (angle >= 22.5f && angle < 67.5f)// 45度
				{
					q = G[i - 1][j + 1].g;
					r = G[i + 1][j - 1].g;
				}
				else if (angle >= 67.5f && angle < 112.5f)// 垂直
				{
					q = G[i][j + 1].g;
					r = G[i][j - 1].g;
				}
				else if (angle >= 112.5f && angle < 157.5f)// 135度
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
		// 双阈值
		constexpr TColor WeakColor{ 0xFFFF0000 };
		constexpr TColor WeakLabelColor{ 0xFF00FF00 };
		for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
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
		// 弱边处理
		std::stack<POINT> s{};
		std::queue<POINT> q{};
		BOOL bConnected = FALSE;
		for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
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
								if (PixNeighbor.dw == WeakColor)// 弱点
								{
									s.emplace(pt.x + k, pt.y + l);
									q.emplace(pt.x + k, pt.y + l);
									PixNeighbor.dw = WeakLabelColor;
								}
								if (!bConnected && PixNeighbor.dw == 0xFFFFFFFF)// 强点
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

	// 到灰度图
	constexpr void Grayscale(const CImageMat& Dst) const
	{
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				const auto Pix = Pixel(i, j);
				auto& NewPix = Dst.Pixel(i, j);
				NewPix.r = NewPix.g = NewPix.b =
					BYTE(0.299f * Pix.r + 0.587f * Pix.g + 0.114f * Pix.b);
				NewPix.a = 0xFF;
			}
	}

	// 到灰度图
	EckInline constexpr void Grayscale() const { Grayscale(*this); }

	/// <summary>
	/// 二值化
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="byThreshold">阈值</param>
	/// <param name="eChannel">参考通道</param>
	constexpr void Binary(const CImageMat& Dst, BYTE byThreshold,
		ImageChannel eChannel = ImageChannel::Gray) const
	{
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				const auto Pix = Dst.Pixel(i, j);
				auto& PixNew = Dst.Pixel(i, j);
				switch (eChannel)
				{
				case ImageChannel::Gray:
					PixNew.dw = (BYTE(0.299f * Pix.r + 0.587f * Pix.g + 0.114f * Pix.b) > byThreshold ?
						0xFFFFFFFF : 0);
					break;
				case ImageChannel::Red:
					PixNew.dw = (Pix.r > byThreshold ? 0xFFFFFFFF : 0);
					break;
				case ImageChannel::Green:
					PixNew.dw = (Pix.g > byThreshold ? 0xFFFFFFFF : 0);
					break;
				case ImageChannel::Blue:
					PixNew.dw = (Pix.b > byThreshold ? 0xFFFFFFFF : 0);
					break;
				case ImageChannel::Alpha:
					PixNew.dw = (Pix.a > byThreshold ? 0xFFFFFFFF : 0);
					break;
				default: ECK_UNREACHABLE;
				}
			}
	}

	/// <summary>
	/// 二值化
	/// </summary>
	/// <param name="byThreshold">阈值</param>
	/// <param name="eChannel">参考通道</param>
	EckInline constexpr void Binary(BYTE byThreshold, ImageChannel eChannel = ImageChannel::Gray) const
	{
		Binary(*this, byThreshold, eChannel);
	}

	// 反色
	constexpr void Invert(const CImageMat& Dst) const
	{
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				const auto Pix = Pixel(i, j);
				auto& NewPix = Dst.Pixel(i, j);
				NewPix.r = 0xFF - Pix.r;
				NewPix.g = 0xFF - Pix.g;
				NewPix.b = 0xFF - Pix.b;
			}
	}

	// 反色
	EckInline constexpr void Invert() const { Invert(*this); }

	/// <summary>
	/// 腐蚀
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="pbyStruct">结构元素</param>
	/// <param name="cxStruct">结构元素宽度</param>
	/// <param name="cyStruct">结构元素高度</param>
	void Erode(const CImageMat& Dst, const BYTE* pbyStruct, int cxStruct, int cyStruct) const
	{
		EckAssert(this != &Dst && L"不支持原地操作");
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				auto& NewPix = Dst.Pixel(i, j);
				NewPix.dw = 0;
				for (int k = 0; k < cyStruct; ++k)
					for (int l = 0; l < cxStruct; ++l)
					{
						const int x = i - cxStruct / 2 + l;
						const int y = j - cyStruct / 2 + k;
						if (x >= 0 && x < m_cx && y >= 0 && y < m_cy &&
							pbyStruct[k * cxStruct + l] &&
							Pixel(x, y).dw)
						{
							NewPix.dw = 0xFFFFFFFF;
							goto NextPixel;
						}
					}
			NextPixel:;
			}
	}

	/// <summary>
	/// 腐蚀。
	/// 3x3十字形结构元素
	/// </summary>
	/// <param name="Dst">结果</param>
	EckInline void Erode3x3Cross(const CImageMat& Dst) const { Erode(Dst, StructElem_3x3_Cross, 3, 3); }

	/// <summary>
	/// 膨胀
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="pbyStruct">结构元素</param>
	/// <param name="cxStruct">结构元素宽度</param>
	/// <param name="cyStruct">结构元素高度</param>
	void Dilate(const CImageMat& Dst, const BYTE* pbyStruct, int cxStruct, int cyStruct) const
	{
		EckAssert(this != &Dst && L"不支持原地操作");
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				auto& NewPix = Dst.Pixel(i, j);
				NewPix.dw = 0xFFFFFFFF;
				for (int k = 0; k < cyStruct; ++k)
					for (int l = 0; l < cxStruct; ++l)
					{
						const int x = i - cxStruct / 2 + l;
						const int y = j - cyStruct / 2 + k;
						if (x >= 0 && x < m_cx && y >= 0 && y < m_cy &&
							pbyStruct[k * cxStruct + l] &&
							Pixel(x, y).dw == 0)
						{
							NewPix.dw = 0;
							goto NextPixel;
						}
					}
			NextPixel:;
			}
	}

	/// <summary>
	/// 膨胀。
	/// 3x3十字形结构元素
	/// </summary>
	/// <param name="Dst">结果</param>
	EckInline void Dilate3x3Cross(const CImageMat& Dst) const { Dilate(Dst, StructElem_3x3_Cross, 3, 3); }

	/// <summary>
	/// 开运算
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="pbyStruct">结构元素</param>
	/// <param name="cxStruct">结构元素宽度</param>
	/// <param name="cyStruct">结构元素高度</param>
	void Open(const CImageMat& Dst, const BYTE* pbyStruct, int cxStruct, int cyStruct) const
	{
		Erode(Dst, pbyStruct, cxStruct, cyStruct);
		Dilate(Dst, pbyStruct, cxStruct, cyStruct);
	}

	/// <summary>
	/// 开运算。
	/// 3x3十字形结构元素
	/// </summary>
	/// <param name="Dst">结果</param>
	EckInline void Open3x3Cross(const CImageMat& Dst) const { Open(Dst, StructElem_3x3_Cross, 3, 3); }

	/// <summary>
	/// 闭运算
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="pbyStruct">结构元素</param>
	/// <param name="cxStruct">结构元素宽度</param>
	/// <param name="cyStruct">结构元素高度</param>
	void Close(const CImageMat& Dst, const BYTE* pbyStruct, int cxStruct, int cyStruct) const
	{
		Dilate(Dst, pbyStruct, cxStruct, cyStruct);
		Erode(Dst, pbyStruct, cxStruct, cyStruct);
	}

	/// <summary>
	/// 闭运算。
	/// 3x3十字形结构元素
	/// </summary>
	/// <param name="Dst">结果</param>
	EckInline void Close3x3Cross(const CImageMat& Dst) const { Close(Dst, StructElem_3x3_Cross, 3, 3); }

	/// <summary>
	/// 高斯模糊。
	/// 该方法每次调用都会重新生成核，若要复用核，请直接使用Convolve方法。
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="nRadius">半径</param>
	/// <param name="eBorder">边界选项</param>
	/// <param name="fSigma">标准差</param>
	EckInline void GaussianBlur(const CImageMat& Dst, int nRadius,
		BorderOpt eBorder = BorderOpt::Zero, float fSigma = 3.f) const
	{
		EckAssert(this != &Dst && L"不支持原地操作");
		const auto nSize = nRadius * 2 + 1;
		const auto Kernel = GenerateGaussianKernel(nRadius, fSigma);
		Convolve(Dst, Kernel.Data(), nSize, eBorder);
	}

	// 到某通道灰度图
	constexpr void GrayscaleChannel(const CImageMat& Dst, ImageChannel eChannel) const
	{
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
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

	// 到某通道灰度图
	EckInline constexpr void GrayscaleChannel(ImageChannel eChannel) const
	{
		GrayscaleChannel(*this, eChannel);
	}

	/// <summary>
	/// 取满足指定条件像素区域的外接矩形
	/// </summary>
	/// <param name="rc">结果</param>
	/// <param name="fn">判断过程，应有如下形式：BOOL fn(TArgb cr)，若满足某条件则返回非零</param>
	template<class FProc>
	void GetBoundingRectByCondition(RECT& rc, FProc fn) const
	{
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				if (fn(Pixel(i, j)))
				{
					rc.left = i;
					goto EndL;
				}
			}
	EndL:
		for (int j = 0; j < m_cy; ++j)
			for (int i = m_cx - 1; i >= rc.left; --i)
			{
				if (fn(Pixel(i, j)))
				{
					rc.right = i + 1;
					goto EndR;
				}
			}
	EndR:
		for (int i = rc.left; i < rc.right; ++i)
			for (int j = 0; j < m_cy; ++j)
			{
				if (fn(Pixel(i, j)))
				{
					rc.top = j;
					goto EndT;
				}
			}
	EndT:
		for (int i = rc.left; i < rc.right; ++i)
			for (int j = m_cy - 1; j >= rc.top; --j)
			{
				if (fn(Pixel(i, j)))
				{
					rc.bottom = j + 1;
					goto EndB;
				}
			}
	EndB:;
	}

	// 取非透明像素区域外接矩形
	EckInline void GetOpaqueRect(RECT& rc, BYTE byMinAlpha = 100) const
	{
		GetBoundingRectByCondition(rc, [=](TArgb cr) -> BOOL { return cr.a >= byMinAlpha; });
	}

	/// <summary>
	/// 去除黑色像素
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="byAlpha">Alpha替换值，若为0则自动计算</param>
	constexpr void RemoveBlackPixels(const CImageMat& Dst, BYTE byAlpha = 0) const
	{
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
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

	/// <summary>
	/// 去除黑色像素
	/// </summary>
	/// <param name="byAlpha">Alpha替换值，若为0则自动计算</param>
	constexpr void RemoveBlackPixels(BYTE byAlpha = 0) const
	{
		RemoveBlackPixels(*this, byAlpha);
	}

	/// <summary>
	/// 洪水填充。
	/// 函数原地工作
	/// </summary>
	/// <param name="x">起始X</param>
	/// <param name="y">起始Y</param>
	/// <param name="crNew">替换为</param>
	void FloodFill(int x, int y, TColor crNew) const
	{
		const auto crOld = Pixel(x, y).dw;
		if (crOld == crNew)
			return;

		std::stack<POINT> s{};
		s.emplace(x, y);

		while (!s.empty())
		{
			const auto pt = s.top();
			s.pop();

			// 向左扫描
			int l = pt.x;
			while (l >= 0 && Pixel(l, pt.y).dw == crOld)
				l--;
			l++;// 恢复到最后一个有效位置

			// 向右扫描
			int r = pt.x;
			while (r < m_cx && Pixel(r, pt.y).dw == crOld)
				r++;
			r--;// 恢复到最后一个有效位置

			// 填充这一行
			for (int i = l; i <= r; ++i)
			{
				Pixel(i, pt.y).dw = crNew;
				// 上一行
				if (pt.y - 1 >= 0 && Pixel(i, pt.y - 1).dw == crOld)
					s.emplace(i, pt.y - 1);
				// 下一行
				if (pt.y + 1 < m_cy && Pixel(i, pt.y + 1).dw == crOld)
					s.emplace(i, pt.y + 1);
			}
		}
	}

	/// <summary>
	/// 替换某通道
	/// </summary>
	/// <param name="Ref">替换为</param>
	/// <param name="eChannel">通道</param>
	constexpr void ReplaceChannel(const CImageMat& Ref, ImageChannel eChannel) const
	{
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				auto& Pix = Pixel(i, j);
				const auto RefPix = Ref.Pixel(i, j);
				switch (eChannel)
				{
				case ImageChannel::Red:
					Pix.r = RefPix.r;
					break;
				case ImageChannel::Green:
					Pix.g = RefPix.g;
					break;
				case ImageChannel::Blue:
					Pix.b = RefPix.b;
					break;
				case ImageChannel::Alpha:
					Pix.a = RefPix.a;
					break;
				default: ECK_UNREACHABLE;
				}
			}
	}

	/// <summary>
	/// 灰度蒙版。
	/// 将像素透明度按指定灰度图的对应灰度缩放
	/// </summary>
	/// <param name="Mask">灰度蒙版</param>
	/// <param name="bIgnoreAlpha">是否忽略蒙版Alpha通道</param>
	constexpr void MaskGrayscale(const CImageMat& Mask, BOOL bIgnoreAlpha = TRUE) const
	{
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				auto& Pix = Pixel(i, j);
				const auto MaskPix = Mask.Pixel(i, j);
				if (bIgnoreAlpha)
					Pix.a = BYTE(Pix.a * MaskPix.r / 255);
				else
					Pix.a = BYTE(Pix.a * (MaskPix.r * MaskPix.a / 255) / 255);
			}
	}

	/// <summary>
	/// 灰度蒙版。
	/// 将像素透明度按指定灰度图的对应灰度缩放
	/// </summary>
	/// <param name="Dst">结果</param>
	/// <param name="Mask">灰度蒙版</param>
	/// <param name="bIgnoreAlpha">是否忽略蒙版Alpha通道</param>
	constexpr void MaskGrayscale(const CImageMat& Dst, const CImageMat& Mask,
		BOOL bIgnoreAlpha = TRUE) const
	{
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				const auto Pix = Pixel(i, j);
				const auto MaskPix = Mask.Pixel(i, j);
				auto& NewPix = Dst.Pixel(i, j);
				NewPix.dw = Pix.dw;
				if (bIgnoreAlpha)
					NewPix.a = BYTE(Pix.a * MaskPix.r / 255);
				else
					NewPix.a = BYTE(Pix.a * (MaskPix.r * MaskPix.a / 255) / 255);
			}
	}

	/// <summary>
	/// 清除杂点
	/// </summary>
	/// <param name="nLevel">若某像素的八连通像素数小于此参数则被认为是杂点</param>
	void RemoveNoisePixels(int nLevel = 1) const
	{
		constexpr TColor Marker = 0xFFFF0000;
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				auto& Pix = Pixel(i, j);
				if (Pix.dw)
				{
					int c{};
					for (int k = -1; k <= 1; ++k)
						for (int l = -1; l <= 1; ++l)
						{
							if (Pixel(i + k, j + l, BorderOpt::Zero).dw)
							{
								++c;
								if (c >= nLevel)
									goto NextPixel;
							}
						}
					if (c < nLevel)
						Pix.dw = Marker;
				}
			NextPixel:;
			}
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				auto& Pix = Pixel(i, j);
				if (Pix.dw == Marker)
					Pix.dw = 0;
			}
	}

	/// <summary>
	/// 色彩增强
	/// </summary>
	/// <param name="fFactor">增强因子</param>
	constexpr void EnhanceColor(float fFactor) const
	{
		for (int j = 0; j < m_cy; ++j)
			for (int i = 0; i < m_cx; ++i)
			{
				auto& Pix = Pixel(i, j);
				BYTE byColor[3]{ Pix.r, Pix.g, Pix.b };
				BYTE byIdx[3]{ 0,1,2 };
				for (int k = 0; k < 3; ++k)
					for (int l = k + 1; l < 3; ++l)
						if (byColor[k] < byColor[l])
						{
							std::swap(byColor[k], byColor[l]);
							std::swap(byIdx[k], byIdx[l]);
						}
				byColor[0] = (BYTE)std::min(byColor[0] + fFactor * 0.5f, 255.f);
				byColor[1] = (BYTE)std::max(byColor[1] - fFactor * 0.1687f, 0.f);
				byColor[2] = (BYTE)std::max(byColor[2] - fFactor * 0.3313f, 0.f);
				for (int k = 0; k < 3; ++k)
					if (byIdx[k] == 0)
						Pix.r = byColor[k];
					else if (byIdx[k] == 1)
						Pix.g = byColor[k];
					else
						Pix.b = byColor[k];
			}
	}

	constexpr static TColor TBMarkPt{ 0xFFFF0000 };
private:
	// ================================================
	// Part of the code was written by Alexbeast-CN and 
	// modified here, under the Apache License 2.0
	// https://github.com/Alexbeast-CN/findContours
	// ================================================

	constexpr static int TBInvalidPt{ (int)0x80000000 };

	constexpr POINT TraceBoundary_FindNeighbor(POINT ptCenter, POINT ptStart, bool ClockWise) const
	{
		constexpr POINT Neighbors[]{ {0,0},{0,1},{0,2},{1,2},{2,2},{2,1},{2,0},{1,0} };
		constexpr int Index[3][3]{ {0,1,2},{7,0,3},{6,5,4} };
		const int idxStart = Index[ptStart.x - ptCenter.x + 1][ptStart.y - ptCenter.y + 1];
		const int iWeight = (ClockWise ? 1 : -1);

		for (int i = 1; i < 8 + 1; ++i)
		{
			const int idxCurr = (idxStart + i * iWeight + 8) % 8;
			const int x = ptCenter.x + Neighbors[idxCurr].x - 1;
			const int y = ptCenter.y + Neighbors[idxCurr].y - 1;
			if (Pixel(x, y).dw == 0xFFFFFFFF)
				return { x,y };
		}
		return { TBInvalidPt,TBInvalidPt };
	}

	constexpr void TraceBoundary_Follow(POINT ptCenter, POINT ptStart, BOOL bClockWise,
		std::vector<POINT>& vEdge) const
	{
		Pixel(ptCenter.x, ptCenter.y).dw = TBMarkPt;
		POINT ptNewCenter = ptCenter;
		POINT ptNeighbor = ptStart;
		POINT ptNewNeighbor = TraceBoundary_FindNeighbor(ptNewCenter, ptNeighbor, bClockWise);
		while (ptNewNeighbor.x != TBInvalidPt && ptNewNeighbor.y != TBInvalidPt)
		{
			int x = ptNewCenter.x;
			int y = ptNewCenter.y;
			Pixel(ptNewCenter.x, ptNewCenter.y).dw = TBMarkPt;
			vEdge.emplace_back(ptNewCenter);

			ptNeighbor = ptNewCenter;
			ptNewCenter = ptNewNeighbor;
			ptNewNeighbor = TraceBoundary_FindNeighbor(ptNewCenter, ptNeighbor, bClockWise);
		}
	}
public:
	/// <summary>
	/// 跟踪边界。
	/// 函数分析二值图像的边界，且分析出的每条边都满足以下条件：
	/// 1.点数大于等于cMinPoints；
	/// 2.每个点都与前后两个点构成八连通；
	/// 3.与其他任意一条边不相交。
	/// 注意：调用此函数后，原图像内容将被破坏（原有白色像素被设为CImageMat::TBMarkPt），若要执行复原，请调用TraceBoundary_Restore()
	/// </summary>
	/// <param name="vBoundary">结果边</param>
	/// <param name="cMinPoints">边中至少包含的点数</param>
	constexpr void TraceBoundary(std::vector<std::vector<POINT>>& vBoundary, size_t cMinPoints = 10) const
	{
		for (int i = 0; i < m_cy; ++i)
			for (int j = 1; j < m_cx - 1; ++j)
			{
				if (Pixel(j, i).dw == 0xFFFFFFFF && Pixel(j - 1, i).dw == 0)// out
				{
					TraceBoundary_Follow({ j,i }, { j - 1,i }, FALSE, vBoundary.emplace_back());
					if (vBoundary.back().size() < cMinPoints)
						vBoundary.pop_back();
				}
				else if (Pixel(j, i).dw == 0xFFFFFFFF && Pixel(j + 1, i).dw == 0)// in
				{
					TraceBoundary_Follow({ j,i }, { j + 1,i }, TRUE, vBoundary.emplace_back());
					if (vBoundary.back().size() < cMinPoints)
						vBoundary.pop_back();
				}
			}
	}

	// 恢复由TraceBoundary函数破坏的图像内容
	constexpr void TraceBoundary_Restore() const
	{
		for (int i = 0; i < m_cx; ++i)
			for (int j = 0; j < m_cy; ++j)
			{
				if (Pixel(i, j).dw == TBMarkPt)
					Pixel(i, j).dw = 0xFFFFFFFF;
			}
	}

	/// <summary>
	/// 化简边界。
	/// 化简由TraceBoundary分析出的边界
	/// </summary>
	/// <param name="vBoundary">边界</param>
	/// <param name="fDistMin">平行线间的最小距离，设为负值表示不移除平行线</param>
	/// <param name="fDistDelta">平行线贴边最大浮动距离</param>
	/// <param name="cLineStep">化简直线时保留点的步长，设为0不执行直线化简，设为INT_MAX执行最大化简</param>
	/// <param name="iLineTolerance">化简直线时的容差</param>
	static void TraceBoundary_Simplify(std::vector<std::vector<POINT>>& vBoundary,
		float fDistMin, float fDistDelta, int cLineStep, int iLineTolerance)
	{
		// 移除平行边
		if (fDistMin >= 0.f)
		{
			std::set<size_t> hsIdxToRemove{};
			for (size_t i = 0; i < vBoundary.size(); ++i)
			{
				for (size_t j = i + 1; j < vBoundary.size(); ++j)
				{
					if (hsIdxToRemove.contains(i))
						goto NextI;
					if (hsIdxToRemove.contains(j))
						continue;
					size_t idxMin, idxMax;
					if (vBoundary[i].size() < vBoundary[j].size())
						idxMin = i, idxMax = j;
					else
						idxMin = j, idxMax = i;
					const auto& vMin = vBoundary[idxMin];
					const auto& vMax = vBoundary[idxMax];

					float fDist{ FLT_MAX };
					size_t idxBegin{};
					for (size_t k{}; const auto & e : vMax)
					{
						const auto f = CalcLineLength(e, vMin[0]);
						if (f < fDist)
						{
							fDist = f;
							idxBegin = k;
						}
						++k;
					}
					if (fDist < fDistMin)
					{
						// 是否应被删除
						BOOL b1{ TRUE }, b2{ TRUE };
						// 向后
						size_t k{ idxBegin };
						EckLoop()
						{
							if (k - idxBegin >= vMin.size())// 小边结束，应该删除
							{
								b1 = TRUE;
								break;
							}
							if (k >= vMax.size())// 大边结束，若小边也结束，应该删除
							{
								b1 = (k - idxBegin == vMin.size());
								break;
							}
							const auto f = CalcLineLength(vMax[k], vMin[k - idxBegin]);
							if (fabsf(f - fDist) > fDistDelta)// 浮动超出容差，不应该删除
							{
								b1 = FALSE;
								break;
							}
							++k;
						}
						// 向前
						if (!b1)
						{
							k = idxBegin;
							EckLoop()
							{
								if (idxBegin - k >= vMin.size())// 小边结束，应该删除
								{
									b2 = TRUE;
									break;
								}
								if (k == 0)// 大边结束，若小边也结束，应该删除
								{
									b2 = (idxBegin - k == vMin.size());
									break;
								}
								const auto f = CalcLineLength(vMax[k], vMin[idxBegin - k]);
								if (fabsf(f - fDist) > fDistDelta)// 浮动超出容差，不应该删除
								{
									b2 = FALSE;
									break;
								}
								--k;
							}
						}
						if (b1 || b2)
							hsIdxToRemove.insert(idxMin);
					}
				}
			NextI:;
			}
			for (auto it = hsIdxToRemove.rbegin(); it != hsIdxToRemove.rend(); ++it)
				vBoundary.erase(vBoundary.begin() + *it);
		}
		// 化简直线
		if (cLineStep > 0)
			for (auto& v : vBoundary)
			{
				if (v.size() < 3)
					continue;
				size_t idxRef{};
				std::vector<size_t> vIdxToRemove{};
				for (size_t i = 2; i < v.size() - 1; ++i)
				{
					if (CalcPointToLineDistance(v[i], v[idxRef], v[idxRef + 1]) > iLineTolerance)
						idxRef = i - 1;
					else
						vIdxToRemove.push_back(i);
				}
				if (cLineStep == INT_MAX)
					for (auto it = vIdxToRemove.rbegin(); it != vIdxToRemove.rend(); ++it)
						v.erase(v.begin() + *it);
				else
				{
					size_t idxLastRemoved{ vIdxToRemove.back() };
					for (auto it = vIdxToRemove.rbegin(); it != vIdxToRemove.rend(); ++it)
					{
						if (*it - idxLastRemoved < cLineStep)
							v.erase(v.begin() + *it);
						else
							idxLastRemoved = *it;
					}
				}
			}
	}

	template<class FProc>
	CRegion GetRegionByCondition(FProc fn) const
	{
		CRegion Ret{};
		auto rpr = Ret.RpBegin();
		for (int j = 0; j < m_cy; ++j)
		{
			rpr.BeginLine(j);
			for (int i = 0; i < m_cx; ++i)
			{
				if (fn(Pixel(i, j)))
					rpr.AddPoint(i);
			}
			rpr.EndLine();
		}
		Ret.RpEnd(rpr);
		return Ret;
	}

	EckInline CRegion GetOpaqueRegion(BYTE byMinAlpha = 100) const
	{
		return GetRegionByCondition([=](TArgb cr) { return cr.a >= byMinAlpha; });
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
			return GdipBitmapLockBits(m_pBitmap, nullptr,
				Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
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

	CPureImageMat(const CImageMat& Src)
		: m_Data{ (size_t)Src.GetWidth(), (size_t)Src.GetHeight() }
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