#include "pch.h"
#include "../eck/CArray.h"
#include "../eck/CArray2D.h"

TS_NS_BEGIN
TEST_CLASS(TsCArray)
{
public:
    TEST_METHOD(TsArray)
    {
        eck::CArray<int> arr{ 10,10,2,4,5 };

        Assert::AreEqual(size_t(10 * 10 * 2 * 4 * 5), arr.Size());
        Assert::AreEqual((size_t)2, arr.Size(2));

        arr[3].Data() = 100;
        Assert::AreEqual(100, arr[3][0][0][0][0].Data());
    }

    TEST_METHOD(TsArray2D)
    {
        eck::CArray2D<int> arr{ 10,4 };
        Assert::AreEqual(size_t(10 * 4), arr.Size());
        Assert::AreEqual((size_t)10, arr.Size(0));
        Assert::AreEqual((size_t)4, arr.Size(1));
        arr[3][2] = 100;
        Assert::AreEqual(100, arr.Data()[3 * 4 + 2]);
    }
};
TS_NS_END