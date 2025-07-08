import os
import json
import shutil
from pathlib import Path
from datetime import datetime

# ===================== Configuration =====================
CONFIG = {
    "cmake_file"        :   "CMakeLists.txt",           # CMakeLists.txt文件路径
    "backup_folder"     :   "tools/scripts/backup",     # 备份目录
    "max_backups"       :   5,                          # 最大备份数量
    "config_file"       :   "tools/scripts/ProjectInfo.json",    # 配置文件路径
    "optimize_lvl"      :   "-O0 -g",                   # 编译优化等级
    "system_inc_dir"    :   "system/include",           # system的公用include目录
    "mcu_common_dir"    :   "system/platform/arm-cortex/common",   # MCU平台的common目录
    "mcu_parent_dir"    :   "system/platform/arm-cortex",   # MCU平台的父级目录
    "win_parent_dir"    :   "system/platform/windows",      # Windows平台的父级目录
    # SDK根目录下，要编译的源码和头文件所在目录（平台HAL相关代码的目录，该脚本会自动处理）
    "compile_dir"       :   ["apps", "quectel"],        # 要编译的目录
    "exclude_dir"       :   ["Debug", "build", "tools", ".settings", ".vscode"],    # 要排除的目录
    "source_ext"        :   (".cpp", ".c", ".s"),       # 源码扩展名
    "header_ext"        :   (".h", ".hpp")              # 头文件扩展名
}
# =================================================

class CMakeManager:
    def __init__(self, config):
        self.cfg = config
        self.root_path      = Path(__file__).parent.resolve()
        self.cmake_path     = self.root_path / self.cfg["cmake_file"]
        self.backup_path    = self.root_path / self.cfg["backup_folder"]
        self.config_path    = self.root_path / self.cfg["config_file"]


    def _create_cmakefile(self):
        """创建CMakeLists.txt文件"""
        with open(self.cmake_path, "w", encoding="utf-8") as file:

            # CMake版本和GCC编译器等配置
            CMakeLists = (f"###############################################################################\n"
                        + f"# CMake Toolchain Configuration for STM32 Embedded Project\n"
                        + f"# THIS FILE IS AUTO GENERATED FROM THE TEMPLATE! DO NOT CHANGE!\n"
                        + f"###############################################################################\n"
                        + f"cmake_minimum_required(VERSION 3.20)\n"
                        + f"set(CMAKE_SYSTEM_NAME Generic)\n"
                        + f"set(CMAKE_SYSTEM_VERSION 1)\n\n"
                        + f"# Cross-compilation Toolchain Setup\n"
                        + f"set(CMAKE_C_COMPILER arm-none-eabi-gcc)\n"
                        + f"set(CMAKE_CXX_COMPILER arm-none-eabi-g++)\n"
                        + f"set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)\n"
                        + f"set(CMAKE_AR arm-none-eabi-ar)\n"
                        + f"set(CMAKE_OBJCOPY arm-none-eabi-objcopy)\n"
                        + f"set(CMAKE_OBJDUMP arm-none-eabi-objdump)\n"
                        + f"set(SIZE arm-none-eabi-size)\n"
                        + f"set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)\n"
                        + f"set(CMAKE_C_STANDARD 11)\n"
                        + f"set(CMAKE_CXX_STANDARD 17)\n" )

            # 解析ProjectInfo.json 获取配置参数
            with open(self.config_path, "r", encoding="utf-8") as f:
                json_data = json.load(f)
            Chip        = json_data["chip"]
            ProjectName = json_data["version"]
            Series      = json_data["series"].lower()
            MCU_MACRO   = json_data["macro"]
            Ld_Script   = json_data["load"]


            # 项目名称版本和语言
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Project Settings\n"
                        + f"project(\"{ProjectName}\" C CXX ASM)\n"
                        + f"message(STATUS \"----- PROJECT_NAME [${{PROJECT_NAME}}] -----\")\n" )

            # 编译器优化等级
            opt_lvl = self.cfg["optimize_lvl"]
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Compiler optimization level\n"
                        + f"set(OPTIMIZE_LEVEL {opt_lvl})\n"
                        + f"add_compile_options(${{OPTIMIZE_LEVEL}})\n"
                        + f"message(STATUS \"Compiler Optimization Level [${{OPTIMIZE_LEVEL}}]\")\n"
                        + f"message(STATUS \"Using ${{CMAKE_GENERATOR}} generator\")\n" )

            # 编译器选项配置
            if Series == "f0":
                cpu_cortex = "cpu=cortex-m0"
            elif Series == "f1":
                cpu_cortex = "cpu=cortex-m3"
            elif Series == "f3" or Series == "f4":
                cpu_cortex = "cpu=cortex-m4"
            else:
                cpu_cortex = "cpu=cortex-m4"
            
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Compiler options configuration\n"
                        + f"add_compile_options(-m{cpu_cortex} -mthumb -mthumb-interwork)\n"
                        + f"add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)\n"
                        + f"add_compile_options(-std=gnu11 -Wall -fstack-usage)\n" )

            # F3/F4全系支持FPU 导入硬浮点配置
            if Series == "f3" or Series == "f4":
                CMakeLists = ( CMakeLists + "\n"
                        + f"# Hardware FPU configuration (uncomment to enable)\n"
                        + f"add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)\n"
                        + f"add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)\n"
                        + f"add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)\n" )
            
            # 其他系列用软浮点 默认屏蔽
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Software floating-point (enable when disabling hardware FPU)\n"
                        + f"# add_compile_options(-mfloat-abi=soft)\n\n"
                        + f"# Mitigate c++17 absolute addresses warnings (uncomment to enable)\n"
                        + f"# set(CMAKE_CXX_FLAGS \"${{CMAKE_CXX_FLAGS}} -Wno-register\")\n" )
            
            # MCU宏设置
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Global MACRO definitions\n"
                        + f"add_definitions(-DUSE_HAL_DRIVER -D{MCU_MACRO})\n\n" )
            


            # 根据平台和芯片型号 来更新要编译的目录列表compile_dir
            HAL_Dir_List = []
            if Series == "windows":
                # HAL_Dir_List = self.cfg["win_parent_dir"]
                HAL_Dir_List.append(self.cfg["win_parent_dir"])
            else:
                # HAL_Dir_List = self.cfg["mcu_parent_dir"] + "/" + Chip
                HAL_Dir_List.append(self.cfg["mcu_parent_dir"] + "/" + Chip) # Must locate in HAL_Dir_List[0]
                HAL_Dir_List.append(self.cfg["mcu_common_dir"])

            HAL_Dir_List.append(self.cfg["system_inc_dir"])
            self.cfg["compile_dir"].extend(HAL_Dir_List)

            compile_set = set(self.cfg["compile_dir"])  # 列表转换为集合去重
            print("\n----------- Directories to be compiled (Relative to SDK root) -------------")
            for dir in compile_set:
                print(f"-- {dir}")
            print("---------------------------------------------------------------------------")


            # 自动递归查找要编译的文件
            sources = self._recurve_find_files(
                compile_set,                # 要编译的目录集合
                self.cfg["source_ext"],     # 源文件扩展名列表
                self.cfg["exclude_dir"]     # 要排除的目录
            )
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Add source file paths recursively (.c/.s files) with dynamic refresh\n"
                        + f"# [CONFIGURE DEPENDS] supports dynamic lookup of add-on files\n"
                        + f"file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS\n" )
            for source in sources:
                CMakeLists = ( CMakeLists + f"    ${{CMAKE_SOURCE_DIR}}/{source}\n" )
            CMakeLists = ( CMakeLists + f")\n\n" )


            # 自动递归查找要包含的目录
            includes = self._recurve_find_files(
                compile_set,                # 要包含的目录集合
                self.cfg["header_ext"],     # 头文件扩展名列表
                self.cfg["exclude_dir"]     # 要排除的目录
            )
            includes = sorted({str(Path(f).parent.as_posix()) for f in includes}) # 去除扩展名，只留目录名

            CMakeLists = ( CMakeLists + "\n"
                        + f"# Add global include paths (.h files)\n"
                        + f"include_directories(\n" )   
            for include in includes:
                CMakeLists = ( CMakeLists + f"    ${{CMAKE_SOURCE_DIR}}/{include}\n" )
            CMakeLists = ( CMakeLists + f")\n\n" )



            # 链接器配置
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Linker Configuration\n"
                        + f"set(LINKER_SCRIPT ${{CMAKE_SOURCE_DIR}}/{HAL_Dir_List[0]}/{Ld_Script})\n"
                        + f"add_link_options(-T ${{LINKER_SCRIPT}})\n"
                        + f"add_link_options(-m{cpu_cortex} -mthumb -mthumb-interwork)\n"
                        + f"add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${{PROJECT_BINARY_DIR}}/${{PROJECT_NAME}}.map)\n"
                        + f"add_link_options(-specs=nano.specs -specs=nosys.specs)\n"
                        + f"add_link_options(-static -Wl,--start-group -lc -lm -Wl,--end-group)\n"
                        + f"# Enable float printf support (+~10KB firmware size)\n"
                        + f"# add_link_options(-u _printf_float)\n" )

            # 生成ELF可执行文件
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Generate ELF output\n"
                        + f"add_executable(${{PROJECT_NAME}}.elf ${{SOURCES}})\n" )
            # 设置项目版本号宏
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Set the pre-processor MACRO\n"
                        + f"target_compile_definitions(${{PROJECT_NAME}}.elf PRIVATE QUECTEL_PROJECT_VERSION=\"${{PROJECT_NAME}}\")\n" )
            # 设置输出文件路径
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Set output file paths\n"
                        + f"set(HEX_FILE ${{PROJECT_BINARY_DIR}}/${{PROJECT_NAME}}.hex)\n"
                        + f"set(BIN_FILE ${{PROJECT_BINARY_DIR}}/${{PROJECT_NAME}}.bin)\n" )
            # 设置HEX/BIN生成规则
            CMakeLists = ( CMakeLists + "\n"
                        + f"# HEX/BIN generation rules\n"
                        + f"add_custom_command(TARGET ${{PROJECT_NAME}}.elf POST_BUILD\n"
                        + f"        COMMAND ${{CMAKE_OBJCOPY}} -Oihex $<TARGET_FILE:${{PROJECT_NAME}}.elf> ${{HEX_FILE}}\n"
                        + f"        COMMAND ${{CMAKE_OBJCOPY}} -Obinary $<TARGET_FILE:${{PROJECT_NAME}}.elf> ${{BIN_FILE}}\n"
                        + f"        COMMENT \"Generating build artifacts:\\nHEX: ${{HEX_FILE}}\\nBIN: ${{BIN_FILE}}\"\n"
                        + f"        VERBATIM\n"
                        + f")\n" )

            # 回写文件 CMakeLists.txt
            print("-- Generating is in progress...")
            file.write(CMakeLists)

            # 输出摘要信息
            print(f"\n----------------Summary Info----------------")
            print(f"-- Sources:  [{len(sources)}] directories found")
            print(f"-- Includes: [{len(includes)}] directories found")
            print(f"-- Project name [{ProjectName}] updated successfully")
            print(f"-- Backup protection: [{self.backup_path}]")
            print(f"-- CMakeLists.txt: [{self.cmake_path}]")
            print(f"-- Done !")



    def _create_backup(self):
        """创建带时间戳的备份"""
        if self.cmake_path.exists():
            self.backup_path.mkdir(exist_ok=True)

            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            backup_file = self.backup_path / f"{self.cmake_path.name}.bak_{timestamp}"
            shutil.copy(self.cmake_path, backup_file)
            print(f"-- Backup created: {backup_file.name}")
            
            # 清理旧备份
            backups = sorted(self.backup_path.glob("*.bak_*"), key=os.path.getmtime)
            while len(backups) > self.cfg["max_backups"]:
                os.remove(backups.pop(0))


    def _recurve_find_files(self, dirs, exts, exclude):
        """查找包含源文件的目录，生成对应扩展名的通配符路径"""
        dir_ext_map = {}  # 存储目录路径到扩展名集合的映射(字典)

        # 遍历要编译的目录(compile_dir列表)
        for base_dir in dirs:
            base_path = self.root_path / base_dir
            if not base_path.exists():
                continue

            for root, dirs, files in os.walk(base_path):
                # 动态排除目录
                dirs[:] = [d for d in dirs if d not in exclude]

                # 收集当前目录的有效扩展名
                found_exts = set()
                for f in files:
                    file_ext = Path(f).suffix.lower()
                    if file_ext in exts:
                        found_exts.add(file_ext)

                if found_exts:
                    try:
                        # 转换为Unix风格目录路径
                        dir_path = Path(root).relative_to(self.root_path).as_posix()
                        
                        # 为每个扩展名生成通配符
                        if dir_path not in dir_ext_map:
                            dir_ext_map[dir_path] = set()
                        dir_ext_map[dir_path].update(found_exts)
                        
                    except ValueError:
                        continue  # 忽略项目根目录外的文件

        # 生成最终路径列表
        result = []
        for dir_path, exts in dir_ext_map.items():
            for ext in exts:
                result.append(f"{dir_path}/*{ext}")     
        return sorted(result)


    def run(self):
        """Start to generate configuration files"""
        try:
            # Create backup
            self._create_backup()

            # Create a CMakeLists.txt file
            self._create_cmakefile()

        except Exception as e:
            print(f"\nError: {str(e)}")
            print("Update aborted. Original file remains unchanged\n")


if __name__ == "__main__":
    manager = CMakeManager(CONFIG)
    manager.run()