#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>

#include <boost/lexical_cast.hpp>

#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#include <tbb/tick_count.h>
#include <tbb/mutex.h>

#include <gtest/gtest.h>

size_t const testSize = 100*1000*1000;
size_t const grainSize = 100;
size_t const numofRuns = 2;
bool const logEnabled = false;

typedef int ElemType;
typedef std::basic_string<ElemType> String;
typedef tbb::mutex LoggingMutexType;

class NumbersConcatenator
{
public:
    NumbersConcatenator() {}
    NumbersConcatenator(NumbersConcatenator const &src, tbb::split) {}
    String const &result() const { return mResult; }

    void operator ()(tbb::blocked_range<ElemType> const &range)
    {
        logCall(range);

        size_t oldSize = mResult.size();
        mResult.resize(oldSize + range.size());

        ElemType value = range.begin();
        for (size_t i = oldSize; i < mResult.size(); ++i) {
            mResult[i] = value;
            ++value;
        }
    }

    void join(NumbersConcatenator const &other)
    {
        logJoin(other);

        mResult.append(other.mResult);
    }

    void logCall(tbb::blocked_range<ElemType> const &range) const;
    void logJoin(NumbersConcatenator const &other) const;

    void clear()
    {
        String().swap(mResult);
    }

private:
    String mResult;

    static LoggingMutexType mLoggingMutex;
};

LoggingMutexType NumbersConcatenator::mLoggingMutex;

void NumbersConcatenator::logCall(tbb::blocked_range<ElemType> const &range) const
{
    if (!logEnabled) {
        return;
    }
    LoggingMutexType::scoped_lock lock(mLoggingMutex);

    std::clog
        << "call operator (): this->mResult.size() == " << std::left << std::setw(10) << mResult.size()
        << " this->mResult.back() " << std::left << std::setw(13) << (mResult.size() == 0
                ? std::string("undefined")
                : "== " + boost::lexical_cast<std::string>(mResult.back()))
        << " range (size: " << std::left << std::setw(10) << range.size() << ") = ["
        << range.begin() << ", " << range.end() << ")"
        << std::endl;
}

void NumbersConcatenator::logJoin(NumbersConcatenator const &other) const
{
    if (!logEnabled) {
        return;
    }
    LoggingMutexType::scoped_lock lock(mLoggingMutex);

    std::clog
        << "call join: this->mResult.size() == " << std::left << std::setw(10) << mResult.size()
        << " this->mResult.back() " << std::left << std::setw(13) << (mResult.size() == 0
                ? std::string("undefined")
                : " == " + boost::lexical_cast<std::string>(mResult.back()))
        << " other.mResult.size() == " << std::left << std::setw(10) << other.mResult.size()
        << " other.mResult.front() " << std::left << std::setw(13) << (other.mResult.size() == 0
                ? std::string("undefined")
                : " == " + boost::lexical_cast<std::string>(other.mResult.front()))
        << std::endl;
}

TEST(NumbersConcatenatorTest, ParallelReduceCommutativity)
{
    ElemType const first(0);
    ElemType const last(testSize);

    NumbersConcatenator concatenator;
    tbb::tick_count startTime = tbb::tick_count::now();

    for (size_t i = 0; i < numofRuns; ++i) {
        concatenator.clear();
        tbb::parallel_reduce(tbb::blocked_range<ElemType>(first, last, grainSize), concatenator);
    }

    double elapsedTime = (tbb::tick_count::now() - startTime).seconds();
    String const &result = concatenator.result();

    EXPECT_EQ(testSize, result.size());
    EXPECT_TRUE(is_sorted(begin(result), end(result)));
    EXPECT_EQ(first, result.front());
    EXPECT_EQ(last - 1, result.back());

    std::cout << "Elapsed time: " << elapsedTime << " seconds" << std::endl;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
