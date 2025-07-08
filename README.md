# Quectel User Friendly SDK 使用说明

## 主要特性

1. 本SDK编译框架支持 **`STM32全系所有型号`**
2. 不同型号MCU的所有相关参数、文件，**`自动适配`**
3. CMakeLists.txt、CMakePresets.json 等构建文件 **`自动生成`**
4. build.bat 主要功能：**`构建`**、**`编译`**、**`清理`**、**`下载`**、**`调试`**
5. SDK内置交叉编译工具链，编译环境 **`无需手动搭建`**
6. 配套自动化脚本，一键完成上述所有操作，无需手动配置，**`开箱即用`**

## 目录结构

    ├── 📁 .vscode              # VSCode调试环境配置（可选，脚本自动生成）
    ├── 📁 apps                 # 应用程序目录，包含各功能example和app入口
    ├── 📁 build                # 构建输出目录，包含编译生成的中间文件和可执行文件
    ├── 📁 quectel              # Quectel相关代码适配目录
    ├── 📁 system               # 系统平台目录，包含各型号MCU驱动适配HAL相关代码
    ├── 📁 tools                # 工具脚本目录，包含交叉编译工具链、脚本、配置等
    ├── 📄 .clang-format        # Clang代码风格规范文件
    ├── 📄 .editorconfig        # 跨编辑器格式统一配置文件
    ├── 📄 .gitignore           # Git版本控制忽略规则
    ├── 📄 build.bat            # SDK脚本，执行构建、编译、下载、调试等命令
    ├── 📄 CMakeLists.txt       # CMake项目构建主配置（脚本自动生成）
    ├── 📄 CMakePresets.json    # CMake项目构建预设参数（脚本自动生成）
    └── 📄 README.md            # 说明文档

## 构建环境

    本SDK在tools目录中已提供交叉编译工具链。


## 编译命令：
    在SDK根目录下，执行：

    build.bat config      # 构建系统
    build.bat all         # 编译
    build.bat clean       # 清理
    build.bat download    # 下载
    build.bat debug       # 调试

## 说明：
    build.bat config 命令后面可带 [芯片型号][版本号] 两个参数，例如：
    build.bat config STM32F413RGT6 your_firmware_version

    [芯片型号][版本号] 两个参数缺省的情况下，使用上次配置的芯片型号和版本号。
    若首次使用无先前配置记录，则芯片默认使用STM32F413RGT6，版本号默认使用格式Quectel_UFP_Chip_Date，例如Quectel_UFP_STM32F413RGT6_20250430
