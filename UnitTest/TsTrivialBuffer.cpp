#include "pch.h"
#include "../eck/CTrivialBuffer.h"

using eck::CTrivialBuffer;

TS_NS_BEGIN
TEST_CLASS(TsTrivialBuffer)
{
public:
    // 测试默认构造函数
    TEST_METHOD(TestDefaultConstructor)
    {
        CTrivialBuffer<int> buffer;
        Assert::AreEqual(size_t(0), buffer.Size());
        Assert::IsNull(buffer.Data());
    }

    // 测试带大小参数的构造函数
    TEST_METHOD(TestSizeConstructor)
    {
        CTrivialBuffer<int> buffer(10);
        Assert::AreEqual(size_t(10), buffer.Size());
        Assert::IsNotNull(buffer.Data());
    }

    // 测试从数组构造
    TEST_METHOD(TestArrayConstructor)
    {
        int arr[] = { 1, 2, 3, 4, 5 };
        CTrivialBuffer<int> buffer(arr, 5);
        Assert::AreEqual(size_t(5), buffer.Size());
        for (size_t i = 0; i < 5; ++i)
        {
            Assert::AreEqual(arr[i], buffer[i]);
        }
    }

    // 测试拷贝构造函数
    TEST_METHOD(TestCopyConstructor)
    {
        int arr[] = { 1, 2, 3 };
        CTrivialBuffer<int> buffer1(arr, 3);
        CTrivialBuffer<int> buffer2(buffer1);

        Assert::AreEqual(buffer1.Size(), buffer2.Size());
        for (size_t i = 0; i < buffer1.Size(); ++i)
        {
            Assert::AreEqual(buffer1[i], buffer2[i]);
        }
        // 确保是深拷贝
        Assert::AreNotEqual((void*)buffer1.Data(), (void*)buffer2.Data());
    }

    // 测试移动构造函数
    TEST_METHOD(TestMoveConstructor)
    {
        int arr[] = { 1, 2, 3 };
        CTrivialBuffer<int> buffer1(arr, 3);
        int* oldPtr = buffer1.Data();
        size_t oldSize = buffer1.Size();

        CTrivialBuffer<int> buffer2(std::move(buffer1));

        Assert::AreEqual(oldSize, buffer2.Size());
        Assert::AreEqual((void*)oldPtr, (void*)buffer2.Data());
        Assert::AreEqual(size_t(0), buffer1.Size());
    }

    // 测试拷贝赋值运算符
    TEST_METHOD(TestCopyAssignment)
    {
        int arr1[] = { 1, 2, 3 };
        int arr2[] = { 4, 5, 6, 7 };
        CTrivialBuffer<int> buffer1(arr1, 3);
        CTrivialBuffer<int> buffer2(arr2, 4);

        buffer1 = buffer2;

        Assert::AreEqual(buffer2.Size(), buffer1.Size());
        for (size_t i = 0; i < buffer2.Size(); ++i)
        {
            Assert::AreEqual(buffer2[i], buffer1[i]);
        }
    }

    // 测试自赋值
    TEST_METHOD(TestSelfAssignment)
    {
        int arr[] = { 1, 2, 3 };
        CTrivialBuffer<int> buffer(arr, 3);
        buffer = buffer;

        Assert::AreEqual(size_t(3), buffer.Size());
        for (size_t i = 0; i < 3; ++i)
        {
            Assert::AreEqual(arr[i], buffer[i]);
        }
    }

    // 测试移动赋值运算符
    TEST_METHOD(TestMoveAssignment)
    {
        int arr1[] = { 1, 2, 3 };
        int arr2[] = { 4, 5, 6, 7 };
        CTrivialBuffer<int> buffer1(arr1, 3);
        CTrivialBuffer<int> buffer2(arr2, 4);

        buffer1 = std::move(buffer2);

        Assert::AreEqual(size_t(4), buffer1.Size());
        Assert::AreEqual(4, buffer1[0]);
    }

    // 测试 Reserve
    TEST_METHOD(TestReserve)
    {
        CTrivialBuffer<int> buffer;
        buffer.Reserve(100);

        // 容量应该至少为100，但 Size 应该仍为0
        Assert::AreEqual(size_t(0), buffer.Size());
        Assert::IsNotNull(buffer.Data());
    }

    // 测试 ReSize
    TEST_METHOD(TestReSize)
    {
        CTrivialBuffer<int> buffer;
        buffer.ReSize(50);

        Assert::AreEqual(size_t(50), buffer.Size());
        Assert::IsNotNull(buffer.Data());
    }

    // 测试 Clear
    TEST_METHOD(TestClear)
    {
        int arr[] = { 1, 2, 3 };
        CTrivialBuffer<int> buffer(arr, 3);
        buffer.Clear();

        Assert::AreEqual(size_t(0), buffer.Size());
    }

    // 测试 PushBack 单个元素
    TEST_METHOD(TestPushBackSingle)
    {
        CTrivialBuffer<int> buffer;
        buffer.PushBack(42);

        Assert::AreEqual(size_t(1), buffer.Size());
        Assert::AreEqual(42, buffer[0]);
    }

    // 测试 PushBack 多个元素
    TEST_METHOD(TestPushBackMultiple)
    {
        CTrivialBuffer<int> buffer;
        int arr[] = { 1, 2, 3, 4, 5 };
        buffer.PushBack(arr, 5);

        Assert::AreEqual(size_t(5), buffer.Size());
        for (size_t i = 0; i < 5; ++i)
        {
            Assert::AreEqual(arr[i], buffer[i]);
        }
    }

    // 测试 EmplaceBack
    TEST_METHOD(TestEmplaceBack)
    {
        CTrivialBuffer<int> buffer;
        buffer.EmplaceBack(100);

        Assert::AreEqual(size_t(1), buffer.Size());
        Assert::AreEqual(100, buffer[0]);
    }

    // 测试 PopBack
    TEST_METHOD(TestPopBack)
    {
        int arr[] = { 1, 2, 3, 4, 5 };
        CTrivialBuffer<int> buffer(arr, 5);
        buffer.PopBack();

        Assert::AreEqual(size_t(4), buffer.Size());
        buffer.PopBack(2);
        Assert::AreEqual(size_t(2), buffer.Size());
    }

    // 测试 Erase
    TEST_METHOD(TestErase)
    {
        int arr[] = { 1, 2, 3, 4, 5 };
        CTrivialBuffer<int> buffer(arr, 5);
        buffer.Erase(1, 2); // 删除索引1和2的元素

        Assert::AreEqual(size_t(3), buffer.Size());
        Assert::AreEqual(1, buffer[0]);
        Assert::AreEqual(4, buffer[1]);
        Assert::AreEqual(5, buffer[2]);
    }

    // 测试 Replace
    TEST_METHOD(TestReplace)
    {
        int arr[] = { 1, 2, 3, 4, 5 };
        int newArr[] = { 10, 20 };
        CTrivialBuffer<int> buffer(arr, 5);
        buffer.Replace(1, 2, newArr, 2); // 替换索引1-2的元素

        Assert::AreEqual(size_t(5), buffer.Size());
        Assert::AreEqual(1, buffer[0]);
        Assert::AreEqual(10, buffer[1]);
        Assert::AreEqual(20, buffer[2]);
        Assert::AreEqual(4, buffer[3]);
        Assert::AreEqual(5, buffer[4]);
    }

    // 测试 At 方法
    TEST_METHOD(TestAt)
    {
        int arr[] = { 10, 20, 30 };
        CTrivialBuffer<int> buffer(arr, 3);

        Assert::AreEqual(10, buffer.At(0));
        Assert::AreEqual(20, buffer.At(1));
        Assert::AreEqual(30, buffer.At(2));
    }

    // 测试下标运算符
    TEST_METHOD(TestSubscriptOperator)
    {
        int arr[] = { 10, 20, 30 };
        CTrivialBuffer<int> buffer(arr, 3);

        Assert::AreEqual(10, buffer[0]);
        Assert::AreEqual(20, buffer[1]);
        Assert::AreEqual(30, buffer[2]);

        buffer[1] = 100;
        Assert::AreEqual(100, buffer[1]);
    }

    // 测试迭代器
    TEST_METHOD(TestIterators)
    {
        int arr[] = { 1, 2, 3, 4, 5 };
        CTrivialBuffer<int> buffer(arr, 5);

        int sum = 0;
        for (auto it = buffer.begin(); it != buffer.end(); ++it)
        {
            sum += *it;
        }
        Assert::AreEqual(15, sum);
    }

    // 测试范围for循环
    TEST_METHOD(TestRangeBasedFor)
    {
        int arr[] = { 1, 2, 3, 4, 5 };
        CTrivialBuffer<int> buffer(arr, 5);

        int sum = 0;
        for (const auto& val : buffer)
        {
            sum += val;
        }
        Assert::AreEqual(15, sum);
    }

    // 测试 ByteSize
    TEST_METHOD(TestByteSize)
    {
        CTrivialBuffer<int> buffer(10);
        Assert::AreEqual(size_t(10 * sizeof(int)), buffer.ByteSize());
    }

    // 测试不同类型
    TEST_METHOD(TestDifferentTypes)
    {
        CTrivialBuffer<char> charBuffer;
        charBuffer.PushBack('A');
        Assert::AreEqual('A', charBuffer[0]);

        CTrivialBuffer<double> doubleBuffer;
        doubleBuffer.PushBack(3.14);
        Assert::AreEqual(3.14, doubleBuffer[0], 0.001);
    }

    // 测试空缓冲区操作
    TEST_METHOD(TestEmptyBuffer)
    {
        CTrivialBuffer<int> buffer;
        Assert::AreEqual(size_t(0), buffer.Size());
        Assert::AreEqual(size_t(0), buffer.ByteSize());

        buffer.Clear(); // 应该不会崩溃
        Assert::AreEqual(size_t(0), buffer.Size());
    }

    // 测试大量数据
    TEST_METHOD(TestLargeData)
    {
        const size_t largeSize = 10000;
        CTrivialBuffer<int> buffer(largeSize);

        for (size_t i = 0; i < largeSize; ++i)
        {
            buffer[i] = static_cast<int>(i);
        }

        for (size_t i = 0; i < largeSize; ++i)
        {
            Assert::AreEqual(static_cast<int>(i), buffer[i]);
        }
    }
};
TS_NS_END