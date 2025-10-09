import os
import json
import shutil
from pathlib import Path
from datetime import datetime

# ===================== Configuration =====================
CONFIG = {
    "cmake_file"        :   "system/platform/arm-cortex/bootloader/CMakeLists.txt", # Path to CMakeLists.txt file
    "config_file"       :   "tools/scripts/ProjectInfo.json",    # Path to configuration file
    "optimize_lvl"      :   "-O0 -g",                   # Compilation optimization level
    "bootloader_dir"    :   "system/platform/arm-cortex/bootloader",    # Bootloader directory for MCU platform
    "mcu_common_dir"    :   "system/platform/arm-cortex/common",    # Common directory for MCU platform
    "mcu_parent_dir"    :   "system/platform/arm-cortex",           # Parent directory for MCU platform
    # Directories containing source and header files to be compiled under SDK root
    # Platform HAL related directories will be handled automatically
    "compile_dir"       :   [ ],        # Directories to compile
    "exclude_dir"       :   ["osal", "FreeRTOS", "Debug", "build", "tools", ".settings", ".vscode"],    # Directories to exclude
    "source_ext"        :   (".cpp", ".c", ".s"),       # Source file extensions
    "header_ext"        :   (".h", ".hpp")              # Header file extensions
}
# =================================================

class CMakeManager:
    def __init__(self, config):
        self.cfg = config
        self.root_path      = Path(__file__).resolve().parent.parent.parent.parent.parent
        self.cmake_path     = self.root_path / self.cfg["cmake_file"]
        self.config_path    = self.root_path / self.cfg["config_file"]


    def _create_cmakefile(self):
        """Create CMakeLists.txt file"""
        with open(self.cmake_path, "w", encoding="utf-8") as file:

            # CMake version and GCC compiler configuration
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

            # Parse ProjectInfo.json to get configuration parameters
            with open(self.config_path, "r", encoding="utf-8") as f:
                json_data = json.load(f)
            Chip        = json_data["chip"]
            ProjectName = json_data["version"]
            Series      = json_data["series"].lower()
            MCU_MACRO   = json_data["macro"]
            Ld_Script   = json_data["load"]
            buildEnv    = json_data["env"].upper()


            # Project name and language
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Project Settings\n"
                        + f"project(\"{ProjectName}_Bootloader\" C CXX ASM)\n"
                        + f"message(STATUS \"----- PROJECT_NAME [${{PROJECT_NAME}}] -----\")\n" )

            # Compiler optimization level
            opt_lvl = self.cfg["optimize_lvl"]
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Compiler optimization level\n"
                        + f"set(OPTIMIZE_LEVEL {opt_lvl})\n"
                        + f"add_compile_options(${{OPTIMIZE_LEVEL}})\n"
                        + f"message(STATUS \"Compiler Optimization Level [${{OPTIMIZE_LEVEL}}]\")\n"
                        + f"message(STATUS \"Using ${{CMAKE_GENERATOR}} generator\")\n" )

            # Compiler options configuration
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

            # F3/F4 series support FPU, enable hardware floating-point configuration
            if Series == "f3" or Series == "f4":
                CMakeLists = ( CMakeLists + "\n"
                        + f"# Hardware FPU configuration (uncomment to enable)\n"
                        + f"add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)\n"
                        + f"add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)\n"
                        + f"add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)\n" )

            # Other series use software floating-point (disabled by default)
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Software floating-point (enable when disabling hardware FPU)\n"
                        + f"# add_compile_options(-mfloat-abi=soft)\n\n"
                        + f"# Mitigate c++17 absolute addresses warnings (uncomment to enable)\n"
                        + f"# set(CMAKE_CXX_FLAGS \"${{CMAKE_CXX_FLAGS}} -Wno-register\")\n" )

            # MCU macro settings
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Global MACRO definitions\n"
                        + f"add_definitions(-DUSE_HAL_DRIVER -D{MCU_MACRO})\n" )

            # Get SDK Root directory
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Get SDK Root directory\n"
                        + f"get_filename_component(SDK_ROOT_DIR \"${{CMAKE_SOURCE_DIR}}/../../../../\" ABSOLUTE)\n\n" )



            # Update the compile_dir list based on platform and chip
            HAL_Dir_List = []
            # HAL_Dir_List = self.cfg["mcu_parent_dir"] + "/" + Chip + "/Bootloader"
            HAL_Dir_List.append(self.cfg["mcu_parent_dir"] + "/" + Chip + "/Bootloader") # Must locate in HAL_Dir_List[0]
            HAL_Dir_List.append(self.cfg["mcu_common_dir"])

            HAL_Dir_List.append(self.cfg["bootloader_dir"])
            self.cfg["compile_dir"].extend(HAL_Dir_List)

            compile_set = set(self.cfg["compile_dir"])  # Convert list to set for deduplication
            print("\n----------- [Bootloader] Directories to be compiled (Relative to SDK root) -------------")
            for dir in compile_set:
                print(f"-- {dir}")
            print("----------------------------------------------------------------------------------------")


            # Automatically recursively find files to compile
            sources = self._recurve_find_files(
                compile_set,                # Set of directories to compile
                self.cfg["source_ext"],     # List of source file extensions
                self.cfg["exclude_dir"]     # Directories to exclude
            )
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Add source file paths recursively (.c/.s files) with dynamic refresh\n"
                        + f"# [CONFIGURE DEPENDS] supports dynamic lookup of add-on files\n"
                        + f"file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS\n" )
            for source in sources:
                CMakeLists = ( CMakeLists + f"    ${{SDK_ROOT_DIR}}/{source}\n" )
            CMakeLists = ( CMakeLists + f")\n\n" )


            # Automatically recursively find directories to include
            includes = self._recurve_find_files(
                compile_set,                # Set of directories to include
                self.cfg["header_ext"],     # List of header file extensions
                self.cfg["exclude_dir"]     # Directories to exclude
            )
            includes = sorted({str(Path(f).parent.as_posix()) for f in includes}) # Remove extensions, keep only directory names

            CMakeLists = ( CMakeLists + "\n"
                        + f"# Add global include paths (.h files)\n"
                        + f"include_directories(\n" )
            for include in includes:
                CMakeLists = ( CMakeLists + f"    ${{SDK_ROOT_DIR}}/{include}\n" )
            CMakeLists = ( CMakeLists + f")\n\n" )



            # Linker configuration
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Linker Configuration\n"
                        + f"set(LINKER_SCRIPT ${{SDK_ROOT_DIR}}/{HAL_Dir_List[0]}/{Ld_Script})\n"
                        + f"add_link_options(-T ${{LINKER_SCRIPT}})\n"
                        + f"add_link_options(-m{cpu_cortex} -mthumb -mthumb-interwork)\n"
                        + f"add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${{PROJECT_BINARY_DIR}}/${{PROJECT_NAME}}.map)\n"
                        + f"add_link_options(-specs=nano.specs -specs=nosys.specs)\n"
                        + f"add_link_options(-static -Wl,--start-group -lc -lm -Wl,--end-group)\n"
                        + f"# Enable float printf support (+~10KB firmware size)\n"
                        + f"# add_link_options(-u _printf_float)\n" )

           # Generate ELF executable
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Generate ELF output\n"
                        + f"add_executable(${{PROJECT_NAME}}.elf ${{SOURCES}})\n" )
            # Set QUECTEL_PROJECT_VERSION & QUECTEL_BUILD_ENV macros
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Set the pre-processor MACRO\n"
                        + f"target_compile_definitions(${{PROJECT_NAME}}.elf PRIVATE QUECTEL_PROJECT_VERSION=\"${{PROJECT_NAME}}\" PRIVATE QUECTEL_BUILD_ENV=\"{buildEnv}\")\n" )
            # Set output file paths
            CMakeLists = ( CMakeLists + "\n"
                        + f"# Set output file paths\n"
                        + f"set(HEX_FILE ${{PROJECT_BINARY_DIR}}/${{PROJECT_NAME}}.hex)\n"
                        + f"set(BIN_FILE ${{PROJECT_BINARY_DIR}}/${{PROJECT_NAME}}.bin)\n" )
            # Set HEX/BIN generation rules
            CMakeLists = ( CMakeLists + "\n"
                        + f"# HEX/BIN generation rules\n"
                        + f"add_custom_command(TARGET ${{PROJECT_NAME}}.elf POST_BUILD\n"
                        + f"        COMMAND ${{CMAKE_OBJCOPY}} -Oihex $<TARGET_FILE:${{PROJECT_NAME}}.elf> ${{HEX_FILE}}\n"
                        + f"        COMMAND ${{CMAKE_OBJCOPY}} -Obinary $<TARGET_FILE:${{PROJECT_NAME}}.elf> ${{BIN_FILE}}\n"
                        + f"        COMMENT \"Generating build artifacts:\\nHEX: ${{HEX_FILE}}\\nBIN: ${{BIN_FILE}}\"\n"
                        + f"        VERBATIM\n"
                        + f")\n" )

            # Write to file CMakeLists.txt
            print("-- [Bootloader] Generating is in progress...")
            file.write(CMakeLists)
            print(f"-- [{self.cmake_path}] generated successfully!")
            print(f"-- [Bootloader] Done !")



    def _recurve_find_files(self, dirs, exts, exclude):
        """Find directories containing source files, generate wildcard paths for corresponding extensions"""
        dir_ext_map = {}  # Store mapping of directory paths to sets of extensions (Dictionary)

        # Traverse directories to compile (compile_dir list)
        for base_dir in dirs:
            base_path = self.root_path / base_dir
            if not base_path.exists():
                continue

            for root, dirs, files in os.walk(base_path):
                # Dynamically exclude directories
                dirs[:] = [d for d in dirs if d not in exclude]

                # Collect valid extensions in current directory
                found_exts = set()
                for f in files:
                    file_ext = Path(f).suffix.lower()
                    if file_ext in exts:
                        found_exts.add(file_ext)

                if found_exts:
                    try:
                        # Convert to Unix-style directory path
                        dir_path = Path(root).relative_to(self.root_path).as_posix()

                        # Generate wildcard for each extension
                        if dir_path not in dir_ext_map:
                            dir_ext_map[dir_path] = set()
                        dir_ext_map[dir_path].update(found_exts)

                    except ValueError:
                        continue  # Ignore files outside project root

        # Generate final path list
        result = []
        for dir_path, exts in dir_ext_map.items():
            for ext in exts:
                result.append(f"{dir_path}/*{ext}")
        return sorted(result)


    def run(self):
        """Start to generate configuration files"""
        try:
            # Create the CMakeLists.txt file
            self._create_cmakefile()

        except Exception as e:
            print(f"\nError: {str(e)}")
            print("Update aborted. Original file remains unchanged\n")


if __name__ == "__main__":
    manager = CMakeManager(CONFIG)
    manager.run()
