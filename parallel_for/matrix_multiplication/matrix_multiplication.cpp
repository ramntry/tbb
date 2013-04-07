#include <vector>
#include <cstdio>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <tbb/tick_count.h>
#include <gtest/gtest.h>

typedef int ValueType;
typedef std::vector<ValueType> Row;
typedef std::vector<Row> Matrix;

void seqMatrixMul(Matrix const &lhs, Matrix const &rhs, Matrix &res)
{
    size_t const numofRows = lhs.size();
    size_t const vectorSize = rhs.size();
    size_t const numofCols = rhs.front().size();
    for (size_t i = 0; i < numofRows; ++i) {
        for (size_t j = 0; j < numofCols; ++j) {
            ValueType acc = 0;
            for (size_t k = 0; k < vectorSize; ++k) {
                acc += lhs[i][k] + rhs[k][j];
            }
            res[i][j] = acc;
        }
    }
}

void parallelMatrixMul(Matrix const &lhs, Matrix const &rhs, Matrix &res)
{
    size_t const numofRows = lhs.size();
    size_t const vectorSize = rhs.size();
    size_t const numofCols = rhs.front().size();

    tbb::parallel_for(tbb::blocked_range2d<size_t>(0, numofRows, 0, numofCols)
        , [&](tbb::blocked_range2d<size_t> const &r)
        {
            for (size_t i = r.rows().begin(); i < r.rows().end(); ++i) {
                for (size_t j = r.cols().begin(); j < r.cols().end(); ++j) {
                    ValueType acc = 0;
                    for (size_t k = 0; k < vectorSize; ++k) {
                        acc += lhs[i][k] + rhs[k][j];
                    }
                    res[i][j] = acc;
                }
            }
        });
}

Matrix makeMatrix(size_t rows, size_t cols)
{
    size_t counter = 0;
    Matrix matrix(rows, Row(cols));
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            matrix[i][j] = ++counter % 17;
        }
    }
    return matrix;
}

TEST(ParallelMatrixMultiplication, correctness_parallelMatrixMul)
{
    Matrix m1 = makeMatrix(100, 20);
    Matrix m2 = makeMatrix(20, 50);
    Matrix seqRes(100, Row(50));
    Matrix parRes(100, Row(50));

    seqMatrixMul(m1, m2, seqRes);
    parallelMatrixMul(m1, m2, parRes);

    EXPECT_EQ(seqRes, parRes);
}

TEST(ParallelMatrixMultiplication, speed_parallelMatrixMul)
{
    Matrix m1 = makeMatrix(500, 500);
    Matrix m2 = makeMatrix(500, 500);
    Matrix parRes(500, Row(500));

    tbb::tick_count startTime = tbb::tick_count::now();
    parallelMatrixMul(m1, m2, parRes);
    printf("Elapsed time: %.0lf ms\n", 1000.0 * (tbb::tick_count::now() - startTime).seconds());
}

TEST(ParallelMatrixMultiplication, speed_seqMatrixMul)
{
    Matrix m1 = makeMatrix(500, 500);
    Matrix m2 = makeMatrix(500, 500);
    Matrix parRes(500, Row(500));

    tbb::tick_count startTime = tbb::tick_count::now();
    seqMatrixMul(m1, m2, parRes);
    printf("Elapsed time: %.0lf ms\n", 1000.0 * (tbb::tick_count::now() - startTime).seconds());
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

