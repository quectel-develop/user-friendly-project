#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import sys
import json
import os

# ------------------------- Configuration -------------------------
# Bootloader size (in bytes). Adjust this value according to your project.
BOOT_SIZE = 0x10000     # Default 64 KB
# -----------------------------------------------------------------

def parse_map(map_file):
    """
    Parse the linker map file to accurately calculate ROM and RAM usage.
    It extracts the addresses and sizes of .data, .bss and ._user_heap_stack sections,
    and uses them along with the FLASH/RAM memory configuration.
    """
    with open(map_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # ---------- 1. Get FLASH/RAM configuration ----------
    mem_config = {}
    mem_re = re.compile(r'^(RAM|FLASH)\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)')
    for line in lines:
        m = mem_re.match(line.strip())
        if m:
            name, origin_str, length_str = m.groups()
            mem_config[name] = {
                'origin': int(origin_str, 16),
                'length': int(length_str, 16)
            }
    if 'RAM' not in mem_config or 'FLASH' not in mem_config:
        raise ValueError("Cannot find RAM or FLASH configuration in map file")

    flash_origin = mem_config['FLASH']['origin']
    flash_total = mem_config['FLASH']['length']
    ram_origin = mem_config['RAM']['origin']
    ram_total = mem_config['RAM']['length']

    # ---------- 2. Extract critical sections (.data, .bss, ._user_heap_stack) ----------
    data_vma = data_size = data_load = None
    bss_vma = bss_size = None
    heap_vma = heap_size = None

    def extract_vma_size(line):
        """Extract two hex numbers (VMA and SIZE) from a line."""
        m = re.search(r'(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)', line)
        if m:
            return int(m.group(1), 16), int(m.group(2), 16)
        return None, None

    i = 0
    while i < len(lines):
        line = lines[i]

        # .data section (may contain load address on the same line)
        if '.data' in line and 'load address' in line:
            m = re.search(r'\.data\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)\s+load address\s+(0x[0-9a-fA-F]+)', line)
            if m:
                data_vma, data_size, data_load = int(m.group(1), 16), int(m.group(2), 16), int(m.group(3), 16)

        # .bss section (usually on one line)
        elif '.bss' in line and '0x' in line:
            vma, size = extract_vma_size(line)
            if vma is not None:
                bss_vma, bss_size = vma, size

        # ._user_heap_stack section (section name often on its own line, address on next)
        elif '._user_heap_stack' in line:
            # Try next line
            if i + 1 < len(lines):
                next_line = lines[i + 1]
                vma, size = extract_vma_size(next_line)
                if vma is not None:
                    heap_vma, heap_size = vma, size
            # If not found, try current line (just in case)
            if heap_vma is None:
                vma, size = extract_vma_size(line)
                if vma is not None:
                    heap_vma, heap_size = vma, size

        i += 1

    # Validate that we found all necessary information
    missing = []
    if data_vma is None: missing.append('.data VMA')
    if data_size is None: missing.append('.data size')
    if data_load is None: missing.append('.data load address')
    if bss_vma is None: missing.append('.bss VMA')
    if bss_size is None: missing.append('.bss size')
    if heap_vma is None: missing.append('._user_heap_stack VMA')
    if heap_size is None: missing.append('._user_heap_stack size')
    if missing:
        raise ValueError(f"Cannot extract the following from map file: {', '.join(missing)}")

    # ---------- 3. Calculate raw usage ----------
    # Flash usage = (data load start + data size) - flash start
    flash_used_raw = (data_load + data_size) - flash_origin

    # RAM usage = (bss start + bss size + heap size) - ram start
    ram_used_raw = (bss_vma + bss_size + heap_size) - ram_origin

    # ---------- 4. Apply bootloader offset ----------
    # For ROM: add BOOT_SIZE to both total and used, keeping free the same.
    # This reflects that the bootloader occupies the first BOOT_SIZE bytes of flash.
    flash_total_with_boot = flash_total + BOOT_SIZE
    flash_used_with_boot = flash_used_raw + BOOT_SIZE

    # RAM is unaffected by bootloader
    ram_total_final = ram_total
    ram_used_final = ram_used_raw

    # Recalculate percentages and free
    flash_free = flash_total_with_boot - flash_used_with_boot
    ram_free = ram_total_final - ram_used_final

    flash_percent = (flash_used_with_boot / flash_total_with_boot * 100) if flash_total_with_boot else 0
    ram_percent = (ram_used_final / ram_total_final * 100) if ram_total_final else 0

    return {
        "ROM": {
            "total": flash_total_with_boot,
            "used": flash_used_with_boot,
            "free": flash_free,
            "percentage": round(flash_percent, 2)
        },
        "RAM": {
            "total": ram_total_final,
            "used": ram_used_final,
            "free": ram_free,
            "percentage": round(ram_percent, 2)
        }
    }

def main():
    if len(sys.argv) != 3:
        print("Usage: python mem.py <map_file_path> <output_json_path>")
        sys.exit(1)

    map_file = sys.argv[1]
    json_file = sys.argv[2]

    if not os.path.isfile(map_file):
        print(f"Error: map file '{map_file}' does not exist")
        sys.exit(1)

    try:
        stats = parse_map(map_file)
        with open(json_file, 'w', encoding='utf-8') as f:
            json.dump(stats, f, indent=2)
        print(f"-- Memory analysis complete. Results saved to {json_file}")
        # print(json.dumps(stats, indent=2))
    except Exception as e:
        print(f"** Memory analysis failed: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()