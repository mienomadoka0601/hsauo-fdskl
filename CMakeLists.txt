# 最低 CMake 版本要求（兼容大多数系统，建议 3.16+）
cmake_minimum_required(VERSION 3.16)

# 项目名称 + 语言（C++17 足够支持所有特性：filesystem、std::optional 等）
project(gitlite 
    VERSION 1.0 
    LANGUAGES CXX
)

# ===================== 基础配置 =====================
# 设置 C++ 标准（强制 C++17，因为用到 std::filesystem、std::string_view 等）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用编译器扩展，保证跨平台

# 编译选项：调试模式加 -g，发布模式加优化；添加警告提示
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -Wall -Wextra -Werror) # 调试模式：警告视为错误
else()
    add_compile_options(-O2 -Wall -Wextra) # 发布模式：优化 + 警告
endif()

# 设置输出目录（可执行文件、库文件统一输出到 build/bin 和 build/lib）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ===================== 头文件路径 =====================
# 包含项目头文件目录（让编译器能找到所有 .h 文件）
include_directories(
    ${PROJECT_SOURCE_DIR}/src          # src 下的头文件
    ${PROJECT_SOURCE_DIR}/include      # 公共头文件（可选）
)

# ===================== 源文件收集 =====================
# 核心模块源文件（Commit、StagingArea、Repository 等）
set(CORE_SOURCES
    src/Blob.cpp
    src/Repository.cpp
    src/StagingArea.cpp
    src/Commit.cpp
    src/SomeObj.cpp
    src/object.cpp
    src/Tree.cpp
    src/Branch.cpp
)

# 工具类源文件（Utils、GitliteException 等）
set(UTILS_SOURCES
    src/Utils.cpp
    src/GitliteException.cpp
)

# 命令模块源文件（commit、rm、log 等命令）
set(COMMAND_SOURCES
    src/command/commit.cpp
    src/command/rm.cpp
    src/command/log.cpp
    src/command/init.cpp
    src/command/add.cpp
    src/command/status.cpp
    src/command/add-remote.cpp
    src/command/rm-remote.cpp
    src/command/branch.cpp
    src/command/checkout.cpp
    src/command/fetch.cpp
    src/command/global-log.cpp
    src/command/merge.cpp
    src/command/pull.cpp
    src/command/push.cpp
    src/command/remote.cpp
    src/command/reset.cpp
    src/command/rm-branch.cpp
    src/command/find.cpp
)

# 主程序入口
set(MAIN_SOURCE
    main.cpp
)

# ===================== 依赖库链接 =====================
# 1. 链接 std::filesystem（C++17 需显式链接，不同编译器处理不同）
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # GCC/Clang 需要链接 stdc++fs
    link_libraries(stdc++fs)
endif()

# ===================== 生成可执行文件 =====================
# 合并所有源文件，生成可执行文件 gitlite
add_executable(gitlite
    ${MAIN_SOURCE}
    ${CORE_SOURCES}
    ${UTILS_SOURCES}
    ${COMMAND_SOURCES}
)

# 链接依赖库（OpenSSL 的 crypto 库 + 系统库）
target_link_libraries(gitlite
    PRIVATE
    ${OPENSSL_CRYPTO_LIBRARY}
)