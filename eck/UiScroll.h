#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
ECK_UIBASIC_NAMESPACE_BEGIN
template<CcpNumber TCoord>
struct __declspec(novtable) IScrollControllerT
{
    struct SCC_CALLBACK_DATA
    {
        TCoord fPos;
        TCoord fPrevPos;
        BOOLEAN bAnimating;
        void* pUser;
    };
    using FSccCallback = void(*)(const SCC_CALLBACK_DATA& Data);

    virtual ~IScrollControllerT() = default;

    virtual TCoord SccGetPosition() const noexcept = 0;
    virtual void SccSetPosition(TCoord x) noexcept = 0;
    virtual TCoord SccGetPage() const noexcept = 0;
    virtual void SccSetPage(TCoord x) noexcept = 0;
    virtual TCoord SccGetMinimum() const noexcept = 0;
    virtual void SccSetMinimum(TCoord x) noexcept = 0;
    virtual TCoord SccGetMaximum() const noexcept = 0;
    virtual void SccSetMaximum(TCoord x) noexcept = 0;
    virtual void SccSetRange(TCoord Min, TCoord Max) noexcept = 0;
    virtual void SccSetVisible(BOOL b) noexcept = 0;
    virtual void SccRedraw() noexcept = 0;

    virtual void SccMouseWheel(TCoord d) noexcept = 0;
    virtual void SccScrollDelta(TCoord d, BOOL bSmooth) noexcept = 0;

    virtual void SccSetCallback(FSccCallback pfnCallback, void* pUser) noexcept = 0;

    /*
    TCoord SccGetPosition() const noexcept override {}
    void SccSetPosition(TCoord x) noexcept override {}
    TCoord SccGetPage() const noexcept override {}
    void SccSetPage(TCoord x) noexcept override {}
    TCoord SccGetMinimum() const noexcept override {}
    void SccSetMinimum(TCoord x) noexcept override {}
    TCoord SccGetMaximum() const noexcept override {}
    void SccSetMaximum(TCoord x) noexcept override {}
    void SccSetRange(TCoord Min, TCoord Max) noexcept override {}
    void SccSetVisible(BOOL b) noexcept override {}
    void SccRedraw() noexcept override {}

    void SccMouseWheel(TCoord d) noexcept override {}
    void SccScrollDelta(TCoord d, BOOL bSmooth) noexcept override {}

    void SccSetCallback(FSccCallback pfnCallback, void* pUser) noexcept override {}
    */
};
ECK_UIBASIC_NAMESPACE_END
ECK_NAMESPACE_END