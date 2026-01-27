#pragma once
#include "CObject.h"

ECK_NAMESPACE_BEGIN
using TLytCoord = float;

struct LYTRECT
{
    TLytCoord x;
    TLytCoord y;
    TLytCoord cx;
    TLytCoord cy;
};
struct LYTPOINT
{
    TLytCoord x;
    TLytCoord y;
};
struct LYTSIZE
{
    TLytCoord cx;
    TLytCoord cy;
};
struct LYTMARGINS
{
    TLytCoord l;
    TLytCoord t;
    TLytCoord r;
    TLytCoord b;
};

EckNfInlineCe BOOL AdjustRectToFitAnother(_Inout_ LYTRECT& rc, const LYTRECT& rcRef)
{
    const float
        cxMax = rcRef.cx,
        cyMax = rcRef.cy,
        cx0 = rc.cx,
        cy0 = rc.cy;
    if (cxMax <= 0 || cyMax <= 0 || cx0 <= 0 || cy0 <= 0)
        return FALSE;
    float cx, cy;
    if (cxMax * cy0 > cx0 * cyMax)// y对齐
    {
        cy = cyMax;
        cx = cx0 * cy / cy0;
    }
    else// x对齐
    {
        cx = cxMax;
        cy = cx * cy0 / cx0;
    }

    rc.x = rcRef.x + (cxMax - cx) / 2;
    rc.y = rcRef.y + (cyMax - cy) / 2;
    rc.cx = cx;
    rc.cy = cy;
    return TRUE;
}

// 表示一个可调整位置和大小的对象，并提供非原子布局（如布局器）的管理能力
// CWnd、Dui::CElem和内置布局器实现此接口
struct __declspec(novtable) ILayout : public CObject
{
    ECK_RTTI(ILayout, CObject);

    // 输入参照尺寸（可为0），基于参照尺寸计算理想尺寸，更具体的行为由实现决定
    // 如果未实现此函数，返回FALSE
    virtual BOOL LoGetIdealSize(_Inout_ LYTSIZE& Size) noexcept { return FALSE; }
    virtual void LoSetPosition(LYTPOINT pt) noexcept = 0;
    virtual void LoSetSize(LYTSIZE size) noexcept = 0;
    virtual void LoSetRect(const LYTRECT& rc) noexcept = 0;
    virtual LYTPOINT LoGetPosition() noexcept = 0;
    virtual LYTSIZE LoGetSize() noexcept = 0;
    virtual void LoShow(BOOL bShow) noexcept = 0;
    // 提交对布局所做的修改，通常仅对非原子布局生效
    virtual void LoCommit() noexcept {}
    // 若当前类为窗口，则返回窗口句柄
    // 内置布局器使用DeferWindowPos，而不是LoSetXxx
    virtual HWND LoGetHWND() noexcept { return nullptr; }
    // 指示布局持有数据已使用指定DPI缩放，通常仅对非原子布局生效
    virtual void LoInitializeDpi(int iDpi) noexcept {}
    // 布局的DPI已改变，通常仅对非原子布局生效
    virtual void LoOnDpiChanged(int iDpi) noexcept {}
    // 指示布局器在后续排列过程中将窗口操作录制到该DWP句柄
    // 如果布局器接受，则应返回TRUE，句柄的所有权转移到布局器，排列操作完成后，
    // 调用方使用LoDwpDetach取回新的DWP句柄
    //
    // 使用布局器时，必须对最顶层布局器显式调用LoDwpAttach，内置布局器自动传播
    // 到其下的所有子布局
    // WARNING Win32要求被录制窗口必须具有相同的父窗口，若确实可能发生此情况，
    // 调用方可对布局器使用SetUseDwp
    virtual BOOL LoDwpAttach(HDWP hDwp) noexcept { return FALSE; }
    // 取回DWP句柄，若返回值有效，所有权归属调用方
    // LoDwpAttach成功后必须调用本函数
    virtual HDWP LoDwpDetach() noexcept { return nullptr; }
};
ECK_NAMESPACE_END