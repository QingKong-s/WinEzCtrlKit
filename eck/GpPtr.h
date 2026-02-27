#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template <class T>
class GpPtr
{
private:
    T* p{};

    EckInline void ClearInternal() noexcept
    {
        if constexpr (std::is_same_v<T, GpEffect>)
            GdipDeleteEffect(p);
        else if constexpr (std::is_same_v<T, GpCachedBitmap>)
            GdipDeleteCachedBitmap(p);

        else if constexpr (std::is_same_v<T, GpGraphics>)
            GdipDeleteGraphics(p);
        else if constexpr (std::is_base_of_v<GpBrush, T>)
            GdipDeleteBrush(p);
        else if constexpr (std::is_same_v<T, GpPen>)
            GdipDeletePen(p);
        else if constexpr (std::is_base_of_v<GpCustomLineCap, T>)
            GdipDeleteCustomLineCap(p);
        else if constexpr (std::is_base_of_v<GpImage, T>)
            GdipDisposeImage(p);
        else if constexpr (std::is_same_v<T, GpImageAttributes>)
            GdipDisposeImageAttributes(p);
        else if constexpr (std::is_same_v<T, GpPath>)
            GdipDeletePath(p);
        else if constexpr (std::is_same_v<T, GpRegion>)
            GdipDeleteRegion(p);
        else if constexpr (std::is_same_v<T, GpPathIterator>)
            GdipDeletePathIter(p);
        else if constexpr (std::is_same_v<T, GpFontFamily>)
            GdipDeleteFontFamily(p);
        else if constexpr (std::is_same_v<T, GpFont>)
            GdipDeleteFont(p);
        else if constexpr (std::is_same_v<T, GpStringFormat>)
            GdipDeleteStringFormat(p);
        else if constexpr (std::is_base_of_v<GpFontCollection, T>)
            GdipDeletePrivateFontCollection(p);
        else if constexpr (std::is_same_v<T, GpMatrix>)
            GdipDeleteMatrix(p);
        else
            static_assert(!sizeof(T), "Unsupported GDI+ type");
    }
public:
    constexpr GpPtr() = default;
    explicit constexpr GpPtr(T* p_) noexcept : p{ p_ } {}

    GpPtr(const GpPtr&) = delete;
    GpPtr& operator=(const GpPtr&) = delete;

    GpPtr(GpPtr&& x) noexcept { std::swap(p, x.p); }
    GpPtr& operator=(GpPtr&& x) noexcept
    {
        std::swap(p, x.p);
        return *this;
    }

    ~GpPtr()
    {
        if (p)
            ClearInternal();
    }

    EckInlineNdCe T* Get() const noexcept { return p; }

    EckInlineNdCe T** operator&() noexcept { return &p; }
    EckInlineNdCe T* const* operator&() const noexcept { return &p; }

    EckInlineNdCe T** AddrOf() noexcept { return &p; }
    EckInlineNdCe T* const* AddrOf() const noexcept { return &p; }

    EckInlineNdCe T** AddrOfClear() noexcept
    {
        Clear();
        return &p;
    }

    EckInline void Clear() noexcept
    {
        if (p)
        {
            ClearInternal();
            p = nullptr;
        }
    }

    EckInline void Attach(T* p_) noexcept
    {
        Clear();
        p = p_;
    }

    EckInlineNdCe T* Detach() noexcept
    {
        const auto t = p;
        p = nullptr;
        return t;
    }

    EckInline GpStatus Clone(GpPtr& pNew) noexcept
    {
        if (!p)
        {
            pNew.Clear();
            return Gdiplus::GenericError;
        }
        if constexpr (
            std::is_same_v<T, GpEffect> ||
            std::is_same_v<T, GpCachedBitmap>)
            static_assert(!sizeof(T), "Unsupported Clone GDI+ type");
        else if constexpr (std::is_base_of_v<GpBrush, T>)
            return GdipCloneBrush(p, (GpBrush**)pNew.AddrOfClear());
        else if constexpr (std::is_same_v<T, GpPen>)
            return GdipClonePen(p, pNew.AddrOfClear());
        else if constexpr (std::is_base_of_v<GpCustomLineCap, T>)
            return GdipCloneCustomLineCap(p, (GpCustomLineCap**)pNew.AddrOfClear());
        else if constexpr (std::is_base_of_v<GpImage, T>)
            return GdipCloneImage(p, (GpImage**)pNew.AddrOfClear());
        else if constexpr (std::is_same_v<T, GpImageAttributes>)
            return GdipCloneImageAttributes(p, pNew.AddrOfClear());
        else if constexpr (std::is_same_v<T, GpPath>)
            return GdipClonePath(p, pNew.AddrOfClear());
        else if constexpr (std::is_same_v<T, GpRegion>)
            return GdipCloneRegion(p, pNew.AddrOfClear());
        else if constexpr (std::is_same_v<T, GpMatrix>)
            return GdipCloneMatrix(p, pNew.AddrOfClear());
        else if constexpr (std::is_same_v<T, GpFontFamily>)
            return GdipCloneFontFamily(p, pNew.AddrOfClear());
        else if constexpr (std::is_same_v<T, GpFont>)
            return GdipCloneFont(p, pNew.AddrOfClear());
        else if constexpr (std::is_same_v<T, GpStringFormat>)
            return GdipCloneStringFormat(p, pNew.AddrOfClear());
        else
            static_assert(!sizeof(T), "Unsupported Clone GDI+ type");
    }
};
ECK_NAMESPACE_END