#include <algorithm>
#include <iostream>
#include <list>
#include <tbb/parallel_sort.h>
#include "SortTest.h"

template <typename BDIter>
inline void permutationSort(BDIter first, BDIter last)
{
    if (first == last) {
        return;
    }
    for (BDIter curr(first); ++curr != last;) {
        BDIter right(curr);
        BDIter left(curr);
        while (left != first && *right < *--left) {
            std::swap(*left, *right--);
        }
    }
}

inline unsigned quickRandom()
{
    static unsigned seed = 0;
    seed = 214013 * seed + 2531011;
    return seed;
}

template <typename RAIter>
inline RAIter quickSplit(RAIter first, RAIter last)
{
    typedef typename std::iterator_traits<RAIter>::value_type ValueType;
    std::swap(*first, *(first + quickRandom() % (last - first)));
    ValueType &pivot = *first++;

    RAIter lessBound(first);
    RAIter greaterBound(first);
    while (greaterBound != last) {
        if (*greaterBound < pivot) {
            std::swap(*lessBound, *greaterBound);
            ++lessBound;
        }
        ++greaterBound;
    }
    std::swap(pivot, *--lessBound);
    return lessBound;
}

template <typename RAIter>
void quickSort(RAIter first, RAIter last)
{
    typedef typename std::iterator_traits<RAIter>::value_type ValueType;
    static long const cutOff = std::max(size_t(8), size_t(256) / sizeof(ValueType));

    if (last - first < cutOff) {
        permutationSort(first, last);
        return;
    }
    RAIter border = quickSplit(first, last);
    quickSort(first, border);
    quickSort(++border, last);
}

template <typename RAIter>
class QuickSortTask : public tbb::task
{
public:
    QuickSortTask(RAIter first, RAIter last)
        : first_(first)
        , last_(last)
    {}

    virtual tbb::task *execute();

    static long const cutOff = 2000;

private:
    RAIter first_;
    RAIter last_;
};

template <typename RAIter>
tbb::task *QuickSortTask<RAIter>::execute()
{
    if (last_ - first_ < cutOff) {
        quickSort(first_, last_);
        return nullptr;
    }
    RAIter border = quickSplit(first_, last_);
    QuickSortTask<RAIter> &a = *new(allocate_child()) QuickSortTask<RAIter>(first_, border);
    QuickSortTask<RAIter> &b = *new(allocate_child()) QuickSortTask<RAIter>(++border, last_);
    set_ref_count(3);
    spawn(a);
    spawn_and_wait_for_all(b);
    return nullptr;
}

template <typename RAIter>
void parallelQuickSort(RAIter first, RAIter last)
{
    QuickSortTask<RAIter> &root = *new(tbb::task::allocate_root()) QuickSortTask<RAIter>(first, last);
    tbb::task::spawn_root_and_wait(root);
}


SORT_TEST_CHECK_CORRECTNESS(permutationSort)

TEST_F(SortTest, correctness_for_lists_permutationSort)
{
    std::list<SortTest::ElemType> smallRandomList(mRandomSmall.begin(), mRandomSmall.end());
    permutationSort(smallRandomList.begin(), smallRandomList.end());
    EXPECT_TRUE(is_sorted(smallRandomList.begin(), smallRandomList.end()));
}

TEST_F(SortTest, correctness_quickSplit)
{
    SortTest::Iter border = quickSplit(mSortedBig.begin(), mSortedBig.end());
    EXPECT_TRUE(mSortedBig.begin() == border
            || *max_element(mSortedBig.begin(), border) < *min_element(border, mSortedBig.end()));

    border = quickSplit(mBackSortedBig.begin(), mBackSortedBig.end());
    EXPECT_TRUE(*max_element(mBackSortedBig.begin(), border) < *min_element(border, mBackSortedBig.end()));

    border = quickSplit(mRandomBig.begin(), mRandomBig.end());
    EXPECT_TRUE(*max_element(mRandomBig.begin(), border) < *min_element(border, mRandomBig.end()));
}

SORT_TEST_CHECK_CORRECTNESS(quickSort)
SORT_TEST_CHECK_CORRECTNESS(parallelQuickSort)

SORT_TEST_CHECK_SPEED(quickSort)
SORT_TEST_CHECK_SPEED(sort)
SORT_TEST_CHECK_SPEED(parallelQuickSort)

using tbb::parallel_sort;
SORT_TEST_CHECK_SPEED(parallel_sort)


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
