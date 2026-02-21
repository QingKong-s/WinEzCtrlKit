#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
constexpr IID IID_IMemoryView
{ 0xb23501eb, 0x3f09, 0x4a54, { 0x94, 0xd4, 0xf2, 0x5, 0x4f, 0x17, 0xa3, 0x81 } };

struct __declspec(uuid("B23501EB-3F09-4A54-94D4-F2054F17A381"))
    IMemoryView : public IUnknown
{
    // 锁定对象，返回对象关联的内存地址和大小，在调用MemUnlock之前一直有效
    STDMETHOD(MemLock)(void** ppData, SIZE_T* pSize) = 0;

    // 解锁对象
    STDMETHOD(MemUnlock)() = 0;

    // 获取对象关联的内存地址和大小，当修改对象后返回的信息可能失效
    STDMETHOD(MemGetPtr)(void** ppData, SIZE_T* pSize) = 0;

    // 是否已锁定
    STDMETHOD(MemIsLocked)(BOOL* pIsLocked) = 0;
};
ECK_NAMESPACE_END