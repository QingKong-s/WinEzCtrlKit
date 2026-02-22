#include "pch.h"
#include "../eck/CEventChain.h"

using namespace eck;

TS_NS_BEGIN
class TempClass
{
public:
    int value = 0;
    std::vector<int> calls;

    void Method(int x)
    {
        value = x;
        calls.push_back(x);
    }

    void MethodConst(int x) const
    {
        ((TempClass*)this)->calls.push_back(x);
    }

    int MethodWithReturn(int x)
    {
        value = x;
        return x * 2;
    }

    void InterceptMethod(int x, SlotCtx& ctx)
    {
        value = x;
        if (x > 100)
            ctx.Processed(TRUE);
    }

    int InterceptMethodWithReturn(int x, SlotCtx& ctx)
    {
        value = x;
        if (x > 100)
        {
            ctx.Processed(TRUE);
            return x * 10;
        }
        return x * 2;
    }
};

// 全局测试函数
static int g_testValue = 0;
static std::vector<int> g_calls;

void TestFunc(int x)
{
    g_testValue = x;
    g_calls.push_back(x);
}

int TestFuncWithReturn(int x)
{
    g_testValue = x;
    return x * 3;
}

void InterceptFunc(int x, SlotCtx& ctx)
{
    g_testValue = x;
    if (x > 50)
        ctx.Processed(TRUE);
}

int InterceptFuncWithReturn(int x, SlotCtx& ctx)
{
    g_testValue = x;
    if (x > 50)
    {
        ctx.Processed(TRUE);
        return x * 5;
    }
    return x * 3;
}

TEST_CLASS(CSignalBasicTest)
{
public:
    TEST_METHOD_INITIALIZE(Setup)
    {
        g_testValue = 0;
        g_calls.clear();
    }

    // 测试基本的 void 信号发射
    TEST_METHOD(TsBasicEmit_Void)
    {
        CEventChain<NoIntercept_T, void, int> signal;

        int result = 0;
        signal.Connect([&](int x) { result = x; });

        signal.Emit(42);
        Assert::AreEqual(42, result);
    }

    // 测试带返回值的信号发射
    TEST_METHOD(TsBasicEmit_WithReturn)
    {
        CEventChain<Intercept_T, int, int> signal;

        signal.Connect([](int x, SlotCtx& Ctx)
            {
                Ctx.Processed(TRUE);
                return x * 2; });

        int result = signal.Emit(21);
        Assert::AreEqual(42, result);
    }

    // 测试空信号发射
    TEST_METHOD(TsEmit_EmptySignal)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        signal.Emit(42); // 不应崩溃
        Assert::IsTrue(true);
    }

    // 测试空信号返回默认值
    TEST_METHOD(TsEmit_EmptySignalWithReturn)
    {
        CEventChain<NoIntercept_T, int, int> signal;
        int result = signal.Emit(42);
        Assert::AreEqual(0, result); // 默认构造的 int
    }

    // 测试多个槽按顺序执行
    TEST_METHOD(TsMultipleSlots_ExecutionOrder)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        std::vector<int> order;

        signal.Connect([&](int x) { order.push_back(1); });
        signal.Connect([&](int x) { order.push_back(2); });
        signal.Connect([&](int x) { order.push_back(3); });

        signal.Emit(0);

        // 默认插入到最前，所以顺序是反的
        Assert::AreEqual(3, (int)order.size());
        Assert::AreEqual(3, order[0]);
        Assert::AreEqual(2, order[1]);
        Assert::AreEqual(1, order[2]);
    }

    // 测试连接全局函数
    TEST_METHOD(TsConnect_GlobalFunction)
    {
        CEventChain<NoIntercept_T, void, int> signal;

        signal.Connect(TestFunc);
        signal.Emit(100);

        Assert::AreEqual(100, g_testValue);
        Assert::AreEqual(1, (int)g_calls.size());
    }

    // 测试连接成员函数
    TEST_METHOD(TsConnect_MemberFunction)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        TempClass obj;

        signal.Connect(&obj, &TempClass::Method);
        signal.Emit(50);

        Assert::AreEqual(50, obj.value);
        Assert::AreEqual(1, (int)obj.calls.size());
    }

    // 测试连接 const 成员函数
    TEST_METHOD(TsConnect_ConstMemberFunction)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        TempClass obj;

        signal.Connect(&obj, &TempClass::MethodConst);
        signal.Emit(60);

        Assert::AreEqual(1, (int)obj.calls.size());
        Assert::AreEqual(60, obj.calls[0]);
    }

    // 测试连接 Lambda
    TEST_METHOD(TsConnect_Lambda)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        int captured = 0;

        signal.Connect([&captured](int x) { captured = x * 2; });
        signal.Emit(15);

        Assert::AreEqual(30, captured);
    }

    // 测试连接多个不同类型的槽
    TEST_METHOD(TsConnect_MixedTypes)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        TempClass obj;
        int lambda_result = 0;

        signal.Connect(TestFunc);
        signal.Connect(&obj, &TempClass::Method);
        signal.Connect([&lambda_result](int x) { lambda_result = x; });

        signal.Emit(77);

        Assert::AreEqual(77, g_testValue);
        Assert::AreEqual(77, obj.value);
        Assert::AreEqual(77, lambda_result);
    }

    // 测试插入到顶部（默认行为）
    TEST_METHOD(TsInsertPosition_Top)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        std::vector<int> order;

        auto h1 = signal.Connect([&](int x) { order.push_back(1); }, 0, CEventChain<NoIntercept_T, void, int>::SlotTop);
        auto h2 = signal.Connect([&](int x) { order.push_back(2); }, 0, CEventChain<NoIntercept_T, void, int>::SlotTop);
        auto h3 = signal.Connect([&](int x) { order.push_back(3); }, 0, CEventChain<NoIntercept_T, void, int>::SlotTop);

        signal.Emit(0);

        Assert::AreEqual(3, (int)order.size());
        Assert::AreEqual(3, order[0]);
        Assert::AreEqual(2, order[1]);
        Assert::AreEqual(1, order[2]);
    }

    // 测试插入到底部
    TEST_METHOD(TsInsertPosition_Bottom)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        std::vector<int> order;

        signal.Connect([&](int x) { order.push_back(1); }, 0, CEventChain<NoIntercept_T, void, int>::SlotBottom);
        signal.Connect([&](int x) { order.push_back(2); }, 0, CEventChain<NoIntercept_T, void, int>::SlotBottom);
        signal.Connect([&](int x) { order.push_back(3); }, 0, CEventChain<NoIntercept_T, void, int>::SlotBottom);

        signal.Emit(0);

        Assert::AreEqual(3, (int)order.size());
        Assert::AreEqual(1, order[0]);
        Assert::AreEqual(2, order[1]);
        Assert::AreEqual(3, order[2]);
    }

    // 测试插入到指定位置之后
    TEST_METHOD(TsInsertPosition_AfterSpecific)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        std::vector<int> order;

        auto h1 = signal.Connect([&](int x) { order.push_back(1); });
        auto h2 = signal.Connect([&](int x) { order.push_back(2); }, 0, h1); // 在 h1 之后
        auto h3 = signal.Connect([&](int x) { order.push_back(3); }, 0, h1); // 也在 h1 之后

        signal.Emit(0);

        Assert::AreEqual(3, (int)order.size());
        Assert::AreEqual(1, order[0]);
        Assert::AreEqual(3, order[1]); // 最后插入的
        Assert::AreEqual(2, order[2]); // 先插入的
    }

    // 测试混合插入位置
    TEST_METHOD(TsInsertPosition_Mixed)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        std::vector<int> order;

        auto h1 = signal.Connect([&](int x) { order.push_back(1); }, 0, CEventChain<NoIntercept_T, void, int>::SlotBottom);
        auto h2 = signal.Connect([&](int x) { order.push_back(2); }, 0, CEventChain<NoIntercept_T, void, int>::SlotTop);
        auto h3 = signal.Connect([&](int x) { order.push_back(3); }, 0, h1);
        auto h4 = signal.Connect([&](int x) { order.push_back(4); }, 0, CEventChain<NoIntercept_T, void, int>::SlotBottom);

        signal.Emit(0);

        Assert::AreEqual(4, (int)order.size());
        Assert::AreEqual(2, order[0]); // Top
        Assert::AreEqual(1, order[1]); // First Bottom
        Assert::AreEqual(3, order[2]); // After h1
        Assert::AreEqual(4, order[3]); // Last Bottom
    }

    // 测试基本拦截功能
    TEST_METHOD(TsIntercept_BasicVoid)
    {
        CEventChain<Intercept_T, void, int> signal;
        std::vector<int> order;

        signal.Connect([&](int x, SlotCtx& ctx) { order.push_back(3); });
        signal.Connect([&](int x, SlotCtx& ctx) { order.push_back(2); });
        signal.Connect([&](int x, SlotCtx& ctx) {
            order.push_back(1);
            if (x > 50)
                ctx.Processed(TRUE);
            });

        signal.Emit(100);

        // 第一个槽拦截了，后续不执行
        Assert::AreEqual(1, (int)order.size());
        Assert::AreEqual(1, order[0]);

        order.clear();
        signal.Emit(25);

        // 不满足拦截条件，都执行
        Assert::AreEqual(3, (int)order.size());
    }

    // 测试拦截并返回值
    TEST_METHOD(TsIntercept_WithReturn)
    {
        CEventChain<Intercept_T, int, int> signal;

        signal.Connect([](int x, SlotCtx& ctx) -> int {
            if (x > 50)
            {
                ctx.Processed(TRUE);
                return x * 10;
            }
            return x * 2;
            });
        signal.Connect([](int x, SlotCtx& ctx) -> int { return x * 5; });

        int result1 = signal.Emit(100);
        Assert::AreEqual(1000, result1); // 被拦截，返回 100 * 10

        int result2 = signal.Emit(10);
        Assert::AreEqual(0, result2); // 不拦截，返回默认构造
    }

    // 测试全局函数拦截
    TEST_METHOD(TsIntercept_GlobalFunction)
    {
        CEventChain<Intercept_T, void, int> signal;

        signal.Connect([](int x, SlotCtx& ctx) { g_calls.push_back(999); });
        signal.Connect(InterceptFunc);

        signal.Emit(100);
        Assert::AreEqual(100, g_testValue);
        Assert::AreEqual(0, (int)g_calls.size()); // 被拦截，第二个槽不执行

        signal.Emit(25);
        Assert::AreEqual(1, (int)g_calls.size()); // 不拦截，第二个槽执行
    }

    // 测试成员函数拦截
    TEST_METHOD(TsIntercept_MemberFunction)
    {
        CEventChain<Intercept_T, void, int> signal;
        TempClass obj;

        signal.Connect([](int x, SlotCtx& ctx) { g_calls.push_back(888); });
        signal.Connect(&obj, &TempClass::InterceptMethod);

        signal.Emit(200);
        Assert::AreEqual(200, obj.value);
        Assert::AreEqual(0, (int)g_calls.size());

        signal.Emit(50);
        Assert::AreEqual(1, (int)g_calls.size());
    }

    // 测试 CallNext 功能
    TEST_METHOD(TsIntercept_CallNext)
    {
        CEventChain<Intercept_T, int, int> signal;

        signal.Connect([](int x, SlotCtx& ctx) -> int {
            ctx.Processed(TRUE);
            return x + 100;
            });
        signal.Connect([&signal](int x, SlotCtx& ctx) -> int {
            if (x > 50)
            {
                // 修改参数并调用下一个槽
                return signal.CallNext(ctx, x * 2);
            }
            return x;
            });

        int result1 = signal.Emit(60);
        Assert::AreEqual(220, result1); // 60 * 2 + 100

        int result2 = signal.Emit(10);
        Assert::AreEqual(110, result2); // 10 (第一个槽直接返回) 但第二个槽还会执行
    }

    // 测试通过句柄断开连接
    TEST_METHOD(TsDisconnect_ByHandle)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        int count = 0;

        auto h1 = signal.Connect([&](int x) { count++; });
        auto h2 = signal.Connect([&](int x) { count++; });

        signal.Emit(0);
        Assert::AreEqual(2, count);

        signal.Disconnect(h1);
        count = 0;
        signal.Emit(0);
        Assert::AreEqual(1, count);
    }

    // 测试通过 ID 断开连接
    TEST_METHOD(TsDisconnect_ById)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        int count = 0;

        signal.Connect([&](int x) { count++; }, 100);
        signal.Connect([&](int x) { count++; }, 200);

        signal.Emit(0);
        Assert::AreEqual(2, count);

        BOOL result = signal.Disconnect(100);
        Assert::IsTrue(result);

        count = 0;
        signal.Emit(0);
        Assert::AreEqual(1, count);
    }

    // 测试断开不存在的 ID
    TEST_METHOD(TsDisconnect_NonExistentId)
    {
        CEventChain<NoIntercept_T, void, int> signal;

        signal.Connect([](int x) {}, 100);

        BOOL result = signal.Disconnect(999);
        Assert::IsFalse(result);
    }

    // 测试 Clear 方法
    TEST_METHOD(TsClear)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        int count = 0;

        signal.Connect([&](int x) { count++; });
        signal.Connect([&](int x) { count++; });
        signal.Connect([&](int x) { count++; });

        signal.Emit(0);
        Assert::AreEqual(3, count);

        signal.Clear();
        count = 0;
        signal.Emit(0);
        Assert::AreEqual(0, count);
    }

    // 测试立即清理
    TEST_METHOD(TsCleanupNow)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        int count = 0;

        auto h1 = signal.Connect([&](int x) { count++; });
        auto h2 = signal.Connect([&](int x) { count++; });

        signal.Disconnect(h1);
        // 此时 h1 被标记删除但可能未真正删除

        signal.CleanupNow();
        // 现在应该被真正删除了

        count = 0;
        signal.Emit(0);
        Assert::AreEqual(1, count);
    }

    // 测试查找槽
    TEST_METHOD(TsFindSlot_Existing)
    {
        CEventChain<NoIntercept_T, void, int> signal;

        auto h1 = signal.Connect([](int x) {}, 100);
        auto h2 = signal.Connect([](int x) {}, 200);

        auto found = signal.FindSlot(100);
        Assert::IsNotNull((void*)found);
        Assert::AreEqual((void*)h1, (void*)found);
    }

    // 测试查找不存在的槽
    TEST_METHOD(TsFindSlot_NonExistent)
    {
        CEventChain<NoIntercept_T, void, int> signal;

        signal.Connect([](int x) {}, 100);

        auto found = signal.FindSlot(999);
        Assert::IsNull((void*)found);
    }

    // 测试从指定位置开始查找
    TEST_METHOD(TsFindSlot_FromPosition)
    {
        CEventChain<NoIntercept_T, void, int> signal;

        auto h1 = signal.Connect([](int x) {}, 102);
        auto h2 = signal.Connect([](int x) {}, 101);
        auto h3 = signal.Connect([](int x) {}, 100);

        auto found1 = signal.FindSlot(100);
        Assert::IsTrue(found1 == h3);

        auto found2 = signal.FindSlot(100, found1);
        Assert::IsNull((void*)found2);

        auto found3 = signal.FindSlot(101, found1);
        Assert::IsTrue(found3 == h2);
    }

    // 测试信号链接到另一个信号
    TEST_METHOD(TsSignalChain_Basic)
    {
        CEventChain<NoIntercept_T, void, int> signal1;
        CEventChain<NoIntercept_T, void, int> signal2;
        int count = 0;

        signal2.Connect([&](int x) { count = x; });
        signal1.Connect(signal2);

        signal1.Emit(42);
        Assert::AreEqual(42, count);
    }

    // 测试拦截模式下的信号链接
    TEST_METHOD(TsSignalChain_Intercept)
    {
        CEventChain<Intercept_T, void, int> signal1;
        CEventChain<Intercept_T, void, int> signal2;
        std::vector<int> order;

        signal2.Connect([&](int x, SlotCtx& ctx) { order.push_back(21); });
        signal2.Connect([&](int x, SlotCtx& ctx) {
            order.push_back(20);
            if (x > 50)
                ctx.Processed(TRUE);
            });

        signal1.Connect(signal2);
        signal1.Connect([&](int x, SlotCtx& ctx) { order.push_back(10); });

        signal1.Emit(100);

        // signal2 中的第一个槽会拦截
        Assert::AreEqual(2, (int)order.size());
        Assert::AreEqual(10, order[0]);
        Assert::AreEqual(20, order[1]);
    }

    // 测试多参数信号
    TEST_METHOD(TsMultipleParameters)
    {
        CEventChain<NoIntercept_T, void, int, double, std::string> signal;

        int i_result = 0;
        double d_result = 0.0;
        std::string s_result;

        signal.Connect([&](int i, double d, std::string s) {
            i_result = i;
            d_result = d;
            s_result = s;
            });

        signal.Emit(42, 3.14, "test");

        Assert::AreEqual(42, i_result);
        Assert::AreEqual(3.14, d_result, 0.001);
        Assert::AreEqual(std::string("test"), s_result);
    }

    // 测试多参数带返回值
    TEST_METHOD(TsMultipleParameters_WithReturn)
    {
        CEventChain<Intercept_T, std::string, int, std::string> signal;

        signal.Connect([](int i, std::string s, SlotCtx& ctx) {
            return "no2";
            });
        signal.Connect([](int i, std::string s, SlotCtx& ctx) {
            ctx.Processed(TRUE);
            return s + std::to_string(i);
            });
        signal.Connect([](int i, std::string s, SlotCtx& ctx) {
            return "no1";
            });

        std::string result = signal.Emit(123, "Number: ");
        Assert::AreEqual(std::string("Number: 123"), result);
    }

    // 测试在槽内断开连接
    TEST_METHOD(TsDisconnect_DuringEmit)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        int count = 0;

        CEventChain<NoIntercept_T, void, int>::HSlot h1, h2;

        h1 = signal.Connect([&](int x) {
            count++;
            signal.Disconnect(h2); // 在槽内断开另一个连接
            });
        h2 = signal.Connect([&](int x) { count++; });

        signal.Emit(0);

        // h2 被标记删除但这次仍可能执行
        // 行为取决于具体实现
        Assert::IsTrue(count >= 1);
    }

    // 测试在槽内连接新槽
    TEST_METHOD(TsConnect_DuringEmit)
    {
        CEventChain<NoIntercept_T, void, int> signal;
        std::vector<int> order;

        signal.Connect([&](int x) {
            order.push_back(1);
            signal.Connect([&](int x) { order.push_back(3); }); // 在槽内连接
            });
        signal.Connect([&](int x) { order.push_back(2); });

        signal.Emit(0);

        // 新连接的槽这次不会执行（因为遍历已经开始）
        Assert::AreEqual(2, (int)order.size());

        order.clear();
        signal.Emit(0);

        // 第二次发射时新槽会执行
        Assert::AreEqual(3, (int)order.size());
    }

    // 测试返回默认值
    TEST_METHOD(TsEmitWithDefault)
    {
        CEventChain<Intercept_T, int, int> signal;
        SlotCtx ctx;

        int result = signal.EmitWithDefault(ctx, 999, 42);
        Assert::AreEqual(999, result); // 空信号返回默认值

        signal.Connect([](int x, SlotCtx& ctx)
            {
                ctx.Processed(TRUE);
                return x * 2;
            });
        result = signal.EmitWithDefault(ctx, 999, 42);
        Assert::AreEqual(84, result); // 有槽时返回槽的结果
    }

    // 测试无参数信号
    TEST_METHOD(TsNoParameter)
    {
        CEventChain<NoIntercept_T, void> signal;
        int count = 0;

        signal.Connect([&]() { count++; });
        signal.Connect([&]() { count++; });

        signal.Emit();
        Assert::AreEqual(2, count);
    }
};
TS_NS_END