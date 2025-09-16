import os
import json
from pathlib import Path
from datetime import datetime

# ===================== Configuration =====================
CONFIG = {
    "presets_file"      :   "CMakePresets.json",                # Path to CMakePresets.txt file
    "vscode_path"       :   ".vscode",                          # Path to .vscode directory
    "c_cpp_file"        :   ".vscode/c_cpp_properties.json",    # Path to c_cpp_properties.json
    "tasks_file"        :   ".vscode/tasks.json",               # Path to tasks_file.json
    "launch_file"       :   ".vscode/launch.json",              # Path to launch_file.json
    "build_dir"         :   "build",                            # Output directory
    "mcu_parent_dir"    :   "system/platform/arm-cortex",       # Parent directory for MCU platform
    "backup_folder"     :   "tools/scripts/backup",             # Backup directory
    "max_backups"       :   5,                                  # Maximum number of backups
    "config_file"       :   "tools/scripts/ProjectInfo.json",   # Path to configuration file
    "chiplist_file"     :   "tools/scripts/ChipList.json"       # Path to chip support list file
}
# =================================================

true = True
false = False
CMakePresets_Content = {
    "version": 3,
    "configurePresets": [
        {
            "name": "STM32-Base",
            "generator": "Unix Makefiles",
            "binaryDir": "${sourceDir}/build",
            "cmakeExecutable": "${sourceDir}/tools/linux/toolchain/cmake/bin/cmake",
            "environment": {
                "PATH": "${sourceDir}/tools/linux/toolchain/arm-gcc/bin"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM": "${sourceDir}/tools/linux/toolchain/make/bin/make",
                "CMAKE_C_COMPILER":   "arm-none-eabi-gcc",
                "CMAKE_CXX_COMPILER": "arm-none-eabi-g++"
            }
        },
        {
            "name": "STM32-Debug",
            "displayName": "Configuration-Debug",
            "description": "Generates the build system by CMake",
            "inherits": "STM32-Base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_EXPORT_COMPILE_COMMANDS": true
            }
        },
        {
            "name": "STM32-Release",
            "displayName": "Configuration-Release",
            "description": "Generates the build system by CMake",
            "inherits": "STM32-Base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_EXPORT_COMPILE_COMMANDS": false
            }
        }
    ],
    "buildPresets": [
        {
            "name": "STM32-Build",
            "configurePreset": "STM32-Debug",
            "targets": [
                "all"
            ],
            "jobs": 8,
            "cleanFirst": false
        },
        {
            "name": "STM32-Clean",
            "configurePreset": "STM32-Debug",
            "targets": [
                "clean"
            ]
        }
    ]
}

c_cpp_properties_Content = {
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "compilerPath": "${workspaceFolder}/tools/windows/toolchain/mingw64/bin/gcc",
            "cStandard": "gnu17",
            "cppStandard": "gnu++14",
            "intelliSenseMode": "gcc-arm"
        }
    ],
    "version": 4
}

tasks_Content = {
    "tasks": [
        {
            "type": "shell",
            "label": "Download",
            "command": "./tools/linux/toolchain/openocd/bin/openocd",
            "args": [
                "-f",
                "interface/stlink.cfg",
                "-f",
                "target/stm32f4x.cfg",
                "-c",
                "program ./build/AutoProjectName.elf verify reset exit"
            ],
            "problemMatcher": [
                "$gcc"
            ],
            "group": "build",
            "detail": "Download Firmware by OpenOCD."
        }
    ],
    "version": "2.0.0"
}

launch_Content = {
    "configurations": [
        {
            "name": "Debug with OpenOCD",
            "cwd": "${workspaceRoot}",
            "executable": "./build/AutoProjectName.elf",
            "request": "launch",
            "type": "cortex-debug",
            "servertype": "openocd",
            "serverpath": "./tools/linux/toolchain/openocd/bin/openocd",
            "gdbPath": "./tools/linux/toolchain/arm-gcc/bin/arm-none-eabi-gdb",
            "configFiles": [
                "interface/stlink.cfg",
                "target/stm32f4x.cfg"
            ],
            "searchDir": [],
            "runToEntryPoint": "main",
            "showDevDebugOutput": "none",
            "svdFile": "./STM32F413.svd"
        }
    ],
    "version": "0.2.0"
}


class JsonManager:
    def __init__(self, config):
        self.cfg = config
        self.root_path          = Path(__file__).parent.resolve()
        self.presets_path       = self.root_path / self.cfg["presets_file"]
        self.vscode_path        = self.root_path / self.cfg["vscode_path"]
        self.c_cpp_path         = self.root_path / self.cfg["c_cpp_file"]
        self.tasks_path         = self.root_path / self.cfg["tasks_file"]
        self.launch_path        = self.root_path / self.cfg["launch_file"]
        self.backup_path        = self.root_path / self.cfg["backup_folder"]
        self.config_path        = self.root_path / self.cfg["config_file"]
        self.chiplist_path      = self.root_path / self.cfg["chiplist_file"]
        self.build_path         = self.cfg["build_dir"]
        self.mcu_parent_path    = self.cfg["mcu_parent_dir"]


    def _create_CMakePreset(self):
        # if self.presets_path.exists():
        #     return;
        data = json.loads(json.dumps(CMakePresets_Content))

        with open(self.config_path, "r", encoding="utf-8") as f:
            Info_Bat = json.load(f)
        buildEnv = Info_Bat["env"].lower()

        if buildEnv == "windows":
            makePath = f"${{sourceDir}}/tools/{buildEnv}/toolchain/mingw64/bin/make"
        else:   # linux
            makePath = f"${{sourceDir}}/tools/{buildEnv}/toolchain/make/bin/make"
        cmakePath = f"${{sourceDir}}/tools/{buildEnv}/toolchain/cmake/bin/cmake"
        gccPath = f"${{sourceDir}}/tools/{buildEnv}/toolchain/arm-gcc/bin"

        data["configurePresets"][0]["cmakeExecutable"] = cmakePath
        data["configurePresets"][0]["cacheVariables"]["CMAKE_MAKE_PROGRAM"] = makePath
        data["configurePresets"][0]["environment"]["PATH"] = gccPath

        with open(self.presets_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=4, ensure_ascii=False)
        print(f"-- [{self.presets_path}] generated successfully!")


    def _create_c_cpp_properties(self):
        with open(self.c_cpp_path, "w", encoding="utf-8") as f:
            json.dump(c_cpp_properties_Content, f, indent=4, ensure_ascii=False)
        print(f"-- [{self.c_cpp_path}] generated successfully!")


    def _update_ProjectInfo(self):
        with open(self.chiplist_path, "r", encoding="utf-8") as f:
            Chip_List = json.load(f)

        with open(self.config_path, "r", encoding="utf-8") as f:
            Info_Bat = json.load(f)
            chip        = Info_Bat["chip"].upper()
            ProjectName = Info_Bat["version"]
            series      = chip[5:7]
            svd         = f"{chip[0:9]}.svd"
            load        = Chip_List[chip][0]
            macro       = Chip_List[chip][1]
            interface   = f"interface/stlink.cfg"
            target      = f"target/{chip[0:7].lower()}x.cfg"

            if ProjectName == "Quectel_UFP_Chip_Date":
                ProjectName = ProjectName.replace("Chip", chip)
                ProjectName = ProjectName.replace("Date", datetime.now().strftime("%Y%m%d"))
            if chip.lower() == "windows":
                series = "windows"

            # Update ProjectInfo.json
            Info_Bat.update({"chip": chip})
            Info_Bat.update({"version": ProjectName})
            Info_Bat.update({"series": series})
            Info_Bat.update({"macro": macro})
            Info_Bat.update({"load": load})
            Info_Bat.update({"svd": svd})
            Info_Bat.update({"interface": interface})
            Info_Bat.update({"target": target})

            # Write back to ProjectInfo.json
            with open(self.config_path, "w", encoding="utf-8") as f:
                json.dump(Info_Bat, f, indent=4, ensure_ascii=False)


    def _create_tasks(self):
        data = json.loads(json.dumps(tasks_Content))

        with open(self.config_path, "r", encoding="utf-8") as f:
            Info_Bat = json.load(f)
        ProjectName = Info_Bat["version"]
        buildEnv = Info_Bat["env"].lower()

        data["tasks"][0]["command"] = f"./tools/{buildEnv}/toolchain/openocd/bin/openocd"
        data["tasks"][0]["args"] = (
                "-f",
                Info_Bat["interface"],
                "-f",
                Info_Bat["target"],
                "-c",
                f"program {self.build_path}/{ProjectName}.elf verify reset exit" )

        # Write back to tasks.json
        with open(self.tasks_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=4, ensure_ascii=False)
        # print(f"-- tasks.json configrated the version: [{ProjectName}.elf]")
        print(f"-- [{self.tasks_path}] generated successfully!")


    def _create_launch(self):
        data = json.loads(json.dumps(launch_Content))

        with open(self.config_path, "r", encoding="utf-8") as f:
            Info_Bat = json.load(f)
        ProjectName = Info_Bat["version"]
        chip = Info_Bat["chip"]
        svd = Info_Bat["svd"]
        buildEnv = Info_Bat["env"].lower()

        openocd_path = f"./tools/{buildEnv}/toolchain/openocd/bin/openocd"
        data["configurations"][0]["serverpath"] = openocd_path  # Configure OpenOCD path

        gdb_path = f"./tools/{buildEnv}/toolchain/arm-gcc/bin/arm-none-eabi-gdb"
        data["configurations"][0]["gdbPath"] = gdb_path         # Configure GDB path

        elfFile_path = f"{self.build_path}/{ProjectName}.elf"
        data["configurations"][0]["executable"] = elfFile_path  # Configure elf file path

        svdFile_path = f"{self.mcu_parent_path}/{chip}/{svd}"
        data["configurations"][0]["svdFile"] = svdFile_path     # Configure svd file path

        data["configurations"][0]["configFiles"] = Info_Bat["interface"],Info_Bat["target"] # Device configuration

        # Write back to launch.json
        with open(self.launch_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=4, ensure_ascii=False)
        # print(f"-- launch.json configrated the version: [{ProjectName}.elf]")
        # print(f"-- launch.json configrated the svd path: [{svdFile_path}.elf]")
        print(f"-- [{self.launch_path}] generated successfully!")


    def _create_vscode_json(self):
        self.vscode_path.mkdir(exist_ok=True)
        self._create_c_cpp_properties()
        self._create_tasks()
        self._create_launch()


    def run(self):
        """Start to generate configuration files"""
        try:
            self._update_ProjectInfo()
            self._create_CMakePreset()
            self._create_vscode_json()

        except Exception as e:
            print(f"\nError: {str(e)}")
            print("Generate aborted. Original file remains unchanged.")


if __name__ == "__main__":
    manager = JsonManager(CONFIG)
    manager.run()
