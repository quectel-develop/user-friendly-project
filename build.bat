@echo off
set CURDIR=%~dp0
set Command=%1
set Project_Chip=%2
set Project_Version=%3

set cmd_7z=%CURDIR%tools\windows\host\7z\7z.exe
set compile_tools_file=%CURDIR%tools\windows\packages\toolchain.7z.001
set compile_tools_dir=%CURDIR%tools\windows\toolchain
set cmd_cmake=%compile_tools_dir%\cmake\bin\cmake.exe
set cmd_python=%compile_tools_dir%\python\python.exe
set script_dir=%CURDIR%tools\scripts
set script_cmake_autogen=%script_dir%\cmake-autogen.py
set script_json_autogen=%script_dir%\json-autogen.py

set script_cmake_autogen_boot=%script_dir%\cmake-autogen-boot.py
set script_json_autogen_boot=%script_dir%\json-autogen-boot.py
set bootloader_dir=%CURDIR%system\platform\arm-cortex\bootloader

set Project_Info_File=%script_dir%\ProjectInfo.json
set ChipList_File=%script_dir%\ChipList.json
set DEFAULT_CHIP=STM32F413RGT6
set DEFAULT_VER=Quectel_UFP_Chip_Date
set chip=
set version=

set cmd_gdb=%compile_tools_dir%\arm-gcc\bin\arm-none-eabi-gdb.exe
set cmd_openocd=%compile_tools_dir%\openocd\bin\openocd.exe
set build_dir=build
set DEFAULT_INTERFACE_CFG=interface\stlink.cfg
set DEFAULT_TARGET_CFG=target\stm32f4x.cfg
set interface=
set target=
set ADAPTER_SPEED=2000
set GBD_PORT=3333


if not exist %compile_tools_dir% (
	echo ------------Uncompress cross-compilation toolchain-------------
	%cmd_7z% x -y %compile_tools_file% -o%compile_tools_dir% || ( echo ------------Uncompress cross-compilation toolchain failed------------- & goto END)
	echo ------------Uncompress cross-compilation toolchain-------------
) else (
    echo -- Cross-compilation toolchain path [%compile_tools_dir%]
)
echo.


setlocal enabledelayedexpansion

if "%Command%" == "config" (
    @call :cmd_config
)

if "%Command%" == "app" (
	@call :cmd_app
)

if "%Command%" == "bootloader" (
	@call :cmd_bootloader
)

if "%Command%" == "merge" (
	@call :cmd_merge
)

if "%Command%" == "all" (
	@call :cmd_all
)

if "%Command%" == "clean" (
	@call :cmd_clean
)

if "%Command%" == "download" (
	@call :cmd_download
)

if "%Command%" == "debug" (
	@call :cmd_debug
)

exit /b 0


:parse_json
set "target_key=%~1"
for /f "tokens=1,* delims=:" %%a in ('type %Project_Info_File% ^| findstr /i "\"%target_key%\""') do (
    set "value=%%b"
    set "value=!value:"=!"
    set "value=!value:,=!"
    set "value=!value: =!"
    set "value=!value:}=!"
    set "%target_key%=!value!"
    goto :eof
)
echo ***[Error]: Key "%target_key%" not found in [%Project_Info_File%].
goto :eof


:cmd_config
if EXIST %Project_Info_File% (
    @call :parse_json chip
    @call :parse_json version
)
set keys=
for /f "tokens=1* delims=:" %%a in ('type "%ChipList_File%" ^| findstr /r /c:"^[ ]*\".*\":[ ]*\["') do (
    set "key=%%a"
    set "key=!key:"=!"
    set "key=!key: =!"
    set "key=!key:,=!"
    if defined keys (
        set "keys=!keys! "!key!""
    ) else (
        set "keys="!key!""
    )
)

if "%Project_Chip%" == "" (
    if "%chip%" == "" (
        set chip=%DEFAULT_CHIP%
    )
    echo -- No chip type input, using default [!chip!]
) else (
    set found=0
    for %%k in (%keys%) do (
        if /i "%%~k" == "%Project_Chip%" set found=1
    )
    if !found! equ 1 (
        echo -- Found [%Project_Chip%] in file [%ChipList_File%]
        echo -- Use new chip type [%Project_Chip%]
        set chip=%Project_Chip%
    ) else (
        echo ***[Error]: NOT found [%Project_Chip%] in file [%ChipList_File%]
        echo ***All supported chip/platform:
        echo ------------------
        for %%k in (!keys!) do echo   %%~k
        echo ------------------
        goto:eof
    )
)

if "%Project_Version%" == "" (
    if "%version%" == "" (
        set version=%DEFAULT_VER%
    )
    echo -- No project version input, using default [!version!]
) else (
    set version=%Project_Version%
    echo -- Use new project name [%Project_Version%]
)

(
echo {
echo    "env": "Windows",
echo    "chip": "%chip%",
echo    "version": "%version%"
echo }
) > %Project_Info_File%

echo.
echo ---------- Start to Automatic Generation Config Files for App... ----------
if exist %script_json_autogen% (
    copy %script_json_autogen% . >nul
    %cmd_python% json-autogen.py
    del json-autogen.py
)
if exist %script_cmake_autogen% (
    copy %script_cmake_autogen% . >nul
    %cmd_python% cmake-autogen.py
    del cmake-autogen.py
)

echo.
echo.
echo ---------- Start to Automatic Generation CMake Files for Bootloader... ----------
if exist %script_json_autogen_boot% (
    copy %script_json_autogen_boot% %bootloader_dir% >nul
    pushd %bootloader_dir%
    %cmd_python% json-autogen-boot.py
    del json-autogen-boot.py
    popd
)
if exist %script_cmake_autogen_boot% (
    copy %script_cmake_autogen_boot% %bootloader_dir% >nul
    pushd %bootloader_dir%
    %cmd_python% cmake-autogen-boot.py
    del cmake-autogen-boot.py
    popd
)

if EXIST %Project_Info_File% (
    @call :parse_json chip
    @call :parse_json version
    @call :parse_json series
    @call :parse_json macro
    @call :parse_json load
    @call :parse_json svd
    @call :parse_json interface
    @call :parse_json target
)
echo.
echo ============ Project Basic Info Setup ============
echo -- chip: %chip%
echo -- version: %version%
echo -- series: %series%
echo -- macro: %macro%
echo -- load: %load%
echo -- svd: %svd%
echo -- interface: %interface%
echo -- target: %target%

echo.
echo ------------CMake starts generating the build system for Bootloader-------------
pushd %bootloader_dir%
%cmd_cmake% --preset=Bootloader-Debug
popd

echo.
echo ------------CMake starts generating the build system for App-------------
%cmd_cmake% --preset=STM32-Debug
goto:eof


:cmd_app
%cmd_cmake% --build --preset=STM32-Build
goto:eof


:cmd_bootloader
pushd %bootloader_dir%
%cmd_cmake% --build --preset=Bootloader-Build
popd
goto:eof


:cmd_merge
if EXIST %Project_Info_File% (
    @call :parse_json version
)
set app_elf_path=%CURDIR%%build_dir%\%version%.elf
set boot_elf_path=%bootloader_dir%\build\%version%_Bootloader.elf
set merge_bin_path=%build_dir%\_Merge\%version%_Merge.bin
if NOT exist %build_dir%\_Merge md %build_dir%\_Merge
@REM Merge bootloader and app
%cmd_python% %script_dir%\merge-firmware.py -b %boot_elf_path% -a %app_elf_path% -o %merge_bin_path%
goto:eof


:cmd_all
@call :cmd_bootloader
@call :cmd_app
@call :cmd_merge
goto:eof


:cmd_clean
pushd %bootloader_dir%
%cmd_cmake% --build --preset=Bootloader-Clean
popd
echo -- Bootloader clean done.
%cmd_cmake% --build --preset=STM32-Clean
echo -- App clean done.
goto:eof


:cmd_download
if EXIST %Project_Info_File% (
    @call :parse_json version
    @call :parse_json interface
    @call :parse_json target
)
set merge_bin_path=%build_dir%\_Merge\%version%_Merge.bin
if "%interface%" == "" set interface=DEFAULT_INTERFACE_CFG
if "%target%" == "" set target=DEFAULT_TARGET_CFG
echo.
echo -- Firmware path [%merge_bin_path%]
echo -- Prepare to download...
echo.
%cmd_openocd% ^
    -f %interface% ^
    -f %target% ^
    -c "program %build_dir%/_Merge/%version%_Merge.bin 0x8000000 verify reset exit"
    @REM -c "program %build_dir%/%version%.elf verify reset exit"
goto:eof


:cmd_debug
if EXIST %Project_Info_File% (
    @call :parse_json version
    @call :parse_json interface
    @call :parse_json target
)
if "%interface%" == "" set interface=DEFAULT_INTERFACE_CFG
if "%target%" == "" set target=DEFAULT_TARGET_CFG

echo --------Connection Information---------
echo -- GDB port:     [%GBD_PORT%]
echo -- Adpter speed: [%ADAPTER_SPEED%] kHz
echo -- Starting OpenOCD debugger...
echo.
@REM 1- Start OpenOCD (Running in the background)
start "OpenOCD" %cmd_openocd% ^
    -f %interface% ^
    -f %target% ^
    -c "gdb port %GBD_PORT%" ^
    -c "adapter speed %ADAPTER_SPEED%"

@REM 2- Wait for OpenOCD to initialize (2 seconds)
timeout /t 2 >nul

@REM 3- Start GDB and execute the debug command
set firmware_path=%CURDIR%%build_dir%\%version%.elf
echo -- Firmware: [%firmware_path%]

%cmd_gdb% %firmware_path% ^
    -ex "target remote localhost:%GBD_PORT%" ^
    -ex "load"

@REM 4- Kill OpenOCD
taskkill /IM openocd* /F /T >nul 2>&1
exit /b 0

:END
