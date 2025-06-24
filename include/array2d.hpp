#pragma once

#include "array2d_iterator.hpp"
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstring>
#include <execution>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

#ifndef QM_ARRAY_RESET_OPT_DEFINED
#define QM_ARRAY_RESET_OPT_DEFINED

namespace qm {
    /**
     * @brief 二维数组内存重置选项枚举
     *
     * 定义了在重置二维数组内存时可用的不同模式。
     */
    enum class Array_reset_opt : std::int8_t {
        All_bits0 = 0,   /**< 将所有位设置为0 */
        All_bits1 = -1,  /**< 将所有位设置为1 */
        Safe_max  = 0x3F /**< 安全的最大值模式 */
    };

    // ================================
    // 概念定义和类型特征
    // ================================

    /**
     * @brief 二维数组兼容类型概念
     *
     * 约束二维数组可以存储的类型必须是：
     * - 对象类型（非函数、非引用）
     * - 非抽象类型
     * - 可计算大小的类型
     *
     * @tparam T 待检验的类型
     */
    template<typename T>
    concept Array2d_compatible =
            std::is_object_v<T> &&
            !std::is_abstract_v<T> &&
            requires { sizeof(T); };

    /**
     * @brief 二维数组索引类型概念
     *
     * 约束索引类型必须是：
     * - 整数类型
     * - 非bool类型（避免特化问题）
     * - 非char类型（避免字符/数值混淆）
     *
     * @tparam Idx 待检验的索引类型
     */
    template<typename Idx>
    concept Array2d_index_type =
            std::is_integral_v<Idx> &&
            !std::is_same_v<Idx, bool> &&
            !std::is_same_v<Idx, char>;

    // ================================
    // array2d 主类定义
    // ================================

    /**
     * @brief 高性能二维数组容器类
     *
     * 提供了连续内存存储的二维数组实现，支持高效的元素访问、
     * 迭代器操作、内存管理和各种矩阵操作。
     *
     * @tparam Ty 元素类型，必须满足Array2d_compatible概念
     * @tparam Idx 索引类型，必须满足Array2d_index_type概念，默认为int
     *
     * @note 内部使用行优先存储（row-major order）
     * @note 提供了针对POD类型的内存操作优化
     */
    template<Array2d_compatible Ty, Array2d_index_type Idx = int>
    class array2d {
    public:
        // ================================
        // 公共类型定义
        // ================================

        using value_type      = Ty;                        /**< 元素类型 */
        using index_type      = Idx;                       /**< 索引类型 */
        using size_type       = std::make_unsigned_t<Idx>; /**< 大小类型 */
        using difference_type = std::make_signed_t<Idx>;   /**< 差值类型 */
        using pointer         = Ty *;                      /**< 指针类型 */
        using const_pointer   = const Ty *;                /**< 常量指针类型 */
        using reference       = Ty &;                      /**< 引用类型 */
        using const_reference = const Ty &;                /**< 常量引用类型 */

        using iterator               = Array2d_iterator<Ty>;                  /**< 迭代器类型 */
        using const_iterator         = Array2d_iterator<const Ty>;            /**< 常量迭代器类型 */
        using reverse_iterator       = std::reverse_iterator<iterator>;       /**< 反向迭代器类型 */
        using const_reverse_iterator = std::reverse_iterator<const_iterator>; /**< 常量反向迭代器类型 */

        // ================================
        // 构造函数
        // ================================

        /**
         * @brief 默认构造函数
         *
         * 创建一个空的二维数组（0行0列）。
         */
        constexpr array2d() noexcept = default;

        /**
         * @brief 构造指定尺寸的二维数组
         *
         * @param rows 行数，必须非负
         * @param cols 列数，必须非负
         *
         * @throws std::invalid_argument 当行数或列数为负时
         * @throws std::overflow_error 当计算总大小溢出时
         * @throws std::bad_alloc 当内存分配失败时
         *
         * @note 元素使用默认构造函数初始化
         */
        array2d(index_type rows, index_type cols)
            : rows_(validate_dimension(rows, "rows")),
              cols_(validate_dimension(cols, "cols")) {

            if (rows_ > 0 && cols_ > 0) {
                const auto size = calculate_size(rows_, cols_);
                data_.resize(size);
            }
        }

        /**
         * @brief 构造指定尺寸和初值的二维数组
         *
         * @param rows 行数，必须非负
         * @param cols 列数，必须非负
         * @param val 用于初始化所有元素的值
         *
         * @throws std::invalid_argument 当行数或列数为负时
         * @throws std::overflow_error 当计算总大小溢出时
         * @throws std::bad_alloc 当内存分配失败时
         */
        array2d(index_type rows, index_type cols, const Ty &val)
            : rows_(validate_dimension(rows, "rows")),
              cols_(validate_dimension(cols, "cols")) {

            if (rows_ > 0 && cols_ > 0) {
                const auto size = calculate_size(rows_, cols_);
                data_.resize(size, val);
            }
        }

        /**
         * @brief 从初始化列表构造二维数组
         *
         * @param init_list 嵌套的初始化列表，外层代表行，内层代表列
         *
         * @throws std::invalid_argument 当各行列数不一致时
         * @throws std::bad_alloc 当内存分配失败时
         *
         * @note 所有行必须具有相同的列数
         *
         * @par 示例:
         * @code
         * array2d<int> arr{{1, 2, 3}, {4, 5, 6}};  // 2x3矩阵
         * @endcode
         */
        array2d(std::initializer_list<std::initializer_list<Ty>> init_list) {
            if (init_list.size() == 0) {
                rows_ = cols_ = 0;
                return;
            }

            rows_ = static_cast<index_type>(init_list.size());
            cols_ = static_cast<index_type>(init_list.begin()->size());

            // 验证所有行的列数相同
            for (const auto &row: init_list) {
                if (static_cast<index_type>(row.size()) != cols_) {
                    throw std::invalid_argument("All rows must have the same number of columns");
                }
            }

            const auto size = calculate_size(rows_, cols_);
            data_.reserve(size);

            for (const auto &row: init_list) {
                data_.insert(data_.end(), row.begin(), row.end());
            }
        }

        /**
         * @brief 从容器构造二维数组
         *
         * @tparam Container 容器类型，必须满足range概念且元素可转换为Ty
         * @param rows 行数，必须非负
         * @param cols 列数，必须非负
         * @param container 提供数据的容器
         *
         * @throws std::invalid_argument 当行数或列数为负，或容器大小与矩阵尺寸不匹配时
         * @throws std::overflow_error 当计算总大小溢出时
         * @throws std::bad_alloc 当内存分配失败时
         *
         * @note 容器的大小必须等于rows * cols
         * @note 数据按行优先顺序复制
         */
        template<typename Container>
        array2d(index_type rows, index_type cols, const Container &container)
            requires std::ranges::range<Container> &&
                             std::convertible_to<std::ranges::range_value_t<Container>, Ty>
            : rows_(validate_dimension(rows, "rows")),
              cols_(validate_dimension(cols, "cols")) {

            const auto expected_size = calculate_size(rows_, cols_);
            if (std::ranges::size(container) != expected_size) {
                throw std::invalid_argument("Container size doesn't match matrix dimensions");
            }

            data_.reserve(expected_size);
            std::ranges::copy(container, std::back_inserter(data_));
        }

        /**
         * @brief 拷贝构造函数
         *
         * 使用编译器生成的默认实现。
         */
        array2d(const array2d &) = default;

        /**
         * @brief 移动构造函数
         *
         * 使用编译器生成的默认实现。
         */
        array2d(array2d &&) noexcept = default;

        /**
         * @brief 拷贝赋值操作符
         *
         * 使用编译器生成的默认实现。
         */
        array2d &operator=(const array2d &) = default;

        /**
         * @brief 移动赋值操作符
         *
         * 使用编译器生成的默认实现。
         */
        array2d &operator=(array2d &&) noexcept = default;

        /**
         * @brief 析构函数
         *
         * 使用编译器生成的默认实现。
         */
        ~array2d() = default;

        // ================================
        // 元素访问方法
        // ================================

        /**
         * @brief 行访问操作符（非常量版本）
         *
         * @param row 行索引
         * @return 指向指定行首元素的指针
         *
         * @note 不进行边界检查（在调试模式下使用断言）
         * @note 返回的指针可用于访问整行元素
         *
         * @par 示例:
         * @code
         * array2d<int> arr(3, 4);
         * arr[1][2] = 42;  // 设置第1行第2列的元素
         * @endcode
         */
        [[nodiscard]] QM_FORCEINLINE pointer operator[](index_type row) noexcept {
            assert_bounds(row, rows_);
            return data_.data() + calculate_offset(row, 0);
        }

        /**
         * @brief 行访问操作符（常量版本）
         *
         * @param row 行索引
         * @return 指向指定行首元素的常量指针
         *
         * @note 不进行边界检查（在调试模式下使用断言）
         */
        [[nodiscard]] QM_FORCEINLINE const_pointer operator[](index_type row) const noexcept {
            assert_bounds(row, rows_);
            return data_.data() + calculate_offset(row, 0);
        }

        /**
         * @brief 二维索引访问操作符（非常量版本）
         *
         * @param row 行索引
         * @param col 列索引
         * @return 指定位置元素的引用
         *
         * @note 不进行边界检查（在调试模式下使用断言）
         */
        [[nodiscard]] QM_FORCEINLINE reference operator()(index_type row, index_type col) noexcept {
            assert_bounds(row, rows_);
            assert_bounds(col, cols_);
            return data_[calculate_offset(row, col)];
        }

        /**
         * @brief 二维索引访问操作符（常量版本）
         *
         * @param row 行索引
         * @param col 列索引
         * @return 指定位置元素的常量引用
         *
         * @note 不进行边界检查（在调试模式下使用断言）
         */
        [[nodiscard]] QM_FORCEINLINE const_reference operator()(index_type row, index_type col) const noexcept {
            assert_bounds(row, rows_);
            assert_bounds(col, cols_);
            return data_[calculate_offset(row, col)];
        }

        /**
         * @brief 带边界检查的元素访问（非常量版本）
         *
         * @param row 行索引
         * @param col 列索引
         * @return 指定位置元素的引用
         *
         * @throws std::out_of_range 当索引超出边界时
         */
        [[nodiscard]] reference at(index_type row, index_type col) {
            return at_impl<false>(row, col);
        }

        /**
         * @brief 带边界检查的元素访问（常量版本）
         *
         * @param row 行索引
         * @param col 列索引
         * @return 指定位置元素的常量引用
         *
         * @throws std::out_of_range 当索引超出边界时
         */
        [[nodiscard]] const_reference at(index_type row, index_type col) const {
            return at_impl<true>(row, col);
        }

        // ================================
        // 迭代器支持
        // ================================

        /**
         * @brief 获取指向首元素的迭代器
         * @return 指向首元素的迭代器
         */
        [[nodiscard]] constexpr iterator begin() noexcept {
            return iterator(data_.data());
        }

        /**
         * @brief 获取指向首元素的常量迭代器
         * @return 指向首元素的常量迭代器
         */
        [[nodiscard]] constexpr const_iterator begin() const noexcept {
            return const_iterator(data_.data());
        }

        /**
         * @brief 获取指向尾后元素的迭代器
         * @return 指向尾后元素的迭代器
         */
        [[nodiscard]] constexpr iterator end() noexcept {
            return iterator(data_.data() + data_.size());
        }

        /**
         * @brief 获取指向尾后元素的常量迭代器
         * @return 指向尾后元素的常量迭代器
         */
        [[nodiscard]] constexpr const_iterator end() const noexcept {
            return const_iterator(data_.data() + data_.size());
        }

        /**
         * @brief 获取指向首元素的常量迭代器
         * @return 指向首元素的常量迭代器
         */
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }

        /**
         * @brief 获取指向尾后元素的常量迭代器
         * @return 指向尾后元素的常量迭代器
         */
        [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

        /**
         * @brief 获取指向末元素的反向迭代器
         * @return 指向末元素的反向迭代器
         */
        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        /**
         * @brief 获取指向末元素的常量反向迭代器
         * @return 指向末元素的常量反向迭代器
         */
        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        /**
         * @brief 获取指向首前元素的反向迭代器
         * @return 指向首前元素的反向迭代器
         */
        [[nodiscard]] constexpr reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        /**
         * @brief 获取指向首前元素的常量反向迭代器
         * @return 指向首前元素的常量反向迭代器
         */
        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        /**
         * @brief 获取指向末元素的常量反向迭代器
         * @return 指向末元素的常量反向迭代器
         */
        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }

        /**
         * @brief 获取指向首前元素的常量反向迭代器
         * @return 指向首前元素的常量反向迭代器
         */
        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

        /**
         * @brief 行迭代器包装器
         *
         * 提供对单行元素的迭代器访问。
         */
        struct row_iterator_wrapper {
            pointer   ptr_;  /**< 指向行首的指针 */
            size_type cols_; /**< 列数 */

            /**
             * @brief 构造行迭代器包装器
             * @param ptr 指向行首的指针
             * @param cols 列数
             */
            row_iterator_wrapper(pointer ptr, size_type cols) : ptr_(ptr), cols_(cols) {}

            /**
             * @brief 获取行的开始迭代器
             * @return 指向行首的迭代器
             */
            [[nodiscard]] iterator begin() const noexcept { return iterator(ptr_); }

            /**
             * @brief 获取行的结束迭代器
             * @return 指向行尾后的迭代器
             */
            [[nodiscard]] iterator end() const noexcept { return iterator(ptr_ + cols_); }
        };

        /**
         * @brief 常量行迭代器包装器
         *
         * 提供对单行元素的常量迭代器访问。
         */
        struct const_row_iterator_wrapper {
            const_pointer ptr_;  /**< 指向行首的常量指针 */
            size_type     cols_; /**< 列数 */

            /**
             * @brief 构造常量行迭代器包装器
             * @param ptr 指向行首的常量指针
             * @param cols 列数
             */
            const_row_iterator_wrapper(const_pointer ptr, size_type cols) : ptr_(ptr), cols_(cols) {}

            /**
             * @brief 获取行的开始常量迭代器
             * @return 指向行首的常量迭代器
             */
            [[nodiscard]] const_iterator begin() const noexcept { return const_iterator(ptr_); }

            /**
             * @brief 获取行的结束常量迭代器
             * @return 指向行尾后的常量迭代器
             */
            [[nodiscard]] const_iterator end() const noexcept { return const_iterator(ptr_ + cols_); }
        };

        /**
         * @brief 获取指定行的迭代器范围
         *
         * @param row 行索引
         * @return 行迭代器包装器，可用于范围for循环
         *
         * @par 示例:
         * @code
         * for (auto& elem : arr.row_range(1)) {
         *     elem *= 2;  // 将第1行所有元素乘以2
         * }
         * @endcode
         */
        [[nodiscard]] row_iterator_wrapper row_range(index_type row) noexcept {
            assert_bounds(row, rows_);
            return {data_.data() + calculate_offset(row, 0), static_cast<size_type>(cols_)};
        }

        /**
         * @brief 获取指定行的常量迭代器范围
         *
         * @param row 行索引
         * @return 常量行迭代器包装器，可用于范围for循环
         */
        [[nodiscard]] const_row_iterator_wrapper row_range(index_type row) const noexcept {
            assert_bounds(row, rows_);
            return {data_.data() + calculate_offset(row, 0), static_cast<size_type>(cols_)};
        }

        // ================================
        // 尺寸和容量查询
        // ================================

        /**
         * @brief 获取行数
         * @return 矩阵的行数
         */
        [[nodiscard]] constexpr index_type rows() const noexcept { return rows_; }

        /**
         * @brief 获取列数
         * @return 矩阵的列数
         */
        [[nodiscard]] constexpr index_type cols() const noexcept { return cols_; }

        /**
         * @brief 获取总元素数
         * @return 矩阵中元素的总数（rows * cols）
         */
        [[nodiscard]] constexpr size_type size() const noexcept { return data_.size(); }

        /**
         * @brief 检查矩阵是否为空
         * @return 如果矩阵为空（无元素）则返回true
         */
        [[nodiscard]] constexpr bool empty() const noexcept { return data_.empty(); }

        /**
         * @brief 获取当前容量
         * @return 在不重新分配内存的情况下可容纳的元素数
         */
        [[nodiscard]] constexpr size_type capacity() const noexcept { return data_.capacity(); }

        /**
         * @brief 检查矩阵是否为方阵
         * @return 如果行数等于列数则返回true
         */
        [[nodiscard]] constexpr bool is_square() const noexcept { return rows_ == cols_; }

        /**
         * @brief 预留内存空间
         *
         * @param rows 预期的行数
         * @param cols 预期的列数
         *
         * @throws std::invalid_argument 当行数或列数为负时
         * @throws std::overflow_error 当计算总大小溢出时
         * @throws std::bad_alloc 当内存分配失败时
         *
         * @note 只影响内存分配，不改变当前矩阵尺寸
         */
        void reserve(index_type rows, index_type cols) {
            const auto new_capacity = calculate_size(
                    validate_dimension(rows, "rows"),
                    validate_dimension(cols, "cols"));
            data_.reserve(new_capacity);
        }

        /**
         * @brief 释放未使用的内存
         *
         * 请求释放未使用的容量，使容量适应当前大小。
         */
        void shrink_to_fit() {
            data_.shrink_to_fit();
        }

        // ================================
        // span 操作和视图
        // ================================

        /**
         * @brief 获取整个矩阵的span视图
         * @return 覆盖所有元素的span
         */
        [[nodiscard]] constexpr std::span<Ty> as_span() noexcept {
            return {data_.data(), data_.size()};
        }

        /**
         * @brief 获取整个矩阵的常量span视图
         * @return 覆盖所有元素的常量span
         */
        [[nodiscard]] constexpr std::span<const Ty> as_span() const noexcept {
            return {data_.data(), data_.size()};
        }

        /**
         * @brief 获取指定行的span视图
         *
         * @param row 行索引
         * @return 覆盖指定行所有元素的span
         *
         * @note 不进行边界检查（在调试模式下使用断言）
         */
        [[nodiscard]] constexpr std::span<Ty> row(index_type row) noexcept {
            assert_bounds(row, rows_);
            return {data_.data() + calculate_offset(row, 0), static_cast<size_type>(cols_)};
        }

        /**
         * @brief 获取指定行的常量span视图
         *
         * @param row 行索引
         * @return 覆盖指定行所有元素的常量span
         *
         * @note 不进行边界检查（在调试模式下使用断言）
         */
        [[nodiscard]] constexpr std::span<const Ty> row(index_type row) const noexcept {
            assert_bounds(row, rows_);
            return {data_.data() + calculate_offset(row, 0), static_cast<size_type>(cols_)};
        }

        /**
         * @brief 获取指定列的所有元素
         *
         * @param col 列索引
         * @return 包含指定列所有元素的vector
         *
         * @note 由于列元素在内存中不连续，需要复制到新容器中
         * @note 不进行边界检查（在调试模式下使用断言）
         */
        [[nodiscard]] std::vector<Ty> col(index_type col) const {
            assert_bounds(col, cols_);
            std::vector<Ty> result;
            result.reserve(static_cast<size_type>(rows_));

            for (index_type i = 0; i < rows_; ++i) {
                result.push_back(data_[calculate_offset(i, col)]);
            }
            return result;
        }

        /**
         * @brief 获取子矩阵的行优先span视图
         *
         * @param start_row 起始行索引
         * @param start_col 起始列索引
         * @param num_rows 行数
         * @param num_cols 列数
         * @return 子矩阵的span视图
         *
         * @note 只有当请求的是连续的完整行时才返回完整的连续span
         * @note 否则只返回第一行的span
         * @note 不进行边界检查（在调试模式下使用断言）
         */
        [[nodiscard]] std::span<Ty> submatrix_row_major(
                index_type start_row, index_type start_col,
                index_type num_rows, index_type num_cols) noexcept {

            assert_bounds(start_row, rows_);
            assert_bounds(start_col, cols_);
            assert_bounds(start_row + num_rows - 1, rows_);
            assert_bounds(start_col + num_cols - 1, cols_);

            // 只有当请求的是连续行且占满整行时才能返回连续span
            if (start_col == 0 && num_cols == cols_) {
                return {data_.data() + calculate_offset(start_row, 0),
                        static_cast<size_type>(num_rows * cols_)};
            }

            // 否则只能返回第一行
            return {data_.data() + calculate_offset(start_row, start_col),
                    static_cast<size_type>(num_cols)};
        }

        // ================================
        // 数据操作和填充
        // ================================

        /**
         * @brief 重置矩阵内容
         *
         * @param opt 重置选项，默认为All_bits0
         *
         * @note 对POD类型使用高效的内存操作
         * @note 对非POD类型使用标准算法
         * @note 该操作不会抛出异常
         */
        void reset(Array_reset_opt opt = Array_reset_opt::All_bits0) noexcept {
            if (data_.empty()) return;

            if constexpr (std::is_trivially_destructible_v<Ty> &&
                          std::is_trivially_default_constructible_v<Ty> &&
                          std::is_standard_layout_v<Ty>) {

                // 对于POD类型，使用更高效的内存操作
                if (opt == Array_reset_opt::All_bits0) {
                    // 使用特殊优化的零初始化
                    if constexpr (sizeof(Ty) % sizeof(std::uint64_t) == 0) {
                        // 8字节对齐的快速清零
                        const auto count = data_.size() * sizeof(Ty) / sizeof(std::uint64_t);
                        auto      *ptr   = reinterpret_cast<std::uint64_t *>(data_.data());
                        std::fill_n(ptr, count, 0ULL);
                    } else {
                        std::memset(data_.data(), 0, data_.size() * sizeof(Ty));
                    }
                } else {
                    std::memset(data_.data(), static_cast<int>(opt), data_.size() * sizeof(Ty));
                }
            } else {
                // 对于非POD类型，使用标准算法
                if constexpr (std::is_nothrow_default_constructible_v<Ty>) {
                    std::fill(data_.begin(), data_.end(), Ty{});
                } else {
                    // 异常安全版本
                    for (auto &element: data_) {
                        element = Ty{};
                    }
                }
            }
        }

        /**
         * @brief 用指定值填充整个矩阵
         *
         * @param val 用于填充的值
         *
         * @note 对单字节POD类型使用memset优化
         * @note 对其他类型使用std::fill
         * @note 异常安全性取决于元素类型的赋值操作
         */
        void fill(const Ty &val) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
            if constexpr (std::is_trivially_copyable_v<Ty> && sizeof(Ty) == 1) {
                // 对于单字节POD类型的优化
                std::memset(data_.data(), *reinterpret_cast<const unsigned char *>(&val), data_.size());
            } else {
                std::fill(data_.begin(), data_.end(), val);
            }
        }

        /**
         * @brief 并行填充矩阵
         *
         * @param val 用于填充的值
         *
         * @note 只对大矩阵（>10000元素）使用并行算法
         * @note 小矩阵使用普通fill以避免并行开销
         * @note 异常安全性取决于元素类型的赋值操作
         */
        void fill_parallel(const Ty &val) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
            if (data_.size() > 10000) {  // 只对大数组使用并行
                std::fill(std::execution::par_unseq, data_.begin(), data_.end(), val);
            } else {
                fill(val);
            }
        }

        // ================================
        // 行操作
        // ================================

        /**
         * @brief 复制一行到另一行
         *
         * @param src_row 源行索引
         * @param dest_row 目标行索引
         *
         * @note 如果源行和目标行相同，则不执行任何操作
         * @note 对POD类型使用memcpy优化
         * @note 不进行边界检查（在调试模式下使用断言）
         * @note 异常安全性取决于元素类型的赋值操作
         */
        void copy_row(index_type src_row, index_type dest_row) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {

            assert_bounds(src_row, rows_);
            assert_bounds(dest_row, rows_);

            if (src_row == dest_row) return;

            const auto src_offset  = calculate_offset(src_row, 0);
            const auto dest_offset = calculate_offset(dest_row, 0);
            const auto copy_size   = static_cast<size_type>(cols_);

            if constexpr (std::is_trivially_copyable_v<Ty>) {
                std::memcpy(data_.data() + dest_offset,
                            data_.data() + src_offset,
                            copy_size * sizeof(Ty));
            } else {
                std::copy_n(data_.data() + src_offset, copy_size, data_.data() + dest_offset);
            }
        }

        /**
         * @brief 交换两行
         *
         * @param row1 第一行索引
         * @param row2 第二行索引
         *
         * @note 如果两行索引相同，则不执行任何操作
         * @note 不进行边界检查（在调试模式下使用断言）
         * @note 异常安全性取决于元素类型的交换操作
         */
        void swap_rows(index_type row1, index_type row2) noexcept(std::is_nothrow_swappable_v<Ty>) {

            assert_bounds(row1, rows_);
            assert_bounds(row2, rows_);

            if (row1 == row2) return;

            const auto offset1   = calculate_offset(row1, 0);
            const auto offset2   = calculate_offset(row2, 0);
            const auto swap_size = static_cast<size_type>(cols_);

            for (size_type i = 0; i < swap_size; ++i) {
                using std::swap;
                swap(data_[offset1 + i], data_[offset2 + i]);
            }
        }

        /**
         * @brief 用指定值填充一行
         *
         * @param row 行索引
         * @param val 用于填充的值
         *
         * @note 不进行边界检查（在调试模式下使用断言）
         * @note 异常安全性取决于元素类型的赋值操作
         */
        void fill_row(index_type row, const Ty &val) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {

            assert_bounds(row, rows_);
            const auto offset = calculate_offset(row, 0);
            std::fill_n(data_.data() + offset, cols_, val);
        }

        // ================================
        // 矩阵变换操作
        // ================================

        /**
         * @brief 就地转置矩阵
         *
         * @throws std::invalid_argument 当矩阵不是方阵时
         *
         * @note 只支持方阵的就地转置
         * @note 使用缓存友好的分块算法
         * @note 对于非方阵，请使用transposed()方法
         */
        void transpose() {
            if (!is_square()) {
                throw std::invalid_argument("transpose: matrix must be square for in-place transpose");
            }

            // 缓存友好的分块转置
            constexpr size_type block_size = 64 / sizeof(Ty);  // 64字节缓存行

            for (index_type i = 0; i < rows_; i += block_size) {
                const auto i_end = std::min(i + static_cast<index_type>(block_size), rows_);

                for (index_type j = i; j < cols_; j += block_size) {
                    const auto j_end = std::min(j + static_cast<index_type>(block_size), cols_);

                    // 转置块内元素
                    for (index_type bi = i; bi < i_end; ++bi) {
                        const auto start_j = (i == j) ? bi + 1 : j;  // 避免重复交换对角线元素
                        for (index_type bj = start_j; bj < j_end; ++bj) {
                            using std::swap;
                            swap((*this)[bi][bj], (*this)[bj][bi]);
                        }
                    }
                }
            }
        }

        /**
         * @brief 返回转置后的新矩阵
         *
         * @return 转置后的矩阵
         *
         * @note 支持任意尺寸的矩阵
         * @note 使用缓存友好的分块算法
         * @note 不修改原矩阵
         */
        [[nodiscard]] array2d transposed() const {
            array2d result(cols_, rows_);

            // 缓存友好的转置
            constexpr size_type block_size = 64 / sizeof(Ty);

            for (index_type i = 0; i < rows_; i += block_size) {
                const auto i_end = std::min(i + static_cast<index_type>(block_size), rows_);

                for (index_type j = 0; j < cols_; j += block_size) {
                    const auto j_end = std::min(j + static_cast<index_type>(block_size), cols_);

                    for (index_type bi = i; bi < i_end; ++bi) {
                        for (index_type bj = j; bj < j_end; ++bj) {
                            result[bj][bi] = (*this)[bi][bj];
                        }
                    }
                }
            }

            return result;
        }

        // ================================
        // 矩阵尺寸调整
        // ================================

        /**
         * @brief 调整矩阵尺寸
         *
         * @param new_rows 新的行数
         * @param new_cols 新的列数
         *
         * @throws std::invalid_argument 当行数或列数为负时
         * @throws std::overflow_error 当计算总大小溢出时
         * @throws std::bad_alloc 当内存分配失败时
         *
         * @note 保留原有数据（在新尺寸范围内）
         * @note 新增元素使用默认构造
         */
        void resize(index_type new_rows, index_type new_cols) {
            resize_impl(new_rows, new_cols, std::nullopt);
        }

        /**
         * @brief 调整矩阵尺寸并指定新元素的值
         *
         * @param new_rows 新的行数
         * @param new_cols 新的列数
         * @param val 新增元素的初始值
         *
         * @throws std::invalid_argument 当行数或列数为负时
         * @throws std::overflow_error 当计算总大小溢出时
         * @throws std::bad_alloc 当内存分配失败时
         *
         * @note 保留原有数据（在新尺寸范围内）
         * @note 新增元素使用指定值初始化
         */
        void resize(index_type new_rows, index_type new_cols, const Ty &val) {
            resize_impl(new_rows, new_cols, val);
        }

        // ================================
        // 数据访问和实用方法
        // ================================

        /**
         * @brief 获取底层数据指针
         * @return 指向底层数据数组的指针
         * @note 数据按行优先顺序存储
         */
        [[nodiscard]] constexpr pointer data() noexcept { return data_.data(); }

        /**
         * @brief 获取底层数据常量指针
         * @return 指向底层数据数组的常量指针
         * @note 数据按行优先顺序存储
         */
        [[nodiscard]] constexpr const_pointer data() const noexcept { return data_.data(); }

        /**
         * @brief 与另一个矩阵交换内容
         *
         * @param other 要交换的矩阵
         *
         * @note 该操作不会抛出异常
         * @note 交换后两个矩阵的内容完全互换
         */
        void swap(array2d &other) noexcept {
            using std::swap;
            swap(rows_, other.rows_);
            swap(cols_, other.cols_);
            swap(data_, other.data_);
        }

        /**
         * @brief 获取底层数据容器的常量引用
         * @return 底层std::vector的常量引用
         * @note 主要用于调试和高级操作
         */
        [[nodiscard]] const auto &get_data() const noexcept { return data_; }

        /**
         * @brief 获取底层数据容器的引用
         * @return 底层std::vector的引用
         * @note 主要用于调试和高级操作
         * @warning 直接修改底层vector可能导致数据不一致
         */
        [[nodiscard]] auto &get_vector() noexcept { return data_; }

        // ================================
        // 比较操作符
        // ================================

        /**
         * @brief 相等比较操作符
         *
         * @param other 要比较的矩阵
         * @return 如果两个矩阵尺寸相同且对应元素相等则返回true
         *
         * @note 异常安全性取决于元素类型的比较操作
         */
        [[nodiscard]] bool operator==(const array2d &other) const
                noexcept(noexcept(std::declval<Ty>() == std::declval<Ty>())) {

            return rows_ == other.rows_ &&
                   cols_ == other.cols_ &&
                   data_ == other.data_;
        }

        /**
         * @brief 三路比较操作符
         *
         * @param other 要比较的矩阵
         * @return 比较结果
         *
         * @note 首先比较行数，然后列数，最后比较数据
         * @note 要求元素类型支持三路比较
         */
        [[nodiscard]] auto operator<=>(const array2d &other) const
            requires std::three_way_comparable<Ty>
        {

            if (auto cmp = rows_ <=> other.rows_; cmp != 0) return cmp;
            if (auto cmp = cols_ <=> other.cols_; cmp != 0) return cmp;
            return data_ <=> other.data_;
        }

    private:
        // ================================
        // 私有辅助方法
        // ================================

        /**
         * @brief 验证维度参数的有效性
         *
         * @param dim 要验证的维度值
         * @param name 维度名称（用于错误消息）
         * @return 验证后的维度值
         *
         * @throws std::invalid_argument 当维度为负时
         */
        static constexpr index_type validate_dimension(index_type dim, const char *name) {
            if (dim < 0) [[unlikely]] {
                throw std::invalid_argument(std::string(name) + " must be non-negative");
            }
            return dim;
        }

        /**
         * @brief 计算矩阵总大小并检查溢出
         *
         * @param rows 行数
         * @param cols 列数
         * @return 总元素数
         *
         * @throws std::overflow_error 当大小计算溢出时
         */
        static constexpr size_type calculate_size(index_type rows, index_type cols) {
            const auto size = static_cast<size_type>(rows) * static_cast<size_type>(cols);

            // 检查溢出
            if (rows > 0 && cols > 0 && size / static_cast<size_type>(rows) != static_cast<size_type>(cols)) {
                throw std::overflow_error("Matrix size calculation overflow");
            }

            return size;
        }

        /**
         * @brief 计算二维索引对应的一维偏移量
         *
         * @param row 行索引
         * @param col 列索引
         * @return 在一维数组中的偏移量
         *
         * @note 使用行优先存储顺序
         */
        constexpr size_type calculate_offset(index_type row, index_type col) const noexcept {
            return static_cast<size_type>(row) * static_cast<size_type>(cols_) + static_cast<size_type>(col);
        }

        /**
         * @brief 调试模式下的边界检查断言
         *
         * @param index 要检查的索引
         * @param limit 索引的上限（不包含）
         *
         * @note 只在调试模式下生效
         */
        static constexpr void assert_bounds([[maybe_unused]] index_type index,
                                            [[maybe_unused]] index_type limit) noexcept {
#ifdef _DEBUG
            assert(index >= 0 && index < limit);
#endif
        }

        /**
         * @brief 格式化边界错误消息
         *
         * @param row 行索引
         * @param col 列索引
         * @return 格式化的错误消息字符串
         */
        std::string format_bounds_error(index_type row, index_type col) const {
            return "array2d: index (" + std::to_string(row) + ", " + std::to_string(col) +
                   ") out of range [0, " + std::to_string(rows_) + ") x [0, " + std::to_string(cols_) + ")";
        }

        /**
         * @brief at方法的模板化实现
         *
         * @tparam IsConst 是否为常量版本
         * @param row 行索引
         * @param col 列索引
         * @return 元素的引用或常量引用
         *
         * @throws std::out_of_range 当索引超出边界时
         */
        template<bool IsConst>
        [[nodiscard]] auto at_impl(index_type row, index_type col) const
                -> std::conditional_t<IsConst, const_reference, reference> {

            if (row < 0 || row >= rows_ || col < 0 || col >= cols_) [[unlikely]] {
                throw std::out_of_range(format_bounds_error(row, col));
            }

            if constexpr (IsConst) {
                return data_[calculate_offset(row, col)];
            } else {
                return const_cast<reference>(data_[calculate_offset(row, col)]);
            }
        }

        /**
         * @brief resize操作的内部实现
         *
         * @param new_rows 新的行数
         * @param new_cols 新的列数
         * @param fill_value 新元素的填充值（可选）
         *
         * @throws std::invalid_argument 当行数或列数为负时
         * @throws std::overflow_error 当计算总大小溢出时
         * @throws std::bad_alloc 当内存分配失败时
         *
         * @note 保留原有数据（在新尺寸范围内）
         * @note 使用原子更新确保异常安全
         */
        void resize_impl(index_type new_rows, index_type new_cols, std::optional<Ty> fill_value) {
            new_rows = validate_dimension(new_rows, "new_rows");
            new_cols = validate_dimension(new_cols, "new_cols");

            if (new_rows == rows_ && new_cols == cols_) return;

            const auto new_size = calculate_size(new_rows, new_cols);

            if (new_size == 0) {
                data_.clear();
                rows_ = new_rows;
                cols_ = new_cols;
                return;
            }

            // 创建新的数据向量
            std::vector<Ty> new_data;
            if (fill_value) {
                new_data.assign(new_size, *fill_value);
            } else {
                new_data.resize(new_size);
            }

            // 拷贝现有数据
            if (rows_ > 0 && cols_ > 0) {
                const auto copy_rows = std::min(rows_, new_rows);
                const auto copy_cols = std::min(cols_, new_cols);

                for (index_type i = 0; i < copy_rows; ++i) {
                    const auto old_offset = static_cast<size_type>(i) * static_cast<size_type>(cols_);
                    const auto new_offset = static_cast<size_type>(i) * static_cast<size_type>(new_cols);

                    if constexpr (std::is_trivially_copyable_v<Ty>) {
                        std::memcpy(new_data.data() + new_offset,
                                    data_.data() + old_offset,
                                    static_cast<size_type>(copy_cols) * sizeof(Ty));
                    } else {
                        std::copy_n(data_.data() + old_offset, copy_cols, new_data.data() + new_offset);
                    }
                }
            }

            // 原子更新
            data_ = std::move(new_data);
            rows_ = new_rows;
            cols_ = new_cols;
        }

    protected:
        index_type      rows_{}; /**< 矩阵行数 */
        index_type      cols_{}; /**< 矩阵列数 */
        std::vector<Ty> data_;   /**< 底层数据存储，按行优先顺序 */
    };

    // ================================
    // 非成员函数
    // ================================

    /**
     * @brief 交换两个矩阵的内容
     *
     * @tparam Ty 元素类型
     * @tparam Idx 索引类型
     * @param lhs 第一个矩阵
     * @param rhs 第二个矩阵
     *
     * @note 该操作不会抛出异常
     */
    template<Array2d_compatible Ty, Array2d_index_type Idx>
    void swap(array2d<Ty, Idx> &lhs, array2d<Ty, Idx> &rhs) noexcept {
        lhs.swap(rhs);
    }

    // ================================
    // 类模板参数推导指引
    // ================================

    /**
     * @brief 基础推导指引 - 从尺寸参数推导
     *
     * 从行数和列数推导出元素类型为double的矩阵。
     */
    template<Array2d_index_type IndexType>
    array2d(IndexType, IndexType) -> array2d<double, IndexType>;

    /**
     * @brief 基础推导指引 - 从尺寸和初值推导
     *
     * 从行数、列数和初始值推导出矩阵类型。
     */
    template<Array2d_index_type IndexType, Array2d_compatible ValueType>
    array2d(IndexType, IndexType, const ValueType &) -> array2d<ValueType, IndexType>;

    /**
     * @brief 初始化列表推导指引
     *
     * 从嵌套初始化列表推导出矩阵类型。
     */
    template<Array2d_compatible ValueType>
    array2d(std::initializer_list<std::initializer_list<ValueType>>) -> array2d<ValueType>;

    /**
     * @brief 容器推导指引
     *
     * 从容器类型推导出矩阵的元素类型。
     */
    template<Array2d_index_type IndexType, typename Container>
        requires std::ranges::range<Container>
    array2d(IndexType, IndexType, const Container &) -> array2d<std::ranges::range_value_t<Container>, IndexType>;

    /**
     * @brief 拷贝推导指引
     *
     * 从现有矩阵推导出相同类型的矩阵。
     */
    template<Array2d_compatible ValueType, Array2d_index_type IndexType>
    array2d(const array2d<ValueType, IndexType> &) -> array2d<ValueType, IndexType>;

    /**
     * @brief 移动推导指引
     *
     * 从现有矩阵推导出相同类型的矩阵。
     */
    template<Array2d_compatible ValueType, Array2d_index_type IndexType>
    array2d(array2d<ValueType, IndexType> &&) -> array2d<ValueType, IndexType>;

}  // namespace qm

#endif  // QM_ARRAY_RESET_OPT_DEFINED