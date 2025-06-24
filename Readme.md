# Array2D - 二维数组容器

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/your-repo/array2d)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)](https://github.com/your-repo/array2d)

`array2d` 是一个 C++20 二维数组，封装了标准库 `std::vector`，提供了直观的矩阵操作接口和优化的内存管理。

> [!WARNING]
> 代码使用 AI 辅助生成，已通过测试。

## ✨ 特性

- 🚀 **高性能**: 连续内存存储，缓存友好的算法实现
- 🛡️ **类型安全**: 使用 C++20 概念进行编译时类型检查
- 🧩 **STL 兼容**: 完整的迭代器支持，兼容标准算法
- 🔧 **易于使用**: 直观的 API 设计，支持多种初始化方式  
- 🔄 **移动语义**: 完整的移动语义支持

## 📋 目录

- [快速开始](#-快速开始)
- [安装](#-安装)
- [基本用法](#-基本用法)
- [高级特性](#-高级特性)
- [API 参考](#-api-参考)
- [许可证](#-许可证)

## 🚀 快速开始

```cpp
#include "array2d.hpp"
using namespace qm;

int main() {
    // 创建 3x4 矩阵并初始化为 0
    array2d<int> matrix(3, 4);
    
    // 使用初始化列表创建矩阵
    array2d<double> mat{{1.0, 2.0, 3.0},
                        {4.0, 5.0, 6.0}};
    
    // 访问元素
    mat[0][1] = 2.5;        // 使用 operator[]
    mat(1, 2) = 6.5;        // 使用 operator()
    
    // 安全访问（带边界检查）
    try {
        double val = mat.at(1, 2);
    } catch (const std::out_of_range& e) {
        // 处理越界访问
    }
    
    // 遍历矩阵
    for (auto& element : mat) {
        element *= 2;
    }
    
    return 0;
}
```

## 📦 安装

### 头文件引入

`array2d` 是一个头文件库，只需要包含头文件即可：

```cpp
#include "array2d.hpp"
#include "array2d_iterator.hpp"  // 如果需要自定义迭代器
```


### 依赖要求

- **编译器**: 支持 C++20 的编译器
  - GCC 10+ 
  - Clang 12+
  - MSVC 19.29+ (Visual Studio 2019 16.10+)
- **标准库**: 支持 C++20 标准库特性

## 📖 基本用法

### 创建矩阵

```cpp
// 默认构造（空矩阵）
array2d<int> empty_matrix;

// 指定尺寸（默认初始化）
array2d<double> matrix1(5, 3);

// 指定尺寸和初值
array2d<int> matrix2(4, 4, 42);

// 初始化列表构造
array2d<float> matrix3{{1.0f, 2.0f}, 
                       {3.0f, 4.0f}};

// 从容器构造
std::vector<int> data{1, 2, 3, 4, 5, 6};
array2d<int> matrix4(2, 3, data);

// 类模板参数推导 (C++17)
auto matrix5 = array2d{{1, 2, 3},
                       {4, 5, 6}};  // 推导为 array2d<int>
```

### 元素访问

```cpp
array2d<int> mat(3, 3, 0);

// 多种访问方式
mat[0][1] = 10;          // operator[] - 快速访问
mat(1, 2) = 20;          // operator() - 函数式访问  
mat.at(2, 0) = 30;       // at() - 安全访问（边界检查）

// 常量访问
const auto& const_mat = mat;
int val = const_mat[0][1];
```

### 迭代器支持

```cpp
array2d<int> mat{{1, 2, 3}, {4, 5, 6}};

// 范围 for 循环
for (auto& elem : mat) {
    elem *= 2;
}

// STL 算法
auto it = std::find(mat.begin(), mat.end(), 8);
std::sort(mat.begin(), mat.end());

// 反向迭代
for (auto it = mat.rbegin(); it != mat.rend(); ++it) {
    std::cout << *it << " ";
}

// 行迭代
for (auto& elem : mat.row_range(0)) {
    elem += 10;  // 只修改第一行
}
```

### 基本操作

```cpp
array2d<double> mat(4, 3, 1.0);

// 查询信息
std::cout << "Rows: " << mat.rows() << std::endl;        // 4
std::cout << "Cols: " << mat.cols() << std::endl;        // 3  
std::cout << "Size: " << mat.size() << std::endl;        // 12
std::cout << "Empty: " << mat.empty() << std::endl;      // false
std::cout << "Square: " << mat.is_square() << std::endl; // false

// 填充操作
mat.fill(3.14);           // 填充所有元素
mat.reset();              // 重置为默认值
mat.fill_row(0, 2.71);    // 只填充第一行

// 尺寸调整
mat.resize(5, 4);         // 调整为 5x4，保留原有数据
mat.resize(3, 3, 0.0);    // 调整为 3x3，新元素用 0.0 填充
```

## 🔧 高级特性

### Span 和视图操作

```cpp
array2d<int> mat{{1, 2, 3, 4},
                 {5, 6, 7, 8},
                 {9, 10, 11, 12}};

// 获取整个矩阵的 span
auto full_span = mat.as_span();

// 获取特定行的 span
auto row1_span = mat.row(1);  // [5, 6, 7, 8]

// 获取列数据（需要复制，因为不连续）
auto col2_data = mat.col(2);  // [3, 7, 11]

// 子矩阵视图（行优先连续部分）
auto sub_span = mat.submatrix_row_major(1, 0, 2, 4);  // 第1-2行完整数据
```

### 行操作

```cpp
array2d<int> mat(4, 3);

// 行复制
mat.copy_row(0, 1);      // 将第0行复制到第1行

// 行交换  
mat.swap_rows(1, 3);     // 交换第1行和第3行

// 行填充
mat.fill_row(2, 42);     // 将第2行填充为42
```

### 矩阵变换

```cpp
// 方阵就地转置
array2d<double> square_mat(3, 3);
square_mat.transpose();  // 就地转置

// 任意尺寸矩阵转置（返回新矩阵）
array2d<int> rect_mat(2, 3);
auto transposed = rect_mat.transposed();  // 3x2 矩阵
```

### 内存管理优化

```cpp
array2d<int> mat;

// 预分配内存
mat.reserve(1000, 1000);  // 为100万元素预分配内存

// 释放多余内存
mat.shrink_to_fit();

// 获取底层数据指针
int* data_ptr = mat.data();
```

### 并行操作

```cpp
array2d<double> large_mat(10000, 10000);

// 并行填充（自动判断是否使用并行）
large_mat.fill_parallel(3.14159);
```

### 自定义索引类型

```cpp
// 使用 long 作为索引类型
array2d<double, long> big_matrix(1000000L, 1000000L);

// 使用 short 节省内存（小矩阵）
array2d<int, short> small_matrix(100, 100);
```

## 📚 API 参考

### 构造函数

| 构造函数 | 描述 |
|----------|------|
| `array2d()` | 默认构造，创建空矩阵 |
| `array2d(rows, cols)` | 创建指定尺寸的矩阵 |
| `array2d(rows, cols, value)` | 创建矩阵并用指定值初始化 |
| `array2d({...})` | 初始化列表构造 |
| `array2d(rows, cols, container)` | 从容器数据构造 |

### 元素访问

| 方法 | 描述 | 异常安全 |
|------|------|----------|
| `operator[](row)` | 返回行指针，支持 `mat[i][j]` | 无边界检查 |
| `operator()(row, col)` | 直接访问元素 | 无边界检查 |
| `at(row, col)` | 安全访问元素 | 抛出 `std::out_of_range` |

### 迭代器

| 方法 | 描述 |
|------|------|
| `begin()`, `end()` | 正向迭代器 |
| `cbegin()`, `cend()` | 常量正向迭代器 |
| `rbegin()`, `rend()` | 反向迭代器 |
| `row_range(row)` | 行范围迭代器 |

### 容量和尺寸

| 方法 | 描述 |
|------|------|
| `rows()`, `cols()` | 获取行数/列数 |
| `size()` | 获取总元素数 |
| `empty()` | 检查是否为空 |
| `is_square()` | 检查是否为方阵 |
| `capacity()` | 获取容量 |
| `reserve(rows, cols)` | 预分配内存 |
| `shrink_to_fit()` | 释放多余内存 |

### 数据操作

| 方法 | 描述 |
|------|------|
| `fill(value)` | 填充所有元素 |
| `fill_parallel(value)` | 并行填充 |
| `reset(option)` | 重置内存 |
| `resize(rows, cols)` | 调整尺寸 |

### 行操作

| 方法 | 描述 |
|------|------|
| `copy_row(src, dest)` | 复制行 |
| `swap_rows(row1, row2)` | 交换行 |
| `fill_row(row, value)` | 填充行 |

### 视图和 Span

| 方法 | 描述 |
|------|------|
| `as_span()` | 整个矩阵的 span |
| `row(index)` | 行 span |
| `col(index)` | 列数据（vector） |
| `submatrix_row_major(...)` | 子矩阵 span |

### 变换操作

| 方法 | 描述 |
|------|------|
| `transpose()` | 就地转置（仅方阵） |
| `transposed()` | 返回转置矩阵 |


## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

---

**如果这个项目对你有帮助，请给我们一个 ⭐！**