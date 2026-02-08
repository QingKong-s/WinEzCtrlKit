#include "pch.h"
#include "../eck/RefPtr.h"

using namespace eck;

TS_NS_BEGIN
class TestObject
{
public:
    int value;
    static int constructCount;
    static int destructCount;

    TestObject() : value(0)
    {
        constructCount++;
    }

    explicit TestObject(int v) : value(v)
    {
        constructCount++;
    }

    TestObject(int a, int b) : value(a + b)
    {
        constructCount++;
    }

    ~TestObject()
    {
        destructCount++;
    }

    static void ResetCounters()
    {
        constructCount = 0;
        destructCount = 0;
    }
};

int TestObject::constructCount = 0;
int TestObject::destructCount = 0;

TEST_CLASS(TsRefPtr)
{
public:
    TEST_METHOD_INITIALIZE(Setup)
    {
        TestObject::ResetCounters();
    }

    TEST_METHOD(TestDefaultConstruction)
    {
        RefPtr<TestObject> ptr;
        Assert::IsNull(ptr.Get());
        Assert::AreEqual(0u, ptr.GetReferenceCount());
    }

    TEST_METHOD(TestMakeWithNoArgs)
    {
        auto ptr = RefPtr<TestObject>::Make();
        Assert::IsNotNull(ptr.Get());
        Assert::AreEqual(0, ptr->value);
        Assert::AreEqual(1u, ptr.GetReferenceCount());
        Assert::AreEqual(1, TestObject::constructCount);
    }

    TEST_METHOD(TestMakeWithSingleArg)
    {
        auto ptr = RefPtr<TestObject>::Make(42);
        Assert::IsNotNull(ptr.Get());
        Assert::AreEqual(42, ptr->value);
        Assert::AreEqual(1u, ptr.GetReferenceCount());
    }

    TEST_METHOD(TestMakeWithMultipleArgs)
    {
        auto ptr = RefPtr<TestObject>::Make(10, 32);
        Assert::IsNotNull(ptr.Get());
        Assert::AreEqual(42, ptr->value);
        Assert::AreEqual(1u, ptr.GetReferenceCount());
    }

    TEST_METHOD(TestDestruction)
    {
        {
            auto ptr = RefPtr<TestObject>::Make(42);
            Assert::AreEqual(1, TestObject::constructCount);
            Assert::AreEqual(0, TestObject::destructCount);
        }
        Assert::AreEqual(1, TestObject::destructCount);
    }

    TEST_METHOD(TestDereferenceOperator)
    {
        auto ptr = RefPtr<TestObject>::Make(42);
        Assert::AreEqual(42, (*ptr).value);
        (*ptr).value = 100;
        Assert::AreEqual(100, ptr->value);
    }

    TEST_METHOD(TestArrowOperator)
    {
        auto ptr = RefPtr<TestObject>::Make(42);
        Assert::AreEqual(42, ptr->value);
        ptr->value = 100;
        Assert::AreEqual(100, (*ptr).value);
    }

    TEST_METHOD(TestGetMethod)
    {
        auto ptr = RefPtr<TestObject>::Make(42);
        TestObject* rawPtr = ptr.Get();
        Assert::IsNotNull(rawPtr);
        Assert::AreEqual(42, rawPtr->value);
    }

    TEST_METHOD(TestGetMethodOnNullPtr)
    {
        RefPtr<TestObject> ptr;
        Assert::IsNull(ptr.Get());
    }

    TEST_METHOD(TestCopyConstruction)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        Assert::AreEqual(1u, ptr1.GetReferenceCount());

        RefPtr<TestObject> ptr2(ptr1);
        Assert::AreEqual(2u, ptr1.GetReferenceCount());
        Assert::AreEqual(2u, ptr2.GetReferenceCount());
        Assert::IsTrue(ptr1.Get() == ptr2.Get());
        Assert::AreEqual(42, ptr2->value);
    }

    TEST_METHOD(TestCopyConstructionFromNull)
    {
        RefPtr<TestObject> ptr1;
        RefPtr<TestObject> ptr2(ptr1);
        Assert::IsNull(ptr2.Get());
        Assert::AreEqual(0u, ptr2.GetReferenceCount());
    }

    TEST_METHOD(TestCopyAssignment)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        auto ptr2 = RefPtr<TestObject>::Make(100);

        Assert::AreEqual(2, TestObject::constructCount);
        Assert::AreEqual(0, TestObject::destructCount);

        ptr2 = ptr1;

        Assert::AreEqual(1, TestObject::destructCount); // ptr2 原对象被销毁
        Assert::AreEqual(2u, ptr1.GetReferenceCount());
        Assert::AreEqual(2u, ptr2.GetReferenceCount());
        Assert::IsTrue(ptr1.Get() == ptr2.Get());
        Assert::AreEqual(42, ptr2->value);
    }

    TEST_METHOD(TestSelfAssignment)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        auto* originalPtr = ptr1.Get();

        ptr1 = ptr1; // 自赋值

        Assert::IsTrue(originalPtr == ptr1.Get());
        Assert::AreEqual(1u, ptr1.GetReferenceCount());
        Assert::AreEqual(42, ptr1->value);
        Assert::AreEqual(0, TestObject::destructCount); // 不应该销毁对象
    }

    TEST_METHOD(TestCopyAssignmentToNull)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        RefPtr<TestObject> ptr2;

        ptr2 = ptr1;
        Assert::AreEqual(2u, ptr1.GetReferenceCount());
        Assert::AreEqual(2u, ptr2.GetReferenceCount());
    }

    TEST_METHOD(TestCopyAssignmentFromNull)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        RefPtr<TestObject> ptr2;

        ptr1 = ptr2;
        Assert::IsNull(ptr1.Get());
        Assert::AreEqual(0u, ptr1.GetReferenceCount());
        Assert::AreEqual(1, TestObject::destructCount);
    }

    TEST_METHOD(TestMoveConstruction)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        auto* originalPtr = ptr1.Get();

        RefPtr<TestObject> ptr2(std::move(ptr1));

        Assert::IsTrue(originalPtr == ptr2.Get());
        Assert::AreEqual(1u, ptr2.GetReferenceCount());
        Assert::AreEqual(42, ptr2->value);
        Assert::AreEqual(0, TestObject::destructCount); // 没有对象被销毁
    }

    TEST_METHOD(TestMoveConstructionFromNull)
    {
        RefPtr<TestObject> ptr1;
        RefPtr<TestObject> ptr2(std::move(ptr1));

        Assert::IsNull(ptr2.Get());
    }

    TEST_METHOD(TestMoveAssignment)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        auto ptr2 = RefPtr<TestObject>::Make(100);
        auto* originalPtr1 = ptr1.Get();

        ptr2 = std::move(ptr1);

        Assert::IsTrue(originalPtr1 == ptr2.Get());
        Assert::AreEqual(1u, ptr2.GetReferenceCount());
        Assert::AreEqual(42, ptr2->value);
    }

    TEST_METHOD(TestMoveAssignmentToNull)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        RefPtr<TestObject> ptr2;
        auto* originalPtr = ptr1.Get();

        ptr2 = std::move(ptr1);

        Assert::IsTrue(originalPtr == ptr2.Get());
        Assert::AreEqual(1u, ptr2.GetReferenceCount());
    }

    TEST_METHOD(TestMoveAssignmentFromNull)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        RefPtr<TestObject> ptr2;
        ptr1 = std::move(ptr2);

        Assert::IsNull(ptr1.Get());
    }

    TEST_METHOD(TestMultipleReferences)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        Assert::AreEqual(1u, ptr1.GetReferenceCount());

        auto ptr2 = ptr1;
        Assert::AreEqual(2u, ptr1.GetReferenceCount());

        auto ptr3 = ptr2;
        Assert::AreEqual(3u, ptr1.GetReferenceCount());

        {
            auto ptr4 = ptr3;
            Assert::AreEqual(4u, ptr1.GetReferenceCount());
        }

        Assert::AreEqual(3u, ptr1.GetReferenceCount());
        Assert::AreEqual(0, TestObject::destructCount);
    }

    TEST_METHOD(TestReferenceCountDecrement)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        {
            auto ptr2 = ptr1;
            auto ptr3 = ptr1;
            Assert::AreEqual(3u, ptr1.GetReferenceCount());
        }
        Assert::AreEqual(1u, ptr1.GetReferenceCount());
        Assert::AreEqual(0, TestObject::destructCount);
    }

    TEST_METHOD(TestObjectDestructionWhenLastReferenceGone)
    {
        {
            auto ptr1 = RefPtr<TestObject>::Make(42);
            {
                auto ptr2 = ptr1;
                auto ptr3 = ptr1;
                Assert::AreEqual(3u, ptr1.GetReferenceCount());
                Assert::AreEqual(0, TestObject::destructCount);
            }
            Assert::AreEqual(1u, ptr1.GetReferenceCount());
            Assert::AreEqual(0, TestObject::destructCount);
        }
        Assert::AreEqual(1, TestObject::destructCount);
    }

    TEST_METHOD(TestChainedAssignment)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        RefPtr<TestObject> ptr2, ptr3, ptr4;

        ptr4 = ptr3 = ptr2 = ptr1;

        Assert::AreEqual(4u, ptr1.GetReferenceCount());
        Assert::IsTrue(ptr1.Get() == ptr2.Get());
        Assert::IsTrue(ptr1.Get() == ptr3.Get());
        Assert::IsTrue(ptr1.Get() == ptr4.Get());
    }

    TEST_METHOD(TestReassignment)
    {
        auto ptr = RefPtr<TestObject>::Make(42);
        Assert::AreEqual(42, ptr->value);

        ptr = RefPtr<TestObject>::Make(100);
        Assert::AreEqual(100, ptr->value);
        Assert::AreEqual(1u, ptr.GetReferenceCount());
        Assert::AreEqual(1, TestObject::destructCount);
    }

    TEST_METHOD(TestModifyThroughMultipleReferences)
    {
        auto ptr1 = RefPtr<TestObject>::Make(42);
        auto ptr2 = ptr1;

        ptr1->value = 100;
        Assert::AreEqual(100, ptr2->value);

        ptr2->value = 200;
        Assert::AreEqual(200, ptr1->value);
    }
};

// 测试用于验证 noexcept 规范的类型
struct NoThrowConstructible
{
    NoThrowConstructible() noexcept = default;
    NoThrowConstructible(int) noexcept {}
};

struct ThrowConstructible
{
    ThrowConstructible() {}
    ThrowConstructible(int) {}
};

TEST_CLASS(TsRefPtrNoexcept)
{
public:
    TEST_METHOD(TestNoexceptSpecifications)
    {
        // 这些是编译时测试
        static_assert(noexcept(RefPtr<NoThrowConstructible>::Make()),
            "Make should be noexcept for noexcept constructible types");

        static_assert(!noexcept(RefPtr<ThrowConstructible>::Make()),
            "Make should not be noexcept for throwing constructible types");

        // 运行时验证可以成功创建
        auto ptr1 = RefPtr<NoThrowConstructible>::Make();
        auto ptr2 = RefPtr<ThrowConstructible>::Make();

        Assert::IsNotNull(ptr1.Get());
        Assert::IsNotNull(ptr2.Get());
    }
};
TS_NS_END