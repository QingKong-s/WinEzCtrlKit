#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
class CLayoutDummy : public CLayoutBase
{
public:
    size_t LobGetObjectCount() const noexcept override { return 0; }
};
ECK_NAMESPACE_END