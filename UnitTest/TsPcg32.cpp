#include "pch.h"

#include "../eck/Random.h"

using eck::CPcg32;


TS_NS_BEGIN
TEST_CLASS(TsPcg32)
{
public:
    TEST_METHOD(TsSeedReproducible)
    {
        CPcg32 a(12345), b(12345);
        for (int i = 0; i < 1000; i++)
            Assert::AreEqual(a.Next<UINT>(), b.Next<UINT>());
    }

    TEST_METHOD(TsRandomNotConstant)
    {
        CPcg32 rng;
        UINT a = rng.Next<UINT>();
        UINT b = rng.Next<UINT>();
        Assert::AreNotEqual(a, b);
    }

    TEST_METHOD(TsIntRange)
    {
        CPcg32 rng(1);

        for (int i = 0; i < 20000; i++)
        {
            auto v = rng.Next<int>(-10, 10);
            Assert::IsTrue(v >= -10 && v <= 10);
        }
    }

    TEST_METHOD(TsUIntRange)
    {
        CPcg32 rng(2);

        for (int i = 0; i < 20000; i++)
        {
            auto v = rng.Next<UINT>(100, 200);
            Assert::IsTrue(v >= 100 && v <= 200);
        }
    }

    TEST_METHOD(TsFloatRange)
    {
        CPcg32 rng(3);

        for (int i = 0; i < 20000; i++)
        {
            double v = rng.Next<double>(-5.0, 5.0);
            Assert::IsTrue(v >= -5 && v <= 5);
        }
    }

    TEST_METHOD(TsDistributionMeanRough)
    {
        CPcg32 rng(4);

        const int N = 500000;
        double sum = 0;

        for (int i = 0; i < N; i++)
            sum += rng.Next<double>(0.0, 1.0);

        double mean = sum / N;

        // 均值理论值 = 0.5，偏差约 ±0.01 为正常
        Assert::IsTrue(mean > 0.49 && mean < 0.51);
    }

    TEST_METHOD(TsTriangularMean)
    {
        CPcg32 rng(5);

        double min = 0, mode = 5, max = 10;

        const int N = 300000;
        double sum = 0;

        for (int i = 0; i < N; i++)
            sum += rng.Triangular<double>(min, mode, max);

        double mean = sum / N;
        double theoretical = (min + mode + max) / 3.0;

        Assert::IsTrue(fabs(mean - theoretical) < 0.1);
    }

    TEST_METHOD(TsGaussianMeanAndVariance)
    {
        CPcg32 rng(6);

        const int N = 300000;

        double sum = 0;
        double sum2 = 0;

        for (int i = 0; i < N; i++)
        {
            double x = rng.Gaussian<double>();
            sum += x;
            sum2 += x * x;
        }

        double mean = sum / N;
        double var = (sum2 / N) - mean * mean;

        Assert::IsTrue(fabs(mean) < 0.05);     // 期望 ~ 0
        Assert::IsTrue(fabs(var - 1.0) < 0.05); // 方差 ~ 1
    }
};
TS_NS_END