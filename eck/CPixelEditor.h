#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
// https://blog.ivank.net/fastest-gaussian-blur.html
inline void CalculateBoxBlurFromGaussian(
    float fSigma,
    _Out_writes_all_(n) UINT* pcxBox,
    UINT n) noexcept
{
    auto wl = (UINT)floorf(sqrtf((12.f * fSigma * fSigma / n) + 1.f));
    if (wl % 2 == 0)
        wl--;
    const auto wu = wl + 2;
    const auto mIdeal =
        (12 * fSigma * fSigma - n * wl * wl - 4 * n * wl - 3 * n) / (-4 * wl - 4);
    const auto m = (UINT)roundf(mIdeal);
    for (UINT i = 0; i < n; i++)
        *pcxBox++ = (i < m ? wl : wu);
}


namespace PixelEditor
{
    template<
        CcpNumber TChannel_,
        BYTE IdxR,
        BYTE IdxG,
        BYTE IdxB,
        BYTE IdxA,
        bool IsPremultiplied,
        bool IsSingleChannel = false
    >
    struct PixelAdapter
    {
        using TChannel = TChannel_;

        constexpr static bool HasAlpha = IdxA < 4;
        static_assert(HasAlpha ? TRUE : !IsPremultiplied,
            "Premultiplied alpha is not supported when there is no alpha channel.");

        static_assert(IsSingleChannel ?
            (IdxR == IdxG && IdxG == IdxB && !IdxR && IdxA >= 4) : TRUE,
            "For single channel formats, R, G, and B indices must be the same and A index must be invalid.");

        constexpr static bool IsFloat = std::floating_point<TChannel>;
        constexpr static size_t ChannelCount = IsSingleChannel ? 1 : (HasAlpha ? 4 : 3);

        using TPixelArray = std::array<TChannel, ChannelCount>;
        using TBigInt = std::conditional_t<(sizeof(TChannel) <= 2), UINT, UINT64>;
        using TFloat = std::conditional_t<std::floating_point<TChannel>, TChannel, float>;
        using TSum = std::conditional_t<IsFloat, TFloat, TBigInt>;

        constexpr static TChannel ChannelMaximum =
            IsFloat ? TChannel(1) : std::numeric_limits<TChannel>::max();
        constexpr static TChannel ChannelMinimum{};

        struct Pixel
        {
            TPixelArray Pix{};

            constexpr Pixel() noexcept = default;
            constexpr Pixel(TChannel R, TChannel G, TChannel B, TChannel A = 0) noexcept
            {
                Pix[IdxR] = R;
                Pix[IdxG] = G;
                Pix[IdxB] = B;
                if constexpr (HasAlpha)
                    Pix[IdxA] = A;
            }

            EckInlineNdCe TChannel& R() noexcept { return Pix[IdxR]; }
            EckInlineNdCe TChannel R() const noexcept { return Pix[IdxR]; }
            EckInlineNdCe TChannel& G() noexcept { return Pix[IdxG]; }
            EckInlineNdCe TChannel G() const noexcept { return Pix[IdxG]; }
            EckInlineNdCe TChannel& B() noexcept { return Pix[IdxB]; }
            EckInlineNdCe TChannel B() const noexcept { return Pix[IdxB]; }
            EckInlineNdCe TChannel& A() noexcept { return Pix[IdxA]; }
            EckInlineNdCe TChannel A() const noexcept { return Pix[IdxA]; }

            EckInlineNdCe bool operator==(const Pixel& x) const noexcept
            {
                return Pix == x.Pix;
            }
        };

        struct PixelSum
        {
            std::array<TSum, ChannelCount> Pix{};

            constexpr PixelSum(const Pixel& x) noexcept
            {
                for (size_t i = 0; i < ChannelCount; ++i)
                    Pix[i] = x.Pix[i];
            }

            constexpr PixelSum& operator+=(const Pixel& x) noexcept
            {
                for (size_t i = 0; i < ChannelCount; ++i)
                    Pix[i] += x.Pix[i];
                return *this;
            }
            constexpr PixelSum& operator-=(const Pixel& x) noexcept
            {
                for (size_t i = 0; i < ChannelCount; ++i)
                    Pix[i] -= x.Pix[i];
                return *this;
            }

            constexpr PixelSum& operator+=(const PixelSum& x) noexcept
            {
                for (size_t i = 0; i < ChannelCount; ++i)
                    Pix[i] += x.Pix[i];
                return *this;
            }
            constexpr PixelSum& operator-=(const PixelSum& x) noexcept
            {
                for (size_t i = 0; i < ChannelCount; ++i)
                    Pix[i] -= x.Pix[i];
                return *this;
            }

            constexpr PixelSum operator+(const PixelSum& x) const noexcept
            {
                PixelSum Result{ *this };
                for (size_t i = 0; i < ChannelCount; ++i)
                    Result.Pix[i] += x.Pix[i];
                return Result;
            }
            constexpr PixelSum operator-(const PixelSum& x) const noexcept
            {
                PixelSum Result{ *this };
                for (size_t i = 0; i < ChannelCount; ++i)
                    Result.Pix[i] -= x.Pix[i];
                return Result;
            }
            constexpr PixelSum operator*(CcpNumber auto x) const noexcept
            {
                PixelSum Result{ *this };
                for (size_t i = 0; i < ChannelCount; ++i)
                    if constexpr (IsFloat)
                        Result.Pix[i] = TSum(Result.Pix[i] * x);
                    else
                        Result.Pix[i] = TSum(Result.Pix[i] * x + 0.5f);
                return Result;
            }
            constexpr PixelSum operator/(CcpNumber auto x) const noexcept
            {
                PixelSum Result{ *this };
                for (size_t i = 0; i < ChannelCount; ++i)
                    if constexpr (IsFloat)
                        Result.Pix[i] = TSum(Result.Pix[i] / x);
                    else
                        Result.Pix[i] = TSum(Result.Pix[i] / x + 0.5f);
                return Result;
            }

            Pixel ToPixel() const noexcept
            {
                Pixel Result;
                for (size_t i = 0; i < ChannelCount; ++i)
                    Result.Pix[i] = TChannel(std::clamp(Pix[i], TSum(ChannelMinimum), TSum(ChannelMaximum)));
                return Result;
            }
        };

        void* Bits{};
        UINT Width{};
        UINT Height{};
        UINT Stride{};

        EckInlineNdCe Pixel& GetRaw(UINT x, UINT y) noexcept
        {
            return *(Pixel*)((BYTE*)Bits + y * Stride + x * sizeof(Pixel));
        }
        EckInlineNdCe Pixel& GetRaw(UINT x, UINT y) const noexcept
        {
            return *(Pixel*)((BYTE*)Bits + y * Stride + x * sizeof(Pixel));
        }

        EckInlineNdCe Pixel Get(UINT x, UINT y) const noexcept
        {
            const auto& Pix = GetRaw(x, y);
            if constexpr (IsPremultiplied)
            {
                const TChannel Alpha = Pix.A();
                if (Alpha == (TChannel)0)
                    return Pix;
                Pixel Result = Pix;
                if constexpr (IsFloat)
                {
                    Result.R() /= Alpha;
                    Result.G() /= Alpha;
                    Result.B() /= Alpha;
                }
                else
                {
                    Result.R() = TChannel((TBigInt(Pix.R()) * ChannelMaximum) / Alpha);
                    Result.G() = TChannel((TBigInt(Pix.G()) * ChannelMaximum) / Alpha);
                    Result.B() = TChannel((TBigInt(Pix.B()) * ChannelMaximum) / Alpha);
                }
                return Result;
            }
            else
                return Pix;
        }

        EckInlineCe void Set(UINT x, UINT y, const Pixel& Pix) noexcept
        {
            auto& Dst = *(Pixel*)((BYTE*)Bits + y * Stride + x * sizeof(Pixel));
            if constexpr (IsPremultiplied)
            {
                const TChannel Alpha = Pix.A();
                if constexpr (IsFloat)
                {
                    Dst.R() = Pix.R() * Alpha;
                    Dst.G() = Pix.G() * Alpha;
                    Dst.B() = Pix.B() * Alpha;
                }
                else
                {
                    Dst.R() = TChannel((TBigInt(Pix.R()) * Alpha) / ChannelMaximum);
                    Dst.G() = TChannel((TBigInt(Pix.G()) * Alpha) / ChannelMaximum);
                    Dst.B() = TChannel((TBigInt(Pix.B()) * Alpha) / ChannelMaximum);
                }
                Dst.A() = Alpha;
            }
            else
                Dst = Pix;
        }
    };

    using RGBA8 = PixelAdapter<BYTE, 0, 1, 2, 3, false>;
    using BGRA8 = PixelAdapter<BYTE, 2, 1, 0, 3, false>;
    using BGRA8P = PixelAdapter<BYTE, 0, 1, 2, 3, true>;

    enum class Channel
    {
        Red,
        Green,
        Blue,
        Alpha,
        Gray
    };
}

template<class TAdapter>
class CPixelEditor
{
public:
    using TChannel = typename TAdapter::TChannel;
    using TPixel = typename TAdapter::Pixel;
    using TFloat = typename TAdapter::TFloat;
    using TPixelSum = typename TAdapter::PixelSum;

    constexpr static TChannel ChannelMaximum = TAdapter::ChannelMaximum;
    constexpr static TChannel ChannelMinimum = TAdapter::ChannelMinimum;
private:
    TAdapter m_Adapter{};
public:
    EckInlineCe void SetData(void* Bits, UINT Width, UINT Height, UINT Stride) noexcept
    {
        m_Adapter.Bits = Bits;
        m_Adapter.Width = Width;
        m_Adapter.Height = Height;
        m_Adapter.Stride = Stride;
    }
    EckInlineNdCe UINT GetWidth() const noexcept { return m_Adapter.Width; }
    EckInlineNdCe UINT GetHeight() const noexcept { return m_Adapter.Height; }

    EckInlineNdCe TPixel GetPixel(UINT x, UINT y) const noexcept { return m_Adapter.Get(x, y); }
    EckInlineCe void SetPixel(UINT x, UINT y, const TPixel& Pix) noexcept { m_Adapter.Set(x, y, Pix); }

    EckInlineNdCe TPixel& GetPixelRaw(UINT x, UINT y) noexcept { return m_Adapter.GetRaw(x, y); }
    EckInlineNdCe const TPixel& GetPixelRaw(UINT x, UINT y) const noexcept { return m_Adapter.GetRaw(x, y); }
private:
    EckInlineNdCe TChannel CalculateGray(const TPixel& Pix) const noexcept
    {
        return TChannel(
            TFloat(0.21264934272065283) * Pix.R() +
            TFloat(0.7151691357059038) * Pix.G() +
            TFloat(0.07218152157344333) * Pix.B());
    }
public:
    template<class T>
    constexpr void Grayscale(CPixelEditor<T>& Dst) const noexcept
    {
        const auto cx = std::min(GetWidth(), Dst.GetWidth());
        const auto cy = std::min(GetHeight(), Dst.GetHeight());
        for (UINT y = 0; y < cy; ++y)
            for (UINT x = 0; x < cx; ++x)
            {
                auto Pix = GetPixel(x, y);
                const auto Gray = CalculateGray(Pix);
                Pix.R() = Pix.G() = Pix.B() = Gray;
                Dst.SetPixel(x, y, Pix);
            }
    }

    constexpr void Grayscale() noexcept { Grayscale(*this); }

    template<class T>
    constexpr void Binary(
        CPixelEditor<T>& Dst,
        TChannel Threshold,
        TChannel Max = ChannelMaximum,
        TChannel Min = ChannelMinimum,
        PixelEditor::Channel Ch = PixelEditor::Channel::Gray) const noexcept
    {
        const auto cx = std::min(GetWidth(), Dst.GetWidth());
        const auto cy = std::min(GetHeight(), Dst.GetHeight());
        switch (Ch)
        {
        case PixelEditor::Channel::Red:
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.R() = (Pix.R() >= Threshold ? Max : Min);
                    Dst.SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Green:
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.G() = (Pix.G() >= Threshold ? Max : Min);
                    Dst.SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Blue:
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.B() = (Pix.B() >= Threshold ? Max : Min);
                    Dst.SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Alpha:
            if constexpr (TAdapter::HasAlpha)
            {
                for (UINT y = 0; y < cy; ++y)
                    for (UINT x = 0; x < cx; ++x)
                    {
                        auto Pix = GetPixel(x, y);
                        Pix.A() = (Pix.A() >= Threshold ? Max : Min);
                        Dst.SetPixel(x, y, Pix);
                    }
            }
            break;
        case PixelEditor::Channel::Gray:
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                {
                    auto Pix = GetPixel(x, y);
                    const auto Gray = CalculateGray(Pix);
                    const auto BinaryValue = (Gray >= Threshold ? Max : Min);
                    Pix.R() = Pix.G() = Pix.B() = BinaryValue;
                    Dst.SetPixel(x, y, Pix);
                }
            break;
        }
    }

    constexpr void Binary(
        TChannel Threshold,
        TChannel Max = ChannelMaximum,
        TChannel Min = ChannelMinimum,
        PixelEditor::Channel Ch = PixelEditor::Channel::Gray) noexcept
    {
        Binary(*this, Threshold, Max, Min, Ch);
    }

    constexpr void SetChannel(PixelEditor::Channel Ch, TChannel Value) const noexcept
    {
        switch (Ch)
        {
        case PixelEditor::Channel::Red:
            for (UINT y = 0; y < GetHeight(); ++y)
                for (UINT x = 0; x < GetWidth(); ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.R() = Value;
                    SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Green:
            for (UINT y = 0; y < GetHeight(); ++y)
                for (UINT x = 0; x < GetWidth(); ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.G() = Value;
                    SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Blue:
            for (UINT y = 0; y < GetHeight(); ++y)
                for (UINT x = 0; x < GetWidth(); ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.B() = Value;
                    SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Alpha:
            if constexpr (TAdapter::HasAlpha)
            {
                for (UINT y = 0; y < GetHeight(); ++y)
                    for (UINT x = 0; x < GetWidth(); ++x)
                    {
                        auto Pix = GetPixel(x, y);
                        Pix.A() = Value;
                        SetPixel(x, y, Pix);
                    }
            }
            break;
        case PixelEditor::Channel::Gray:
            for (UINT y = 0; y < GetHeight(); ++y)
                for (UINT x = 0; x < GetWidth(); ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.R() = Pix.G() = Pix.B() = Value;
                    SetPixel(x, y, Pix);
                }
            break;
        }
    }

    template<class T>
    constexpr void BroadcastChannel(CPixelEditor<T>& Dst,
        PixelEditor::Channel Ch) const noexcept
    {
        const auto cx = std::min(GetWidth(), Dst.GetWidth());
        const auto cy = std::min(GetHeight(), Dst.GetHeight());
        switch (Ch)
        {
        case PixelEditor::Channel::Red:
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.G() = Pix.B() = Pix.R();
                    if constexpr (TAdapter::HasAlpha)
                        Pix.A() = ChannelMaximum;
                    Dst.SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Green:
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.R() = Pix.B() = Pix.G();
                    if constexpr (TAdapter::HasAlpha)
                        Pix.A() = ChannelMaximum;
                    Dst.SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Blue:
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.R() = Pix.G() = Pix.B();
                    if constexpr (TAdapter::HasAlpha)
                        Pix.A() = ChannelMaximum;
                    Dst.SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Alpha:
            if constexpr (TAdapter::HasAlpha)
            {
                for (UINT y = 0; y < cy; ++y)
                    for (UINT x = 0; x < cx; ++x)
                    {
                        auto Pix = GetPixel(x, y);
                        Pix.R() = Pix.G() = Pix.B() = Pix.A();
                        Pix.A() = ChannelMaximum;
                        Dst.SetPixel(x, y, Pix);
                    }
            }
            break;
        case PixelEditor::Channel::Gray:
            EckDbgBreak();
            break;
        }
    }

    constexpr void BroadcastChannel(PixelEditor::Channel Ch) noexcept
    {
        BroadcastChannel(*this, Ch);
    }

    template<class T>
    constexpr void ReplaceChannel(
        const CPixelEditor<T>& Ref, PixelEditor::Channel Ch) noexcept
    {
        const auto cx = std::min(GetWidth(), Ref.GetWidth());
        const auto cy = std::min(GetHeight(), Ref.GetHeight());
        switch (Ch)
        {
        case PixelEditor::Channel::Red:
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.R() = Ref.GetPixel(x, y).R();
                    SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Green:
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.G() = Ref.GetPixel(x, y).G();
                    SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Blue:
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                {
                    auto Pix = GetPixel(x, y);
                    Pix.B() = Ref.GetPixel(x, y).B();
                    SetPixel(x, y, Pix);
                }
            break;
        case PixelEditor::Channel::Alpha:
            if constexpr (TAdapter::HasAlpha)
            {
                for (UINT y = 0; y < cy; ++y)
                    for (UINT x = 0; x < cx; ++x)
                    {
                        auto Pix = GetPixel(x, y);
                        Pix.A() = Ref.GetPixel(x, y).A();
                        SetPixel(x, y, Pix);
                    }
            }
            break;
        case PixelEditor::Channel::Gray:
            EckDbgBreak();
            break;
        }
    }

    template<class T>
    constexpr void Invert(CPixelEditor<T>& Dst) const noexcept
    {
        const auto cx = std::min(GetWidth(), Dst.GetWidth());
        const auto cy = std::min(GetHeight(), Dst.GetHeight());
        for (UINT y = 0; y < cy; ++y)
            for (UINT x = 0; x < cx; ++x)
            {
                auto Pix = GetPixel(x, y);
                Pix.R() = ChannelMaximum - Pix.R();
                Pix.G() = ChannelMaximum - Pix.G();
                Pix.B() = ChannelMaximum - Pix.B();
                Dst.SetPixel(x, y, Pix);
            }
    }

    constexpr void Invert() noexcept { Invert(*this); }

    // Alpha参数为替换值，为0则自动计算
    // 此函数仅支持带有Alpha通道的格式
    template<class T>
    constexpr void ExtractForegroundFromBlack(
        CPixelEditor<T>& Dst, TChannel Alpha = 0) const noexcept
    {
        const auto cx = std::min(GetWidth(), Dst.GetWidth());
        const auto cy = std::min(GetHeight(), Dst.GetHeight());
        for (UINT y = 0; y < cy; ++y)
            for (UINT x = 0; x < cx; ++x)
            {
                auto Pix = GetPixel(x, y);
                const auto Max = std::max(std::max(Pix.R(), Pix.G()), Pix.B());
                Pix.R() = std::min(TChannel(Pix.R() + ChannelMaximum - Max), ChannelMaximum);
                Pix.G() = std::min(TChannel(Pix.G() + ChannelMaximum - Max), ChannelMaximum);
                Pix.B() = std::min(TChannel(Pix.B() + ChannelMaximum - Max), ChannelMaximum);
                if (Alpha)
                    Pix.A() = Alpha;
                else
                    Pix.A() = std::min(Pix.A(), Max);
                Dst.SetPixel(x, y, Pix);
            }
    }

    constexpr void ExtractForegroundFromBlack(TChannel Alpha = 0) noexcept
    {
        ExtractForegroundFromBlack(*this, Alpha);
    }

    constexpr void FloodFill(UINT x, UINT y, const TPixel& PixNew) noexcept
    {
        const auto PixOld = GetPixel(x, y);
        if (PixOld == PixNew)
            return;
        const auto PixOldRaw = GetPixelRaw(x, y);

        struct Point { UINT x, y; };
        std::vector<POINT> s{};
        s.reserve(16);
        s.emplace_back(x, y);

        while (!s.empty())
        {
            const auto pt = s.back();
            s.pop_back();
            const UINT y = pt.y;

            UINT l = pt.x;
            UINT r = pt.x;

            // 向左扫描
            EckLoop()
            {
                SetPixel(l, y, PixNew);
                if (!l)
                    break;
                if (GetPixelRaw(l - 1, y) != PixOldRaw)
                    break;
                --l;
            }
            // 向右扫描
            EckLoop()
            {
                SetPixel(r, y, PixNew);
                if (r == GetWidth() - 1)
                    break;
                if (GetPixelRaw(r + 1, y) != PixOldRaw)
                    break;
                ++r;
            }
            // 检查上下行
            auto FnCheckLine = [&](UINT yNew)
                {
                    BOOL bSpan{};
                    for (UINT i = l; i <= r; ++i)
                    {
                        if (GetPixelRaw(i, yNew) == PixOldRaw)
                        {
                            if (!bSpan)
                            {
                                s.emplace_back(i, yNew);
                                bSpan = TRUE;
                            }
                        }
                        else// 断开
                            bSpan = FALSE;
                    }
                };

            if (y != 0)
                FnCheckLine(y - 1);
            if (y != GetHeight() - 1)
                FnCheckLine(y + 1);
        }
    }

    template<class T>
    constexpr void EnhanceColor(CPixelEditor<T>& Dst, float fFactor) const noexcept
    {
        const auto cx = std::min(GetWidth(), Dst.GetWidth());
        const auto cy = std::min(GetHeight(), Dst.GetHeight());

        const auto Inc0 = TFloat(fFactor * 0.5f) * ChannelMaximum;
        const auto Dec1 = TFloat(fFactor * 0.1687f) * ChannelMaximum;
        const auto Dec2 = TFloat(fFactor * 0.3313f) * ChannelMaximum;

        for (UINT j = 0; j < cy; ++j)
            for (UINT i = 0; i < cx; ++i)
            {
                auto Pix = GetPixel(i, j);
                BYTE* p[3]{ &Pix.R(), &Pix.G(), &Pix.B() };
                // 排序
                if (*p[0] < *p[1]) std::swap(p[0], p[1]);
                if (*p[0] < *p[2]) std::swap(p[0], p[2]);
                if (*p[1] < *p[2]) std::swap(p[1], p[2]);
                *p[0] = TChannel(std::min(*p[0] + Inc0, TFloat(ChannelMaximum)));
                *p[1] = TChannel(std::max(*p[1] - Dec1, TFloat(ChannelMinimum)));
                *p[2] = TChannel(std::max(*p[2] - Dec2, TFloat(ChannelMinimum)));
                Dst.SetPixel(i, j, Pix);
            }
    }

    constexpr void EnhanceColor(float fFactor) noexcept
    {
        EnhanceColor(*this, fFactor);
    }

    template<class T>
    constexpr void Trend(CPixelEditor<T>& Dst,
        const TPixel& TrendTo, float fFactor) const noexcept
    {
        const auto cx = std::min(GetWidth(), Dst.GetWidth());
        const auto cy = std::min(GetHeight(), Dst.GetHeight());
        for (UINT j = 0; j < cy; ++j)
            for (UINT i = 0; i < cx; ++i)
            {
                auto Pix = GetPixel(i, j);
                Pix.R() = TChannel(Pix.R() + fFactor * ((TFloat)TrendTo.R() - Pix.R()));
                Pix.G() = TChannel(Pix.G() + fFactor * ((TFloat)TrendTo.G() - Pix.G()));
                Pix.B() = TChannel(Pix.B() + fFactor * ((TFloat)TrendTo.B() - Pix.B()));
                if constexpr (TAdapter::HasAlpha)
                    Pix.A() = TChannel(Pix.A() + fFactor * ((TFloat)TrendTo.A() - Pix.A()));
                Dst.SetPixel(i, j, Pix);
            }
    }

    constexpr void Trend(const TPixel& TrendTo, float fFactor) noexcept
    {
        Trend(*this, TrendTo, fFactor);
    }
private:
    template<class T>
    constexpr void BoxBlurHorizontal(CPixelEditor<T>& Dst, UINT r) const noexcept
    {
        const auto cx = GetWidth();
        const auto cy = GetHeight();
        const auto cxyBox = r + r + 1;
        for (UINT i = 0; i < cy; ++i)
        {
            UINT ti = 0;
            UINT li = 0;
            UINT ri = r;
            TPixelSum fv = GetPixel(0, i), lv = GetPixel(cx - 1, i);
            TPixelSum v = fv * (r + 1);
            for (UINT j = 0; j < r; ++j)
                v += GetPixel(j, i);
            for (UINT j = 0; j <= r; ++j)
            {
                v += GetPixel(ri++, i);
                v -= fv;
                Dst.SetPixel(ti++, i, (v / cxyBox).ToPixel());
            }
            for (UINT j = r + 1; j < cx - r; ++j)
            {
                v += GetPixel(ri++, i);
                v -= GetPixel(li++, i);
                Dst.SetPixel(ti++, i, (v / cxyBox).ToPixel());
            }
            for (UINT j = cx - r; j < cx; ++j)
            {
                v += lv;
                v -= GetPixel(li++, i);
                Dst.SetPixel(ti++, i, (v / cxyBox).ToPixel());
            }
        }
    }
    template<class T>
    constexpr void BoxBlurVertical(CPixelEditor<T>& Dst, UINT r) const noexcept
    {
        const auto cx = GetWidth();
        const auto cy = GetHeight();
        const auto cxyBox = r + r + 1;
        for (UINT j = 0; j < cx; ++j)
        {
            UINT ti = 0;
            UINT li = 0;
            UINT ri = r;
            TPixelSum fv = GetPixel(j, 0), lv = GetPixel(j, cy - 1);
            TPixelSum v = fv * (r + 1);
            for (UINT i = 0; i < r; ++i)
                v += GetPixel(j, i);
            for (UINT i = 0; i <= r; ++i)
            {
                v += GetPixel(j, ri++);
                v -= fv;
                Dst.SetPixel(j, ti++, (v / cxyBox).ToPixel());
            }
            for (UINT i = r + 1; i < cy - r; ++i)
            {
                v += GetPixel(j, ri++);
                v -= GetPixel(j, li++);
                Dst.SetPixel(j, ti++, (v / cxyBox).ToPixel());
            }
            for (UINT i = cy - r; i < cy; ++i)
            {
                v += lv;
                v -= GetPixel(j, li++);
                Dst.SetPixel(j, ti++, (v / cxyBox).ToPixel());
            }
        }
    }
public:
    template<class T>
    constexpr void GaussianBlur(CPixelEditor<T>& Work, float fRadius) noexcept
    {
        if (Work.GetWidth() != GetWidth() ||
            Work.GetHeight() != GetHeight())
        {
            EckDbgBreak();
            return;
        }
        UINT r[3];
        CalculateBoxBlurFromGaussian(fRadius / 3.f, r, 3);
        const auto rMax = std::min(GetWidth(), GetHeight()) / 2 - 1;
        for (auto& e : r)
        {
            e = (e - 1) / 2;
            if (e > rMax)
                e = rMax;
        }

        BoxBlurHorizontal(Work, r[0]);
        Work.BoxBlurVertical(*this, r[0]);
        BoxBlurHorizontal(Work, r[1]);
        Work.BoxBlurVertical(*this, r[1]);
        BoxBlurHorizontal(Work, r[2]);
        Work.BoxBlurVertical(*this, r[2]);
    }

    constexpr void GetOpaqueBounds(_Out_ RECT& rc, TChannel AlphaMin) const noexcept
    {
        const auto cx = GetWidth();
        const auto cy = GetHeight();
        rc = { 0, 0, cx, cy };
        if constexpr (!TAdapter::HasAlpha)
            return;
        for (UINT y = 0; y < cy; ++y)
            for (UINT x = 0; x < cx; ++x)
            {
                if (GetPixelRaw(x, y).A() >= AlphaMin)
                {
                    rc.top = y;
                    goto ExitT;
                }
            }
    ExitT:
        for (UINT y = cy - 1; y > rc.top; --y)
            for (UINT x = 0; x < cx; ++x)
            {
                if (GetPixelRaw(x, y).A() >= AlphaMin)
                {
                    rc.bottom = y + 1;
                    goto ExitB;
                }
            }
    ExitB:
        for (UINT x = 0; x < cx; ++x)
            for (UINT y = rc.top; y < rc.bottom; ++y)
            {
                if (GetPixelRaw(x, y).A() >= AlphaMin)
                {
                    rc.left = x;
                    goto ExitL;
                }
            }
    ExitL:
        for (UINT x = cx - 1; x > rc.left; --x)
            for (UINT y = rc.top; y < rc.bottom; ++y)
            {
                if (GetPixelRaw(x, y).A() >= AlphaMin)
                {
                    rc.right = x + 1;
                    goto ExitR;
                }
            }
    ExitR:;
    }

    constexpr void FlipX() noexcept
    {
        const auto cx = GetWidth();
        const auto cy = GetHeight();
        for (UINT y = 0; y < cy; ++y)
            for (UINT x = 0; x < cx / 2; ++x)
                std::swap(GetPixelRaw(x, y), GetPixelRaw(cx - 1 - x, y));
    }
    constexpr void FlipY() noexcept
    {
        const auto cx = GetWidth();
        const auto cy = GetHeight();
        for (UINT y = 0; y < cy / 2; ++y)
            for (UINT x = 0; x < cx; ++x)
                std::swap(GetPixelRaw(x, y), GetPixelRaw(x, cy - 1 - y));
    }

    template<class T>
    void BitBlit(CPixelEditor<T>& Dst) const noexcept
    {
        const auto cx = std::min(GetWidth(), Dst.GetWidth());
        const auto cy = std::min(GetHeight(), Dst.GetHeight());
        if constexpr (std::is_same_v<TAdapter, T>)
        {
            if (&Dst == this)
                return;
            for (UINT y = 0; y < cy; ++y)
                memcpy(
                    (BYTE*)Dst.m_Adapter.Bits + y * Dst.m_Adapter.Stride,
                    (BYTE*)m_Adapter.Bits + y * m_Adapter.Stride,
                    cx * sizeof(TPixel));
        }
        else
        {
            for (UINT y = 0; y < cy; ++y)
                for (UINT x = 0; x < cx; ++x)
                    Dst.SetPixel(x, y, GetPixel(x, y));
        }
    }
    template<class T>
    void BitBlit(CPixelEditor<T>& Dst,
        UINT xDst, UINT yDst, UINT xSrc, UINT ySrc, UINT cx, UINT cy) const noexcept
    {
        if (xDst >= Dst.GetWidth() || yDst >= Dst.GetHeight() ||
            xSrc >= GetWidth() || ySrc >= GetHeight())
            return;
        const auto xDstEnd = std::min(xDst + cx, Dst.GetWidth());
        const auto yDstEnd = std::min(yDst + cy, Dst.GetHeight());
        const auto xSrcEnd = std::min(xSrc + cx, GetWidth());
        const auto ySrcEnd = std::min(ySrc + cy, GetHeight());
        const auto cCopyX = std::min(xDstEnd - xDst, xSrcEnd - xSrc);
        const auto cCopyY = std::min(yDstEnd - yDst, ySrcEnd - ySrc);
        if (cCopyX == 0 || cCopyY == 0)
            return;
        if constexpr (std::is_same_v<TAdapter, T>)
        {
            if (&Dst == this)
            {
                if (yDst < ySrc)
                    for (UINT y = 0; y < cCopyY; ++y)
                        memmove(
                            (BYTE*)Dst.m_Adapter.Bits + (yDst + y) * Dst.m_Adapter.Stride + xDst * sizeof(TPixel),
                            (BYTE*)m_Adapter.Bits + (ySrc + y) * m_Adapter.Stride + xSrc * sizeof(TPixel),
                            cCopyX * sizeof(TPixel));
                else
                    for (UINT y = 0; y < cCopyY; ++y)
                        memmove(
                            (BYTE*)Dst.m_Adapter.Bits + (yDst + cCopyY - 1 - y) * Dst.m_Adapter.Stride + xDst * sizeof(TPixel),
                            (BYTE*)m_Adapter.Bits + (ySrc + cCopyY - 1 - y) * m_Adapter.Stride + xSrc * sizeof(TPixel),
                            cCopyX * sizeof(TPixel));
            }
            else
                for (UINT y = 0; y < cCopyY; ++y)
                    memcpy(
                        (BYTE*)Dst.m_Adapter.Bits + (yDst + y) * Dst.m_Adapter.Stride + xDst * sizeof(TPixel),
                        (BYTE*)m_Adapter.Bits + (ySrc + y) * m_Adapter.Stride + xSrc * sizeof(TPixel),
                        cCopyX * sizeof(TPixel));
        }
        else
        {
            for (UINT y = 0; y < cCopyY; ++y)
                for (UINT x = 0; x < cCopyX; ++x)
                    Dst.SetPixel(xDst + x, yDst + y, GetPixel(xSrc + x, ySrc + y));
        }
    }
};
ECK_NAMESPACE_END