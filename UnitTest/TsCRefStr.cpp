#include "pch.h"
#include "../eck/CRefStr.h"

using namespace eck;

TS_NS_BEGIN
TEST_CLASS(TsCRefStr)
{
public:
    TEST_METHOD(TsToBstr)
    {
        CRefStrT<WCHAR> strW(L"Hello, 世界");
        BSTR bstr = strW.ToBSTR();
        Assert::AreEqual((UINT)SysStringLen(bstr), (UINT)strW.Size());
        Assert::AreEqual(0, wcscmp(bstr, L"Hello, 世界"));
        SysFreeString(bstr);
        CRefStrT<CHAR> strA("Hello, World");
        bstr = strA.ToBSTR();
        Assert::AreEqual((UINT)SysStringLen(bstr), (UINT)strA.Size());
        Assert::AreEqual(0, wcscmp(bstr, L"Hello, World"));
        SysFreeString(bstr);

        CRefStrA strEmpty{};
        bstr = strEmpty.ToBSTR();
        Assert::AreEqual(0u, SysStringLen(bstr));
    }
};
TS_NS_END