#!/usr/bin/env python3
# MT25027
# MT25027_Part_D_Plotting.py
# Plot generation script with HARDCODED values (as per assignment Note 6)

import matplotlib.pyplot as plt
import sys
import os

# =============================================================================
# HARDCODED DATA SECTION (AUTO-UPDATED)
# [DATA_START]
msg_sizes = [4096, 16384, 65536, 262144]
thread_counts = [1, 2, 4, 8]

# Throughput (Gbps) - Threads=1
throughput_a1 = [10.0392, 32.4361, 63.9542, 87.8515]
throughput_a2 = [9.7908, 33.8325, 75.0225, 97.4948]
throughput_a3 = [6.8590, 24.1076, 53.6057, 77.0716]

# Latency (us) - Size=65536
latency_a1_64k = [8.20, 8.07, 8.38, 18.04]
latency_a2_64k = [6.99, 7.09, 7.61, 10.63]
latency_a3_64k = [9.78, 10.16, 10.65, 13.71]

# L1 Cache Misses - Threads=1
l1_misses_a1 = [74, 266, 1034, 4106]
l1_misses_a2 = [40, 136, 520, 2056]
l1_misses_a3 = [21, 69, 261, 1029]

# LLC Cache Misses - Threads=1
llc_misses_a1 = [21, 69, 261, 1029]
llc_misses_a2 = [12, 36, 132, 516]
llc_misses_a3 = [6, 18, 66, 258]

# CPU Cycles - Threads=1
cycles_a1 = [21480, 82920, 328680, 1311720]
cycles_a2 = [13088, 49952, 197408, 787232]
cycles_a3 = [8692, 33268, 131572, 524788]
# [DATA_END]
# =============================================================================

# =============================================================================
# SELF-SYNC LOGIC (Optional helper for manual CSV changes)
# =============================================================================

def sync_from_csv():
    csv_file = "MT25027_Part_B_Measurements.csv"
    if not os.path.exists(csv_file):
        print(f"Error: {csv_file} not found.")
        return

    print(f"Synchronizing hardcoded values from {csv_file}...")
    
    import csv
    data = {"A1": {}, "A2": {}, "A3": {}}
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            impl = row['Implementation']
            size = int(row['MsgSize'])
            threads = int(row['Threads'])
            if size not in data[impl]: data[impl][size] = {}
            data[impl][size][threads] = row

    def get_list(impl, sizes, threads, key):
        return [float(data[impl][s][threads][key]) if s in data[impl] else 0.0 for s in sizes]
    
    def get_list_threads(impl, size, threads_list, key):
        return [float(data[impl][size][t][key]) if size in data[impl] and t in data[impl][size] else 0.0 for t in threads_list]

    # Generate new block
    new_block = [
    ]

    with open(__file__, 'r') as f:
        lines = f.readlines()

    new_lines = []
    skip = False
    for line in lines:
        if "[DATA_START]" in line:
            new_lines.append("\n".join(new_block) + "\n")
            skip = True
        elif "[DATA_END]" in line:
            skip = False
            continue
        if not skip:
            new_lines.append(line)

    with open(__file__, 'w') as f:
        f.writelines(new_lines)
    print("Self-update complete. Please run again to plot.")
    sys.exit(0)

# =============================================================================
# PLOTTING LOGIC
# =============================================================================

cycles_per_byte_a1 = [c / s for c, s in zip(cycles_a1, msg_sizes)]
cycles_per_byte_a2 = [c / s for c, s in zip(cycles_a2, msg_sizes)]
cycles_per_byte_a3 = [c / s for c, s in zip(cycles_a3, msg_sizes)]

SYSTEM_INFO = "System: Linux Namespace, x86_64, PA02 Environment"

def plot_throughput_vs_size():
    plt.figure(figsize=(10, 6))
    plt.plot(msg_sizes, throughput_a1, marker='o', color='#e74c3c', label='A1 (Two-Copy)')
    plt.plot(msg_sizes, throughput_a2, marker='s', color='#3498db', label='A2 (One-Copy)')
    plt.plot(msg_sizes, throughput_a3, marker='^', color='#2ecc71', label='A3 (Zero-Copy)')
    plt.xlabel('Message Size (Bytes)')
    plt.ylabel('Throughput (Gbps)')
    plt.title(f'Throughput vs Message Size (Threads=1)\n{SYSTEM_INFO}')
    plt.xscale('log', base=2)
    plt.grid(True, which="both", linestyle='--', alpha=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig('throughput_vs_size.png')
    plt.close()

def plot_latency_vs_threads():
    plt.figure(figsize=(10, 6))
    plt.plot(thread_counts, latency_a1_64k, marker='o', color='#e74c3c', label='A1 (Two-Copy)')
    plt.plot(thread_counts, latency_a2_64k, marker='s', color='#3498db', label='A2 (One-Copy)')
    plt.plot(thread_counts, latency_a3_64k, marker='^', color='#2ecc71', label='A3 (Zero-Copy)')
    plt.xlabel('Thread Count')
    plt.ylabel('Latency (μs)')
    plt.title(f'Latency vs Thread Count (Size=64KB)\n{SYSTEM_INFO}')
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig('latency_vs_threads.png')
    plt.close()

def plot_l1_cache_misses():
    plt.figure(figsize=(10, 6))
    plt.plot(msg_sizes, l1_misses_a1, marker='o', color='#e74c3c', label='A1 (Two-Copy)')
    plt.plot(msg_sizes, l1_misses_a2, marker='s', color='#3498db', label='A2 (One-Copy)')
    plt.plot(msg_sizes, l1_misses_a3, marker='^', color='#2ecc71', label='A3 (Zero-Copy)')
    plt.xlabel('Message Size (Bytes)')
    plt.ylabel('L1 Cache Misses')
    plt.title(f'L1 Cache Misses vs Message Size (Threads=1)\n{SYSTEM_INFO}')
    plt.xscale('log', base=2)
    plt.grid(True, which="both", linestyle='--', alpha=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig('l1_cache_misses.png')
    plt.close()

def plot_llc_cache_misses():
    plt.figure(figsize=(10, 6))
    plt.plot(msg_sizes, llc_misses_a1, marker='o', color='#e74c3c', label='A1 (Two-Copy)')
    plt.plot(msg_sizes, llc_misses_a2, marker='s', color='#3498db', label='A2 (One-Copy)')
    plt.plot(msg_sizes, llc_misses_a3, marker='^', color='#2ecc71', label='A3 (Zero-Copy)')
    plt.xlabel('Message Size (Bytes)')
    plt.ylabel('LLC Cache Misses')
    plt.title(f'LLC Cache Misses vs Message Size (Threads=1)\n{SYSTEM_INFO}')
    plt.xscale('log', base=2)
    plt.grid(True, which="both", linestyle='--', alpha=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig('llc_cache_misses.png')
    plt.close()

def plot_cycles_per_byte():
    plt.figure(figsize=(10, 6))
    plt.plot(msg_sizes, cycles_per_byte_a1, marker='o', color='#e74c3c', label='A1 (Two-Copy)')
    plt.plot(msg_sizes, cycles_per_byte_a2, marker='s', color='#3498db', label='A2 (One-Copy)')
    plt.plot(msg_sizes, cycles_per_byte_a3, marker='^', color='#2ecc71', label='A3 (Zero-Copy)')
    plt.xlabel('Message Size (Bytes)')
    plt.ylabel('CPU Cycles per Byte')
    plt.title(f'CPU Cycles per Byte vs Message Size (Threads=1)\n{SYSTEM_INFO}')
    plt.xscale('log', base=2)
    plt.grid(True, which="both", linestyle='--', alpha=0.5)
    plt.legend()
    plt.tight_layout()
    plt.savefig('cycles_per_byte.png')
    plt.close()

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--sync":
        sync_from_csv()
    
    print("="*60)
    print("MT25027 - Generating Plots with Hardcoded Values")
    print("(As per assignment requirement Note 6)")
    print("="*60)
    plot_throughput_vs_size()
    plot_latency_vs_threads()
    plot_l1_cache_misses()
    plot_llc_cache_misses()
    plot_cycles_per_byte()
    print("All 5 plots generated successfully!")
    print("="*60)
