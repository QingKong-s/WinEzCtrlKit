/*
* WinEzCtrlKit Library
*
* CImgMat.h : 图像矩阵
*
* Copyright(C) 2023-2024 QingKong
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

	void IntEdgeDetect(CImageMat& Dst, const float* KernelX, const float* KernelY,
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

	// 取某点像素
	EckInline constexpr TArgb& Pixel(int x, int y) const
	{
		return *(TArgb*)((BYTE*)m_pBits + y * m_cbStride + x * sizeof(ARGB));
	}

	/// <summary>
	/// 卷积
	/// </summary>
	/// <param name="Kernel">卷积核</param>
	/// <param name="cxyKernel">核大小，奇数</param>
	constexpr void Convolve(CImageMat& Dst, const float* Kernel, int cxyKernel) const
	{
		const int cxyHalf = cxyKernel / 2;
		for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
			for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			{
				TArgb pix;
				float fSum[4]{};
				for (int l = -cxyHalf; l <= cxyHalf; ++l)
					for (int k = -cxyHalf; k <= cxyHalf; ++k)
					{
						pix = Pixel(i + k, j + l);
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
	}

	EckInline void Sobel(CImageMat& Dst, BOOL bBinary, float fThreshold) const
	{
		IntEdgeDetect(Dst, KernelX_Sobel, KernelY_Sobel, 3, bBinary, fThreshold);
	}

	EckInline void Prewitt(CImageMat& Dst, BOOL bBinary, float fThreshold) const
	{
		IntEdgeDetect(Dst, KernelX_Prewitt, KernelY_Prewitt, 3, bBinary, fThreshold);
	}

	/// <summary>
	/// Canny边缘检测。
	/// 此方法不修改当前图像，图像处理前应高斯滤波
	/// </summary>
	/// <param name="Dst">检测结果</param>
	/// <param name="fLowThresh">低阈值</param>
	/// <param name="fHighThresh">高阈值</param>
	void Canny(CImageMat& Dst, float fLowThresh, float fHighThresh) const
	{
		struct GRADIENT
		{
			float g;
			float fAngle;
		};
		CArray2D<GRADIENT> G{};
		G.ReDim(m_cx, m_cy);
		// 计算梯度
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

		CArray2D<float> Suppressed{};
		Suppressed.ReDim(m_cx, m_cy);
		// 非极大值抑制
		for (int i = cxyHalf; i < m_cx - cxyHalf; ++i)
			for (int j = cxyHalf; j < m_cy - cxyHalf; ++j)
			{
				const auto& e = G[i][j];
				auto& f = Suppressed[i][j];
				float angle = (e.fAngle < 0.f ? (e.fAngle + 180.f) : e.fAngle);
				angle = fmod(angle, 180.f);

				float q, r;
				if (angle < 22.5 || angle >= 157.5)// 水平
				{
					q = G[i + 1][j].g;
					r = G[i - 1][j].g;
				}
				else if (angle >= 22.5 && angle < 67.5)// 45度
				{
					q = G[i - 1][j + 1].g;
					r = G[i + 1][j - 1].g;
				}
				else if (angle >= 67.5 && angle < 112.5)// 垂直
				{
					q = G[i][j + 1].g;
					r = G[i][j - 1].g;
				}
				else if (angle >= 112.5 && angle < 157.5)// 135度
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
		// 弱边处理
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
								if (PixNeighbor.dw == WeakColor)// 弱点
								{
									s.emplace(pt.x + k, pt.y + l);
									q.emplace(pt.x + k, pt.y + l);
									PixNeighbor.dw = WeakLabelColor;
								}
								if (bConnected == FALSE && PixNeighbor.dw == 0xFFFFFFFF)// 强点
									bConnected = TRUE;
							}
					}

					if (bConnected == FALSE)
					{
						while (!q.empty())
						{
							const POINT pt = q.front();
							q.pop();
							Dst.Pixel(pt.x, pt.y).dw = 0;
						}
					}
					else
					{
						while (!q.empty())
						{
							const POINT pt = q.front();
							q.pop();
							Dst.Pixel(pt.x, pt.y).dw = 0xFFFFFFFF;
						}
						bConnected = FALSE;
					}
				}
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
ECK_NAMESPACE_END