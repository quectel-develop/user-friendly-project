import os
import json
import subprocess
import sys
import argparse
from pathlib import Path

# Define bootloader start address
BOOT_START_ADDR = 0x8000000

class ElfPaser:
    def __init__(self, boot_elf, app_elf):
        self.root_path  = Path(__file__).parent.parent.parent.resolve()
        self.boot_elf   = boot_elf
        self.app_elf    = app_elf
        self.Offset = 0
        self.AppStart = 0

    def parse_elf(self):
        # Parse ProjectInfo.json to get configuration parameters
        with open(f"{self.root_path}/tools/scripts/ProjectInfo.json", "r", encoding="utf-8") as f:
            json_data = json.load(f)
        buildEnv = json_data["env"].lower()

        READELF_TOOL = f"{self.root_path}/tools/{buildEnv}/toolchain/arm-gcc/bin/arm-none-eabi-readelf"

        result = subprocess.run(
            [READELF_TOOL, '-l', self.app_elf],
            capture_output=True, text=True, check=True
        )
        # print(result.stdout)

        # Find the first LOAD segment
        for line in result.stdout.split('\n'):
            if line.startswith('  LOAD'):
                parts = [p for p in line.split() if p]
                if len(parts) >= 6:
                    Offset = int(parts[1], 16)
                    VirtAddr = int(parts[2], 16)
                    PhysAddr = int(parts[3], 16)
                    FileSiz = int(parts[4], 16)

                    # Typically use physical address as load address
                    # print(f"-- AppStart : 0x{PhysAddr:X}")
                    # print(f"-- Offset   : 0x{Offset:X}")
                    self.AppStart = PhysAddr
                    return True

        print("** Error: LOAD segment not found")
        return False

    def run(self):
        try:
            ret = self.parse_elf()
            return ret

        except (subprocess.CalledProcessError, FileNotFoundError):
            print("** Error: Unable to execute arm-none-eabi-readelf")
            return False
        except (ValueError, IndexError) as e:
            print(f"** Error: Parse error: {e}")
            return False

class BinGenerator:
    def __init__(self, boot_elf, app_elf):
        self.root_path  = Path(__file__).parent.parent.parent.resolve()
        self.boot_elf   = boot_elf
        self.app_elf    = app_elf

        self.boot_bin   = ""
        self.app_bin    = ""

    def elf_to_bin(self):
        try:
            # Parse ProjectInfo.json to get configuration parameters
            with open(f"{self.root_path}/tools/scripts/ProjectInfo.json", "r", encoding="utf-8") as f:
                json_data = json.load(f)
            buildEnv = json_data["env"].lower()

            OBJCOPY_TOOL = f"{self.root_path}/tools/{buildEnv}/toolchain/arm-gcc/bin/arm-none-eabi-objcopy"

            self.boot_bin = self.boot_elf[:-4] + ".bin"
            subprocess.run(
                [OBJCOPY_TOOL, '-O', 'binary', self.boot_elf, self.boot_bin],
                capture_output=True, text=True, check=True
            )

            self.app_bin = self.app_elf[:-4] + ".bin"
            subprocess.run(
                [OBJCOPY_TOOL, '-O', 'binary', self.app_elf, self.app_bin],
                capture_output=True, text=True, check=True
            )
            return True

        except Exception as e:
            print(f"** Error: Processing failed: {e}")
            return False


class BinMerger:
    def __init__(self, boot_bin, app_bin, output_bin, boot_addr, app_addr):
        self.boot_bin   = boot_bin
        self.app_bin    = app_bin
        self.output_bin = output_bin
        self.boot_addr  = boot_addr
        self.app_addr   = app_addr

        self.boot_size  = 0
        self.app_size   = 0
        self.padding_size = 0

    def merge_files(self):
        # Calculate bin file and padding size
        self.boot_size = Path(self.boot_bin).stat().st_size
        self.app_size = Path(self.app_bin).stat().st_size

        # Calculate padding size
        self.padding_size = self.app_addr - self.boot_addr - self.boot_size

        if self.padding_size < 0:
            raise ValueError("** Error: Bootloader size exceeds reserved space")

        # Perform file merging
        with open(self.output_bin, 'wb') as out_file:
            # Write boot
            with open(self.boot_bin, 'rb') as f:
                out_file.write(f.read())

            # Write padding
            if self.padding_size > 0:
                out_file.write(b'\xFF' * self.padding_size)

            # Write application
            with open(self.app_bin, 'rb') as f:
                out_file.write(f.read())


        # Verify output file
        output_size = Path(self.output_bin).stat().st_size
        if output_size != self.app_addr - self.boot_addr + self.app_size:
            print("** Error: Output file size doesn't match expected size")

        # Print table information
        print("----------------------------------------------------")
        print("\t\tStart Address\t\tRegion Size")
        print(f" Bootloader\t0x{self.boot_addr:X}\t\t{self.boot_size} B")
        print(f" App\t\t0x{self.app_addr:X}\t\t{self.app_size} B")
        print(f" Merge\t\t0x{self.boot_addr:X}\t\t{output_size} B")
        print("----------------------------------------------------")
        return True

    def run(self):
        try:
            ret = self.merge_files()
            print(f"-- Merge completed: [{self.output_bin}]")
            return ret

        except Exception as e:
            print(f"** Error: Processing failed: {e}")
            return False


def main():
    # ----- Parse arguments -----
    parser = argparse.ArgumentParser(description='Merge Bootloader and Application files')
    parser.add_argument('-b', '--boot', required=True, help='Bootloader ELF file path')
    parser.add_argument('-a', '--app', required=True, help='Application ELF file path')
    parser.add_argument('-o', '--output', required=True, help='Output BIN file path')
    args = parser.parse_args()

    # ----- Verify input files exist -----
    if not Path(args.boot).exists():
        raise FileNotFoundError(f"** Error: Bootloader file not found: {args.boot}")

    if not Path(args.app).exists():
        raise FileNotFoundError(f"** Error: Application file not found: {args.app}")

    # ----- Parse ELF files -----
    parser = ElfPaser(
        boot_elf=args.boot,
        app_elf=args.app
    )
    ret = parser.run()
    if(ret == False):
        sys.exit(1)

    # ----- Generate BIN files from ELF files ------
    generator = BinGenerator(
        boot_elf=args.boot,
        app_elf=args.app
    )
    ret = generator.elf_to_bin()
    if(ret == False):
        sys.exit(1)

    # ----- Merge BIN files -----
    merger = BinMerger(
        boot_bin=generator.boot_bin,
        app_bin=generator.app_bin,
        output_bin=args.output,
        boot_addr=BOOT_START_ADDR,
        app_addr=parser.AppStart
    )
    ret = merger.run()
    sys.exit(0 if ret else 1)

if __name__ == "__main__":
    main()
