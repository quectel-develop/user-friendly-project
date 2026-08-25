###############################################################################
# CMake Toolchain Configuration for STM32 Embedded Project
# THIS FILE IS AUTO GENERATED FROM THE TEMPLATE! DO NOT CHANGE!
###############################################################################
cmake_minimum_required(VERSION 3.20)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

# Cross-compilation Toolchain Setup
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Project Settings
project("Quectel_UFP_STM32F413RGT6_20251223" C CXX ASM)
message(STATUS "----- PROJECT_NAME [${PROJECT_NAME}] -----")

# Compiler optimization level
set(OPTIMIZE_LEVEL -O0 -g)
add_compile_options(${OPTIMIZE_LEVEL})
message(STATUS "Compiler Optimization Level [${OPTIMIZE_LEVEL}]")
message(STATUS "Using ${CMAKE_GENERATOR} generator")

# Compiler options configuration
add_compile_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)
add_compile_options(-std=gnu11 -Wall -fstack-usage)

# Hardware FPU configuration (uncomment to enable)
add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

# Software floating-point (enable when disabling hardware FPU)
# add_compile_options(-mfloat-abi=soft)

# Mitigate c++17 absolute addresses warnings (uncomment to enable)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

# Global MACRO definitions
add_definitions(-DUSE_HAL_DRIVER -DSTM32F413xx)


# Add source file paths recursively (.c/.s files) with dynamic refresh
# [CONFIGURE DEPENDS] supports dynamic lookup of add-on files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/apps/example/fota/*.c
    ${CMAKE_SOURCE_DIR}/apps/example/fs/*.c
    ${CMAKE_SOURCE_DIR}/apps/example/ftp/*.c
    ${CMAKE_SOURCE_DIR}/apps/example/http/*.c
    ${CMAKE_SOURCE_DIR}/apps/example/mqtt/*.c
    ${CMAKE_SOURCE_DIR}/apps/example/net/*.c
    ${CMAKE_SOURCE_DIR}/apps/example/psm/*.c
    ${CMAKE_SOURCE_DIR}/apps/example/sockets/*.c
    ${CMAKE_SOURCE_DIR}/apps/test/*.c
    ${CMAKE_SOURCE_DIR}/apps/test/file/*.c
    ${CMAKE_SOURCE_DIR}/apps/test/fota/*.c
    ${CMAKE_SOURCE_DIR}/apps/test/ftp/*.c
    ${CMAKE_SOURCE_DIR}/apps/test/http/*.c
    ${CMAKE_SOURCE_DIR}/apps/test/mqtt/*.c
    ${CMAKE_SOURCE_DIR}/apps/test/network/*.c
    ${CMAKE_SOURCE_DIR}/apps/test/psm/*.c
    ${CMAKE_SOURCE_DIR}/apps/test/sockets/*.c
    ${CMAKE_SOURCE_DIR}/apps/user/*.c
    ${CMAKE_SOURCE_DIR}/quectel/common/src/*.c
    ${CMAKE_SOURCE_DIR}/quectel/modules/src/*.c
    ${CMAKE_SOURCE_DIR}/quectel/third_party/at_client/src/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/STM32F413RGT6/App/Core/Src/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/STM32F413RGT6/App/Core/Startup/*.s
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/STM32F413RGT6/App/Drivers/STM32F4xx_HAL_Driver/Src/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/FATFS/App/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/FATFS/Target/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/Middlewares/Third_Party/FatFs/src/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/Middlewares/Third_Party/FatFs/src/option/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/Middlewares/Third_Party/FreeRTOS/Source/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/hal/*.c
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/osal/*.c
)


# Add global include paths (.h files)
include_directories(
    ${CMAKE_SOURCE_DIR}/apps/example/fota
    ${CMAKE_SOURCE_DIR}/apps/example/fs
    ${CMAKE_SOURCE_DIR}/apps/example/ftp
    ${CMAKE_SOURCE_DIR}/apps/example/http
    ${CMAKE_SOURCE_DIR}/apps/example/mqtt
    ${CMAKE_SOURCE_DIR}/apps/example/net
    ${CMAKE_SOURCE_DIR}/apps/example/psm
    ${CMAKE_SOURCE_DIR}/apps/example/sockets
    ${CMAKE_SOURCE_DIR}/apps/test
    ${CMAKE_SOURCE_DIR}/apps/test/file
    ${CMAKE_SOURCE_DIR}/apps/test/fota
    ${CMAKE_SOURCE_DIR}/apps/test/ftp
    ${CMAKE_SOURCE_DIR}/apps/test/http
    ${CMAKE_SOURCE_DIR}/apps/test/mqtt
    ${CMAKE_SOURCE_DIR}/apps/test/network
    ${CMAKE_SOURCE_DIR}/apps/test/psm
    ${CMAKE_SOURCE_DIR}/apps/test/sockets
    ${CMAKE_SOURCE_DIR}/apps/user
    ${CMAKE_SOURCE_DIR}/quectel
    ${CMAKE_SOURCE_DIR}/quectel/common/inc
    ${CMAKE_SOURCE_DIR}/quectel/modules/inc
    ${CMAKE_SOURCE_DIR}/quectel/third_party/at_client/include
    ${CMAKE_SOURCE_DIR}/system/include/hal
    ${CMAKE_SOURCE_DIR}/system/include/osal
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/STM32F413RGT6/App/Core/Inc
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/STM32F413RGT6/App/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/STM32F413RGT6/App/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/STM32F413RGT6/App/Drivers/STM32F4xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/STM32F413RGT6/App/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/FATFS/App
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/FATFS/Target
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/Middlewares/Third_Party/FatFs/src
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/component/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/common/hal
)


# Linker Configuration
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/system/platform/arm-cortex/STM32F413RGT6/App/STM32F413RGTX_FLASH.ld)
add_link_options(-T ${LINKER_SCRIPT})
add_link_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-specs=nano.specs -specs=nosys.specs)
add_link_options(-static -Wl,--start-group -lc -lm -Wl,--end-group)
# Enable float printf support (+~10KB firmware size)
# add_link_options(-u _printf_float)

# Generate ELF output
add_executable(${PROJECT_NAME}.elf ${SOURCES})

# Set the pre-processor MACRO
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE QUECTEL_PROJECT_VERSION="${PROJECT_NAME}" PRIVATE QUECTEL_BUILD_ENV="WINDOWS")

# Set output file paths
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

# HEX/BIN generation rules
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Generating build artifacts:\nHEX: ${HEX_FILE}\nBIN: ${BIN_FILE}"
        VERBATIM
)
