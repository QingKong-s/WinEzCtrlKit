#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct ICONDIRENTRY
{
    BYTE bWidth;		// 宽度
    BYTE bHeight;		// 高度
    BYTE bColorCount;	// 颜色数
    BYTE bReserved;		// 保留，必须为0
    WORD wPlanes;		// 颜色平面数，必须为1
    WORD wBitCount;		// 位深度
    DWORD dwBytesInRes;	// 在资源中的字节数
    DWORD dwImageOffset;// 在映像文件中的偏移
};

struct ICONDIR
{
    WORD idReserved;	// 保留，必须为0
    WORD idType;		// 资源类型，1 = 图标
    WORD idCount;		// 图标数
    // ICONDIRENTRY idEntries[1]; // 图像数组
};

class CIcoFileReader
{
private:
    PCBYTE m_pData{};
    const ICONDIR* m_pHeader{};
    const ICONDIRENTRY* m_pEntry{};
public:
    constexpr CIcoFileReader(PCBYTE pData) noexcept { AnalyzeData(pData); }

    // 返回图标个数
    EckInlineCe int AnalyzeData(PCBYTE pData) noexcept
    {
        m_pData = pData;
        m_pHeader = (ICONDIR*)pData;
        m_pEntry = (ICONDIRENTRY*)(pData + sizeof(ICONDIR));
        return m_pHeader->idCount;
    }

    EckInlineNdCe auto GetHeader() const noexcept { return m_pHeader; }
    EckInlineNdCe auto GetEntry() const noexcept { return m_pEntry; }

    EckInlineNdCe PCVOID GetIconData(int idx) const noexcept
    {
        EckAssert(idx >= 0 && idx < GetIconCount());
        return m_pData + m_pEntry[idx].dwImageOffset;
    }

    EckInlineNdCe UINT GetIconDataSize(int idx) const noexcept
    {
        EckAssert(idx >= 0 && idx < GetIconCount());
        return m_pEntry[idx].dwBytesInRes;
    }

    EckInlineNdCe int GetIconCount() const noexcept { return m_pHeader->idCount; }

    EckInlineNdCe int FindIcon(int cx, int cy) const noexcept
    {
        EckCounter(GetIconCount(), i)
        {
            if (m_pEntry[i].bWidth == cx && m_pEntry[i].bHeight == cy)
                return i;
        }
        return -1;
    }

    EckInlineNd HICON CreateIcon(int idx,
        int cx = 0, int cy = 0, UINT uFlags = 0u) const noexcept
    {
        EckAssert(idx >= 0 && idx < GetIconCount());
        return CreateIconFromResourceEx(
            (BYTE*)GetIconData(idx), GetIconDataSize(idx),
            TRUE, 0x00030000, cx, cy, uFlags);
    }

    EckInlineNdCe auto At(int idx) const noexcept
    {
        EckAssert(idx >= 0 && idx < GetIconCount());
        return m_pEntry + idx;
    }
};
ECK_NAMESPACE_END