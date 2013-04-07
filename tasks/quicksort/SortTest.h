#pragma once
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <vector>
#include <gtest/gtest.h>
#include <tbb/tick_count.h>

class SortTest : public ::testing::Test
{
protected:
    typedef int ElemType;
    typedef std::vector<ElemType> Vector;
    typedef Vector::iterator Iter;

    static size_t const smallSize = 10*1000;
    static size_t const bigSize = 5*1000*1000;

    virtual void SetUp()
    {
        mSortedBig = mSortedPattern;
        mBackSortedBig = mBackSortedPattern;
        mRandomBig = mRandomPattern;

        mSortedSmall = Vector(mSortedPattern.begin(), mSortedPattern.begin() + smallSize);
        mBackSortedSmall = Vector(mBackSortedPattern.begin(), mBackSortedPattern.begin() + smallSize);
        mRandomSmall = Vector(mRandomPattern.begin(), mRandomPattern.begin() + smallSize);
    }

    static void SetUpTestCase()
    {
        srand(time(NULL));
        for (size_t i = 0; i < bigSize; ++i) {
            mSortedPattern[i] = i;
            mBackSortedPattern[bigSize - 1 - i] = i;
            mRandomPattern[i] = rand();
        }
    }

    Vector mSortedBig;
    Vector mBackSortedBig;
    Vector mRandomBig;

    Vector mSortedSmall;
    Vector mBackSortedSmall;
    Vector mRandomSmall;

private:
    static Vector mSortedPattern;
    static Vector mBackSortedPattern;
    static Vector mRandomPattern;
};

SortTest::Vector SortTest::mSortedPattern(SortTest::bigSize);
SortTest::Vector SortTest::mBackSortedPattern(SortTest::bigSize);
SortTest::Vector SortTest::mRandomPattern(SortTest::bigSize);

inline void printTime(tbb::tick_count &startTime, char const *testName, const char *dataName)
{
    printf("Elapsed time for test %s | %11s: %.1lf ms\n", testName, dataName
            , 1000.0 * (tbb::tick_count::now() - startTime).seconds());
    startTime = tbb::tick_count::now();
}

inline void printTimeSilent(tbb::tick_count &startTime, char const *testName, const char *dataName)
{
    tbb::tick_count stopTime = tbb::tick_count::now();
    printf("                      %*s | %11s: %.1lf ms\n", (int)strlen(testName), "", dataName
            , 1000.0 * (stopTime - startTime).seconds());
    startTime = tbb::tick_count::now();
}

#define SORT_TEST_WITH(testName, functionName, sizeSuffix) \
    TEST_F(SortTest, testName) { \
        tbb::tick_count startTime = tbb::tick_count::now(); \
        \
        functionName(mSorted##sizeSuffix.begin(), mSorted##sizeSuffix.end()); \
        printTime(startTime, #testName, "sorted"); \
        \
        functionName(mBackSorted##sizeSuffix.begin(), mBackSorted##sizeSuffix.end()); \
        printTimeSilent(startTime, #testName, "back sorted"); \
        \
        functionName(mRandom##sizeSuffix.begin(), mRandom##sizeSuffix.end()); \
        printTimeSilent(startTime, #testName, "random"); \
        \
        EXPECT_TRUE(is_sorted(mSorted##sizeSuffix.begin(), mSorted##sizeSuffix.end())); \
        EXPECT_TRUE(is_sorted(mBackSorted##sizeSuffix.begin(), mBackSorted##sizeSuffix.end())); \
        EXPECT_TRUE(is_sorted(mRandom##sizeSuffix.begin(), mRandom##sizeSuffix.end())); \
    }

#define SORT_TEST_CHECK_CORRECTNESS(functionName) \
    SORT_TEST_WITH(correctness_##functionName, functionName, Small)

#define SORT_TEST_CHECK_SPEED(functionName) \
    SORT_TEST_WITH(speed_##functionName, functionName, Big)
