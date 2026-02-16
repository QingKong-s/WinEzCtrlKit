#pragma once
#include "KwDef.h"
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
KW2D_NAMESPACE_BEGIN
class CLinearGradient
{
public:
    enum class ExtendMode : BYTE
    {
        Clamp,
        Wrap,
        Mirror
    };

    enum class GammaMode : BYTE
    {
        Gamma1_0,
        Gamma2_2,
        User,
    };

    struct STOP
    {
        float k;// 位置，注意可能超出[0, 1]范围
        UINT argb;
    };
private:
    CTrivialBuffer<STOP> m_vStop{};
    Vec2 m_ptStart{}, m_ptEnd{};
    float m_kStart{}, m_kEnd{};

    Vec2 m_vecDirection{};
    float m_fDirVecLenSq{};

    float m_fGammaUser{ 2.2f };
    GammaMode m_eGammaMode{ GammaMode::Gamma2_2 };
    ExtendMode m_eExtendMode{ ExtendMode::Clamp };

    float MapK(float k) const noexcept
    {
        const auto kMin = m_vStop.Front().k;
        const auto kMax = m_vStop.Back().k;
        const auto fRange = kMax - kMin;
        if (fRange < 1e-6f)
            return kMin;

        switch (m_eExtendMode)
        {
        case ExtendMode::Clamp:
            if (k < kMin)
                return kMin;
            if (k > kMax)
                return kMax;
            return k;
        case ExtendMode::Wrap:
        {
            auto t = fmod(k - kMin, fRange);
            if (t < 0.0f)
                t += fRange;
            return kMin + t;
        }
        case ExtendMode::Mirror:
        {
            const auto Range2 = fRange * 2.0f;
            auto t = fmod(k - kMin, Range2);
            if (t < 0.0f)
                t += Range2;
            if (t > fRange)
                t = Range2 - t;
            return kMin + t;
        }
        }
        return k;
    }
public:
    EckInlineNd static float Gamma22ToLinear(float x) noexcept
    {
        return x * x * (0.2624f * x + 0.7376f);
    }
    EckInlineNd static float Gamma22ToGamma(float x) noexcept
    {
        return sqrtf(x) * (1.085f - 0.085f * x);
    }

    BYTE Lerp(BYTE v0, BYTE v1, float t) const noexcept
    {
        switch (m_eGammaMode)
        {
        case GammaMode::Gamma1_0:
            return BYTE(v0 + (v1 - v0) * t);
        case GammaMode::Gamma2_2:
        {
            const auto v0f = Gamma22ToLinear(v0 / 255.f);
            const auto r = (v0f + (Gamma22ToLinear(v1 / 255.f) - v0f) * t);
            return BYTE(Gamma22ToGamma(r) * 255.f);
        }
        case GammaMode::User:
        {
            const auto v0f = powf(v0 / 255.f, m_fGammaUser);
            const auto r = v0f + (powf(v1 / 255.f, m_fGammaUser) - v0f) * t;
            return BYTE(powf(r, 1.f / m_fGammaUser) * 255.f);
        }
        default:
            ECK_UNREACHABLE;
        }
    }

    UINT CalculateColor(Vec2 pt) const noexcept
    {
        if (m_vStop.IsEmpty())
            return 0;
        if (m_vStop.Size() == 1)
            return m_vStop[0].argb;

        float kRaw;
        if (m_fDirVecLenSq < 1e-6f)
            kRaw = m_kStart;
        else
        {
            const auto v = pt - m_ptStart;
            const auto fDot = v.x * m_vecDirection.x + v.y * m_vecDirection.y;
            kRaw = m_kStart + (fDot / m_fDirVecLenSq) * (m_kEnd - m_kStart);
        }

        const auto kActual = MapK(kRaw);
        if (kActual <= m_vStop.Front().k)
            return m_vStop.Front().argb;
        if (kActual >= m_vStop.Back().k)
            return m_vStop.Back().argb;

        const auto it = std::upper_bound(m_vStop.begin(), m_vStop.end(), kActual,
            [](float v, const STOP& s) { return v < s.k; });
        const auto& s1 = *it;
        const auto& s0 = *(it - 1);
        const auto fScale = (kActual - s0.k) / (s1.k - s0.k);

        const auto pby0 = (PCBYTE)&s0.argb;
        const auto pby1 = (PCBYTE)&s1.argb;
        UINT u;
        const auto pbyDst = (BYTE*)&u;
        pbyDst[0] = Lerp(pby0[0], pby1[0], fScale);
        pbyDst[1] = Lerp(pby0[1], pby1[1], fScale);
        pbyDst[2] = Lerp(pby0[2], pby1[2], fScale);
        pbyDst[3] = Lerp(pby0[3], pby1[3], fScale);
        return u;
    }

    void SetStartEndPoint(
        Vec2 pt0, float k0,
        Vec2 pt1, float k1) noexcept
    {
        m_ptStart = pt0;
        m_ptEnd = pt1;
        m_kStart = k0;
        m_kEnd = k1;

        m_vecDirection = pt1 - pt0;
        m_fDirVecLenSq = m_vecDirection.x * m_vecDirection.x +
            m_vecDirection.y * m_vecDirection.y;
    }

    EckInlineCe void SetExtendMode(ExtendMode e) noexcept { m_eExtendMode = e; }
    EckInlineNdCe ExtendMode GetExtendMode() const noexcept { return m_eExtendMode; }

    EckInlineCe void SetGammaMode(GammaMode e) noexcept { m_eGammaMode = e; }
    EckInlineNdCe GammaMode GetGammaMode() const noexcept { return m_eGammaMode; }

    EckInlineCe void SetUserGamma(float f) noexcept { m_fGammaUser = f; }
    EckInlineNdCe float GetUserGamma() const noexcept { return m_fGammaUser; }

    // WARNING 必须按k值升序排序
    EckInlineNdCe auto& GetStopBuffer() noexcept { return m_vStop; }
    EckInlineNdCe auto& GetStopBuffer() const noexcept { return m_vStop; }
};
KW2D_NAMESPACE_END
ECK_NAMESPACE_END