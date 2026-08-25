import os
import json
from pathlib import Path
from datetime import datetime

# ===================== Configuration =====================
CONFIG = {
    "presets_file"      :   "system/platform/arm-cortex/bootloader/CMakePresets.json",   # Path to CMakePresets.txt file
    "config_file"       :   "tools/scripts/ProjectInfo.json",   # Path to configuration file
    "build_dir"         :   "build",                            # Output directory
}
# =================================================

true = True
false = False
CMakePresets_Content = {
    "version": 3,
    "configurePresets": [
        {
            "name": "env_config",
            "hidden": true,
            "environment": {
                "RootDir": "${sourceDir}/../../../../"
            }
        },
        {
            "name": "Bootloader-Base",
            "inherits": "env_config",
            "generator": "Unix Makefiles",
            "binaryDir": "${sourceDir}/build",
            "cmakeExecutable": "$env{RootDir}/tools/linux/toolchain/cmake/bin/cmake",
            "environment": {
                "PATH": "$env{RootDir}/tools/linux/toolchain/arm-gcc/bin"
            },
            "cacheVariables": {
                "CMAKE_MAKE_PROGRAM": "$env{RootDir}/tools/linux/toolchain/make/bin/make",
                "CMAKE_C_COMPILER":   "arm-none-eabi-gcc",
                "CMAKE_CXX_COMPILER": "arm-none-eabi-g++"
            }
        },
        {
            "name": "Bootloader-Debug",
            "displayName": "Configuration-Debug",
            "description": "Generates the build system by CMake",
            "inherits": "Bootloader-Base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_EXPORT_COMPILE_COMMANDS": true
            }
        },
        {
            "name": "Bootloader-Release",
            "displayName": "Configuration-Release",
            "description": "Generates the build system by CMake",
            "inherits": "Bootloader-Base",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_EXPORT_COMPILE_COMMANDS": false
            }
        }
    ],
    "buildPresets": [
        {
            "name": "Bootloader-Build",
            "configurePreset": "Bootloader-Debug",
            "targets": [
                "all"
            ],
            "jobs": 8,
            "cleanFirst": false
        },
        {
            "name": "Bootloader-Clean",
            "configurePreset": "Bootloader-Debug",
            "targets": [
                "clean"
            ]
        }
    ]
}


class JsonManager:
    def __init__(self, config):
        self.cfg = config
        self.root_path          = Path(__file__).resolve().parent.parent.parent.parent.parent
        self.presets_path       = self.root_path / self.cfg["presets_file"]
        self.config_path        = self.root_path / self.cfg["config_file"]
        self.build_path         = self.cfg["build_dir"]


    def _create_CMakePreset(self):
        # if self.presets_path.exists():
        #     return;
        data = json.loads(json.dumps(CMakePresets_Content))

        with open(self.config_path, "r", encoding="utf-8") as f:
            Info_Bat = json.load(f)
        buildEnv = Info_Bat["env"].lower()

        if buildEnv == "windows":
            makePath = f"$env{{RootDir}}/tools/{buildEnv}/toolchain/mingw64/bin/make"
        else:   # linux
            makePath = f"$env{{RootDir}}/tools/{buildEnv}/toolchain/make/bin/make"
        cmakePath = f"$env{{RootDir}}/tools/{buildEnv}/toolchain/cmake/bin/cmake"
        gccPath = f"$env{{RootDir}}/tools/{buildEnv}/toolchain/arm-gcc/bin"

        data["configurePresets"][1]["cmakeExecutable"] = cmakePath
        data["configurePresets"][1]["cacheVariables"]["CMAKE_MAKE_PROGRAM"] = makePath
        data["configurePresets"][1]["environment"]["PATH"] = gccPath

        with open(self.presets_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=4, ensure_ascii=False)
        print(f"-- [{self.presets_path}] generated successfully!")


    def run(self):
        """Start to generate configuration files"""
        try:
            self._create_CMakePreset()

        except Exception as e:
            print(f"\n[Bootloader] Error: {str(e)}")
            print("[Bootloader] Generate aborted. Original file remains unchanged.")


if __name__ == "__main__":
    manager = JsonManager(CONFIG)
    manager.run()
