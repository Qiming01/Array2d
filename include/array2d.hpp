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
     * @brief 内存重置选项
     */
    enum class Array_reset_opt : std::int8_t {
        All_bits0 = 0,    ///< 将所有位设置为0
        All_bits1 = -1,   ///< 将所有位设置为1
        Safe_max  = 0x3F  ///< 安全的最大值模式
    };

    // ================================
    // 概念和类型特征
    // ================================
    template<typename T>
    concept Array2d_compatible =
            std::is_object_v<T> &&
            !std::is_abstract_v<T> && requires { sizeof(T); };

    template<typename Idx>
    concept Array2d_index_type =
            std::is_integral_v<Idx> &&
            !std::is_same_v<Idx, bool> &&
            !std::is_same_v<Idx, char>;

    // ================================
    // array2d 类
    // ================================

    template<Array2d_compatible Ty, Array2d_index_type Idx = int>
    class array2d {
    public:
        // 类型定义
        using value_type      = Ty;
        using index_type      = Idx;
        using size_type       = std::make_unsigned_t<Idx>;
        using difference_type = std::make_signed_t<Idx>;
        using pointer         = Ty *;
        using const_pointer   = const Ty *;
        using reference       = Ty &;
        using const_reference = const Ty &;

        using iterator               = Array2d_iterator<Ty>;
        using const_iterator         = Array2d_iterator<const Ty>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // ================================
        // 构造函数
        // ================================

        constexpr array2d() noexcept = default;

        // 添加尺寸验证和溢出检查
        array2d(index_type rows, index_type cols)
            : rows_(validate_dimension(rows, "rows")), cols_(validate_dimension(cols, "cols")) {

            if (rows_ > 0 && cols_ > 0) {
                const auto size = calculate_size(rows_, cols_);
                data_.resize(size);
            }
        }

        array2d(index_type rows, index_type cols, const Ty &val)
            : rows_(validate_dimension(rows, "rows")), cols_(validate_dimension(cols, "cols")) {

            if (rows_ > 0 && cols_ > 0) {
                const auto size = calculate_size(rows_, cols_);
                data_.resize(size, val);
            }
        }

        // 添加初始化列表构造函数
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

        // 添加从容器构造的函数
        template<typename Container>
        array2d(index_type rows, index_type cols, const Container &container)
            requires std::ranges::range<Container> &&
                             std::convertible_to<std::ranges::range_value_t<Container>, Ty>
            : rows_(validate_dimension(rows, "rows")), cols_(validate_dimension(cols, "cols")) {

            const auto expected_size = calculate_size(rows_, cols_);
            if (std::ranges::size(container) != expected_size) {
                throw std::invalid_argument("Container size doesn't match matrix dimensions");
            }

            data_.reserve(expected_size);
            std::ranges::copy(container, std::back_inserter(data_));
        }

        // 默认的拷贝/移动操作
        array2d(const array2d &) = default;

        array2d(array2d &&) noexcept = default;

        array2d &operator=(const array2d &) = default;

        array2d &operator=(array2d &&) noexcept = default;

        ~array2d() = default;

        // ================================
        // 元素访问
        // ================================

        [[nodiscard]] QM_FORCEINLINE pointer operator[](index_type row) noexcept {
            assert_bounds(row, rows_);
            return data_.data() + calculate_offset(row, 0);
        }

        [[nodiscard]] QM_FORCEINLINE const_pointer operator[](index_type row) const noexcept {
            assert_bounds(row, rows_);
            return data_.data() + calculate_offset(row, 0);
        }

        // 添加 operator() 重载
        [[nodiscard]] QM_FORCEINLINE reference operator()(index_type row, index_type col) noexcept {
            assert_bounds(row, rows_);
            assert_bounds(col, cols_);
            return data_[calculate_offset(row, col)];
        }

        [[nodiscard]] QM_FORCEINLINE const_reference operator()(index_type row, index_type col) const noexcept {
            assert_bounds(row, rows_);
            assert_bounds(col, cols_);
            return data_[calculate_offset(row, col)];
        }

        // 使用模板减少代码重复
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

        [[nodiscard]] reference at(index_type row, index_type col) {
            return at_impl<false>(row, col);
        }

        [[nodiscard]] const_reference at(index_type row, index_type col) const {
            return at_impl<true>(row, col);
        }

        // ================================
        // 迭代器
        // ================================

        [[nodiscard]] constexpr iterator begin() noexcept {
            return iterator(data_.data());
        }

        [[nodiscard]] constexpr const_iterator begin() const noexcept {
            return const_iterator(data_.data());
        }

        [[nodiscard]] constexpr iterator end() noexcept {
            return iterator(data_.data() + data_.size());
        }

        [[nodiscard]] constexpr const_iterator end() const noexcept {
            return const_iterator(data_.data() + data_.size());
        }

        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }

        [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

        // 反向迭代器
        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        }

        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        }

        [[nodiscard]] constexpr reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        }

        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        }

        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }

        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

        // 添加行迭代器
        struct row_iterator_wrapper {
            pointer   ptr_;
            size_type cols_;

            row_iterator_wrapper(pointer ptr, size_type cols) : ptr_(ptr), cols_(cols) {}

            [[nodiscard]] iterator begin() const noexcept { return iterator(ptr_); }

            [[nodiscard]] iterator end() const noexcept { return iterator(ptr_ + cols_); }
        };

        struct const_row_iterator_wrapper {
            const_pointer ptr_;
            size_type     cols_;

            const_row_iterator_wrapper(const_pointer ptr, size_type cols) : ptr_(ptr), cols_(cols) {}

            [[nodiscard]] const_iterator begin() const noexcept { return const_iterator(ptr_); }

            [[nodiscard]] const_iterator end() const noexcept { return const_iterator(ptr_ + cols_); }
        };

        [[nodiscard]] row_iterator_wrapper row_range(index_type row) noexcept {
            assert_bounds(row, rows_);
            return {data_.data() + calculate_offset(row, 0), static_cast<size_type>(cols_)};
        }

        [[nodiscard]] const_row_iterator_wrapper row_range(index_type row) const noexcept {
            assert_bounds(row, rows_);
            return {data_.data() + calculate_offset(row, 0), static_cast<size_type>(cols_)};
        }

        // ================================
        // 尺寸和容量
        // ================================

        [[nodiscard]] constexpr index_type rows() const noexcept { return rows_; }

        [[nodiscard]] constexpr index_type cols() const noexcept { return cols_; }

        [[nodiscard]] constexpr size_type size() const noexcept { return data_.size(); }

        [[nodiscard]] constexpr bool empty() const noexcept { return data_.empty(); }

        [[nodiscard]] constexpr size_type capacity() const noexcept { return data_.capacity(); }

        [[nodiscard]] constexpr bool is_square() const noexcept { return rows_ == cols_; }

        // 添加内存管理方法
        void reserve(index_type rows, index_type cols) {
            const auto new_capacity = calculate_size(
                    validate_dimension(rows, "rows"),
                    validate_dimension(cols, "cols"));
            data_.reserve(new_capacity);
        }

        void shrink_to_fit() {
            data_.shrink_to_fit();
        }

        // ================================
        // span 操作
        // ================================

        [[nodiscard]] constexpr std::span<Ty> as_span() noexcept {
            return {data_.data(), data_.size()};
        }

        [[nodiscard]] constexpr std::span<const Ty> as_span() const noexcept {
            return {data_.data(), data_.size()};
        }

        [[nodiscard]] constexpr std::span<Ty> row(index_type row) noexcept {
            assert_bounds(row, rows_);
            return {data_.data() + calculate_offset(row, 0), static_cast<size_type>(cols_)};
        }

        [[nodiscard]] constexpr std::span<const Ty> row(index_type row) const noexcept {
            assert_bounds(row, rows_);
            return {data_.data() + calculate_offset(row, 0), static_cast<size_type>(cols_)};
        }

        // 添加列提取（需要拷贝，因为不连续）
        [[nodiscard]] std::vector<Ty> col(index_type col) const {
            assert_bounds(col, cols_);
            std::vector<Ty> result;
            result.reserve(static_cast<size_type>(rows_));

            for (index_type i = 0; i < rows_; ++i) {
                result.push_back(data_[calculate_offset(i, col)]);
            }
            return result;
        }

        // 添加子矩阵视图
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
        // 数据操作
        // ================================

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

        void fill(const Ty &val) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
            if constexpr (std::is_trivially_copyable_v<Ty> && sizeof(Ty) == 1) {
                // 对于单字节POD类型的优化
                std::memset(data_.data(), *reinterpret_cast<const unsigned char *>(&val), data_.size());
            } else {
                std::fill(data_.begin(), data_.end(), val);
            }
        }

        // 添加并行fill
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

        // 添加行交换
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

        // 添加行填充
        void fill_row(index_type row, const Ty &val) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {

            assert_bounds(row, rows_);
            const auto offset = calculate_offset(row, 0);
            std::fill_n(data_.data() + offset, cols_, val);
        }

        // ================================
        // 转置
        // ================================

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

        // 添加非正方形矩阵转置（返回新矩阵）
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
        // resize
        // ================================

        void resize(index_type new_rows, index_type new_cols) {
            resize_impl(new_rows, new_cols, std::nullopt);
        }

        void resize(index_type new_rows, index_type new_cols, const Ty &val) {
            resize_impl(new_rows, new_cols, val);
        }

        // ================================
        // 数据访问和工具方法
        // ================================

        [[nodiscard]] constexpr pointer data() noexcept { return data_.data(); }

        [[nodiscard]] constexpr const_pointer data() const noexcept { return data_.data(); }

        void swap(array2d &other) noexcept {
            using std::swap;
            swap(rows_, other.rows_);
            swap(cols_, other.cols_);
            swap(data_, other.data_);
        }

        [[nodiscard]] const auto &get_data() const noexcept { return data_; }

        [[nodiscard]] auto &get_vector() noexcept { return data_; }

        // ================================
        // 比较操作符
        // ================================

        [[nodiscard]] bool operator==(const array2d &other) const
                noexcept(noexcept(std::declval<Ty>() == std::declval<Ty>())) {

            return rows_ == other.rows_ &&
                   cols_ == other.cols_ &&
                   data_ == other.data_;
        }

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

        static constexpr index_type validate_dimension(index_type dim, const char *name) {
            if (dim < 0) [[unlikely]] {
                throw std::invalid_argument(std::string(name) + " must be non-negative");
            }
            return dim;
        }

        static constexpr size_type calculate_size(index_type rows, index_type cols) {
            const auto size = static_cast<size_type>(rows) * static_cast<size_type>(cols);

            // 检查溢出
            if (rows > 0 && cols > 0 && size / static_cast<size_type>(rows) != static_cast<size_type>(cols)) {
                throw std::overflow_error("Matrix size calculation overflow");
            }

            return size;
        }

        constexpr size_type calculate_offset(index_type row, index_type col) const noexcept {
            return static_cast<size_type>(row) * static_cast<size_type>(cols_) + static_cast<size_type>(col);
        }

        static constexpr void assert_bounds([[maybe_unused]] index_type index,
                                            [[maybe_unused]] index_type limit) noexcept {
#ifdef _DEBUG
            assert(index >= 0 && index < limit);
#endif
        }

        std::string format_bounds_error(index_type row, index_type col) const {
            return "array2d: index (" + std::to_string(row) + ", " + std::to_string(col) +
                   ") out of range [0, " + std::to_string(rows_) + ") x [0, " + std::to_string(cols_) + ")";
        }

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
        index_type      rows_{};
        index_type      cols_{};
        std::vector<Ty> data_;
    };

    // ================================
    // 非成员函数
    // ================================

    template<Array2d_compatible Ty, Array2d_index_type Idx>
    void swap(array2d<Ty, Idx> &lhs, array2d<Ty, Idx> &rhs) noexcept {
        lhs.swap(rhs);
    }

    // 基础推导指引
    template<Array2d_index_type IndexType>
    array2d(IndexType, IndexType) -> array2d<double, IndexType>;

    template<Array2d_index_type IndexType, Array2d_compatible ValueType>
    array2d(IndexType, IndexType, const ValueType &) -> array2d<ValueType, IndexType>;

    // 初始化列表推导
    template<Array2d_compatible ValueType>
    array2d(std::initializer_list<std::initializer_list<ValueType>>) -> array2d<ValueType>;

    // 容器推导
    template<Array2d_index_type IndexType, typename Container>
        requires std::ranges::range<Container>
    array2d(IndexType, IndexType, const Container &) -> array2d<std::ranges::range_value_t<Container>, IndexType>;

    // 拷贝推导（虽然通常不需要，但为了完整性）
    template<Array2d_compatible ValueType, Array2d_index_type IndexType>
    array2d(const array2d<ValueType, IndexType> &) -> array2d<ValueType, IndexType>;

    // 移动推导
    template<Array2d_compatible ValueType, Array2d_index_type IndexType>
    array2d(array2d<ValueType, IndexType> &&) -> array2d<ValueType, IndexType>;

}  // namespace qm

#endif  // QM_ARRAY_RESET_OPT_DEFINED