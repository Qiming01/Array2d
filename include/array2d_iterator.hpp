#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <concepts>
#include <compare>

/**
 * @brief 强制内联宏优化
 */
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
#endif // QM_FORCEINLINE_DEFINED


namespace qm {

    /**
     * @brief 迭代器兼容性概念
     *
     * 确保类型适合用作二维数组迭代器的元素类型：
     * - 必须是对象类型（非函数、非引用）
     * - 不能是抽象类型
     * - 必须是完整类型
     */
    template<typename T>
    concept Array2d_iterator_compatible =
    std::is_object_v<T> &&
    !std::is_abstract_v<T> &&
    requires { sizeof(T); }; // 确保是完整类型

    /**
     * @brief 二维数组的连续内存迭代器
     *
     * @tparam T 元素类型，必须满足 Array2d_iterator_compatible 概念
     *
     * 提供符合 C++20 标准的连续迭代器，支持所有迭代器操作
     * 包括随机访问、算术运算、三路比较以及 const/non-const 类型转换
     *
     * @note 此迭代器专为连续内存存储的二维数组设计，提供最佳性能
     */
    template<Array2d_iterator_compatible T>
    class Array2d_iterator {
    public:
        // C++20 迭代器类型定义
        using iterator_category = std::contiguous_iterator_tag;
        using iterator_concept = std::contiguous_iterator_tag;
        using value_type = std::remove_cv_t<T>;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        /**
         * @brief 默认构造函数
         *
         * 创建一个空的迭代器，指针初始化为 nullptr
         */
        constexpr Array2d_iterator() noexcept = default;

        /**
         * @brief 从指针构造迭代器
         *
         * @param ptr 指向元素的指针
         *
         * @note 此构造函数为 explicit，防止意外的隐式转换
         */
        constexpr explicit Array2d_iterator(pointer ptr) noexcept : ptr_(ptr) {}

        /**
         * @brief 类型转换构造函数
         *
         * 支持 const/non-const 迭代器之间的转换，例如：
         * - iterator -> const_iterator (总是允许)
         * - const_iterator -> iterator (在适当条件下)
         *
         * @tparam U 源迭代器的元素类型
         * @param other 源迭代器
         */
        template<Array2d_iterator_compatible U>
        constexpr explicit Array2d_iterator(const Array2d_iterator<U>& other) noexcept
        requires std::convertible_to<U*, T*>
                : ptr_(other.data()) {}

        /**
         * @brief 解引用操作符
         *
         * @return 对当前元素的引用
         *
         * @pre 迭代器必须指向有效元素（非end迭代器）
         */
        [[nodiscard]] QM_FORCEINLINE constexpr reference operator*() const noexcept {
            return *ptr_;
        }

        /**
         * @brief 成员访问操作符
         *
         * @return 指向当前元素的指针
         *
         * @pre 迭代器必须指向有效元素（非end迭代器）
         */
        [[nodiscard]] QM_FORCEINLINE constexpr pointer operator->() const noexcept {
            return ptr_;
        }

        /**
         * @brief 前置递增操作符
         *
         * 将迭代器向前移动一个位置
         *
         * @return 递增后的迭代器引用
         */
        QM_FORCEINLINE constexpr Array2d_iterator& operator++() noexcept {
            ++ptr_;
            return *this;
        }

        /**
         * @brief 后置递增操作符
         *
         * 将迭代器向前移动一个位置，返回移动前的副本
         *
         * @return 递增前的迭代器副本
         */
        QM_FORCEINLINE constexpr Array2d_iterator operator++(int) noexcept {
            return Array2d_iterator(ptr_++);
        }

        /**
         * @brief 前置递减操作符
         *
         * 将迭代器向后移动一个位置
         *
         * @return 递减后的迭代器引用
         */
        QM_FORCEINLINE constexpr Array2d_iterator& operator--() noexcept {
            --ptr_;
            return *this;
        }

        /**
         * @brief 后置递减操作符
         *
         * 将迭代器向后移动一个位置，返回移动前的副本
         *
         * @return 递减前的迭代器副本
         */
        QM_FORCEINLINE constexpr Array2d_iterator operator--(int) noexcept {
            return Array2d_iterator(ptr_--);
        }

        /**
         * @brief 复合加法赋值操作符
         *
         * 将迭代器向前移动指定的偏移量
         *
         * @param offset 偏移量（可以为负数）
         * @return 操作后的迭代器引用
         */
        constexpr auto operator+=(const difference_type offset) noexcept -> Array2d_iterator& {
            ptr_ += offset;
            return *this;
        }

        /**
         * @brief 加法操作符
         *
         * 创建一个新的迭代器，位置为当前位置加上偏移量
         *
         * @param offset 偏移量（可以为负数）
         * @return 新的迭代器
         */
        [[nodiscard]] constexpr auto operator+(const difference_type offset) const noexcept -> Array2d_iterator {
            return Array2d_iterator(ptr_ + offset);
        }

        /**
         * @brief 复合减法赋值操作符
         *
         * 将迭代器向后移动指定的偏移量
         *
         * @param offset 偏移量（可以为负数）
         * @return 操作后的迭代器引用
         */
        constexpr auto operator-=(const difference_type offset) noexcept -> Array2d_iterator& {
            ptr_ -= offset;
            return *this;
        }

        /**
         * @brief 减法操作符
         *
         * 创建一个新的迭代器，位置为当前位置减去偏移量
         *
         * @param offset 偏移量（可以为负数）
         * @return 新的迭代器
         */
        [[nodiscard]] constexpr auto operator-(const difference_type offset) const noexcept -> Array2d_iterator {
            return Array2d_iterator(ptr_ - offset);
        }

        /**
         * @brief 迭代器距离计算
         *
         * 计算两个迭代器之间的距离（元素个数）
         *
         * @param other 另一个迭代器
         * @return 两个迭代器之间的距离
         *
         * @note 如果 other 在当前迭代器之前，结果为负数
         */
        [[nodiscard]] constexpr auto operator-(const Array2d_iterator& other) const noexcept -> difference_type {
            return ptr_ - other.ptr_;
        }

        /**
         * @brief 跨类型迭代器距离计算
         *
         * @tparam U 另一个迭代器的元素类型
         * @param other 另一个迭代器
         * @return 两个迭代器之间的距离
         */
        template<Array2d_iterator_compatible U>
        [[nodiscard]] constexpr auto operator-(const Array2d_iterator<U>& other) const noexcept -> difference_type
        requires std::convertible_to<U*, T*> || std::convertible_to<T*, U*>
        {
            return ptr_ - other.data();
        }

        /**
         * @brief 下标访问操作符
         *
         * 访问相对于当前位置偏移指定距离的元素
         *
         * @param offset 偏移量（可以为负数）
         * @return 指定位置元素的引用
         *
         * @note 等价于 *((*this) + offset)
         */
        [[nodiscard]] constexpr auto operator[](const difference_type offset) const noexcept -> reference {
            return ptr_[offset];
        }

        /**
         * @brief C++20 三路比较操作符
         *
         * 提供完整的比较功能，支持 <、<=、>、>=、== 和 != 操作
         *
         * @param other 另一个相同类型的迭代器
         * @return 比较结果（std::strong_ordering）
         */
        [[nodiscard]] constexpr std::strong_ordering operator<=>(const Array2d_iterator& other) const noexcept  {
            return ptr_ <=> other.ptr_;
        }

        /**
         * @brief 跨类型三路比较操作符
         *
         * @tparam U 另一个迭代器的元素类型
         * @param other 另一个迭代器
         * @return 比较结果
         */
        template<Array2d_iterator_compatible U>
        [[nodiscard]] constexpr auto operator<=>(const Array2d_iterator<U>& other) const noexcept
        requires std::three_way_comparable_with<T*, U*>
        {
            return ptr_ <=> other.data();
        }

        /**
         * @brief 相等比较操作符
         *
         * @param other 另一个相同类型的迭代器
         * @return 两个迭代器是否指向相同位置
         */
        [[nodiscard]] QM_FORCEINLINE constexpr bool operator==(const Array2d_iterator& other) const noexcept {
            return ptr_ == other.ptr_;
        }

        /**
         * @brief 跨类型相等比较操作符
         *
         * @tparam U 另一个迭代器的元素类型
         * @param other 另一个迭代器
         * @return 两个迭代器是否指向相同位置
         */
        template<Array2d_iterator_compatible U>
        [[nodiscard]] constexpr auto operator==(const Array2d_iterator<U>& other) const noexcept -> bool
        requires std::equality_comparable_with<T*, U*>
        {
            return ptr_ == other.data();
        }

        /**
         * @brief 获取底层指针
         *
         * @return 底层指针
         *
         * @note 此方法主要用于STL算法优化、容器内部实现和类型转换
         */
        [[nodiscard]] constexpr auto data() const noexcept -> pointer {
            return ptr_;
        }

    private:
        pointer ptr_ = nullptr; ///< 指向当前元素的指针
    };

    // ================================
    // 全局操作符定义
    // ================================

    /**
     * @brief 全局加法操作符（支持 offset + iterator）
     *
     * 允许将偏移量写在迭代器前面，如：3 + iterator
     *
     * @tparam T 迭代器元素类型
     * @param offset 偏移量
     * @param iter 迭代器
     * @return 新的迭代器，位置为 iter + offset
     */
    template<Array2d_iterator_compatible T>
    [[nodiscard]] constexpr Array2d_iterator<T> operator+(
            typename Array2d_iterator<T>::difference_type offset,
            const Array2d_iterator<T>& iter) noexcept
    {
        return iter + offset;
    }

    // ================================
    // 类型特征和辅助模板
    // ================================

    /**
     * @brief 检查类型是否为 Array2d_iterator 的特化
     */
    template<typename T>
    struct is_array2d_iterator : std::false_type {};

    template<Array2d_iterator_compatible T>
    struct is_array2d_iterator<Array2d_iterator<T>> : std::true_type {};

    /**
     * @brief is_array2d_iterator 的便捷变量模板
     */
    template<typename T>
    inline constexpr bool is_array2d_iterator_v = is_array2d_iterator<T>::value;

    /**
     * @brief Array2d_iterator 的元素类型萃取
     */
    template<typename T>
    struct array2d_iterator_traits {};

    template<Array2d_iterator_compatible T>
    struct array2d_iterator_traits<Array2d_iterator<T>> {
        using value_type = typename Array2d_iterator<T>::value_type;
        using pointer = typename Array2d_iterator<T>::pointer;
        using reference = typename Array2d_iterator<T>::reference;
        using difference_type = typename Array2d_iterator<T>::difference_type;
        using iterator_category = typename Array2d_iterator<T>::iterator_category;
    };

    // ================================
    // 便捷类型别名
    // ================================

    /**
     * @brief 常用迭代器类型的别名
     */
    template<typename T>
    using array2d_iterator = Array2d_iterator<T>;

    template<typename T>
    using array2d_const_iterator = Array2d_iterator<const T>;

} // namespace qm

// ================================
// 标准库特化
// ================================

namespace std {

    /**
     * @brief 为 qm::Array2d_iterator 特化 std::iterator_traits
     *
     * 确保与标准库算法完全兼容
     */
    template<qm::Array2d_iterator_compatible T>
    struct iterator_traits<qm::Array2d_iterator<T>> {
    using iterator_category = std::contiguous_iterator_tag;
    using iterator_concept = std::contiguous_iterator_tag;
    using value_type = std::remove_cv_t<T>;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
};

/**
 * @brief 为 qm::Array2d_iterator 特化 std::pointer_traits
 *
 * 提供指针特征信息，用于某些高级算法优化
 */
template<qm::Array2d_iterator_compatible T>
struct pointer_traits<qm::Array2d_iterator<T>> {
    using pointer = qm::Array2d_iterator<T>;
    using element_type = T;
    using difference_type = std::ptrdiff_t;

    static constexpr pointer pointer_to(element_type& r) noexcept {
        return qm::Array2d_iterator<T>(std::addressof(r));
    }
};

} // namespace std