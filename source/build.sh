#!/bin/bash

CURDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
Command=$1
Project_Chip=$2
Project_Version=$3

compile_tools_dir="$CURDIR/tools/linux/toolchain"
tools_pkg_dir="$CURDIR/tools/linux/packages"
arm_gcc_pkg="$tools_pkg_dir/arm-gnu-toolchain-12.3.rel1-x86_64-arm-none-eabi.tar.xz"
cmake_pkg="$tools_pkg_dir/cmake-3.31.3-linux-x86_64.tar.xz"
make_pkg="$tools_pkg_dir/make-4.4.1.tar.gz"
openocd_pkg="$tools_pkg_dir/xpack-openocd-0.12.0-6-linux-x64.tar.gz"

cmd_cmake="$compile_tools_dir/cmake/bin/cmake"
script_cmake_autogen="$CURDIR/tools/scripts/cmake-autogen.py"
script_json_autogen="$CURDIR/tools/scripts/json-autogen.py"

Project_Info_File="$CURDIR/tools/scripts/ProjectInfo.json"
ChipList_File="$CURDIR/tools/scripts/ChipList.json"
DEFAULT_CHIP="STM32F413RGT6"
DEFAULT_VER="Quectel_UFP_Chip_Date"
chip=""
version=""

cmd_gdb="$compile_tools_dir/arm-gcc/bin/arm-none-eabi-gdb"
cmd_openocd="$compile_tools_dir/openocd/bin/openocd"
build_dir="build"
DEFAULT_INTERFACE_CFG="interface/stlink.cfg"
DEFAULT_TARGET_CFG="target/stm32f4x.cfg"
interface=""
target=""
ADAPTER_SPEED=2000
GBD_PORT=3333


if command -v python3 &> /dev/null; then
    echo "-- Check python3 OK, version: $(python3 --version | cut -d' ' -f2)"
else
    echo "***[Error] Check python3 failed...please install python3 and add it into PATH first."
    exit 1
fi


if [ ! -d "$compile_tools_dir" ]; then
    echo "------------Uncompress cross-compilation toolchain-------------"
	mkdir "$compile_tools_dir"
    cat "${arm_gcc_pkg}.part"[0-9][0-9] > "$arm_gcc_pkg" || { echo "***[Error] Uncompress failed."; exit 1; }
    tar -xJpf "$arm_gcc_pkg" -C "$compile_tools_dir" --checkpoint=2000 --checkpoint-action=echo="-- [ARM-GCC] Uncompressed %u/82000 files..." || { echo "***[Error] Uncompress failed."; exit 1; }
	rm "$arm_gcc_pkg"
	tar -xJpf "$cmake_pkg" -C "$compile_tools_dir" --checkpoint=1000 --checkpoint-action=echo="-- [CMake] Uncompressed %u/16000 files..." || { echo "***[Error] Uncompress failed."; exit 1; }
	tar -xzpf "$make_pkg" -C "$compile_tools_dir" --checkpoint=100 --checkpoint-action=echo="-- [Make] Uncompressed %u/200 files..." || { echo "***[Error] Uncompress failed."; exit 1; }
	tar -xzpf "$openocd_pkg" -C "$compile_tools_dir" --checkpoint=100 --checkpoint-action=echo="-- [OpenOCD] Uncompressed %u/800 files..." || { echo "***[Error] Uncompress failed."; exit 1; }
    echo "------------Uncompress cross-compilation toolchain-------------"
else
    echo "-- Cross-compilation toolchain path [$compile_tools_dir]"
fi
echo


get_json_value() {
    local key="$1"
    python3 -c "import json; data=json.load(open('$Project_Info_File')); print(data.get('$key', ''), end='')"
}

get_json_keys() {
    local file="$1"
    python3 -c "import json; print('\n'.join(json.load(open('$file')).keys()))"
}

cmd_config() {
    if [ -f "$Project_Info_File" ]; then
		echo "==========Project_Info_File exist....."
        chip=$(get_json_value "chip")
        version=$(get_json_value "version")
    fi

    if [ -z "$Project_Chip" ]; then
        if [ -z "$chip" ]; then
            chip="$DEFAULT_CHIP"
        fi
        echo "-- No chip type input, using default [$chip]"
    else
        local found=0
        local keys=$(get_json_keys "$ChipList_File")
        for k in $keys; do
            if [ "$k" = "$Project_Chip" ]; then
                found=1
                break
            fi
        done
        if [ $found -eq 1 ]; then
            echo "-- Found [$Project_Chip] in file [$ChipList_File]"
            echo "-- Use new chip type [$Project_Chip]"
            chip="$Project_Chip"
        else
            echo "***[Error]: NOT found [$Project_Chip] in file [$ChipList_File]"
            echo "***All supported chip/platform:"
            echo "------------------"
            for k in $keys; do
                echo "  $k"
            done
            echo "------------------"
            return
        fi
    fi

    if [ -z "$Project_Version" ]; then
        if [ -z "$version" ]; then
            version="$DEFAULT_VER"
        fi
        echo "-- No project version input, using default [$version]"
    else
        version="$Project_Version"
        echo "-- Use new project name [$Project_Version]"
    fi

    cat > "$Project_Info_File" << EOF
{
	"env": "Linux",
    "chip": "$chip",
    "version": "$version"
}
EOF

    echo
    echo "---------- Start to Automatic Generation Config Files... ----------"
    if [ -f "$script_json_autogen" ]; then
        cp "$script_json_autogen" .
        python3 json-autogen.py
        rm json-autogen.py
    fi
    if [ -f "$script_cmake_autogen" ]; then
        cp "$script_cmake_autogen" .
        python3 cmake-autogen.py
        rm cmake-autogen.py
    fi

    if [ -f "$Project_Info_File" ]; then
        chip=$(get_json_value "chip")
        version=$(get_json_value "version")
        series=$(get_json_value "series")
        macro=$(get_json_value "macro")
        load=$(get_json_value "load")
        svd=$(get_json_value "svd")
        interface=$(get_json_value "interface")
        target=$(get_json_value "target")
    fi

    echo
    echo "============ Project Basic Info Setup ============"
    echo "-- chip: $chip"
    echo "-- version: $version"
    echo "-- series: $series"
    echo "-- macro: $macro"
    echo "-- load: $load"
    echo "-- svd: $svd"
    echo "-- interface: $interface"
    echo "-- target: $target"

    echo
    echo "------------CMake starts generating the build system-------------"
    "$cmd_cmake" --preset=STM32-Debug
}

cmd_all() {
    "$cmd_cmake" --build --preset=STM32-Build
}

cmd_clean() {
    "$cmd_cmake" --build --preset=STM32-Clean
    echo "-- Project clean done !"
}

cmd_download() {
    if [ -f "$Project_Info_File" ]; then
        version=$(get_json_value "version")
        interface=$(get_json_value "interface")
        target=$(get_json_value "target")
    fi
    firmware_path="$CURDIR/$build_dir/$version.elf"
    if [ -z "$interface" ]; then interface="$DEFAULT_INTERFACE_CFG"; fi
    if [ -z "$target" ]; then target="$DEFAULT_TARGET_CFG"; fi
    echo
    echo "-- Firmware path [$firmware_path]"
    echo "-- Prepare to download..."
    echo
    "$cmd_openocd" \
        -f "$interface" \
        -f "$target" \
        -c "program $build_dir/$version.elf verify reset exit"
}

cmd_debug() {
    if [ -f "$Project_Info_File" ]; then
        version=$(get_json_value "version")
        interface=$(get_json_value "interface")
        target=$(get_json_value "target")
    fi
    if [ -z "$interface" ]; then interface="$DEFAULT_INTERFACE_CFG"; fi
    if [ -z "$target" ]; then target="$DEFAULT_TARGET_CFG"; fi

    echo "--------Connection Information---------"
    echo "-- GDB port:     [$GBD_PORT]"
    echo "-- Adpter speed: [$ADAPTER_SPEED] kHz"
    echo "-- Starting OpenOCD debugger..."
	
	# 0- Kill openocd process first (Last time exit abnormally)
	kill -9 $(pgrep openocd)
	
    # 1- Start OpenOCD (Running in the background)
    "$cmd_openocd" \
        -f "$interface" \
        -f "$target" \
        -c "gdb port $GBD_PORT" \
        -c "adapter speed $ADAPTER_SPEED" &
   	local openocd_pid=$!
	echo "-- openocd pid:  [$openocd_pid]"
	echo

    # 2- Wait for OpenOCD to initialize (2 seconds)
    sleep 2

    # 3- Start GDB and execute the debug command
    firmware_path="$CURDIR/$build_dir/$version.elf"
    echo "-- Firmware: [$firmware_path]"

    "$cmd_gdb" "$firmware_path" \
        -ex "target remote localhost:$GBD_PORT" \
        -ex "load"

    # 4- Kill OpenOCD
    kill $openocd_pid 2>/dev/null
}

case "$Command" in
    "config")
        cmd_config
        ;;
    "all")
        cmd_all
        ;;
    "clean")
        cmd_clean
        ;;
    "download")
        cmd_download
        ;;
    "debug")
        cmd_debug
        ;;
    *)
        echo "Usage: $0 {config|all|clean|download|debug} [chip] [version]"
        exit 1
        ;;
esac

exit 0

