/**
 * @file example.cpp
 * @brief array2d 类的完整使用示例
 * @author AI Assistant
 * @date 2024
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>
#include <cassert>
#include <iomanip>
#include <map>
#include "array2d.hpp"

using namespace qm;

// ================================
// 辅助函数和工具
// ================================

/**
 * @brief 打印矩阵内容
 */
template<typename T, typename Idx>
void print_matrix(const array2d<T, Idx> &matrix, const std::string &name = "") {
    if (!name.empty()) {
        std::cout << "\n=== " << name << " ===\n";
    }

    std::cout << "Size: " << matrix.rows() << " x " << matrix.cols() << "\n";

    if (matrix.empty()) {
        std::cout << "(Empty matrix)\n";
        return;
    }

    // 设置输出格式
    std::cout << std::fixed << std::setprecision(2);

    for (int i = 0; i < matrix.rows(); ++i) {
        for (int j = 0; j < matrix.cols(); ++j) {
            std::cout << std::setw(8) << matrix[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

/**
 * @brief 生成随机矩阵
 */
template<typename T = double>
array2d<T> generate_random_matrix(int rows, int cols, T min_val = T{0}, T max_val = T{100}) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    array2d<T> matrix(rows, cols);

    if constexpr (std::is_integral_v<T>) {
        std::uniform_int_distribution<T> dis(min_val, max_val);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                matrix[i][j] = dis(gen);
            }
        }
    } else {
        std::uniform_real_distribution<T> dis(min_val, max_val);
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                matrix[i][j] = dis(gen);
            }
        }
    }

    return matrix;
}

/**
 * @brief 计时器类
 */
class Timer {
    std::chrono::high_resolution_clock::time_point start_time;
    std::string name;

public:
    explicit Timer(std::string timer_name) : name(std::move(timer_name)) {
        start_time = std::chrono::high_resolution_clock::now();
    }

    ~Timer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "[Timer] " << name << ": " << duration.count() << " μs\n";
    }
};

// ================================
// 基础用法示例
// ================================

void basic_usage_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "基础用法示例\n";
    std::cout << std::string(50, '=') << "\n";

    // 1. 构造函数示例
    {
        std::cout << "\n1. 构造函数示例:\n";

        // 默认构造
        array2d<int> empty_matrix;
        print_matrix(empty_matrix, "默认构造的空矩阵");

        // 指定尺寸构造
        array2d<double> matrix1(3, 4);
        print_matrix(matrix1, "3x4 矩阵（默认初始化）");

        // 指定尺寸和初始值构造
        array2d<int> matrix2(2, 3, 42);
        print_matrix(matrix2, "2x3 矩阵（初始值为42）");

        // 初始化列表构造
        array2d<int> matrix3{{1, 2, 3},
                             {4, 5, 6},
                             {7, 8, 9}};
        print_matrix(matrix3, "初始化列表构造的3x3矩阵");

        // 从容器构造
        std::vector<double> data{1.1, 2.2, 3.3, 4.4, 5.5, 6.6};
        array2d<double> matrix4(2, 3, data);
        print_matrix(matrix4, "从vector构造的2x3矩阵");
    }

    // 2. 元素访问示例
    {
        std::cout << "\n2. 元素访问示例:\n";

        array2d<int> matrix(3, 3, 0);

        // 使用 operator[] 访问
        matrix[0][0] = 1;
        matrix[0][1] = 2;
        matrix[0][2] = 3;

        // 使用 operator() 访问
        matrix(1, 0) = 4;
        matrix(1, 1) = 5;
        matrix(1, 2) = 6;

        // 使用 at() 方法（带边界检查）
        try {
            matrix.at(2, 0) = 7;
            matrix.at(2, 1) = 8;
            matrix.at(2, 2) = 9;
            // matrix.at(3, 0) = 10;  // 这会抛出异常
        } catch (const std::out_of_range &e) {
            std::cout << "捕获边界检查异常: " << e.what() << "\n";
        }

        print_matrix(matrix, "元素访问后的矩阵");

        // 读取元素
        std::cout << "matrix[1][1] = " << matrix[1][1] << "\n";
        std::cout << "matrix(2, 2) = " << matrix(2, 2) << "\n";
        std::cout << "matrix.at(0, 2) = " << matrix.at(0, 2) << "\n";
    }
}

// ================================
// 迭代器使用示例
// ================================

void iterator_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "迭代器使用示例\n";
    std::cout << std::string(50, '=') << "\n";

    array2d<int> matrix{{1, 2, 3},
                        {4, 5, 6},
                        {7, 8, 9}};
    print_matrix(matrix, "原始矩阵");

    // 1. 基础迭代器使用
    {
        std::cout << "\n1. 正向迭代器遍历:\n";
        std::cout << "元素: ";
        for (auto it = matrix.begin(); it != matrix.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << "\n";

        // 范围for循环
        std::cout << "范围for: ";
        for (const auto &element: matrix) {
            std::cout << element << " ";
        }
        std::cout << "\n";
    }

    // 2. 反向迭代器
    {
        std::cout << "\n2. 反向迭代器遍历:\n";
        std::cout << "逆序元素: ";
        for (auto it = matrix.rbegin(); it != matrix.rend(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << "\n";
    }

    // 3. const迭代器
    {
        const auto &const_matrix = matrix;
        std::cout << "\n3. const迭代器遍历:\n";
        std::cout << "const元素: ";
        for (auto it = const_matrix.cbegin(); it != const_matrix.cend(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << "\n";
    }

    // 4. 使用STL算法
    {
        std::cout << "\n4. STL算法示例:\n";

        // std::find
        auto found = std::find(matrix.begin(), matrix.end(), 5);
        if (found != matrix.end()) {
            std::cout << "找到元素5，位置: " << std::distance(matrix.begin(), found) << "\n";
        }

        // std::count
        auto count = std::count_if(matrix.begin(), matrix.end(), [](int x) { return x > 5; });
        std::cout << "大于5的元素数量: " << count << "\n";

        // std::transform
        array2d<int> doubled_matrix(matrix.rows(), matrix.cols());
        std::transform(matrix.begin(), matrix.end(), doubled_matrix.begin(),
                       [](int x) { return x * 2; });
        print_matrix(doubled_matrix, "所有元素翻倍后的矩阵");

        // std::accumulate
        auto sum = std::accumulate(matrix.begin(), matrix.end(), 0);
        std::cout << "所有元素的和: " << sum << "\n";

        // std::sort (按行排序整个矩阵)
        array2d<int> sorted_matrix = matrix;
        std::sort(sorted_matrix.begin(), sorted_matrix.end());
        print_matrix(sorted_matrix, "排序后的矩阵");
    }

    // 5. 行迭代器
    {
        std::cout << "\n5. 行迭代器示例:\n";
        for (int i = 0; i < matrix.rows(); ++i) {
            std::cout << "第" << i << "行: ";
            auto row_range = matrix.row_range(i);
            for (auto it = row_range.begin(); it != row_range.end(); ++it) {
                std::cout << *it << " ";
            }
            std::cout << "\n";
        }
    }
}

// ================================
// span 操作示例
// ================================

void span_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "span 操作示例\n";
    std::cout << std::string(50, '=') << "\n";

    array2d<int> matrix{{1, 2,  3,  4},
                        {5, 6,  7,  8},
                        {9, 10, 11, 12}};
    print_matrix(matrix, "原始矩阵");

    // 1. 整个矩阵的span
    {
        auto full_span = matrix.as_span();
        std::cout << "\n1. 整个矩阵作为span:\n";
        std::cout << "span大小: " << full_span.size() << "\n";
        std::cout << "span内容: ";
        for (const auto &element: full_span) {
            std::cout << element << " ";
        }
        std::cout << "\n";

        // 修改span中的元素
        if (!full_span.empty()) {
            full_span[0] = 999;
        }
        print_matrix(matrix, "修改span后的矩阵");
    }

    // 2. 单行span
    {
        std::cout << "\n2. 单行span操作:\n";
        for (int i = 0; i < matrix.rows(); ++i) {
            auto row_span = matrix.row(i);
            std::cout << "第" << i << "行span: ";
            for (const auto &element: row_span) {
                std::cout << element << " ";
            }
            std::cout << "\n";

            // 对行进行操作
            if (i == 1) {
                std::fill(row_span.begin(), row_span.end(), 88);
            }
        }
        print_matrix(matrix, "修改第1行后的矩阵");
    }

    // 3. const span
    {
        const auto &const_matrix = matrix;
        auto const_span = const_matrix.as_span();
        std::cout << "\n3. const span:\n";
        std::cout << "const span内容: ";
        for (const auto &element: const_span) {
            std::cout << element << " ";
        }
        std::cout << "\n";
    }

    // 4. 子矩阵span（如果实现了的话）
    {
        std::cout << "\n4. 子矩阵span:\n";
        try {
            auto sub_span = matrix.submatrix_row_major(0, 1, 2, 2);
            std::cout << "子矩阵span内容: ";
            for (const auto &element: sub_span) {
                std::cout << element << " ";
            }
            std::cout << "\n";
        } catch (const std::exception &e) {
            std::cout << "子矩阵操作: " << e.what() << "\n";
        }
    }
}

// ================================
// 数据操作示例
// ================================

void data_manipulation_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "数据操作示例\n";
    std::cout << std::string(50, '=') << "\n";

    // 1. fill 操作
    {
        std::cout << "\n1. fill 操作:\n";
        array2d<int> matrix(3, 4);
        matrix.fill(42);
        print_matrix(matrix, "fill(42)后的矩阵");

        // 并行fill（如果矩阵足够大）
        array2d<double> large_matrix(100, 100);
        {
            Timer timer("并行fill操作");
            large_matrix.fill_parallel(3.14);
        }
        std::cout << "大矩阵并行fill完成，元素[50][50] = " << large_matrix[50][50] << "\n";
    }

    // 2. reset 操作
    {
        std::cout << "\n2. reset 操作:\n";
        array2d<int> matrix(2, 3, 99);
        print_matrix(matrix, "reset前的矩阵");

        matrix.reset(Array_reset_opt::All_bits0);
        print_matrix(matrix, "reset(All_bits0)后的矩阵");

        matrix.fill(100);
        matrix.reset(Array_reset_opt::All_bits1);
        print_matrix(matrix, "reset(All_bits1)后的矩阵");
    }

    // 3. 行操作
    {
        std::cout << "\n3. 行操作:\n";
        array2d<int> matrix{{1, 2, 3},
                            {4, 5, 6},
                            {7, 8, 9}};
        print_matrix(matrix, "原始矩阵");

        // 复制行
        matrix.copy_row(0, 2);  // 将第0行复制到第2行
        print_matrix(matrix, "copy_row(0, 2)后的矩阵");

        // 交换行
        matrix.swap_rows(0, 1);  // 交换第0行和第1行
        print_matrix(matrix, "swap_rows(0, 1)后的矩阵");

        // 填充行
        matrix.fill_row(1, 999);  // 用999填充第1行
        print_matrix(matrix, "fill_row(1, 999)后的矩阵");
    }

    // 4. 列操作
    {
        std::cout << "\n4. 列操作:\n";
        array2d<int> matrix{{1, 2, 3},
                            {4, 5, 6},
                            {7, 8, 9}};
        print_matrix(matrix, "原始矩阵");

        // 提取列
        auto col1 = matrix.col(1);  // 提取第1列
        std::cout << "第1列内容: ";
        for (const auto &element: col1) {
            std::cout << element << " ";
        }
        std::cout << "\n";
    }
}

// ================================
// 转置操作示例
// ================================

void transpose_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "转置操作示例\n";
    std::cout << std::string(50, '=') << "\n";

    // 1. 正方形矩阵原地转置
    {
        std::cout << "\n1. 正方形矩阵原地转置:\n";
        array2d<int> square_matrix{{1, 2, 3},
                                   {4, 5, 6},
                                   {7, 8, 9}};
        print_matrix(square_matrix, "转置前的正方形矩阵");

        square_matrix.transpose();
        print_matrix(square_matrix, "转置后的正方形矩阵");
    }

    // 2. 非正方形矩阵转置（返回新矩阵）
    {
        std::cout << "\n2. 非正方形矩阵转置:\n";
        array2d<int> rect_matrix{{1, 2, 3, 4},
                                 {5, 6, 7, 8}};
        print_matrix(rect_matrix, "转置前的矩形矩阵");

        auto transposed = rect_matrix.transposed();
        print_matrix(transposed, "转置后的矩形矩阵");

        // 原矩阵保持不变
        print_matrix(rect_matrix, "原矩阵（应该保持不变）");
    }

    // 3. 大矩阵转置性能测试
    {
        std::cout << "\n3. 大矩阵转置性能测试:\n";
        const int size = 500;
        auto large_matrix = generate_random_matrix<double>(size, size, 0.0, 100.0);

        {
            Timer timer("大矩阵原地转置");
            large_matrix.transpose();
        }

        std::cout << "大矩阵转置完成\n";
    }

    // 4. 转置错误处理
    {
        std::cout << "\n4. 转置错误处理:\n";
        array2d<int> rect_matrix(2, 3, 1);
        try {
            rect_matrix.transpose();  // 这应该抛出异常
        } catch (const std::invalid_argument &e) {
            std::cout << "捕获预期的异常: " << e.what() << "\n";
        }
    }
}

// ================================
// resize 操作示例
// ================================

void resize_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "resize 操作示例\n";
    std::cout << std::string(50, '=') << "\n";

    // 1. 基本resize操作
    {
        std::cout << "\n1. 基本resize操作:\n";
        array2d<int> matrix{{1, 2, 3},
                            {4, 5, 6}};
        print_matrix(matrix, "resize前的矩阵");

        // 扩大矩阵
        matrix.resize(4, 5);
        print_matrix(matrix, "resize(4, 5)后的矩阵");

        // 缩小矩阵
        matrix.resize(2, 2);
        print_matrix(matrix, "resize(2, 2)后的矩阵");
    }

    // 2. 带默认值的resize
    {
        std::cout << "\n2. 带默认值的resize:\n";
        array2d<int> matrix{{1, 2},
                            {3, 4}};
        print_matrix(matrix, "resize前的矩阵");

        matrix.resize(4, 4, 999);
        print_matrix(matrix, "resize(4, 4, 999)后的矩阵");
    }

    // 3. resize到空矩阵
    {
        std::cout << "\n3. resize到空矩阵:\n";
        array2d<int> matrix(3, 3, 42);
        print_matrix(matrix, "resize前的矩阵");

        matrix.resize(0, 0);
        print_matrix(matrix, "resize(0, 0)后的矩阵");

        // 再次resize回非空
        matrix.resize(2, 2, 123);
        print_matrix(matrix, "resize(2, 2, 123)后的矩阵");
    }

    // 4. resize性能测试
    {
        std::cout << "\n4. resize性能测试:\n";
        array2d<double> matrix;

        {
            Timer timer("resize到大矩阵");
            matrix.resize(1000, 1000, 3.14);
        }

        std::cout << "大矩阵创建完成，大小: " << matrix.rows() << "x" << matrix.cols() << "\n";
        std::cout << "随机元素值: " << matrix[500][500] << "\n";

        {
            Timer timer("resize缩小");
            matrix.resize(100, 100);
        }

        std::cout << "矩阵缩小完成，新大小: " << matrix.rows() << "x" << matrix.cols() << "\n";
    }
}

// ================================
// 内存管理示例
// ================================

void memory_management_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "内存管理示例\n";
    std::cout << std::string(50, '=') << "\n";

    // 1. 容量管理
    {
        std::cout << "\n1. 容量管理:\n";
        array2d<int> matrix;
        std::cout << "初始状态 - 大小: " << matrix.size()
                  << ", 容量: " << matrix.capacity() << "\n";

        // reserve操作
        matrix.reserve(10, 10);
        std::cout << "reserve(10, 10)后 - 大小: " << matrix.size()
                  << ", 容量: " << matrix.capacity() << "\n";

        // 实际resize
        matrix.resize(5, 5, 42);
        std::cout << "resize(5, 5, 42)后 - 大小: " << matrix.size()
                  << ", 容量: " << matrix.capacity() << "\n";

        // shrink_to_fit
        matrix.shrink_to_fit();
        std::cout << "shrink_to_fit()后 - 大小: " << matrix.size()
                  << ", 容量: " << matrix.capacity() << "\n";
    }

    // 2. 数据访问
    {
        std::cout << "\n2. 数据访问:\n";
        array2d<int> matrix(2, 3, 100);

        // 直接数据指针访问
        int *raw_data = matrix.data();
        std::cout << "通过data()访问: ";
        for (size_t i = 0; i < matrix.size(); ++i) {
            std::cout << raw_data[i] << " ";
        }
        std::cout << "\n";

        // 修改数据
        raw_data[0] = 999;
        print_matrix(matrix, "修改raw_data[0]后的矩阵");

        // 获取底层vector的引用
        auto &vector_ref = matrix.get_vector();
        std::cout << "底层vector大小: " << vector_ref.size() << "\n";
        std::cout << "底层vector容量: " << vector_ref.capacity() << "\n";
    }

    // 3. 交换操作
    {
        std::cout << "\n3. 交换操作:\n";
        array2d<int> matrix1(2, 2, 1);
        array2d<int> matrix2(3, 3, 2);

        print_matrix(matrix1, "交换前的matrix1");
        print_matrix(matrix2, "交换前的matrix2");

        matrix1.swap(matrix2);

        print_matrix(matrix1, "交换后的matrix1");
        print_matrix(matrix2, "交换后的matrix2");

        // 使用std::swap
        std::swap(matrix1, matrix2);
        print_matrix(matrix1, "std::swap后的matrix1");
        print_matrix(matrix2, "std::swap后的matrix2");
    }
}

// ================================
// 算法和数学操作示例
// ================================

void algorithm_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "算法和数学操作示例\n" <<
              std::string(50, '=') << "\n";

    // 1. 矩阵统计
    {
        std::cout << "\n1. 矩阵统计:\n";
        auto matrix = generate_random_matrix<double>(4, 5, 0.0, 100.0);
        print_matrix(matrix, "随机矩阵");

        // 求和
        double sum = std::accumulate(matrix.begin(), matrix.end(), 0.0);
        std::cout << "矩阵元素总和: " << sum << "\n";

        // 平均值
        double mean = sum / matrix.size();
        std::cout << "矩阵元素平均值: " << mean << "\n";

        // 最大值和最小值
        auto [min_it, max_it] = std::minmax_element(matrix.begin(), matrix.end());
        std::cout << "最小值: " << *min_it << ", 最大值: " << *max_it << "\n";

        // 统计满足条件的元素
        auto count_above_50 = std::count_if(matrix.begin(), matrix.end(),
                                            [](double x) { return x > 50.0; });
        std::cout << "大于50的元素数量: " << count_above_50 << "\n";
    }

    // 2. 矩阵变换
    {
        std::cout << "\n2. 矩阵变换:\n";
        array2d<int> matrix{{1, 2, 3},
                            {4, 5, 6},
                            {7, 8, 9}};
        print_matrix(matrix, "原始矩阵");

        // 所有元素乘以2
        std::transform(matrix.begin(), matrix.end(), matrix.begin(),
                       [](int x) { return x * 2; });
        print_matrix(matrix, "所有元素乘以2");

        // 应用复杂变换
        std::transform(matrix.begin(), matrix.end(), matrix.begin(),
                       [](int x) { return x * x - x + 1; });
        print_matrix(matrix, "应用 f(x) = x² - x + 1");
    }

    // 3. 矩阵搜索
    {
        std::cout << "\n3. 矩阵搜索:\n";
        array2d<int> matrix{{5, 2, 8},
                            {1, 9, 3},
                            {7, 4, 6}};
        print_matrix(matrix, "搜索目标矩阵");

        // 搜索特定值
        int target = 9;
        auto found = std::find(matrix.begin(), matrix.end(), target);
        if (found != matrix.end()) {
            auto index = std::distance(matrix.begin(), found);
            int row = static_cast<int>(index) / matrix.cols();
            int col = static_cast<int>(index) % matrix.cols();
            std::cout << "找到 " << target << " 在位置 (" << row << ", " << col << ")\n";
        }

        // 搜索满足条件的第一个元素
        auto found_condition = std::find_if(matrix.begin(), matrix.end(),
                                            [](int x) { return x > 7; });
        if (found_condition != matrix.end()) {
            std::cout << "第一个大于7的元素: " << *found_condition << "\n";
        }
    }

    // 4. 矩阵排序
    {
        std::cout << "\n4. 矩阵排序:\n";
        array2d<int> matrix{{9, 2, 7},
                            {5, 8, 1},
                            {3, 6, 4}};
        print_matrix(matrix, "排序前的矩阵");

        // 整体排序
        array2d<int> sorted_matrix = matrix;
        std::sort(sorted_matrix.begin(), sorted_matrix.end());
        print_matrix(sorted_matrix, "整体排序后的矩阵");

        // 逐行排序
        array2d<int> row_sorted_matrix = matrix;
        for (int i = 0; i < row_sorted_matrix.rows(); ++i) {
            auto row_span = row_sorted_matrix.row(i);
            std::sort(row_span.begin(), row_span.end());
        }
        print_matrix(row_sorted_matrix, "逐行排序后的矩阵");

        // 逐行逆序排序
        for (int i = 0; i < row_sorted_matrix.rows(); ++i) {
            auto row_span = row_sorted_matrix.row(i);
            std::sort(row_span.begin(), row_span.end(), std::greater<int>());
        }
        print_matrix(row_sorted_matrix, "逐行逆序排序后的矩阵");
    }

    // 5. 矩阵分区和重排
    {
        std::cout << "\n5. 矩阵分区和重排:\n";
        array2d<int> matrix{{1, 8,  3,  6},
                            {9, 2,  7,  4},
                            {5, 10, 11, 12}};
        print_matrix(matrix, "原始矩阵");

        // 分区：小于等于6的在前，大于6的在后
        array2d<int> partitioned_matrix = matrix;
        auto partition_point = std::partition(partitioned_matrix.begin(),
                                              partitioned_matrix.end(),
                                              [](int x) { return x <= 6; });
        print_matrix(partitioned_matrix, "分区后的矩阵（<=6的在前）");

        // 随机重排
        array2d<int> shuffled_matrix = matrix;
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(shuffled_matrix.begin(), shuffled_matrix.end(), g);
        print_matrix(shuffled_matrix, "随机重排后的矩阵");
    }
}

// ================================
// 类型推导和模板示例
// ================================

void template_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "类型推导和模板示例\n";
    std::cout << std::string(50, '=') << "\n";

    // 1. 类型推导指引
    {
        std::cout << "\n1. 类型推导指引:\n";

        // 从初始值推导类型
        array2d m1(3, 4, 42);           // 推导为 array2d<int>
        array2d m2(2, 3, 3.14);         // 推导为 array2d<double>
        array2d m3(2, 2, 'A');          // 推导为 array2d<char>

        print_matrix(m1, "推导为 array2d<int>");
        print_matrix(m2, "推导为 array2d<double>");
        print_matrix(m3, "推导为 array2d<char>");

        // 从初始化列表推导
        array2d m4{{1, 2, 3},
                   {4, 5, 6}};              // 推导为 array2d<int>
        array2d m5{{1.1, 2.2},
                   {3.3, 4.4}};           // 推导为 array2d<double>

        print_matrix(m4, "从初始化列表推导 array2d<int>");
        print_matrix(m5, "从初始化列表推导 array2d<double>");
    }

    // 2. 不同索引类型
    {
        std::cout << "\n2. 不同索引类型:\n";

        // 使用 long 作为索引类型
        array2d<double, long> long_matrix(2L, 3L, 1.23);
        print_matrix(long_matrix, "使用 long 索引的矩阵");

        // 使用 size_t 作为索引类型（需要注意有符号无符号转换）
        // array2d<int, size_t> size_t_matrix(2U, 3U, 456);  // 如果支持的话

        std::cout << "long矩阵的行数类型: " << typeid(long_matrix.rows()).name() << "\n";
    }

    // 3. 复杂类型的矩阵
    {
        std::cout << "\n3. 复杂类型的矩阵:\n";

        // 字符串矩阵
        array2d<std::string> string_matrix(2, 3, "hello");
        string_matrix[0][0] = "world";
        string_matrix[1][2] = "!";

        std::cout << "字符串矩阵内容:\n";
        for (int i = 0; i < string_matrix.rows(); ++i) {
            for (int j = 0; j < string_matrix.cols(); ++j) {
                std::cout << std::setw(8) << string_matrix[i][j] << " ";
            }
            std::cout << "\n";
        }

        // vector矩阵
        array2d<std::vector<int>> vector_matrix(2, 2);
        vector_matrix[0][0] = {1, 2, 3};
        vector_matrix[0][1] = {4, 5};
        vector_matrix[1][0] = {6, 7, 8, 9};
        vector_matrix[1][1] = {10};

        std::cout << "\nvector矩阵内容:\n";
        for (int i = 0; i < vector_matrix.rows(); ++i) {
            for (int j = 0; j < vector_matrix.cols(); ++j) {
                std::cout << "[";
                for (size_t k = 0; k < vector_matrix[i][j].size(); ++k) {
                    if (k > 0) std::cout << ",";
                    std::cout << vector_matrix[i][j][k];
                }
                std::cout << "] ";
            }
            std::cout << "\n";
        }
    }
}

// ================================
// 比较操作示例
// ================================

void comparison_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "比较操作示例\n";
    std::cout << std::string(50, '=') << "\n";

    // 1. 相等比较
    {
        std::cout << "\n1. 相等比较:\n";

        array2d<int> matrix1{{1, 2},
                             {3, 4}};
        array2d<int> matrix2{{1, 2},
                             {3, 4}};
        array2d<int> matrix3{{1, 2},
                             {3, 5}};
        array2d<int> matrix4(3, 3, 0);

        print_matrix(matrix1, "matrix1");
        print_matrix(matrix2, "matrix2");
        print_matrix(matrix3, "matrix3");
        print_matrix(matrix4, "matrix4");

        std::cout << "matrix1 == matrix2: " << (matrix1 == matrix2) << "\n";
        std::cout << "matrix1 == matrix3: " << (matrix1 == matrix3) << "\n";
        std::cout << "matrix1 == matrix4: " << (matrix1 == matrix4) << "\n";
        std::cout << "matrix1 != matrix3: " << (matrix1 != matrix3) << "\n";
    }

    // 2. 三路比较（如果支持）
    {
        std::cout << "\n2. 三路比较:\n";

        array2d<int> matrix1{{1, 2},
                             {3, 4}};
        array2d<int> matrix2{{1, 2},
                             {3, 5}};
        array2d<int> matrix3{{1, 2, 3},
                             {4, 5, 6}};

        // 注意：这需要 C++20 的三路比较支持
        try {
            auto cmp1 = matrix1 <=> matrix2;
            std::cout << "matrix1 <=> matrix2: ";
            if (cmp1 < 0) std::cout << "less\n";
            else if (cmp1 > 0) std::cout << "greater\n";
            else std::cout << "equal\n";

            auto cmp2 = matrix1 <=> matrix3;
            std::cout << "matrix1 <=> matrix3: ";
            if (cmp2 < 0) std::cout << "less\n";
            else if (cmp2 > 0) std::cout << "greater\n";
            else std::cout << "equal\n";

        } catch (const std::exception &e) {
            std::cout << "三路比较暂不支持或出现错误: " << e.what() << "\n";
        }
    }
}

// ================================
// 性能测试示例
// ================================

void performance_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "性能测试示例\n";
    std::cout << std::string(50, '=') << "\n";

    const int test_size = 1000;

    // 1. 构造性能
    {
        std::cout << "\n1. 构造性能测试:\n";

        {
            Timer timer("默认构造 + resize");
            array2d<double> matrix;
            matrix.resize(test_size, test_size, 1.0);
        }

        {
            Timer timer("直接构造");
            array2d<double> matrix(test_size, test_size, 1.0);
        }

        {
            Timer timer("构造 + reserve");
            array2d<double> matrix;
            matrix.reserve(test_size, test_size);
            matrix.resize(test_size, test_size, 1.0);
        }
    }

    // 2. 元素访问性能
    {
        std::cout << "\n2. 元素访问性能测试:\n";

        array2d<double> matrix(test_size, test_size, 1.0);
        volatile double sum = 0.0;  // 防止编译器优化

        {
            Timer timer("operator[] 访问");
            for (int i = 0; i < matrix.rows(); ++i) {
                for (int j = 0; j < matrix.cols(); ++j) {
                    sum += matrix[i][j];
                }
            }
        }

        sum = 0.0;
        {
            Timer timer("operator() 访问");
            for (int i = 0; i < matrix.rows(); ++i) {
                for (int j = 0; j < matrix.cols(); ++j) {
                    sum += matrix(i, j);
                }
            }
        }

        sum = 0.0;
        {
            Timer timer("迭代器访问");
            for (const auto &element: matrix) {
                sum += element;
            }
        }

        sum = 0.0;
        {
            Timer timer("原始指针访问");
            const double *data = matrix.data();
            const size_t size = matrix.size();
            for (size_t i = 0; i < size; ++i) {
                sum += data[i];
            }
        }

        std::cout << "最终sum值（防止优化）: " << sum << "\n";
    }

    // 3. 算法性能
    {
        std::cout << "\n3. 算法性能测试:\n";

        auto matrix1 = generate_random_matrix<double>(test_size, test_size, 0.0, 100.0);
        auto matrix2 = matrix1;  // 复制一份用于测试

        {
            Timer timer("std::fill");
            std::fill(matrix1.begin(), matrix1.end(), 42.0);
        }

        {
            Timer timer("matrix.fill()");
            matrix2.fill(42.0);
        }

        // 重置矩阵
        matrix1 = generate_random_matrix<double>(test_size, test_size, 0.0, 100.0);
        matrix2 = matrix1;

        double sum1, sum2;
        {
            Timer timer("std::accumulate");
            sum1 = std::accumulate(matrix1.begin(), matrix1.end(), 0.0);
        }

        {
            Timer timer("手动求和");
            sum2 = 0.0;
            for (const auto &element: matrix2) {
                sum2 += element;
            }
        }

        std::cout << "两种求和结果差异: " << std::abs(sum1 - sum2) << "\n";
    }

    // 4. 内存操作性能
    {
        std::cout << "\n4. 内存操作性能测试:\n";

        array2d<int> source_matrix = generate_random_matrix<int>(test_size, test_size, 0, 1000);

        {
            Timer timer("拷贝构造");
            array2d<int> copy_matrix(source_matrix);
        }

        {
            Timer timer("移动构造");
            array2d<int> temp_matrix = generate_random_matrix<int>(test_size, test_size, 0, 1000);
            array2d<int> move_matrix(std::move(temp_matrix));
        }

        array2d<int> target_matrix;
        {
            Timer timer("拷贝赋值");
            target_matrix = source_matrix;
        }

        {
            Timer timer("移动赋值");
            array2d<int> temp_matrix = generate_random_matrix<int>(test_size, test_size, 0, 1000);
            target_matrix = std::move(temp_matrix);
        }

        {
            Timer timer("swap操作");
            array2d<int> swap_matrix(100, 100, 999);
            target_matrix.swap(swap_matrix);
        }
    }
}

// ================================
// 错误处理和边界情况示例
// ================================

void error_handling_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "错误处理和边界情况示例\n";
    std::cout << std::string(50, '=') << "\n";

    // 1. 边界检查
    {
        std::cout << "\n1. 边界检查:\n";

        array2d<int> matrix(3, 3, 42);

        try {
            // 正常访问
            std::cout << "matrix.at(1, 1) = " << matrix.at(1, 1) << "\n";

            // 边界外访问
            std::cout << "尝试访问 matrix.at(5, 5)...\n";
            volatile int x = matrix.at(5, 5);  // 这应该抛出异常
            std::cout << "意外：没有抛出异常，值为: " << x << "\n";

        } catch (const std::out_of_range &e) {
            std::cout << "捕获预期的边界检查异常: " << e.what() << "\n";
        }

        try {
            // 负索引
            std::cout << "尝试访问 matrix.at(-1, 0)...\n";
            volatile int y = matrix.at(-1, 0);
            std::cout << "意外：没有抛出异常，值为: " << y << "\n";

        } catch (const std::out_of_range &e) {
            std::cout << "捕获负索引异常: " << e.what() << "\n";
        }
    }

    // 2. 构造函数错误处理
    {
        std::cout << "\n2. 构造函数错误处理:\n";

        try {
            // 负尺寸
            std::cout << "尝试创建负尺寸矩阵...\n";
            array2d<int> bad_matrix(-1, 5);
            std::cout << "意外：负尺寸矩阵创建成功\n";

        } catch (const std::invalid_argument &e) {
            std::cout << "捕获负尺寸异常: " << e.what() << "\n";
        } catch (const std::exception &e) {
            std::cout << "捕获其他异常: " << e.what() << "\n";
        }

        try {
            // 非常大的尺寸（可能导致溢出）
            std::cout << "尝试创建超大矩阵...\n";
            const int huge_size = std::numeric_limits<int>::max() / 2;
            array2d<int> huge_matrix(huge_size, huge_size);
            std::cout << "意外：超大矩阵创建成功\n";

        } catch (const std::overflow_error &e) {
            std::cout << "捕获溢出异常: " << e.what() << "\n";
        } catch (const std::bad_alloc &e) {
            std::cout << "捕获内存分配异常: " << e.what() << "\n";
        } catch (const std::exception &e) {
            std::cout << "捕获其他异常: " << e.what() << "\n";
        }
    }

    // 3. resize错误处理
    {
        std::cout << "\n3. resize错误处理:\n";

        array2d<int> matrix(2, 2, 1);

        try {
            std::cout << "尝试resize到负尺寸...\n";
            matrix.resize(-1, -1);
            std::cout << "意外：resize到负尺寸成功\n";

        } catch (const std::invalid_argument &e) {
            std::cout << "捕获resize负尺寸异常: " << e.what() << "\n";
        }
    }

    // 4. 转置错误处理
    {
        std::cout << "\n4. 转置错误处理:\n";

        array2d<int> rect_matrix(2, 3, 1);
        print_matrix(rect_matrix, "矩形矩阵");

        try {
            std::cout << "尝试对矩形矩阵进行原地转置...\n";
            rect_matrix.transpose();
            std::cout << "意外：矩形矩阵转置成功\n";

        } catch (const std::invalid_argument &e) {
            std::cout << "捕获矩形矩阵转置异常: " << e.what() << "\n";
        }
    }

    // 5. 空矩阵操作
    {
        std::cout << "\n5. 空矩阵操作:\n";

        array2d<int> empty_matrix;
        print_matrix(empty_matrix, "空矩阵");

        std::cout << "空矩阵 empty(): " << empty_matrix.empty() << "\n";
        std::cout << "空矩阵 size(): " << empty_matrix.size() << "\n";
        std::cout << "空矩阵 rows(): " << empty_matrix.rows() << "\n";
        std::cout << "空矩阵 cols(): " << empty_matrix.cols() << "\n";

        // 对空矩阵的安全操作
        empty_matrix.fill(999);  // 应该是安全的
        empty_matrix.reset();    // 应该是安全的

        std::cout << "空矩阵操作完成，仍为空: " << empty_matrix.empty() << "\n";
    }
}

// ================================
// 实际应用示例
// ================================

void practical_examples() {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "实际应用示例\n";
    std::cout << std::string(50, '=') << "\n";

    // 1. 图像处理模拟
    {
        std::cout << "\n1. 图像处理模拟:\n";

        // 创建一个"图像"（灰度值0-255）
        array2d<unsigned char> image(10, 10);

        // 生成渐变图案
        for (int i = 0; i < image.rows(); ++i) {
            for (int j = 0; j < image.cols(); ++j) {
                image[i][j] = static_cast<unsigned char>((i + j) * 255 / (image.rows() + image.cols() - 2));
            }
        }

        std::cout << "原始图像:\n";
        for (int i = 0; i < image.rows(); ++i) {
            for (int j = 0; j < image.cols(); ++j) {
                std::cout << std::setw(4) << static_cast<int>(image[i][j]) << " ";
            }
            std::cout << "\n";
        }

        // 应用简单的模糊滤镜（3x3平均）
        array2d<unsigned char> blurred_image = image;
        for (int i = 1; i < image.rows() - 1; ++i) {
            for (int j = 1; j < image.cols() - 1; ++j) {
                int sum = 0;
                for (int di = -1; di <= 1; ++di) {
                    for (int dj = -1; dj <= 1; ++dj) {
                        sum += image[i + di][j + dj];
                    }
                }
                blurred_image[i][j] = static_cast<unsigned char>(sum / 9);
            }
        }

        std::cout << "\n模糊后的图像:\n";
        for (int i = 0; i < blurred_image.rows(); ++i) {
            for (int j = 0; j < blurred_image.cols(); ++j) {
                std::cout << std::setw(4) << static_cast<int>(blurred_image[i][j]) << " ";
            }
            std::cout << "\n";
        }
    }

    // 2. 游戏地图
    {
        std::cout << "\n2. 游戏地图示例:\n";

        enum class TerrainType : char {
            Water = '~',
            Grass = '.',
            Mountain = '^',
            Forest = '#',
            Road = '-'
        };

        array2d<TerrainType> game_map(8, 12, TerrainType::Grass);

        // 创建一些地形特征
        // 水域
        for (int j = 0; j < 3; ++j) {
            for (int i = 0; i < game_map.rows(); ++i) {
                game_map[i][j] = TerrainType::Water;
            }
        }

        // 山脉
        for (int i = 2; i < 6; ++i) {
            for (int j = 8; j < 11; ++j) {
                game_map[i][j] = TerrainType::Mountain;
            }
        }

        // 森林
        game_map[0][5] = TerrainType::Forest;
        game_map[1][5] = TerrainType::Forest;
        game_map[1][6] = TerrainType::Forest;
        game_map[2][6] = TerrainType::Forest;

        // 道路
        for (int j = 4; j < 8; ++j) {
            game_map[4][j] = TerrainType::Road;
        }

        std::cout << "游戏地图:\n";
        for (int i = 0; i < game_map.rows(); ++i) {
            for (int j = 0; j < game_map.cols(); ++j) {
                std::cout << static_cast<char>(game_map[i][j]) << " ";
            }
            std::cout << "\n";
        }

        // 统计不同地形类型的数量
        std::map<TerrainType, int> terrain_counts;
        for (const auto &terrain: game_map) {
            terrain_counts[terrain]++;
        }

        std::cout << "\n地形统计:\n";
        for (const auto &[terrain, count]: terrain_counts) {
            std::cout << "'" << static_cast<char>(terrain) << "': " << count << " tiles\n";
        }
    }

    // 3. 科学计算示例
    {
        std::cout << "\n3. 科学计算示例（热传导模拟）:\n";

        const int grid_size = 10;
        const double dt = 0.01;        // 时间步长
        const double dx = 1.0;         // 空间步长
        const double alpha = 0.1;      // 热扩散系数
        const double r = alpha * dt / (dx * dx);

        // 初始温度分布
        array2d<double> temperature(grid_size, grid_size, 20.0);  // 环境温度20度

        // 设置热源
        temperature[grid_size / 2][grid_size / 2] = 100.0;  // 中心100度热源

        std::cout << "初始温度分布:\n";
        for (int i = 0; i < grid_size; ++i) {
            for (int j = 0; j < grid_size; ++j) {
                std::cout << std::setw(6) << std::fixed << std::setprecision(1)
                          << temperature[i][j] << " ";
            }
            std::cout << "\n";
        }

        // 时间演化（简化的有限差分法）
        array2d<double> new_temperature = temperature;
        const int time_steps = 50;

        for (int step = 0; step < time_steps; ++step) {
            for (int i = 1; i < grid_size - 1; ++i) {
                for (int j = 1; j < grid_size - 1; ++j) {
                    new_temperature[i][j] = temperature[i][j] + r * (
                            temperature[i - 1][j] + temperature[i + 1][j] +
                            temperature[i][j - 1] + temperature[i][j + 1] -
                            4 * temperature[i][j]
                    );
                }
            }
            temperature = new_temperature;
        }

        std::cout << "\n" << time_steps << "个时间步后的温度分布:\n";
        for (int i = 0; i < grid_size; ++i) {
            for (int j = 0; j < grid_size; ++j) {
                std::cout << std::setw(6) << std::fixed << std::setprecision(1)
                          << temperature[i][j] << " ";
            }
            std::cout << "\n";
        }
    }

    // 4. 数据分析示例
    {
        std::cout << "\n4. 数据分析示例:\n";

        // 模拟销售数据：12个月 × 5个产品
        array2d<double> sales_data(12, 5);

        // 填充随机销售数据
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(1000.0, 10000.0);

        for (int month = 0; month < 12; ++month) {
            for (int product = 0; product < 5; ++product) {
                // 添加季节性变化
                double seasonal_factor = 1.0 + 0.3 * std::sin(2 * 3.141592653589793238 * month / 12.0);
                sales_data[month][product] = dis(gen) * seasonal_factor;
            }
        }

        // 计算每月总销售额
        std::cout << "每月总销售额:\n";
        for (int month = 0; month < 12; ++month) {
            double monthly_total = 0.0;
            auto month_span = sales_data.row(month);
            monthly_total = std::accumulate(month_span.begin(), month_span.end(), 0.0);
            std::cout << "月份 " << std::setw(2) << (month + 1) << ": "
                      << std::setw(10) << std::fixed << std::setprecision(2)
                      << monthly_total << "\n";
        }

        // 计算每个产品的年度总销售额
        std::cout << "\n每个产品的年度总销售额:\n";
        for (int product = 0; product < 5; ++product) {
            auto product_data = sales_data.col(product);
            double product_total = std::accumulate(product_data.begin(), product_data.end(), 0.0);
            std::cout << "产品 " << (product + 1) << ": "
                      << std::setw(10) << std::fixed << std::setprecision(2)
                      << product_total << "\n";
        }

        // 找到最佳月份和产品
        auto max_it = std::max_element(sales_data.begin(), sales_data.end());
        if (max_it != sales_data.end()) {
            auto index = std::distance(sales_data.begin(), max_it);
            int best_month = static_cast<int>(index) / sales_data.cols();
            int best_product = static_cast<int>(index) % sales_data.cols();
            std::cout << "\n最高单月单产品销售额: " << *max_it
                      << " (月份: " << (best_month + 1) << ", 产品: " << (best_product + 1) << ")\n";
        }
    }
}

int main() {
    std::cout << "array2d 类完整使用示例\n";
    std::cout << "编译时间: " << __DATE__ << " " << __TIME__ << "\n";

    try {
        // 运行所有示例
        basic_usage_examples();
        iterator_examples();
        span_examples();
        data_manipulation_examples();
        transpose_examples();
        resize_examples();
        memory_management_examples();
        algorithm_examples();
        template_examples();
        comparison_examples();
        performance_examples();
        error_handling_examples();
        practical_examples();

    } catch (const std::exception &e) {
        std::cerr << "程序执行过程中发生异常: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "程序执行过程中发生未知异常\n";
        return 2;
    }

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "所有示例执行完成！\n";
    std::cout << std::string(60, '=') << "\n";

    return 0;
}