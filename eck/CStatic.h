#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
inline constexpr int CDV_STATIC_1 = 1;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_STATIC
{
    int iVer;
};
#pragma pack(pop)

inline constexpr DWORD StaticTypeMask = (SS_LEFT | SS_CENTER | SS_RIGHT | SS_ICON |
    SS_BLACKRECT | SS_GRAYRECT | SS_WHITERECT | SS_BLACKFRAME | SS_GRAYFRAME |
    SS_WHITEFRAME | SS_USERITEM | SS_SIMPLE | SS_LEFTNOWORDWRAP | SS_OWNERDRAW |
    SS_BITMAP | SS_ENHMETAFILE | SS_ETCHEDHORZ | SS_ETCHEDVERT | SS_ETCHEDFRAME);

class CStatic : public CWindow
{
public:
    ECK_RTTI(CStatic, CWindow);
    ECK_CWND_NOSINGLEOWNER(CStatic);
    ECK_CWND_CREATE_CLS(WC_STATICW);

    ECK_CWNDPROP_STYLE(ShowBitmap, SS_BITMAP);
    ECK_CWNDPROP_STYLE_MASK(BlackFrame, SS_BLACKFRAME, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(BlackRect, SS_BLACKRECT, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(AlignCenter, SS_CENTER, StaticTypeMask);
    ECK_CWNDPROP_STYLE(CenterImage, SS_CENTERIMAGE);
    ECK_CWNDPROP_STYLE(EditControl, SS_EDITCONTROL);
    ECK_CWNDPROP_STYLE(EndEllipsis, SS_ENDELLIPSIS);
    ECK_CWNDPROP_STYLE_MASK(EnhMetaFile, SS_ENHMETAFILE, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(EtchedFrame, SS_ETCHEDHORZ, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(EtchedHorz, SS_ETCHEDHORZ, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(EtchedVert, SS_ETCHEDVERT, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(GrayFrame, SS_GRAYFRAME, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(GrayRect, SS_GRAYRECT, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(Icon, SS_ICON, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(AlignLeft, SS_LEFT, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(LeftNoWordWrap, SS_LEFTNOWORDWRAP, StaticTypeMask);
    ECK_CWNDPROP_STYLE(NoPrefix, SS_NOPREFIX);
    ECK_CWNDPROP_STYLE(Notify, SS_NOTIFY);
    ECK_CWNDPROP_STYLE_MASK(OwnerDraw, SS_OWNERDRAW, StaticTypeMask);
    ECK_CWNDPROP_STYLE(PathEllipsis, SS_PATHELLIPSIS);
    ECK_CWNDPROP_STYLE(RealSizeImage, SS_REALSIZEIMAGE);
    ECK_CWNDPROP_STYLE(RealSizeControl, SS_REALSIZECONTROL);
    ECK_CWNDPROP_STYLE_MASK(AlignRight, SS_RIGHT, StaticTypeMask);
    ECK_CWNDPROP_STYLE(RightJust, SS_RIGHTJUST);
    ECK_CWNDPROP_STYLE_MASK(Simple, SS_SIMPLE, StaticTypeMask);
    ECK_CWNDPROP_STYLE(Sunken, SS_SUNKEN);
    ECK_CWNDPROP_STYLE_MASK(WhiteFrame, SS_WHITEFRAME, StaticTypeMask);
    ECK_CWNDPROP_STYLE_MASK(WhiteRect, SS_WHITERECT, StaticTypeMask);
    ECK_CWNDPROP_STYLE(WordEllipsis, SS_WORDELLIPSIS);

    EckInlineNdCe static PCVOID SkipBaseData(PCVOID p)
    {
        return PointerStepBytes(CWindow::SkipBaseData(p), sizeof(CTRLDATA_STATIC));
    }

    void SerializeData(CByteBuffer& rb, const SERIALIZE_OPT* pOpt = nullptr) noexcept override
    {
        CWindow::SerializeData(rb, pOpt);
        constexpr auto cbSize = sizeof(CTRLDATA_STATIC);
        CMemoryWalker w(rb.PushBack(cbSize), cbSize);
        CTRLDATA_STATIC* p;
        w.SkipPointer(p);
        p->iVer = CDV_STATIC_1;
    }

    EckInline HICON GetIcon() const noexcept
    {
        return (HICON)SendMessage(STM_GETICON, 0, 0);
    }

    /// <summary>
    /// 取图像
    /// </summary>
    /// <param name="uType">图像类型，IMAGE_常量</param>
    /// <returns></returns>
    EckInline HANDLE GetImage(UINT uType = IMAGE_BITMAP) const noexcept
    {
        return (HANDLE)SendMessage(STM_GETIMAGE, uType, 0);
    }

    EckInline HICON SetIcon(HICON hIcon) const noexcept
    {
        return (HICON)SendMessage(STM_SETICON, (WPARAM)hIcon, 0);
    }

    /// <summary>
    /// 置图像
    /// </summary>
    /// <param name="h">图像句柄，含义由uType决定</param>
    /// <param name="uType">图像类型，IMAGE_常量</param>
    /// <returns>先前的图像句柄</returns>
    EckInline HANDLE SetImage(HANDLE h, UINT uType = IMAGE_BITMAP) const noexcept
    {
        return (HANDLE)SendMessage(STM_SETIMAGE, uType, (LPARAM)h);
    }
};
ECK_NAMESPACE_END