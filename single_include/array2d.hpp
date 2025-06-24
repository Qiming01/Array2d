#pragma once
#include <algorithm>
#include <bit>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <execution>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>
#ifndef QM_FORCEINLINE_DEFINED
#define QM_FORCEINLINE_DEFINED
#if defined(_MSC_VER)
#define QM_FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define QM_FORCEINLINE __attribute__((always_inline)) inline
#elif defined(__INTEL_COMPILER)
#define QM_FORCEINLINE __forceinline
#else
#define QM_FORCEINLINE inline
#endif
#endif
namespace qm {
    template<typename T>
    concept Array2d_iterator_compatible =
            std::is_object_v<T> &&
            !std::is_abstract_v<T> &&
            requires { sizeof(T); };
    template<Array2d_iterator_compatible T>
    class Array2d_iterator {
    public:
        using iterator_category               = std::contiguous_iterator_tag;
        using iterator_concept                = std::contiguous_iterator_tag;
        using value_type                      = std::remove_cv_t<T>;
        using difference_type                 = std::ptrdiff_t;
        using pointer                         = T *;
        using reference                       = T &;
        constexpr Array2d_iterator() noexcept = default;
        constexpr explicit Array2d_iterator(pointer ptr) noexcept : ptr_(ptr) {}
        template<Array2d_iterator_compatible U>
        constexpr explicit Array2d_iterator(const Array2d_iterator<U> &other) noexcept
            requires std::convertible_to<U *, T *>
            : ptr_(other.data()) {}
        [[nodiscard]] QM_FORCEINLINE constexpr reference operator*() const noexcept {
            return *ptr_;
        }
        [[nodiscard]] QM_FORCEINLINE constexpr pointer operator->() const noexcept {
            return ptr_;
        }
        QM_FORCEINLINE constexpr Array2d_iterator &operator++() noexcept {
            ++ptr_;
            return *this;
        }
        QM_FORCEINLINE constexpr Array2d_iterator operator++(int) noexcept {
            return Array2d_iterator(ptr_++);
        }
        QM_FORCEINLINE constexpr Array2d_iterator &operator--() noexcept {
            --ptr_;
            return *this;
        }
        QM_FORCEINLINE constexpr Array2d_iterator operator--(int) noexcept {
            return Array2d_iterator(ptr_--);
        }
        constexpr auto operator+=(const difference_type offset) noexcept -> Array2d_iterator & {
            ptr_ += offset;
            return *this;
        }
        [[nodiscard]] constexpr auto operator+(const difference_type offset) const noexcept -> Array2d_iterator {
            return Array2d_iterator(ptr_ + offset);
        }
        constexpr auto operator-=(const difference_type offset) noexcept -> Array2d_iterator & {
            ptr_ -= offset;
            return *this;
        }
        [[nodiscard]] constexpr auto operator-(const difference_type offset) const noexcept -> Array2d_iterator {
            return Array2d_iterator(ptr_ - offset);
        }
        [[nodiscard]] constexpr auto operator-(const Array2d_iterator &other) const noexcept -> difference_type {
            return ptr_ - other.ptr_;
        }
        template<Array2d_iterator_compatible U>
        [[nodiscard]] constexpr auto operator-(const Array2d_iterator<U> &other) const noexcept -> difference_type
            requires std::convertible_to<U *, T *> || std::convertible_to<T *, U *>
        {
            return ptr_ - other.data();
        }
        [[nodiscard]] constexpr auto operator[](const difference_type offset) const noexcept -> reference {
            return ptr_[offset];
        }
        [[nodiscard]] constexpr std::strong_ordering operator<=>(const Array2d_iterator &other) const noexcept {
            return ptr_ <=> other.ptr_;
        }
        template<Array2d_iterator_compatible U>
        [[nodiscard]] constexpr auto operator<=>(const Array2d_iterator<U> &other) const noexcept
            requires std::three_way_comparable_with<T *, U *>
        {
            return ptr_ <=> other.data();
        }
        [[nodiscard]] QM_FORCEINLINE constexpr bool operator==(const Array2d_iterator &other) const noexcept {
            return ptr_ == other.ptr_;
        }
        template<Array2d_iterator_compatible U>
        [[nodiscard]] constexpr auto operator==(const Array2d_iterator<U> &other) const noexcept -> bool
            requires std::equality_comparable_with<T *, U *>
        {
            return ptr_ == other.data();
        }
        [[nodiscard]] constexpr auto data() const noexcept -> pointer {
            return ptr_;
        }

    private:
        pointer ptr_ = nullptr;
    };
    template<Array2d_iterator_compatible T>
    [[nodiscard]] constexpr Array2d_iterator<T> operator+(
            typename Array2d_iterator<T>::difference_type offset,
            const Array2d_iterator<T>                    &iter) noexcept {
        return iter + offset;
    }
    template<typename T>
    struct is_array2d_iterator : std::false_type {};
    template<Array2d_iterator_compatible T>
    struct is_array2d_iterator<Array2d_iterator<T>> : std::true_type {};
    template<typename T>
    inline constexpr bool is_array2d_iterator_v = is_array2d_iterator<T>::value;
    template<typename T>
    struct array2d_iterator_traits {};
    template<Array2d_iterator_compatible T>
    struct array2d_iterator_traits<Array2d_iterator<T>> {
        using value_type        = typename Array2d_iterator<T>::value_type;
        using pointer           = typename Array2d_iterator<T>::pointer;
        using reference         = typename Array2d_iterator<T>::reference;
        using difference_type   = typename Array2d_iterator<T>::difference_type;
        using iterator_category = typename Array2d_iterator<T>::iterator_category;
    };
    template<typename T>
    using array2d_iterator = Array2d_iterator<T>;
    template<typename T>
    using array2d_const_iterator = Array2d_iterator<const T>;
}  // namespace qm
namespace std {
    template<qm::Array2d_iterator_compatible T>
    struct iterator_traits<qm::Array2d_iterator<T>> {
        using iterator_category = std::contiguous_iterator_tag;
        using iterator_concept  = std::contiguous_iterator_tag;
        using value_type        = std::remove_cv_t<T>;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;
    };
    template<qm::Array2d_iterator_compatible T>
    struct pointer_traits<qm::Array2d_iterator<T>> {
        using pointer         = qm::Array2d_iterator<T>;
        using element_type    = T;
        using difference_type = std::ptrdiff_t;
        static constexpr pointer pointer_to(element_type &r) noexcept {
            return qm::Array2d_iterator<T>(std::addressof(r));
        }
    };
}  // namespace std
#ifndef QM_ARRAY_RESET_OPT_DEFINED
#define QM_ARRAY_RESET_OPT_DEFINED
namespace qm {
    enum class Array_reset_opt : std::int8_t {
        All_bits0 = 0,
        All_bits1 = -1,
        Safe_max  = 0x3F
    };
    template<typename T>
    concept Array2d_compatible =
            std::is_object_v<T> &&
            !std::is_abstract_v<T> &&
            requires { sizeof(T); };
    template<typename Idx>
    concept Array2d_index_type =
            std::is_integral_v<Idx> &&
            !std::is_same_v<Idx, bool> &&
            !std::is_same_v<Idx, char>;
    template<Array2d_compatible Ty, Array2d_index_type Idx = int>
    class array2d {
    public:
        using value_type             = Ty;
        using index_type             = Idx;
        using size_type              = std::make_unsigned_t<Idx>;
        using difference_type        = std::make_signed_t<Idx>;
        using pointer                = Ty *;
        using const_pointer          = const Ty *;
        using reference              = Ty &;
        using const_reference        = const Ty &;
        using iterator               = Array2d_iterator<Ty>;
        using const_iterator         = Array2d_iterator<const Ty>;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        constexpr array2d() noexcept = default;
        array2d(index_type rows, index_type cols)
            : rows_(validate_dimension(rows, "rows")),
              cols_(validate_dimension(cols, "cols")) {
            if (rows_ > 0 && cols_ > 0) {
                const auto size = calculate_size(rows_, cols_);
                data_.resize(size);
            }
        }
        array2d(index_type rows, index_type cols, const Ty &val)
            : rows_(validate_dimension(rows, "rows")),
              cols_(validate_dimension(cols, "cols")) {
            if (rows_ > 0 && cols_ > 0) {
                const auto size = calculate_size(rows_, cols_);
                data_.resize(size, val);
            }
        }
        array2d(std::initializer_list<std::initializer_list<Ty>> init_list) {
            if (init_list.size() == 0) {
                rows_ = cols_ = 0;
                return;
            }
            rows_ = static_cast<index_type>(init_list.size());
            cols_ = static_cast<index_type>(init_list.begin()->size());
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
        array2d(const array2d &)                = default;
        array2d(array2d &&) noexcept            = default;
        array2d &operator=(const array2d &)     = default;
        array2d &operator=(array2d &&) noexcept = default;
        ~array2d()                              = default;
        [[nodiscard]] QM_FORCEINLINE pointer operator[](index_type row) noexcept {
            assert_bounds(row, rows_);
            return data_.data() + calculate_offset(row, 0);
        }
        [[nodiscard]] QM_FORCEINLINE const_pointer operator[](index_type row) const noexcept {
            assert_bounds(row, rows_);
            return data_.data() + calculate_offset(row, 0);
        }
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
        [[nodiscard]] reference at(index_type row, index_type col) {
            return at_impl<false>(row, col);
        }
        [[nodiscard]] const_reference at(index_type row, index_type col) const {
            return at_impl<true>(row, col);
        }
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
        [[nodiscard]] constexpr const_iterator   cbegin() const noexcept { return begin(); }
        [[nodiscard]] constexpr const_iterator   cend() const noexcept { return end(); }
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
        [[nodiscard]] constexpr index_type rows() const noexcept { return rows_; }
        [[nodiscard]] constexpr index_type cols() const noexcept { return cols_; }
        [[nodiscard]] constexpr size_type  size() const noexcept { return data_.size(); }
        [[nodiscard]] constexpr bool       empty() const noexcept { return data_.empty(); }
        [[nodiscard]] constexpr size_type  capacity() const noexcept { return data_.capacity(); }
        [[nodiscard]] constexpr bool       is_square() const noexcept { return rows_ == cols_; }
        void                               reserve(index_type rows, index_type cols) {
            const auto new_capacity = calculate_size(
                    validate_dimension(rows, "rows"),
                    validate_dimension(cols, "cols"));
            data_.reserve(new_capacity);
        }
        void shrink_to_fit() {
            data_.shrink_to_fit();
        }
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
        [[nodiscard]] std::vector<Ty> col(index_type col) const {
            assert_bounds(col, cols_);
            std::vector<Ty> result;
            result.reserve(static_cast<size_type>(rows_));
            for (index_type i = 0; i < rows_; ++i) {
                result.push_back(data_[calculate_offset(i, col)]);
            }
            return result;
        }
        [[nodiscard]] std::span<Ty> submatrix_row_major(
                index_type start_row, index_type start_col,
                index_type num_rows, index_type num_cols) noexcept {
            assert_bounds(start_row, rows_);
            assert_bounds(start_col, cols_);
            assert_bounds(start_row + num_rows - 1, rows_);
            assert_bounds(start_col + num_cols - 1, cols_);
            if (start_col == 0 && num_cols == cols_) {
                return {data_.data() + calculate_offset(start_row, 0),
                        static_cast<size_type>(num_rows * cols_)};
            }
            return {data_.data() + calculate_offset(start_row, start_col),
                    static_cast<size_type>(num_cols)};
        }
        void reset(Array_reset_opt opt = Array_reset_opt::All_bits0) noexcept {
            if (data_.empty()) return;
            if constexpr (std::is_trivially_destructible_v<Ty> &&
                          std::is_trivially_default_constructible_v<Ty> &&
                          std::is_standard_layout_v<Ty>) {
                if (opt == Array_reset_opt::All_bits0) {
                    if constexpr (sizeof(Ty) % sizeof(std::uint64_t) == 0) {
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
                if constexpr (std::is_nothrow_default_constructible_v<Ty>) {
                    std::fill(data_.begin(), data_.end(), Ty{});
                } else {
                    for (auto &element: data_) {
                        element = Ty{};
                    }
                }
            }
        }
        void fill(const Ty &val) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
            if constexpr (std::is_trivially_copyable_v<Ty> && sizeof(Ty) == 1) {
                std::memset(data_.data(), *reinterpret_cast<const unsigned char *>(&val), data_.size());
            } else {
                std::fill(data_.begin(), data_.end(), val);
            }
        }
        void fill_parallel(const Ty &val) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
            if (data_.size() > 10000) {
                std::fill(std::execution::par_unseq, data_.begin(), data_.end(), val);
            } else {
                fill(val);
            }
        }
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
        void fill_row(index_type row, const Ty &val) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
            assert_bounds(row, rows_);
            const auto offset = calculate_offset(row, 0);
            std::fill_n(data_.data() + offset, cols_, val);
        }
        void transpose() {
            if (!is_square()) {
                throw std::invalid_argument("transpose: matrix must be square for in-place transpose");
            }
            constexpr size_type block_size = 64 / sizeof(Ty);
            for (index_type i = 0; i < rows_; i += block_size) {
                const auto i_end = std::min(i + static_cast<index_type>(block_size), rows_);
                for (index_type j = i; j < cols_; j += block_size) {
                    const auto j_end = std::min(j + static_cast<index_type>(block_size), cols_);
                    for (index_type bi = i; bi < i_end; ++bi) {
                        const auto start_j = (i == j) ? bi + 1 : j;
                        for (index_type bj = start_j; bj < j_end; ++bj) {
                            using std::swap;
                            swap((*this)[bi][bj], (*this)[bj][bi]);
                        }
                    }
                }
            }
        }
        [[nodiscard]] array2d transposed() const {
            array2d             result(cols_, rows_);
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
        void resize(index_type new_rows, index_type new_cols) {
            resize_impl(new_rows, new_cols, std::nullopt);
        }
        void resize(index_type new_rows, index_type new_cols, const Ty &val) {
            resize_impl(new_rows, new_cols, val);
        }
        [[nodiscard]] constexpr pointer       data() noexcept { return data_.data(); }
        [[nodiscard]] constexpr const_pointer data() const noexcept { return data_.data(); }
        void                                  swap(array2d &other) noexcept {
            using std::swap;
            swap(rows_, other.rows_);
            swap(cols_, other.cols_);
            swap(data_, other.data_);
        }
        [[nodiscard]] const auto &get_data() const noexcept { return data_; }
        [[nodiscard]] auto       &get_vector() noexcept { return data_; }
        [[nodiscard]] bool        operator==(const array2d &other) const
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
        static constexpr index_type validate_dimension(index_type dim, const char *name) {
            if (dim < 0) [[unlikely]] {
                throw std::invalid_argument(std::string(name) + " must be non-negative");
            }
            return dim;
        }
        static constexpr size_type calculate_size(index_type rows, index_type cols) {
            const auto size = static_cast<size_type>(rows) * static_cast<size_type>(cols);
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
            std::vector<Ty> new_data;
            if (fill_value) {
                new_data.assign(new_size, *fill_value);
            } else {
                new_data.resize(new_size);
            }
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
            data_ = std::move(new_data);
            rows_ = new_rows;
            cols_ = new_cols;
        }

    protected:
        index_type      rows_{};
        index_type      cols_{};
        std::vector<Ty> data_;
    };
    template<Array2d_compatible Ty, Array2d_index_type Idx>
    void swap(array2d<Ty, Idx> &lhs, array2d<Ty, Idx> &rhs) noexcept {
        lhs.swap(rhs);
    }
    template<Array2d_index_type IndexType>
    array2d(IndexType, IndexType) -> array2d<double, IndexType>;
    template<Array2d_index_type IndexType, Array2d_compatible ValueType>
        requires(!std::ranges::range<ValueType> || std::is_arithmetic_v<ValueType>)
    array2d(IndexType, IndexType, const ValueType &) -> array2d<ValueType, IndexType>;
    template<Array2d_compatible ValueType>
    array2d(std::initializer_list<std::initializer_list<ValueType>>) -> array2d<ValueType>;
    template<Array2d_index_type IndexType, typename Container>
        requires std::ranges::range<Container> &&
                 (!std::is_arithmetic_v<Container>) &&
                 Array2d_compatible<std::ranges::range_value_t<Container>>
    array2d(IndexType, IndexType, const Container &) -> array2d<std::ranges::range_value_t<Container>, IndexType>;
    template<Array2d_compatible ValueType, Array2d_index_type IndexType>
    array2d(const array2d<ValueType, IndexType> &) -> array2d<ValueType, IndexType>;
    template<Array2d_compatible ValueType, Array2d_index_type IndexType>
    array2d(array2d<ValueType, IndexType> &&) -> array2d<ValueType, IndexType>;
}  // namespace qm
#endif