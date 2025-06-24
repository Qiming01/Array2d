//
// Created by qiming on 25-6-24.
//
// test_array2d.cpp
#include "array2d.hpp"  // 假设头文件名为 array2d.hpp
#include <algorithm>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <list>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

using namespace qm;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;

// ================================
// 测试夹具类
// ================================

/**
 * @brief Array2d 基础测试夹具
 */
class Array2dTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试用的矩阵
        small_matrix_       = array2d<int>(2, 3);
        small_matrix_[0][0] = 1;
        small_matrix_[0][1] = 2;
        small_matrix_[0][2] = 3;
        small_matrix_[1][0] = 4;
        small_matrix_[1][1] = 5;
        small_matrix_[1][2] = 6;

        square_matrix_ = array2d<double>(3, 3);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                square_matrix_[i][j] = i * 3 + j + 1;
            }
        }
    }

    array2d<int>    small_matrix_;
    array2d<double> square_matrix_;
};

/**
 * @brief 类型化测试夹具，用于测试不同的数值类型
 */
template<typename T>
class Array2dTypedTest : public ::testing::Test {
protected:
    using ValueType = T;

    void SetUp() override {
        matrix_ = array2d<T>(3, 4);
        fill_matrix();
    }

private:
    void fill_matrix() {
        T value = T{1};
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                matrix_[i][j] = value++;
            }
        }
    }

protected:
    array2d<T> matrix_;
};

// 定义要测试的类型
using NumericTypes = ::testing::Types<int, float, double, long long>;
TYPED_TEST_SUITE(Array2dTypedTest, NumericTypes);

// ================================
// 构造函数测试
// ================================

TEST_F(Array2dTest, DefaultConstructor) {
    array2d<int> arr;

    EXPECT_EQ(arr.rows(), 0);
    EXPECT_EQ(arr.cols(), 0);
    EXPECT_EQ(arr.size(), 0);
    EXPECT_TRUE(arr.empty());
}

TEST_F(Array2dTest, SizeConstructor) {
    array2d<int> arr(5, 7);

    EXPECT_EQ(arr.rows(), 5);
    EXPECT_EQ(arr.cols(), 7);
    EXPECT_EQ(arr.size(), 35);
    EXPECT_FALSE(arr.empty());

    // 检查默认初始化
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 7; ++j) {
            EXPECT_EQ(arr[i][j], 0);
        }
    }
}

TEST_F(Array2dTest, SizeValueConstructor) {
    array2d<int> arr(3, 4, 42);

    EXPECT_EQ(arr.rows(), 3);
    EXPECT_EQ(arr.cols(), 4);
    EXPECT_EQ(arr.size(), 12);

    // 检查值初始化
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_EQ(arr[i][j], 42);
        }
    }
}

TEST_F(Array2dTest, InitializerListConstructor) {
    array2d<int> arr{{1, 2, 3}, {4, 5, 6}};

    EXPECT_EQ(arr.rows(), 2);
    EXPECT_EQ(arr.cols(), 3);
    EXPECT_EQ(arr.size(), 6);

    EXPECT_EQ(arr[0][0], 1);
    EXPECT_EQ(arr[0][1], 2);
    EXPECT_EQ(arr[0][2], 3);
    EXPECT_EQ(arr[1][0], 4);
    EXPECT_EQ(arr[1][1], 5);
    EXPECT_EQ(arr[1][2], 6);
}

TEST_F(Array2dTest, InitializerListConstructorEmpty) {
    array2d<int> arr{};

    EXPECT_EQ(arr.rows(), 0);
    EXPECT_EQ(arr.cols(), 0);
    EXPECT_TRUE(arr.empty());
}

TEST_F(Array2dTest, ContainerConstructor) {
    std::vector<int> data{1, 2, 3, 4, 5, 6};
    array2d<int>     arr(2, 3, data);

    EXPECT_EQ(arr.rows(), 2);
    EXPECT_EQ(arr.cols(), 3);
    EXPECT_EQ(arr.size(), 6);

    EXPECT_EQ(arr[0][0], 1);
    EXPECT_EQ(arr[0][1], 2);
    EXPECT_EQ(arr[0][2], 3);
    EXPECT_EQ(arr[1][0], 4);
    EXPECT_EQ(arr[1][1], 5);
    EXPECT_EQ(arr[1][2], 6);
}

TEST_F(Array2dTest, CopyConstructor) {
    array2d<int> original = small_matrix_;
    array2d<int> copy(original);

    EXPECT_EQ(copy.rows(), original.rows());
    EXPECT_EQ(copy.cols(), original.cols());

    for (int i = 0; i < original.rows(); ++i) {
        for (int j = 0; j < original.cols(); ++j) {
            EXPECT_EQ(copy[i][j], original[i][j]);
        }
    }

    // 修改原矩阵，确保拷贝独立
    original[0][0] = 999;
    EXPECT_NE(copy[0][0], 999);
}

TEST_F(Array2dTest, MoveConstructor) {
    array2d<int> original      = small_matrix_;
    const auto   original_data = original.data();
    const auto   rows          = original.rows();
    const auto   cols          = original.cols();

    array2d<int> moved(std::move(original));

    EXPECT_EQ(moved.rows(), rows);
    EXPECT_EQ(moved.cols(), cols);
    EXPECT_EQ(moved.data(), original_data);  // 验证数据被移动

    // 原对象应该处于有效但未指定状态
    EXPECT_TRUE(original.empty() || original.size() > 0);  // 两种状态都可能
}

// ================================
// 构造函数异常测试
// ================================

TEST_F(Array2dTest, ConstructorInvalidDimensions) {
    EXPECT_THROW(array2d<int>(-1, 5), std::invalid_argument);
    EXPECT_THROW(array2d<int>(5, -1), std::invalid_argument);
    EXPECT_THROW(array2d<int>(-1, -1), std::invalid_argument);
}

TEST_F(Array2dTest, InitializerListConstructorInconsistentRows) {
    EXPECT_THROW((array2d<int>{{1, 2, 3}, {4, 5}}), std::invalid_argument);
    EXPECT_THROW((array2d<int>{{1, 2}, {3, 4, 5}}), std::invalid_argument);
}

TEST_F(Array2dTest, ContainerConstructorSizeMismatch) {
    std::vector<int> data{1, 2, 3, 4, 5};
    EXPECT_THROW(array2d<int>(2, 3, data), std::invalid_argument);  // 需要6个元素，只有5个
}

// ================================
// 元素访问测试
// ================================

TEST_F(Array2dTest, OperatorBracket) {
    EXPECT_EQ(small_matrix_[0][0], 1);
    EXPECT_EQ(small_matrix_[0][1], 2);
    EXPECT_EQ(small_matrix_[1][2], 6);

    // 修改测试
    small_matrix_[1][1] = 99;
    EXPECT_EQ(small_matrix_[1][1], 99);
}

TEST_F(Array2dTest, OperatorBracketConst) {
    const auto &const_matrix = small_matrix_;

    EXPECT_EQ(const_matrix[0][0], 1);
    EXPECT_EQ(const_matrix[1][2], 6);
}

TEST_F(Array2dTest, OperatorParentheses) {
    EXPECT_EQ(small_matrix_(0, 0), 1);
    EXPECT_EQ(small_matrix_(0, 1), 2);
    EXPECT_EQ(small_matrix_(1, 2), 6);

    // 修改测试
    small_matrix_(1, 1) = 88;
    EXPECT_EQ(small_matrix_(1, 1), 88);
}

TEST_F(Array2dTest, AtMethod) {
    EXPECT_EQ(small_matrix_.at(0, 0), 1);
    EXPECT_EQ(small_matrix_.at(1, 2), 6);

    // 修改测试
    small_matrix_.at(1, 1) = 77;
    EXPECT_EQ(small_matrix_.at(1, 1), 77);
}

TEST_F(Array2dTest, AtMethodConst) {
    const auto &const_matrix = small_matrix_;

    EXPECT_EQ(const_matrix.at(0, 0), 1);
    EXPECT_EQ(const_matrix.at(1, 2), 6);
}

TEST_F(Array2dTest, AtMethodBoundsChecking) {
    EXPECT_THROW(small_matrix_.at(-1, 0), std::out_of_range);
    EXPECT_THROW(small_matrix_.at(0, -1), std::out_of_range);
    EXPECT_THROW(small_matrix_.at(2, 0), std::out_of_range);
    EXPECT_THROW(small_matrix_.at(0, 3), std::out_of_range);
    EXPECT_THROW(small_matrix_.at(2, 3), std::out_of_range);
}

// ================================
// 迭代器测试
// ================================

TEST_F(Array2dTest, IteratorBeginEnd) {
    std::vector<int> expected{1, 2, 3, 4, 5, 6};
    std::vector<int> actual(small_matrix_.begin(), small_matrix_.end());

    EXPECT_THAT(actual, ElementsAreArray(expected));
}

TEST_F(Array2dTest, ConstIterator) {
    const auto      &const_matrix = small_matrix_;
    std::vector<int> expected{1, 2, 3, 4, 5, 6};
    std::vector<int> actual(const_matrix.begin(), const_matrix.end());

    EXPECT_THAT(actual, ElementsAreArray(expected));
}

TEST_F(Array2dTest, ReverseIterator) {
    std::vector<int> expected{6, 5, 4, 3, 2, 1};
    std::vector<int> actual(small_matrix_.rbegin(), small_matrix_.rend());

    EXPECT_THAT(actual, ElementsAreArray(expected));
}

TEST_F(Array2dTest, RangeBasedFor) {
    std::vector<int> actual;
    for (const auto &elem: small_matrix_) {
        actual.push_back(elem);
    }

    std::vector<int> expected{1, 2, 3, 4, 5, 6};
    EXPECT_THAT(actual, ElementsAreArray(expected));
}

TEST_F(Array2dTest, RowRange) {
    auto             row0_range = small_matrix_.row_range(0);
    std::vector<int> row0_data(row0_range.begin(), row0_range.end());
    EXPECT_THAT(row0_data, ElementsAre(1, 2, 3));

    auto             row1_range = small_matrix_.row_range(1);
    std::vector<int> row1_data(row1_range.begin(), row1_range.end());
    EXPECT_THAT(row1_data, ElementsAre(4, 5, 6));
}

TEST_F(Array2dTest, RowRangeModification) {
    auto row_range = small_matrix_.row_range(0);
    for (auto &elem: row_range) {
        elem *= 10;
    }

    EXPECT_EQ(small_matrix_[0][0], 10);
    EXPECT_EQ(small_matrix_[0][1], 20);
    EXPECT_EQ(small_matrix_[0][2], 30);
    EXPECT_EQ(small_matrix_[1][0], 4);  // 未修改
}

// ================================
// span 操作测试
// ================================

TEST_F(Array2dTest, AsSpan) {
    auto span = small_matrix_.as_span();

    EXPECT_EQ(span.size(), 6);
    EXPECT_EQ(span.data(), small_matrix_.data());

    std::vector<int> expected{1, 2, 3, 4, 5, 6};
    std::vector<int> actual(span.begin(), span.end());
    EXPECT_THAT(actual, ElementsAreArray(expected));
}

TEST_F(Array2dTest, RowSpan) {
    auto row0_span = small_matrix_.row(0);
    auto row1_span = small_matrix_.row(1);

    EXPECT_EQ(row0_span.size(), 3);
    EXPECT_EQ(row1_span.size(), 3);

    std::vector<int> row0_data(row0_span.begin(), row0_span.end());
    std::vector<int> row1_data(row1_span.begin(), row1_span.end());

    EXPECT_THAT(row0_data, ElementsAre(1, 2, 3));
    EXPECT_THAT(row1_data, ElementsAre(4, 5, 6));
}

TEST_F(Array2dTest, ColExtraction) {
    auto col0 = small_matrix_.col(0);
    auto col1 = small_matrix_.col(1);
    auto col2 = small_matrix_.col(2);

    EXPECT_THAT(col0, ElementsAre(1, 4));
    EXPECT_THAT(col1, ElementsAre(2, 5));
    EXPECT_THAT(col2, ElementsAre(3, 6));
}

TEST_F(Array2dTest, SubmatrixRowMajor) {
    array2d<int> matrix(4, 4);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            matrix[i][j] = i * 4 + j + 1;
        }
    }

    // 获取完整行的子矩阵
    auto             subspan = matrix.submatrix_row_major(1, 0, 2, 4);
    std::vector<int> subdata(subspan.begin(), subspan.end());

    EXPECT_THAT(subdata, ElementsAre(5, 6, 7, 8, 9, 10, 11, 12));

    // 获取部分列的子矩阵（只返回第一行）
    auto             partial_subspan = matrix.submatrix_row_major(1, 1, 2, 2);
    std::vector<int> partial_data(partial_subspan.begin(), partial_subspan.end());

    EXPECT_THAT(partial_data, ElementsAre(6, 7));
}

// ================================
// 尺寸和容量测试
// ================================

TEST_F(Array2dTest, SizeQueries) {
    EXPECT_EQ(small_matrix_.rows(), 2);
    EXPECT_EQ(small_matrix_.cols(), 3);
    EXPECT_EQ(small_matrix_.size(), 6);
    EXPECT_FALSE(small_matrix_.empty());
    EXPECT_TRUE(small_matrix_.capacity() >= 6);
    EXPECT_FALSE(small_matrix_.is_square());

    EXPECT_TRUE(square_matrix_.is_square());
}

TEST_F(Array2dTest, Reserve) {
    array2d<int> matrix(2, 2);
    const auto   initial_capacity = matrix.capacity();

    matrix.reserve(10, 10);
    EXPECT_GE(matrix.capacity(), 100);
    EXPECT_GE(matrix.capacity(), initial_capacity);

    // 尺寸不应改变
    EXPECT_EQ(matrix.rows(), 2);
    EXPECT_EQ(matrix.cols(), 2);
}

TEST_F(Array2dTest, ShrinkToFit) {
    array2d<int> matrix(2, 2);
    matrix.reserve(10, 10);

    const auto before_shrink = matrix.capacity();
    matrix.shrink_to_fit();
    const auto after_shrink = matrix.capacity();

    EXPECT_LE(after_shrink, before_shrink);
    EXPECT_GE(after_shrink, matrix.size());
}

// ================================
// 数据操作测试
// ================================

TEST_F(Array2dTest, Reset) {
    small_matrix_.reset();

    for (int i = 0; i < small_matrix_.rows(); ++i) {
        for (int j = 0; j < small_matrix_.cols(); ++j) {
            EXPECT_EQ(small_matrix_[i][j], 0);
        }
    }
}

TEST_F(Array2dTest, ResetWithOptions) {
    array2d<unsigned char> matrix(2, 3, 100);

    matrix.reset(Array_reset_opt::All_bits1);

    for (int i = 0; i < matrix.rows(); ++i) {
        for (int j = 0; j < matrix.cols(); ++j) {
            EXPECT_EQ(matrix[i][j], 255);  // 所有位为1
        }
    }
}

TEST_F(Array2dTest, Fill) {
    small_matrix_.fill(42);

    for (int i = 0; i < small_matrix_.rows(); ++i) {
        for (int j = 0; j < small_matrix_.cols(); ++j) {
            EXPECT_EQ(small_matrix_[i][j], 42);
        }
    }
}

TEST_F(Array2dTest, FillParallel) {
    array2d<int> large_matrix(100, 100);
    large_matrix.fill_parallel(123);

    for (int i = 0; i < large_matrix.rows(); ++i) {
        for (int j = 0; j < large_matrix.cols(); ++j) {
            EXPECT_EQ(large_matrix[i][j], 123);
        }
    }
}

// ================================
// 行操作测试
// ================================

TEST_F(Array2dTest, CopyRow) {
    small_matrix_.copy_row(0, 1);

    EXPECT_EQ(small_matrix_[1][0], 1);
    EXPECT_EQ(small_matrix_[1][1], 2);
    EXPECT_EQ(small_matrix_[1][2], 3);

    // 原行应该保持不变
    EXPECT_EQ(small_matrix_[0][0], 1);
    EXPECT_EQ(small_matrix_[0][1], 2);
    EXPECT_EQ(small_matrix_[0][2], 3);
}

TEST_F(Array2dTest, CopyRowSame) {
    auto original = small_matrix_;
    small_matrix_.copy_row(0, 0);  // 复制到自己

    // 应该没有变化
    for (int i = 0; i < small_matrix_.rows(); ++i) {
        for (int j = 0; j < small_matrix_.cols(); ++j) {
            EXPECT_EQ(small_matrix_[i][j], original[i][j]);
        }
    }
}

TEST_F(Array2dTest, SwapRows) {
    auto row0_original = small_matrix_.row(0);
    auto row1_original = small_matrix_.row(1);

    std::vector<int> row0_data(row0_original.begin(), row0_original.end());
    std::vector<int> row1_data(row1_original.begin(), row1_original.end());

    small_matrix_.swap_rows(0, 1);

    // 检查行已交换
    auto row0_after = small_matrix_.row(0);
    auto row1_after = small_matrix_.row(1);

    std::vector<int> row0_after_data(row0_after.begin(), row0_after.end());
    std::vector<int> row1_after_data(row1_after.begin(), row1_after.end());

    EXPECT_THAT(row0_after_data, ElementsAreArray(row1_data));
    EXPECT_THAT(row1_after_data, ElementsAreArray(row0_data));
}

TEST_F(Array2dTest, SwapRowsSame) {
    auto original = small_matrix_;
    small_matrix_.swap_rows(0, 0);  // 与自己交换

    // 应该没有变化
    for (int i = 0; i < small_matrix_.rows(); ++i) {
        for (int j = 0; j < small_matrix_.cols(); ++j) {
            EXPECT_EQ(small_matrix_[i][j], original[i][j]);
        }
    }
}

TEST_F(Array2dTest, FillRow) {
    small_matrix_.fill_row(0, 99);

    EXPECT_EQ(small_matrix_[0][0], 99);
    EXPECT_EQ(small_matrix_[0][1], 99);
    EXPECT_EQ(small_matrix_[0][2], 99);

    // 其他行应该保持不变
    EXPECT_EQ(small_matrix_[1][0], 4);
    EXPECT_EQ(small_matrix_[1][1], 5);
    EXPECT_EQ(small_matrix_[1][2], 6);
}

// ================================
// 转置测试
// ================================

TEST_F(Array2dTest, TransposeSquare) {
    square_matrix_.transpose();

    // 验证转置结果
    EXPECT_EQ(square_matrix_[0][0], 1);
    EXPECT_EQ(square_matrix_[0][1], 4);
    EXPECT_EQ(square_matrix_[0][2], 7);
    EXPECT_EQ(square_matrix_[1][0], 2);
    EXPECT_EQ(square_matrix_[1][1], 5);
    EXPECT_EQ(square_matrix_[1][2], 8);
    EXPECT_EQ(square_matrix_[2][0], 3);
    EXPECT_EQ(square_matrix_[2][1], 6);
    EXPECT_EQ(square_matrix_[2][2], 9);
}

TEST_F(Array2dTest, TransposeNonSquare) {
    EXPECT_THROW(small_matrix_.transpose(), std::invalid_argument);
}

TEST_F(Array2dTest, Transposed) {
    auto transposed = small_matrix_.transposed();

    EXPECT_EQ(transposed.rows(), 3);
    EXPECT_EQ(transposed.cols(), 2);

    EXPECT_EQ(transposed[0][0], 1);
    EXPECT_EQ(transposed[0][1], 4);
    EXPECT_EQ(transposed[1][0], 2);
    EXPECT_EQ(transposed[1][1], 5);
    EXPECT_EQ(transposed[2][0], 3);
    EXPECT_EQ(transposed[2][1], 6);

    // 原矩阵应该保持不变
    EXPECT_EQ(small_matrix_[0][0], 1);
    EXPECT_EQ(small_matrix_[0][1], 2);
    EXPECT_EQ(small_matrix_[0][2], 3);
    EXPECT_EQ(small_matrix_[1][0], 4);
    EXPECT_EQ(small_matrix_[1][1], 5);
    EXPECT_EQ(small_matrix_[1][2], 6);
}

TEST_F(Array2dTest, TransposedSquare) {
    auto transposed = square_matrix_.transposed();

    EXPECT_EQ(transposed.rows(), 3);
    EXPECT_EQ(transposed.cols(), 3);

    // 验证转置结果
    EXPECT_EQ(transposed[0][0], 1);
    EXPECT_EQ(transposed[0][1], 4);
    EXPECT_EQ(transposed[0][2], 7);
    EXPECT_EQ(transposed[1][0], 2);
    EXPECT_EQ(transposed[1][1], 5);
    EXPECT_EQ(transposed[1][2], 8);
    EXPECT_EQ(transposed[2][0], 3);
    EXPECT_EQ(transposed[2][1], 6);
    EXPECT_EQ(transposed[2][2], 9);
}

// ================================
// resize 测试
// ================================

TEST_F(Array2dTest, ResizeLarger) {
    small_matrix_.resize(3, 4);

    EXPECT_EQ(small_matrix_.rows(), 3);
    EXPECT_EQ(small_matrix_.cols(), 4);
    EXPECT_EQ(small_matrix_.size(), 12);

    // 原有数据应该保持
    EXPECT_EQ(small_matrix_[0][0], 1);
    EXPECT_EQ(small_matrix_[0][1], 2);
    EXPECT_EQ(small_matrix_[0][2], 3);
    EXPECT_EQ(small_matrix_[1][0], 4);
    EXPECT_EQ(small_matrix_[1][1], 5);
    EXPECT_EQ(small_matrix_[1][2], 6);

    // 新位置应该是默认初始化的
    EXPECT_EQ(small_matrix_[0][3], 0);
    EXPECT_EQ(small_matrix_[2][0], 0);
}

TEST_F(Array2dTest, ResizeSmaller) {
    small_matrix_.resize(1, 2);

    EXPECT_EQ(small_matrix_.rows(), 1);
    EXPECT_EQ(small_matrix_.cols(), 2);
    EXPECT_EQ(small_matrix_.size(), 2);

    // 保留的数据应该正确
    EXPECT_EQ(small_matrix_[0][0], 1);
    EXPECT_EQ(small_matrix_[0][1], 2);
}

TEST_F(Array2dTest, ResizeWithValue) {
    small_matrix_.resize(3, 4, 42);

    EXPECT_EQ(small_matrix_.rows(), 3);
    EXPECT_EQ(small_matrix_.cols(), 4);

    // 原有数据应该保持
    EXPECT_EQ(small_matrix_[0][0], 1);
    EXPECT_EQ(small_matrix_[0][1], 2);
    EXPECT_EQ(small_matrix_[0][2], 3);
    EXPECT_EQ(small_matrix_[1][0], 4);
    EXPECT_EQ(small_matrix_[1][1], 5);
    EXPECT_EQ(small_matrix_[1][2], 6);

    // 新位置应该使用指定值初始化
    EXPECT_EQ(small_matrix_[0][3], 42);
    EXPECT_EQ(small_matrix_[2][0], 42);
}

TEST_F(Array2dTest, ResizeToZero) {
    small_matrix_.resize(0, 0);

    EXPECT_EQ(small_matrix_.rows(), 0);
    EXPECT_EQ(small_matrix_.cols(), 0);
    EXPECT_EQ(small_matrix_.size(), 0);
    EXPECT_TRUE(small_matrix_.empty());
}

TEST_F(Array2dTest, ResizeSame) {
    auto original = small_matrix_;
    small_matrix_.resize(2, 3);

    EXPECT_EQ(small_matrix_.rows(), 2);
    EXPECT_EQ(small_matrix_.cols(), 3);

    // 数据应该保持不变
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            EXPECT_EQ(small_matrix_[i][j], original[i][j]);
        }
    }
}

// ================================
// 数据访问和工具方法测试
// ================================

TEST_F(Array2dTest, DataAccess) {
    auto       *data_ptr       = small_matrix_.data();
    const auto *const_data_ptr = static_cast<const decltype(small_matrix_) &>(small_matrix_).data();

    EXPECT_EQ(data_ptr, const_data_ptr);
    EXPECT_EQ(data_ptr[0], 1);
    EXPECT_EQ(data_ptr[1], 2);
    EXPECT_EQ(data_ptr[5], 6);
}

TEST_F(Array2dTest, Swap) {
    array2d<int> other(1, 4, 99);

    auto small_rows = small_matrix_.rows();
    auto small_cols = small_matrix_.cols();
    auto other_rows = other.rows();
    auto other_cols = other.cols();

    small_matrix_.swap(other);
    EXPECT_EQ(small_matrix_.rows(), other_rows);
    EXPECT_EQ(small_matrix_.cols(), other_cols);
    EXPECT_EQ(other.rows(), small_rows);
    EXPECT_EQ(other.cols(), small_cols);

    // 检查数据是否正确交换
    for (int i = 0; i < small_matrix_.rows(); ++i) {
        for (int j = 0; j < small_matrix_.cols(); ++j) {
            EXPECT_EQ(small_matrix_[i][j], 99);
        }
    }

    EXPECT_EQ(other[0][0], 1);
    EXPECT_EQ(other[0][1], 2);
    EXPECT_EQ(other[0][2], 3);
    EXPECT_EQ(other[1][0], 4);
    EXPECT_EQ(other[1][1], 5);
    EXPECT_EQ(other[1][2], 6);
}

TEST_F(Array2dTest, GetData) {
    const auto &data_vector = small_matrix_.get_data();

    EXPECT_EQ(data_vector.size(), 6);
    EXPECT_THAT(data_vector, ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST_F(Array2dTest, GetVector) {
    auto &data_vector = small_matrix_.get_vector();

    data_vector[0] = 999;
    EXPECT_EQ(small_matrix_[0][0], 999);
}

// ================================
// 比较操作符测试
// ================================

TEST_F(Array2dTest, EqualityOperator) {
    array2d<int> same{{1, 2, 3}, {4, 5, 6}};
    array2d<int> different{{1, 2, 3}, {4, 5, 7}};
    array2d<int> different_size{{1, 2}, {3, 4}};

    EXPECT_TRUE(small_matrix_ == same);
    EXPECT_FALSE(small_matrix_ == different);
    EXPECT_FALSE(small_matrix_ == different_size);

    EXPECT_FALSE(small_matrix_ != same);
    EXPECT_TRUE(small_matrix_ != different);
    EXPECT_TRUE(small_matrix_ != different_size);
}

TEST_F(Array2dTest, ThreeWayComparison) {
    array2d<int> smaller{{1, 2, 3}, {4, 5, 5}};
    array2d<int> same{{1, 2, 3}, {4, 5, 6}};
    array2d<int> larger{{1, 2, 3}, {4, 5, 7}};
    array2d<int> smaller_size{{1, 2}, {3, 4}};
    array2d<int> larger_size{{1, 2, 3, 4}, {5, 6, 7, 8}};

    EXPECT_TRUE(small_matrix_ < larger);
    EXPECT_TRUE(small_matrix_ > smaller);
    EXPECT_TRUE(small_matrix_ == same);
    EXPECT_TRUE(small_matrix_ > smaller_size);
    EXPECT_TRUE(small_matrix_ < larger_size);
}

// ================================
// 非成员函数测试
// ================================

TEST_F(Array2dTest, NonMemberSwap) {
    array2d<int> other(1, 4, 99);

    auto small_rows = small_matrix_.rows();
    auto small_cols = small_matrix_.cols();
    auto other_rows = other.rows();
    auto other_cols = other.cols();

    swap(small_matrix_, other);

    EXPECT_EQ(small_matrix_.rows(), other_rows);
    EXPECT_EQ(small_matrix_.cols(), other_cols);
    EXPECT_EQ(other.rows(), small_rows);
    EXPECT_EQ(other.cols(), small_cols);
}

// ================================
// 类模板参数推导测试
// ================================

TEST_F(Array2dTest, DeductionGuides) {
    // 从尺寸推导
    auto arr1 = array2d(3, 4);
    static_assert(std::is_same_v<decltype(arr1), array2d<double, int>>);

    // 从尺寸和值推导
    auto arr2 = array2d(3, 4, 42);
    static_assert(std::is_same_v<decltype(arr2), array2d<int, int>>);

    auto arr3 = array2d(3, 4, 3.14);
    static_assert(std::is_same_v<decltype(arr3), array2d<double, int>>);

    // 从初始化列表推导
    auto arr4 = array2d{{1, 2}, {3, 4}};
    static_assert(std::is_same_v<decltype(arr4), array2d<int>>);

    auto arr5 = array2d{{1.0, 2.0}, {3.0, 4.0}};
    static_assert(std::is_same_v<decltype(arr5), array2d<double>>);

    // 从容器推导(未实现)
    // std::vector<int> vec{1, 2, 3, 4};
    // auto             arr6 = array2d(2, 2, vec);
    // static_assert(std::is_same_v<decltype(arr6), array2d<int, int>>);
}

// ================================
// 类型化测试
// ================================

TYPED_TEST(Array2dTypedTest, BasicOperations) {
    using T = typename TestFixture::ValueType;

    EXPECT_EQ(this->matrix_.rows(), 3);
    EXPECT_EQ(this->matrix_.cols(), 4);
    EXPECT_EQ(this->matrix_.size(), 12);

    // 测试元素访问
    EXPECT_EQ(this->matrix_[0][0], T{1});
    EXPECT_EQ(this->matrix_[2][3], T{12});

    // 测试填充
    this->matrix_.fill(T{99});
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_EQ(this->matrix_[i][j], T{99});
        }
    }
}

TYPED_TEST(Array2dTypedTest, ArithmeticOperations) {
    using T = typename TestFixture::ValueType;

    // 测试算术运算
    for (int i = 0; i < this->matrix_.rows(); ++i) {
        for (int j = 0; j < this->matrix_.cols(); ++j) {
            auto &elem = this->matrix_[i][j];
            elem       = elem * T{2} + T{1};
        }
    }

    EXPECT_EQ(this->matrix_[0][0], T{3});   // (1 * 2) + 1
    EXPECT_EQ(this->matrix_[0][1], T{5});   // (2 * 2) + 1
    EXPECT_EQ(this->matrix_[2][3], T{25});  // (12 * 2) + 1
}

// ================================
// 性能测试
// ================================

class Array2dPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        large_matrix_ = array2d<double>(1000, 1000, 1.0);
    }

    array2d<double> large_matrix_;
};

TEST_F(Array2dPerformanceTest, FillPerformance) {
    // 测试fill性能
    auto start = std::chrono::high_resolution_clock::now();
    large_matrix_.fill(42.0);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Fill time: " << duration.count() << " ms" << std::endl;

    // 验证结果
    EXPECT_EQ(large_matrix_[0][0], 42.0);
    EXPECT_EQ(large_matrix_[999][999], 42.0);
}

TEST_F(Array2dPerformanceTest, ParallelFillPerformance) {
    // 测试并行fill性能
    auto start = std::chrono::high_resolution_clock::now();
    large_matrix_.fill_parallel(42.0);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Parallel fill time: " << duration.count() << " ms" << std::endl;

    // 验证结果
    EXPECT_EQ(large_matrix_[0][0], 42.0);
    EXPECT_EQ(large_matrix_[999][999], 42.0);
}

TEST_F(Array2dPerformanceTest, TransposePerformance) {
    // 测试转置性能
    auto start = std::chrono::high_resolution_clock::now();
    large_matrix_.transpose();
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Transpose time: " << duration.count() << " ms" << std::endl;

    // 验证结果
    EXPECT_EQ(large_matrix_.rows(), 1000);
    EXPECT_EQ(large_matrix_.cols(), 1000);
}

// ================================
// 异常安全性测试
// ================================

class ThrowingType {
public:
    static int  construction_count_;
    static int  destruction_count_;
    static bool should_throw_;

    ThrowingType() {
        if (should_throw_ && construction_count_ >= 5) {
            throw std::runtime_error("Construction failed");
        }
        ++construction_count_;
    }

    ThrowingType(const ThrowingType &) {
        if (should_throw_ && construction_count_ >= 5) {
            throw std::runtime_error("Copy construction failed");
        }
        ++construction_count_;
    }

    ThrowingType &operator=(const ThrowingType &) {
        if (should_throw_) {
            throw std::runtime_error("Assignment failed");
        }
        return *this;
    }

    ~ThrowingType() {
        ++destruction_count_;
    }

    static void reset_counters() {
        construction_count_ = 0;
        destruction_count_  = 0;
        should_throw_       = false;
    }
};

int  ThrowingType::construction_count_ = 0;
int  ThrowingType::destruction_count_  = 0;
bool ThrowingType::should_throw_       = false;

class Array2dExceptionSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        ThrowingType::reset_counters();
    }

    void TearDown() override {
        ThrowingType::reset_counters();
    }
};

TEST_F(Array2dExceptionSafetyTest, ConstructorExceptionSafety) {
    ThrowingType::should_throw_ = true;

    EXPECT_THROW(array2d<ThrowingType>(3, 3), std::runtime_error);

    // 验证没有内存泄漏
    EXPECT_EQ(ThrowingType::construction_count_, ThrowingType::destruction_count_);
}

TEST_F(Array2dExceptionSafetyTest, ResizeExceptionSafety) {
    array2d<ThrowingType> matrix(2, 2);

    ThrowingType::should_throw_ = true;

    EXPECT_THROW(matrix.resize(5, 5), std::runtime_error);

    // 原矩阵应该保持不变
    EXPECT_EQ(matrix.rows(), 2);
    EXPECT_EQ(matrix.cols(), 2);
    EXPECT_EQ(matrix.size(), 4);
}

// ================================
// 边界情况测试
// ================================

class Array2dBoundaryTest : public ::testing::Test {};

TEST_F(Array2dBoundaryTest, ZeroSizeMatrix) {
    array2d<int> matrix(0, 5);

    EXPECT_EQ(matrix.rows(), 0);
    EXPECT_EQ(matrix.cols(), 5);
    EXPECT_EQ(matrix.size(), 0);
    EXPECT_TRUE(matrix.empty());

    array2d<int> matrix2(5, 0);
    EXPECT_EQ(matrix2.rows(), 5);
    EXPECT_EQ(matrix2.cols(), 0);
    EXPECT_EQ(matrix2.size(), 0);
    EXPECT_TRUE(matrix2.empty());
}

TEST_F(Array2dBoundaryTest, SingleElementMatrix) {
    array2d<int> matrix(1, 1, 42);

    EXPECT_EQ(matrix.rows(), 1);
    EXPECT_EQ(matrix.cols(), 1);
    EXPECT_EQ(matrix.size(), 1);
    EXPECT_FALSE(matrix.empty());
    EXPECT_TRUE(matrix.is_square());

    EXPECT_EQ(matrix[0][0], 42);
    EXPECT_EQ(matrix(0, 0), 42);
    EXPECT_EQ(matrix.at(0, 0), 42);
}

TEST_F(Array2dBoundaryTest, LargeMatrix) {
    // 测试大矩阵的创建和基本操作
    const int    size = 10000;
    array2d<int> matrix(size, size);

    EXPECT_EQ(matrix.rows(), size);
    EXPECT_EQ(matrix.cols(), size);
    EXPECT_EQ(matrix.size(), static_cast<size_t>(size) * size);

    // 测试边界元素
    matrix[0][0]               = 1;
    matrix[size - 1][size - 1] = 2;

    EXPECT_EQ(matrix[0][0], 1);
    EXPECT_EQ(matrix[size - 1][size - 1], 2);
}

// ================================
// 内存管理测试
// ================================

class Array2dMemoryTest : public ::testing::Test {};

TEST_F(Array2dMemoryTest, MemoryAlignment) {
    array2d<double> matrix(100, 100);

    // 检查内存对齐
    auto ptr = reinterpret_cast<uintptr_t>(matrix.data());
    EXPECT_EQ(ptr % alignof(double), 0);
}

TEST_F(Array2dMemoryTest, MemoryContiguity) {
    array2d<int> matrix(3, 4);
    for (int i = 0; i < 12; ++i) {
        matrix.data()[i] = i + 1;
    }

    // 验证内存是连续的
    EXPECT_EQ(matrix[0][0], 1);
    EXPECT_EQ(matrix[0][1], 2);
    EXPECT_EQ(matrix[0][2], 3);
    EXPECT_EQ(matrix[0][3], 4);
    EXPECT_EQ(matrix[1][0], 5);
    EXPECT_EQ(matrix[2][3], 12);
}

TEST_F(Array2dMemoryTest, MoveSemantics) {
    array2d<int> source(1000, 1000, 42);
    auto        *original_data = source.data();

    array2d<int> destination = std::move(source);

    // 验证数据被移动而不是复制
    EXPECT_EQ(destination.data(), original_data);
    EXPECT_EQ(destination.rows(), 1000);
    EXPECT_EQ(destination.cols(), 1000);
    EXPECT_EQ(destination[0][0], 42);

    // 源对象应该处于有效但未指定状态
    EXPECT_TRUE(source.empty() || source.size() > 0);
}

// ================================
// STL兼容性测试
// ================================

class Array2dSTLCompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        matrix_ = array2d<int>{{1, 2, 3}, {4, 5, 6}};
    }

    array2d<int> matrix_;
};

TEST_F(Array2dSTLCompatibilityTest, STLAlgorithms) {
    // 测试 std::find
    auto it = std::find(matrix_.begin(), matrix_.end(), 4);
    EXPECT_NE(it, matrix_.end());
    EXPECT_EQ(*it, 4);

    // 测试 std::count
    matrix_.fill(42);
    matrix_[0][0] = 99;
    auto count    = std::count(matrix_.begin(), matrix_.end(), 42);
    EXPECT_EQ(count, 5);

    // 测试 std::transform
    std::vector<int> doubled;
    std::transform(matrix_.begin(), matrix_.end(), std::back_inserter(doubled),
                   [](int x) { return x * 2; });

    EXPECT_EQ(doubled[0], 198);  // 99 * 2
    EXPECT_EQ(doubled[1], 84);   // 42 * 2
}

TEST_F(Array2dSTLCompatibilityTest, RangeBasedAlgorithms) {
    // 测试 ranges::find
    auto it = std::ranges::find(matrix_, 5);
    EXPECT_NE(it, matrix_.end());
    EXPECT_EQ(*it, 5);

    // 测试 ranges::all_of
    matrix_.fill(42);
    bool all_42 = std::ranges::all_of(matrix_, [](int x) { return x == 42; });
    EXPECT_TRUE(all_42);
}

// ================================
// 概念约束测试
// ================================

class Array2dConceptTest : public ::testing::Test {};

TEST_F(Array2dConceptTest, Array2dCompatibleConcept) {
    // 这些类型应该满足 Array2d_compatible 概念
    static_assert(Array2d_compatible<int>);
    static_assert(Array2d_compatible<double>);
    static_assert(Array2d_compatible<std::string>);
    static_assert(Array2d_compatible<std::vector<int>>);

    // 这些类型不应该满足 Array2d_compatible 概念
    static_assert(!Array2d_compatible<void>);
    static_assert(!Array2d_compatible<int &>);
    static_assert(!Array2d_compatible<int()>);
}

TEST_F(Array2dConceptTest, Array2dIndexTypeConcept) {
    // 这些类型应该满足 Array2d_index_type 概念
    static_assert(Array2d_index_type<int>);
    static_assert(Array2d_index_type<long>);
    static_assert(Array2d_index_type<short>);
    static_assert(Array2d_index_type<std::ptrdiff_t>);

    // 这些类型不应该满足 Array2d_index_type 概念
    static_assert(!Array2d_index_type<bool>);
    static_assert(!Array2d_index_type<char>);
    static_assert(!Array2d_index_type<float>);
    static_assert(!Array2d_index_type<std::string>);
}