#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>
#include <tbb/task.h>
#include <tbb/tbb_thread.h>
#include <gtest/gtest.h>

template <typename RAIter>
class PartialSumTask : public tbb::task
{
public:
    typedef typename std::iterator_traits<RAIter>::value_type ValueType;

    PartialSumTask(RAIter first, RAIter last, ValueType *sum)
        : first_(first)
        , last_(last)
        , sum_(sum)
    {}

    virtual tbb::task *execute();

private:
    RAIter first_;
    RAIter last_;
    ValueType *sum_;
};

template <typename RAIter>
class IncreaseByTask : public tbb::task
{
public:
    typedef typename std::iterator_traits<RAIter>::value_type ValueType;

    IncreaseByTask(RAIter first, RAIter last, ValueType term)
        : first_(first)
        , last_(last)
        , term_(term)
    {}

    virtual tbb::task *execute();

private:
    RAIter first_;
    RAIter last_;
    ValueType term_;
};

template <typename RAIter>
class PartialSumRootTask : public tbb::task
{
public:
    typedef typename std::iterator_traits<RAIter>::value_type ValueType;

    PartialSumRootTask(RAIter first, RAIter last)
        : first_(first)
        , last_(last)
        , numofTasks_(std::max(unsigned(2), tbb::tbb_thread::hardware_concurrency()))
    {}

    virtual tbb::task *execute();

private:
    RAIter first_;
    RAIter last_;
    unsigned const numofTasks_;
};

template <typename RAIter>
tbb::task *PartialSumTask<RAIter>::execute()
{
    partial_sum(first_, last_, first_);
    *sum_ = *(last_ - 1);
    return NULL;
}

template <typename RAIter>
tbb::task *IncreaseByTask<RAIter>::execute()
{
    for (RAIter it = first_; it != last_; ++it) {
        *it += term_;
    }
    return NULL;
}

template <typename RAIter>
tbb::task *PartialSumRootTask<RAIter>::execute()
{
    set_ref_count(numofTasks_ + 1);
    size_t taskSize = (last_ - first_) / numofTasks_;

    std::vector<ValueType> taskSums(numofTasks_);
    for (size_t i = 1; i < numofTasks_; ++i) {
        PartialSumTask<RAIter> &task = *new(allocate_child())
                PartialSumTask<RAIter>(first_ + (i - 1) * taskSize, first_ + i * taskSize, &taskSums[i - 1]);
        spawn(task);
    }
    spawn_and_wait_for_all(*new(allocate_child())
            PartialSumTask<RAIter>(first_ + (numofTasks_ - 1) * taskSize, last_, &taskSums[numofTasks_ - 1]));

    set_ref_count(numofTasks_);
    partial_sum(taskSums.begin(), taskSums.end(), taskSums.begin());

    for (size_t i = 2; i < numofTasks_; ++i) {
        IncreaseByTask<RAIter> &task = *new(allocate_child())
                IncreaseByTask<RAIter>(first_ + (i - 1) * taskSize, first_ + i * taskSize, taskSums[i - 2]);
        spawn(task);
    }
    spawn_and_wait_for_all(*new(allocate_child())
            IncreaseByTask<RAIter>(first_ + (numofTasks_ - 1) * taskSize, last_, taskSums[numofTasks_ - 2]));

    return NULL;
}

template <typename RAIter>
void parallelPartialSum(RAIter first, RAIter last)
{
    long const cutOff = 20*1000;

    if (last - first < cutOff) {
        partial_sum(first, last, first);
        return;
    }

    PartialSumRootTask<RAIter> &root = *new(tbb::task::allocate_root()) PartialSumRootTask<RAIter>(first, last);
    tbb::task::spawn_root_and_wait(root);
}

TEST(PartialSumTest, correctness_stdInPlace)
{
    size_t const testSize = 1000*1000;
    std::vector<int> testVector(testSize);
    for (size_t i = 0; i < testSize; ++i) {
        testVector[i] = rand() % (std::numeric_limits<int>::max() / testSize);
    }
    std::vector<int> correctPartialSum(testSize);
    partial_sum(testVector.begin(), testVector.end(), correctPartialSum.begin());

    partial_sum(testVector.begin(), testVector.end(), testVector.begin());
    EXPECT_EQ(correctPartialSum, testVector);
}

TEST(PartialSumTest, correctness_parallelPartialSum)
{
    size_t const testSize = 1000*1000;
    std::vector<int> testVector(testSize);
    for (size_t i = 0; i < testSize; ++i) {
        testVector[i] = rand() % (std::numeric_limits<int>::max() / testSize);
    }
    std::vector<int> correctPartialSum(testSize);
    partial_sum(testVector.begin(), testVector.end(), correctPartialSum.begin());

    parallelPartialSum(testVector.begin(), testVector.end());
    EXPECT_EQ(correctPartialSum, testVector);
}

TEST(PartialSumTest, speed_parallelPartialSum)
{
    size_t const testSize = 100*1000*1000;
    std::vector<int> testVector(testSize, 1);

    tbb::tick_count startTime = tbb::tick_count::now();
    parallelPartialSum(testVector.begin(), testVector.end());
    printf("Elapsed time: %.0lf ms\n", 1000.0 * (tbb::tick_count::now() - startTime).seconds());

    EXPECT_EQ(static_cast<int>(testSize), testVector.back());
}

TEST(PartialSumTest, speed_std)
{
    size_t const testSize = 100*1000*1000;
    std::vector<int> testVector(testSize, 1);

    tbb::tick_count startTime = tbb::tick_count::now();
    partial_sum(testVector.begin(), testVector.end(), testVector.begin());
    printf("Elapsed time: %.0lf ms\n", 1000.0 * (tbb::tick_count::now() - startTime).seconds());

    EXPECT_EQ(static_cast<int>(testSize), testVector.back());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
