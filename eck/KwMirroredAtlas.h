#pragma once
#include "KwMirroredTexture.h"
#include "CRectPackSkyline.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
KW2D_NAMESPACE_BEGIN
class CMirroredAtlas
{
public:
    using TId = UINT;// 理论最大值使用29位即可表示
    using TCoord = CMirroredTexture::TCoord;

    constexpr static TId IdInvalid = 0x3FFFFFFF;
private:
    struct ITEM
    {
        TCoord x, y, cx, cy;// 理论最大值使用15位即可表示
        // bUnusedId == TRUE时，u为下一个未分配ID
        // bFree == TRUE时，u为下一个空闲矩形ID
        // 否则，u未定义
        UINT u : 30;
        UINT bFree : 1;
        UINT bUnusedId : 1;
    };

    CMirroredTexture m_Tex{};
    CRectPackSkyline m_Packer{};
    CTrivialBuffer<ITEM> m_Allocated{};
    TId m_idFirstFree{ IdInvalid };
    TId m_idFirstUnused{ IdInvalid };
    TCoord m_yMax{};
    UINT m_uVersion{};
    RectU m_rcDirty{};

    void GrowCpuBuffer(CMirroredTexture& TexNew, TCoord cxMin = 0, TCoord cyMin = 0) noexcept
    {
        TCoord cx, cy;
        m_Tex.CpuGetSize(cx, cy);
        if (cx >= cy)
            cy *= 2;
        else
            cx *= 2;
        cx = std::max(cx, cxMin);
        cy = std::max(cy, cyMin);
        cx = std::min(cx, (TCoord)D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION);
        cy = std::min(cy, (TCoord)D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION);
        TexNew.CpuCreate(4, TRUE, cx, cy);
    }

    void Compact(CMirroredTexture& TexNew) noexcept
    {
        struct TEMP : ITEM
        {
            UINT idxOrg;
        };
        const auto cRect = (UINT)m_Allocated.Size();
        CTrivialBuffer<TEMP> vTemp(cRect);

        TId idLastEmpty{ IdInvalid };
        EckCounter(cRect, i)
        {
            const auto& e = m_Allocated[i];
            auto& f = vTemp[i];

            if (!e.bFree && !e.bUnusedId)
                f = { e };
            else
            {
                f.bFree = FALSE;
                f.bUnusedId = TRUE;
                f.u = idLastEmpty;
#ifdef _DEBUG
                f.x = f.y = f.cx = f.cy = std::numeric_limits<TCoord>::max();
#endif
                idLastEmpty = (TId)i;
            }
            f.idxOrg = i;
        }
        m_idFirstFree = IdInvalid;
        m_idFirstUnused = idLastEmpty;

        std::sort(vTemp.begin(), vTemp.end(),
            [&](const ITEM& e, const ITEM& f) -> bool
            {
                return e.cx * e.cy > f.cx * f.cy;
            });

        TCoord cxNew, cyNew;
        TCoord yMax;
    Retry:
        TexNew.CpuGetSize(cxNew, cyNew);
        m_Packer.Initialize(cxNew, cyNew);
        yMax = 0;

        EckCounter(cRect, i)
        {
            auto& e = vTemp[i];
            if (e.bUnusedId)
                continue;

            CRectPackSkyline::RECT rcNew{ .cx = e.cx, .cy = e.cy };
            if (!m_Packer.AllocateMinimumWaste(rcNew)) [[unlikely]]
            {
                // 如果加入失败，应扩容后再次尝试
                GrowCpuBuffer(TexNew);
                goto Retry;
            }
            e.x = rcNew.x;
            e.y = rcNew.y;
            if (rcNew.y + e.cy > yMax)
                yMax = rcNew.y + e.cy;
        }
        m_yMax = yMax;

        for (const auto& e : vTemp)
        {
            auto& f = m_Allocated[e.idxOrg];
            EckAssert(!e.bFree);
            if (!e.bUnusedId)
            {
                m_Tex.CpuBitBlit(TexNew, e.x, e.y,
                    { f.x, f.y, UINT(f.x + f.cx), UINT(f.y + f.cy) });
                EckAssert(e.cx == f.cx && e.cy == f.cy);
            }
            f = e;
        }
        TexNew.GpuCreate(TRUE);
        m_Tex = std::move(TexNew);
    }

    TId AllocateRectangle(TCoord cx, TCoord cy, _Out_ TCoord& x, _Out_ TCoord& y) noexcept
    {
        CRectPackSkyline::RECT rcNew{ .cx = cx, .cy = cy };
        if (m_Packer.AllocateMinimumWaste(rcNew))
        {
            x = rcNew.x;
            y = rcNew.y;

            TId id;
            if (m_idFirstUnused != IdInvalid)
            {
                id = m_idFirstUnused;
                auto& e = m_Allocated[id];
                EckAssert(!e.bFree && e.bUnusedId);
                m_idFirstUnused = e.u;
                e.bUnusedId = FALSE;
                e = { x, y, cx, cy };
            }
            else
            {
                m_Allocated.PushBack({ x, y, cx, cy });
                id = TId(m_Allocated.Size() - 1);
            }
            if (rcNew.y + cy > m_yMax)
                m_yMax = rcNew.y + cy;
            return id;
        }
        x = 0, y = 0;
        return IdInvalid;
    }
public:
    void Create(
        TCoord cx = 512,
        TCoord cy = 256,
        DXGI_FORMAT eFormat = DXGI_FORMAT_B8G8R8A8_UNORM,
        UINT uBind = D3D11_BIND_SHADER_RESOURCE) noexcept
    {
        m_Tex.CpuCreate(4, TRUE, cx, cy);
        m_Tex.GpuCreate(TRUE, eFormat, uBind);
        m_Packer.Initialize(cx, cy);
        m_Allocated.Clear();
    }

    TId Allocate(
        TCoord cx, TCoord cy,
        _Out_ TCoord& x, _Out_ TCoord& y,
        BOOL bUnionDirty = FALSE,
        BOOL bDiscardable = TRUE) noexcept
    {
        const auto id = AllocateRectangle(cx, cy, x, y);
        if (id != IdInvalid)
        {
            if (bUnionDirty)
                UnionDirtyRect({ x, y, UINT(x + cx), UINT(y + cy) });
            return id;
        }
        else// 分配失败，尝试搜索合适的空闲区域
        {
            auto id = m_idFirstFree;
            auto idLast = IdInvalid;
            while (id != IdInvalid)
            {
                auto& e = m_Allocated[id];
                EckAssert(e.bFree && !e.bUnusedId);
                if (e.cx >= cx && e.cy >= cy)
                {
                    if (idLast == IdInvalid)
                        m_idFirstFree = e.u;
                    else
                        m_Allocated[idLast].u = e.u;
                    x = e.x;
                    y = e.y;
                    e.cx = cx;
                    e.cy = cy;
                    e.bFree = FALSE;
                    UnionDirtyRect({ x, y, UINT(x + cx), UINT(y + cy) });
                    return id;
                }
                idLast = id;
                id = e.u;
            }
        }
        // 搜索失败，扩容
        m_rcDirty = {};
        ++m_uVersion;
        EckCounterNV(3)
        {
            TCoord cxCurr, cyCurr;
            m_Tex.CpuGetSize(cxCurr, cyCurr);

            CMirroredTexture TexNew{};
            TCoord cxMin{}, cyMin{};
            if (cx > cxCurr)
                cxMin = cx;
            if (m_yMax + cy > cyCurr)
                cyMin = m_yMax + cy;

            GrowCpuBuffer(TexNew, cxMin, cyMin);
            Compact(TexNew);
            const auto id = AllocateRectangle(cx, cy, x, y);
            if (id != IdInvalid)
            {
                UnionDirtyRect({ x, y, UINT(x + cx), UINT(y + cy) });
                return id;
            }
        }
        return IdInvalid;
    }

    CRectPackSkyline::RECT ValidateId(TId id) const noexcept
    {
        EckAssert(id < m_Allocated.Size());
        const auto& e = m_Allocated[id];
        EckAssert(!e.bFree && !e.bUnusedId);
        return { e.x, e.y, e.cx, e.cy };
    }

    void Discard(TId id) noexcept
    {
#ifdef _DEBUG
        ValidateId(id);
#endif
        auto& e = m_Allocated[id];
        e.bFree = TRUE;
        e.u = m_idFirstFree;
        m_idFirstFree = id;
    }

    EckInlineNdCe auto& GetMirroredTexture() noexcept { return m_Tex; }
    EckInlineNdCe auto& GetMirroredTexture() const noexcept { return m_Tex; }

    // 取版本号，调用方使用此值判断图集是否发生过扩容
    EckInlineNdCe UINT GetVersion() const noexcept { return m_uVersion; }

    EckInlineNdCe auto& GetDirtyRect() const noexcept { return m_rcDirty; }
    EckInlineCe void UnionDirtyRect(const RectU& rcDirty) noexcept
    {
        UnionRect(m_rcDirty, m_rcDirty, rcDirty);
    }
    void UploadDirtyRect(ID3D11DeviceContext* pContext) noexcept
    {
        if (IsRectEmpty(m_rcDirty))
            return;
        m_Tex.GpuUpload(pContext, m_rcDirty);
        m_rcDirty = {};
    }
};
KW2D_NAMESPACE_END
ECK_NAMESPACE_END