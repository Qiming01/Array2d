//
// Created by qiming on 25-6-24.
//
#include <algorithm>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <numeric>
#include <type_traits>
#include <vector>

#include "array2d_iterator.hpp"

using namespace qm;
using namespace testing;

namespace {

    // ================================
    // 测试辅助类和函数
    // ================================

    /**
     * @brief 测试用的简单结构体
     */
    struct TestStruct {
        int    value;
        double data;

        TestStruct() : value(0), data(0) {}

        TestStruct(int v, double d) : value(v), data(d) {}

        bool operator==(const TestStruct &other) const {
            return value == other.value && data == other.data;
        }

        bool operator!=(const TestStruct &other) const {
            return !(*this == other);
        }

        std::partial_ordering operator<=>(const TestStruct &other) const {
            if (auto cmp = value <=> other.value; cmp != 0) {
                return cmp;
            }
            return data <=> other.data;
        }
    };

    /**
     * @brief 创建测试数据数组
     */
    template<typename T>
    std::unique_ptr<T[]> create_test_array(size_t size) {
        auto arr = std::make_unique<T[]>(size);
        if constexpr (std::is_same_v<T, int>) {
            std::iota(arr.get(), arr.get() + size, 1);
        } else if constexpr (std::is_same_v<T, TestStruct>) {
            for (size_t i = 0; i < size; ++i) {
                arr[i] = TestStruct(static_cast<int>(i + 1), static_cast<double>(i + 1) * 1.5);
            }
        }
        return arr;
    }

}  // anonymous namespace

// ================================
// 基础功能测试
// ================================

class Array2dIteratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        size_         = 10;
        int_array_    = create_test_array<int>(size_);
        struct_array_ = create_test_array<TestStruct>(size_);
    }

    size_t                        size_{};
    std::unique_ptr<int[]>        int_array_;
    std::unique_ptr<TestStruct[]> struct_array_;
};

// 基础构造和类型测试
TEST_F(Array2dIteratorTest, BasicConstruction) {
    // 默认构造
    Array2d_iterator<int> default_iter;
    EXPECT_EQ(default_iter.data(), nullptr);

    // 从指针构造
    Array2d_iterator<int> iter(int_array_.get());
    EXPECT_EQ(iter.data(), int_array_.get());

    // 测试类型特征
    EXPECT_TRUE((std::is_same_v<Array2d_iterator<int>::value_type, int>) );
    EXPECT_TRUE((std::is_same_v<Array2d_iterator<int>::pointer, int *>) );
    EXPECT_TRUE((std::is_same_v<Array2d_iterator<int>::reference, int &>) );
    EXPECT_TRUE((std::is_same_v<Array2d_iterator<int>::iterator_category, std::contiguous_iterator_tag>) );
}

// 解引用操作测试
TEST_F(Array2dIteratorTest, DereferenceOperations) {
    Array2d_iterator<int> iter(int_array_.get());

    // 解引用操作符
    EXPECT_EQ(*iter, 1);

    // 修改值
    *iter = 100;
    EXPECT_EQ(int_array_[0], 100);

    // 下标访问
    EXPECT_EQ(iter[0], 100);
    EXPECT_EQ(iter[1], 2);
    EXPECT_EQ(iter[4], 5);
}

TEST_F(Array2dIteratorTest, ArrowOperatorTests) {
    // 1. 对于基本类型，-> 操作符返回指针
    Array2d_iterator<int> int_iter(int_array_.get());

    // 测试 -> 操作符返回正确的指针
    EXPECT_EQ(int_iter.operator->(), int_array_.get());
    EXPECT_EQ(int_iter.operator->(), &(*int_iter));

    // 2. 对于结构体/类类型，测试成员访问
    struct TestStruct {
        int    value;
        double data;

        explicit TestStruct(int v = 0, double d = 0.0) : value(v), data(d) {}

        void set_value(int v) { value = v; }

        [[nodiscard]] int get_value() const { return value; }
    };

    // 创建结构体数组
    auto struct_array = std::make_unique<TestStruct[]>(3);
    struct_array[0]   = TestStruct{42, 3.14};
    struct_array[1]   = TestStruct{100, 2.71};
    struct_array[2]   = TestStruct{200, 1.41};

    Array2d_iterator<TestStruct> struct_iter(struct_array.get());

    // 测试 -> 操作符访问成员变量
    EXPECT_EQ(struct_iter->value, 42);
    EXPECT_DOUBLE_EQ(struct_iter->data, 3.14);

    // 测试 -> 操作符调用成员函数
    EXPECT_EQ(struct_iter->get_value(), 42);

    // 测试通过 -> 操作符修改成员
    struct_iter->set_value(999);
    EXPECT_EQ(struct_iter->value, 999);
    EXPECT_EQ(struct_array[0].value, 999);

    // 测试移动到下一个元素
    ++struct_iter;
    EXPECT_EQ(struct_iter->value, 100);
    EXPECT_DOUBLE_EQ(struct_iter->data, 2.71);
}

// ================================
// 更复杂的 -> 操作符测试
// ================================

TEST_F(Array2dIteratorTest, ArrowOperatorWithComplexTypes) {
    // 测试包含指针成员的结构体
    struct ComplexStruct {
        std::unique_ptr<int> ptr;
        std::vector<int>     vec;

        ComplexStruct() : ptr(std::make_unique<int>(42)), vec{1, 2, 3} {}

        [[nodiscard]] int get_ptr_value() const { return ptr ? *ptr : -1; }

        [[nodiscard]] size_t get_vec_size() const { return vec.size(); }
    };

    auto                            complex_array = std::make_unique<ComplexStruct[]>(2);
    Array2d_iterator<ComplexStruct> complex_iter(complex_array.get());

    // 测试 -> 操作符访问复杂成员
    EXPECT_EQ(complex_iter->get_ptr_value(), 42);
    EXPECT_EQ(complex_iter->get_vec_size(), 3);
    EXPECT_EQ(complex_iter->vec[0], 1);
    EXPECT_EQ(complex_iter->vec[2], 3);

    // 测试指针成员的访问
    EXPECT_EQ(*(complex_iter->ptr), 42);
}

// ================================
// const 迭代器的 -> 操作符测试
// ================================

TEST_F(Array2dIteratorTest, ConstArrowOperatorTests) {
    struct TestStruct {
        int         value;
        mutable int mutable_value;

        explicit TestStruct(int v = 0) : value(v), mutable_value(v * 2) {}

        int get_value() const { return value; }

        void set_mutable_value(int v) const { mutable_value = v; }
    };

    auto struct_array = std::make_unique<TestStruct[]>(2);
    struct_array[0]   = TestStruct{10};

    // const 迭代器测试
    Array2d_iterator<const TestStruct> const_iter(struct_array.get());

    // 可以通过 const 迭代器读取
    EXPECT_EQ(const_iter->value, 10);
    EXPECT_EQ(const_iter->get_value(), 10);

    // 可以修改 mutable 成员
    const_iter->set_mutable_value(999);
    EXPECT_EQ(const_iter->mutable_value, 999);

    // 以下代码应该编译失败（如果取消注释）
    // const_iter->value = 20;  // 错误：不能修改 const 对象的非 mutable 成员
}

// ================================
// -> 操作符的类型推导测试
// ================================

TEST_F(Array2dIteratorTest, ArrowOperatorTypeDeduction) {
    struct TestStruct {
        int value = 42;

        [[nodiscard]] auto get_lambda() const {
            return [this]() { return value * 2; };
        }
    };

    auto                         struct_array = std::make_unique<TestStruct[]>(1);
    Array2d_iterator<TestStruct> iter(struct_array.get());

    // 测试 -> 操作符与现代 C++ 特性
    auto lambda = iter->get_lambda();
    EXPECT_EQ(lambda(), 84);

    // 测试类型推导
    static_assert(std::is_same_v<decltype(iter.operator->()), TestStruct *>);
    static_assert(std::is_same_v<decltype(iter->value), int>);
}

// 递增递减操作测试
TEST_F(Array2dIteratorTest, IncrementDecrementOperations) {
    Array2d_iterator<int> iter(int_array_.get());

    // 前置递增
    EXPECT_EQ(*iter, 1);
    ++iter;
    EXPECT_EQ(*iter, 2);

    // 后置递增
    auto old_iter = iter++;
    EXPECT_EQ(*old_iter, 2);
    EXPECT_EQ(*iter, 3);

    // 前置递减
    --iter;
    EXPECT_EQ(*iter, 2);

    // 后置递减
    old_iter = iter--;
    EXPECT_EQ(*old_iter, 2);
    EXPECT_EQ(*iter, 1);
}

// 算术操作测试
TEST_F(Array2dIteratorTest, ArithmeticOperations) {
    Array2d_iterator<int> iter(int_array_.get());

    // 加法操作
    auto iter_plus_3 = iter + 3;
    EXPECT_EQ(*iter_plus_3, 4);

    // 全局加法操作符
    auto three_plus_iter = 3 + iter;
    EXPECT_EQ(*three_plus_iter, 4);
    EXPECT_EQ(iter_plus_3, three_plus_iter);

    // 复合加法赋值
    iter += 2;
    EXPECT_EQ(*iter, 3);

    // 减法操作
    auto iter_minus_1 = iter - 1;
    EXPECT_EQ(*iter_minus_1, 2);

    // 复合减法赋值
    iter -= 1;
    EXPECT_EQ(*iter, 2);

    // 迭代器距离计算
    Array2d_iterator<int> iter1(int_array_.get());
    Array2d_iterator<int> iter2(int_array_.get() + 5);
    EXPECT_EQ(iter2 - iter1, 5);
    EXPECT_EQ(iter1 - iter2, -5);
}

// 比较操作测试
TEST_F(Array2dIteratorTest, ComparisonOperations) {
    Array2d_iterator<int> iter1(int_array_.get());
    Array2d_iterator<int> iter2(int_array_.get() + 3);
    Array2d_iterator<int> iter3(int_array_.get());

    // 相等比较
    EXPECT_TRUE(iter1 == iter3);
    EXPECT_FALSE(iter1 == iter2);
    EXPECT_TRUE(iter1 != iter2);
    EXPECT_FALSE(iter1 != iter3);

    // 三路比较
    EXPECT_TRUE(iter1 < iter2);
    EXPECT_TRUE(iter1 <= iter2);
    EXPECT_TRUE(iter1 <= iter3);
    EXPECT_TRUE(iter2 > iter1);
    EXPECT_TRUE(iter2 >= iter1);
    EXPECT_TRUE(iter3 >= iter1);

    // 使用 spaceship 操作符
    EXPECT_TRUE((iter1 <=> iter2) < 0);
    EXPECT_TRUE((iter2 <=> iter1) > 0);
    EXPECT_TRUE((iter1 <=> iter3) == 0);
}

// ================================
// 类型转换测试
// ================================

TEST_F(Array2dIteratorTest, TypeConversionTests) {
    Array2d_iterator<int> iter(int_array_.get());

    // non-const 到 const 转换
    Array2d_iterator<const int> const_iter(iter);
    EXPECT_EQ(const_iter.data(), iter.data());
    EXPECT_EQ(*const_iter, *iter);

    // 跨类型比较
    EXPECT_TRUE(iter == const_iter);
    EXPECT_FALSE(iter != const_iter);
    EXPECT_TRUE((iter <=> const_iter) == 0);

    // 跨类型距离计算
    Array2d_iterator<const int> const_iter2(int_array_.get() + 3);
    EXPECT_EQ(const_iter2 - iter, 3);
    EXPECT_EQ(iter - const_iter2, -3);
}

// ================================
// STL 兼容性测试
// ================================

TEST_F(Array2dIteratorTest, STLCompatibility) {
    Array2d_iterator<int> begin_iter(int_array_.get());
    Array2d_iterator<int> end_iter(int_array_.get() + size_);

    // 测试 std::distance
    EXPECT_EQ(std::distance(begin_iter, end_iter), static_cast<ptrdiff_t>(size_));

    // 测试 std::advance
    auto iter = begin_iter;
    std::advance(iter, 3);
    EXPECT_EQ(*iter, 4);

    // 测试 std::next 和 std::prev
    auto next_iter = std::next(begin_iter, 2);
    EXPECT_EQ(*next_iter, 3);

    auto prev_iter = std::prev(end_iter, 2);
    EXPECT_EQ(*prev_iter, 9);
}

// 标准库算法测试
TEST_F(Array2dIteratorTest, STLAlgorithms) {
    Array2d_iterator<int> begin_iter(int_array_.get());
    Array2d_iterator<int> end_iter(int_array_.get() + size_);

    // std::find
    auto found = std::find(begin_iter, end_iter, 5);
    EXPECT_NE(found, end_iter);
    EXPECT_EQ(*found, 5);

    // std::count
    int_array_[3] = 5;  // 创建重复值
    auto count    = std::count(begin_iter, end_iter, 5);
    EXPECT_EQ(count, 2);

    // std::copy
    std::vector<int> dest(size_);
    std::copy(begin_iter, end_iter, dest.begin());
    EXPECT_THAT(dest, ElementsAre(1, 2, 3, 5, 5, 6, 7, 8, 9, 10));

    // std::transform
    std::vector<int> doubled(size_);
    std::transform(begin_iter, end_iter, doubled.begin(),
                   [](int x) { return x * 2; });
    EXPECT_THAT(doubled, ElementsAre(2, 4, 6, 10, 10, 12, 14, 16, 18, 20));

    // std::accumulate
    auto sum = std::accumulate(begin_iter, end_iter, 0);
    EXPECT_EQ(sum, 56);  // 1+2+3+5+5+6+7+8+9+10
}

// ================================
// 迭代器概念测试
// ================================

TEST_F(Array2dIteratorTest, IteratorConcepts) {
    using Iterator      = Array2d_iterator<int>;
    using ConstIterator = Array2d_iterator<const int>;

    // C++20 迭代器概念测试
    EXPECT_TRUE(std::input_iterator<Iterator>);
    EXPECT_TRUE(std::forward_iterator<Iterator>);
    EXPECT_TRUE(std::bidirectional_iterator<Iterator>);
    EXPECT_TRUE(std::random_access_iterator<Iterator>);
    EXPECT_TRUE(std::contiguous_iterator<Iterator>);

    EXPECT_TRUE(std::input_iterator<ConstIterator>);
    EXPECT_TRUE(std::forward_iterator<ConstIterator>);
    EXPECT_TRUE(std::bidirectional_iterator<ConstIterator>);
    EXPECT_TRUE(std::random_access_iterator<ConstIterator>);
    EXPECT_TRUE(std::contiguous_iterator<ConstIterator>);

    // 自定义概念测试
    EXPECT_TRUE(Array2d_iterator_compatible<int>);
    EXPECT_TRUE(Array2d_iterator_compatible<TestStruct>);
    EXPECT_FALSE(Array2d_iterator_compatible<void>);
    EXPECT_FALSE(Array2d_iterator_compatible<int &>);
}

// ================================
// 边界和异常测试
// ================================

TEST_F(Array2dIteratorTest, BoundaryTests) {
    Array2d_iterator<int> iter(int_array_.get());

    // 测试大幅度跳跃
    auto far_iter  = iter + 1000;
    auto back_iter = far_iter - 1000;
    EXPECT_EQ(iter, back_iter);

    // 测试负数偏移
    auto iter_at_5 = iter + 5;
    auto back_to_2 = iter_at_5 - 3;
    EXPECT_EQ(*(back_to_2), 3);

    // 测试零偏移
    auto same_iter = iter + 0;
    EXPECT_EQ(iter, same_iter);

    same_iter = iter - 0;
    EXPECT_EQ(iter, same_iter);
}

// ================================
// 性能测试
// ================================

TEST_F(Array2dIteratorTest, PerformanceTest) {
    const size_t large_size  = 1000000;
    auto         large_array = create_test_array<int>(large_size);

    Array2d_iterator<int> begin_iter(large_array.get());
    Array2d_iterator<int> end_iter(large_array.get() + large_size);

    // 测试大量递增操作的性能
    auto start = std::chrono::high_resolution_clock::now();

    long long sum = 0;
    for (auto it = begin_iter; it != end_iter; ++it) {
        sum += *it;
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // 验证结果正确性
    long long expected_sum = static_cast<long long>(large_size) * (large_size + 1) / 2;
    EXPECT_EQ(sum, expected_sum);

    // 性能应该在合理范围内（这个测试可能需要根据具体环境调整）
    EXPECT_LT(duration.count(), 10000);  // 应该在10ms内完成
}

// ================================
// 复杂数据类型测试
// ================================

TEST_F(Array2dIteratorTest, ComplexDataTypes) {
    Array2d_iterator<TestStruct> iter(struct_array_.get());

    // 测试结构体访问
    EXPECT_EQ(iter->value, 1);
    EXPECT_DOUBLE_EQ(iter->data, 1.5);

    // 测试结构体比较
    TestStruct expected{1, 1.5};
    EXPECT_EQ(*iter, expected);

    // 测试结构体修改
    iter->value = 999;
    EXPECT_EQ(struct_array_[0].value, 999);

    // 测试算法与结构体
    Array2d_iterator<TestStruct> begin_iter(struct_array_.get());
    Array2d_iterator<TestStruct> end_iter(struct_array_.get() + size_);

    auto found = std::find_if(begin_iter, end_iter,
                              [](const TestStruct &s) { return s.value == 2; });

    EXPECT_NE(found, end_iter);
    EXPECT_EQ(found->value, 2);
    EXPECT_DOUBLE_EQ(found->data, 3.0);
}

// ================================
// 类型特征测试
// ================================

TEST_F(Array2dIteratorTest, TypeTraitsTests) {
    // 测试 is_array2d_iterator
    EXPECT_TRUE(is_array2d_iterator_v<Array2d_iterator<int>>);
    EXPECT_TRUE(is_array2d_iterator_v<Array2d_iterator<const int>>);
    EXPECT_FALSE(is_array2d_iterator_v<int *>);
    EXPECT_FALSE(is_array2d_iterator_v<std::vector<int>::iterator>);

    // 测试 array2d_iterator_traits
    using traits = array2d_iterator_traits<Array2d_iterator<int>>;
    EXPECT_TRUE((std::is_same_v<traits::value_type, int>) );
    EXPECT_TRUE((std::is_same_v<traits::pointer, int *>) );
    EXPECT_TRUE((std::is_same_v<traits::reference, int &>) );

    // 测试标准库特化
    using std_traits = std::iterator_traits<Array2d_iterator<int>>;
    EXPECT_TRUE((std::is_same_v<std_traits::value_type, int>) );
    EXPECT_TRUE((std::is_same_v<std_traits::iterator_category, std::contiguous_iterator_tag>) );
}

// ================================
// 别名测试
// ================================

TEST_F(Array2dIteratorTest, TypeAliasTests) {
    // 测试便捷别名
    EXPECT_TRUE((std::is_same_v<array2d_iterator<int>, Array2d_iterator<int>>) );
    EXPECT_TRUE((std::is_same_v<array2d_const_iterator<int>, Array2d_iterator<const int>>) );

    // 确保别名工作正常
    array2d_iterator<int>       iter1(int_array_.get());
    array2d_const_iterator<int> iter2(iter1);

    EXPECT_EQ(*iter1, *iter2);
    EXPECT_EQ(iter1.data(), iter2.data());
}

// ================================
// 常量正确性测试
// ================================

TEST_F(Array2dIteratorTest, ConstCorrectnessTests) {
    const int                  *const_ptr = int_array_.get();
    Array2d_iterator<const int> const_iter(const_ptr);

    // const 迭代器应该只能读取，不能修改
    EXPECT_EQ(*const_iter, 1);
    // *const_iter = 100; // 这行应该编译失败

    // const 迭代器仍然可以移动
    ++const_iter;
    EXPECT_EQ(*const_iter, 2);

    // const 迭代器可以参与比较
    Array2d_iterator<const int> another_const_iter(const_ptr);
    EXPECT_TRUE(const_iter > another_const_iter);
    EXPECT_EQ(const_iter - another_const_iter, 1);
}