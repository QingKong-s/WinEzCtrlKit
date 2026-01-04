#pragma once
#define ECK_OPT_NO_DBG_MACRO 1

#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define TS_NS_BEGIN namespace Ts {
#define TS_NS_END	}

#define EckAssert(x)                (void)(!!(x) || (DebugBreak(), 0))
#define EckDbgBreak()               DebugBreak()
#define EckDbgPrint(x)              ;
/*
#include "pch.h"

TS_NS_BEGIN
TEST_CLASS(Ts)
{
public:
    TEST_METHOD(Ts)
    {
    }
};
TS_NS_END
*/