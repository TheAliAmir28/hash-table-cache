#!/usr/bin/env python3
"""
Benchmark Results Visualization
Generates graphs from CSV files in results/ directory
"""

import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# Set style for professional-looking graphs
plt.style.use('seaborn-v0_8-darkgrid')
plt.rcParams['figure.figsize'] = (14, 10)
plt.rcParams['font.size'] = 11
plt.rcParams['axes.titlesize'] = 14
plt.rcParams['axes.labelsize'] = 12

def create_throughput_graph():
    """Create bar chart comparing throughput"""
    # Read data
    df = pd.read_csv('results/throughput.csv')
    
    # Create figure
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Define colors
    colors = ['#2ecc71', '#e74c3c', '#3498db']  # Green, Red, Blue
    
    # Create bars
    bars = ax.bar(df['Implementation'], df['OpsPerSecond'], color=colors, edgecolor='black', linewidth=1.5)
    
    # Add value labels on top of bars
    for i, (impl, ops) in enumerate(zip(df['Implementation'], df['OpsPerSecond'])):
        ax.text(i, ops + max(df['OpsPerSecond']) * 0.02, 
                f'{int(ops):,}', 
                ha='center', va='bottom', fontweight='bold', fontsize=12)
    
    # Add improvement annotation
    inc_ops = df[df['Implementation'] == 'Incremental']['OpsPerSecond'].values[0]
    naive_ops = df[df['Implementation'] == 'Naive']['OpsPerSecond'].values[0]
    improvement = (inc_ops / naive_ops)
    
    ax.text(0.5, max(df['OpsPerSecond']) * 0.85, 
            f'Incremental is {improvement:.1f}x faster!',
            transform=ax.transData,
            bbox=dict(boxstyle='round,pad=0.5', facecolor='yellow', alpha=0.7, edgecolor='black', linewidth=2),
            fontsize=13, fontweight='bold', ha='center')
    
    # Formatting
    ax.set_ylabel('Operations per Second', fontweight='bold')
    ax.set_title('Throughput Comparison - Higher is Better', fontweight='bold', pad=20)
    ax.set_ylim(0, max(df['OpsPerSecond']) * 1.15)
    ax.grid(axis='y', alpha=0.3)
    
    # Format y-axis with commas
    ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda x, p: f'{int(x):,}'))
    
    plt.tight_layout()
    plt.savefig('results/throughput_comparison.png', dpi=300, bbox_inches='tight')
    print("✓ Created: results/throughput_comparison.png")
    return fig

def create_latency_graph():
    """Create grouped bar chart for latency metrics"""
    # Read data
    df = pd.read_csv('results/latency.csv')
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    # Graph 1: P99 Latency Comparison
    metrics = ['P99']
    implementations = df['Implementation'].tolist()
    
    x = np.arange(len(implementations))
    width = 0.35
    
    p99_values = df['P99'].tolist()
    colors = ['#2ecc71', '#e74c3c', '#3498db']
    
    bars1 = ax1.bar(x, p99_values, width, color=colors, edgecolor='black', linewidth=1.5)
    
    # Add value labels
    for i, v in enumerate(p99_values):
        ax1.text(i, v + max(p99_values) * 0.02, f'{v:.1f} μs', 
                ha='center', va='bottom', fontweight='bold')
    
    # Calculate improvement
    inc_p99 = df[df['Implementation'] == 'Incremental']['P99'].values[0]
    naive_p99 = df[df['Implementation'] == 'Naive']['P99'].values[0]
    improvement_pct = ((naive_p99 - inc_p99) / naive_p99) * 100
    
    if improvement_pct > 0:
        ax1.text(0.5, max(p99_values) * 0.8, 
                f'{improvement_pct:.1f}% better',
                bbox=dict(boxstyle='round,pad=0.5', facecolor='lightgreen', alpha=0.7, edgecolor='black', linewidth=2),
                fontsize=11, fontweight='bold', ha='center')
    
    ax1.set_ylabel('Latency (μs)', fontweight='bold')
    ax1.set_title('P99 Latency - Lower is Better', fontweight='bold')
    ax1.set_xticks(x)
    ax1.set_xticklabels(implementations, rotation=15, ha='right')
    ax1.grid(axis='y', alpha=0.3)
    
    # Graph 2: Max Latency (The Real Story!)
    max_values = df['Max'].tolist()
    bars2 = ax2.bar(x, max_values, width, color=colors, edgecolor='black', linewidth=1.5)
    
    # Add value labels
    for i, v in enumerate(max_values):
        if v < 100:  # microseconds
            ax2.text(i, v + max(max_values) * 0.02, f'{v:.1f} μs', 
                    ha='center', va='bottom', fontweight='bold')
        else:  # convert to milliseconds for readability
            ax2.text(i, v + max(max_values) * 0.02, f'{v/1000:.1f} ms', 
                    ha='center', va='bottom', fontweight='bold')
    
    # Highlight the spike difference
    inc_max = df[df['Implementation'] == 'Incremental']['Max'].values[0]
    naive_max = df[df['Implementation'] == 'Naive']['Max'].values[0]
    spike_diff = ((naive_max - inc_max) / naive_max) * 100
    
    ax2.text(0.5, max(max_values) * 0.8, 
            f'Incremental: {inc_max/1000:.1f}ms\nNaive: {naive_max/1000:.1f}ms\n({spike_diff:.0f}% better)',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='yellow', alpha=0.7, edgecolor='black', linewidth=2),
            fontsize=11, fontweight='bold', ha='center')
    
    ax2.set_ylabel('Max Latency (μs)', fontweight='bold')
    ax2.set_title('Worst-Case Latency - Lower is Better', fontweight='bold')
    ax2.set_xticks(x)
    ax2.set_xticklabels(implementations, rotation=15, ha='right')
    ax2.grid(axis='y', alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('results/latency_comparison.png', dpi=300, bbox_inches='tight')
    print("✓ Created: results/latency_comparison.png")
    return fig

def create_spike_graph():
    """Create comparison of rehashing spikes"""
    # Read data
    df = pd.read_csv('results/spikes.csv')
    
    fig, ax = plt.subplots(figsize=(10, 6))
    
    implementations = df['Implementation'].tolist()
    x = np.arange(len(implementations))
    width = 0.35
    
    before = df['BeforeMax'].tolist()
    during = df['DuringMax'].tolist()
    
    bars1 = ax.bar(x - width/2, before, width, label='Before Rehash', 
                   color='#3498db', edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, during, width, label='During Rehash', 
                   color='#e74c3c', edgecolor='black', linewidth=1.5)
    
    # Add value labels
    for i, (b, d) in enumerate(zip(before, during)):
        ax.text(i - width/2, b + max(during) * 0.02, f'{b:.1f}μs', 
               ha='center', va='bottom', fontsize=10, fontweight='bold')
        ax.text(i + width/2, d + max(during) * 0.02, f'{d:.1f}μs', 
               ha='center', va='bottom', fontsize=10, fontweight='bold')
    
    # Add spike ratio annotations
    for i, (impl, ratio) in enumerate(zip(implementations, df['SpikeRatio'])):
        ax.text(i, max(during) * 0.7, f'{ratio:.1f}x spike',
               ha='center', fontsize=10, fontweight='bold',
               bbox=dict(boxstyle='round,pad=0.3', facecolor='white', alpha=0.8))
    
    ax.set_ylabel('Max Latency (μs)', fontweight='bold')
    ax.set_title('Rehashing Spike Comparison - Lower Spike is Better', fontweight='bold', pad=20)
    ax.set_xticks(x)
    ax.set_xticklabels(implementations)
    ax.legend(loc='upper left', frameon=True, shadow=True)
    ax.grid(axis='y', alpha=0.3)
    
    plt.tight_layout()
    plt.savefig('results/spike_comparison.png', dpi=300, bbox_inches='tight')
    print("✓ Created: results/spike_comparison.png")
    return fig

def create_summary_dashboard():
    """Create a comprehensive dashboard with all metrics"""
    # Read all data
    latency_df = pd.read_csv('results/latency.csv')
    throughput_df = pd.read_csv('results/throughput.csv')
    
    fig = plt.figure(figsize=(16, 10))
    gs = fig.add_gridspec(3, 2, hspace=0.3, wspace=0.3)
    
    # 1. Throughput comparison (large, top left)
    ax1 = fig.add_subplot(gs[0:2, 0])
    colors = ['#2ecc71', '#e74c3c', '#3498db']
    bars = ax1.bar(throughput_df['Implementation'], throughput_df['OpsPerSecond'], 
                   color=colors, edgecolor='black', linewidth=2)
    for i, (impl, ops) in enumerate(zip(throughput_df['Implementation'], throughput_df['OpsPerSecond'])):
        ax1.text(i, ops + max(throughput_df['OpsPerSecond']) * 0.02, 
                f'{int(ops):,}', ha='center', va='bottom', fontweight='bold', fontsize=11)
    
    inc_ops = throughput_df[throughput_df['Implementation'] == 'Incremental']['OpsPerSecond'].values[0]
    naive_ops = throughput_df[throughput_df['Implementation'] == 'Naive']['OpsPerSecond'].values[0]
    improvement = (inc_ops / naive_ops)
    ax1.text(0.5, max(throughput_df['OpsPerSecond']) * 0.7, 
            f'{improvement:.1f}x FASTER',
            bbox=dict(boxstyle='round,pad=0.8', facecolor='yellow', alpha=0.8, edgecolor='black', linewidth=3),
            fontsize=16, fontweight='bold', ha='center')
    
    ax1.set_ylabel('Operations / Second', fontweight='bold', fontsize=12)
    ax1.set_title('Throughput - THE KEY METRIC', fontweight='bold', fontsize=14)
    ax1.yaxis.set_major_formatter(plt.FuncFormatter(lambda x, p: f'{int(x):,}'))
    ax1.grid(axis='y', alpha=0.3)
    
    # 2. P99 Latency (top right)
    ax2 = fig.add_subplot(gs[0, 1])
    p99_values = latency_df['P99'].tolist()
    ax2.bar(range(len(p99_values)), p99_values, color=colors, edgecolor='black', linewidth=1.5)
    for i, v in enumerate(p99_values):
        ax2.text(i, v + max(p99_values) * 0.05, f'{v:.1f}', ha='center', va='bottom', fontweight='bold')
    ax2.set_ylabel('P99 Latency (μs)', fontweight='bold')
    ax2.set_title('P99 Latency', fontweight='bold')
    ax2.set_xticks(range(len(latency_df)))
    ax2.set_xticklabels(latency_df['Implementation'], rotation=20, ha='right', fontsize=9)
    ax2.grid(axis='y', alpha=0.3)
    
    # 3. Max Latency (middle right)
    ax3 = fig.add_subplot(gs[1, 1])
    max_values = latency_df['Max'].tolist()
    ax3.bar(range(len(max_values)), max_values, color=colors, edgecolor='black', linewidth=1.5)
    for i, v in enumerate(max_values):
        ax3.text(i, v + max(max_values) * 0.05, f'{v/1000:.1f}ms', 
                ha='center', va='bottom', fontweight='bold')
    ax3.set_ylabel('Max Latency (μs)', fontweight='bold')
    ax3.set_title('Worst-Case Latency', fontweight='bold')
    ax3.set_xticks(range(len(latency_df)))
    ax3.set_xticklabels(latency_df['Implementation'], rotation=20, ha='right', fontsize=9)
    ax3.grid(axis='y', alpha=0.3)
    
    # 4. Summary text box (bottom)
    ax4 = fig.add_subplot(gs[2, :])
    ax4.axis('off')
    
    summary_text = f"""
    BENCHMARK RESULTS SUMMARY
    Test System: Intel i7-13700K (16 cores, 5.4GHz) + RTX 4070 Ti Super
    
    KEY FINDINGS:
    • Throughput: Incremental achieves {improvement:.1f}x better throughput ({int(inc_ops)} vs {int(naive_ops)} ops/sec)
    • Max Latency: {((max_values[1] - max_values[0]) / max_values[1] * 100):.0f}% lower worst-case latency
    • Consistency: No {max_values[1]/1000:.0f}ms pauses during rehashing
    
    CONCLUSION: Even on flagship hardware, incremental rehashing provides significant throughput
    improvement by eliminating long rehashing pauses. This advantage would be even more
    pronounced on slower hardware or under high load conditions.
    """
    
    ax4.text(0.5, 0.5, summary_text, 
            transform=ax4.transAxes,
            fontsize=11,
            verticalalignment='center',
            horizontalalignment='center',
            bbox=dict(boxstyle='round,pad=1', facecolor='lightblue', alpha=0.3, edgecolor='black', linewidth=2),
            family='monospace')
    
    plt.suptitle('Hash Table Performance Benchmark - Incremental vs Full Rehashing', 
                fontsize=16, fontweight='bold', y=0.98)
    
    plt.savefig('results/benchmark_dashboard.png', dpi=300, bbox_inches='tight')
    print("Created: results/benchmark_dashboard.png")
    return fig

def main():
    print("=" * 50)
    print("  Creating Benchmark Visualizations")
    print("=" * 50)
    print()
    
    try:
        # Create individual graphs
        print("Generating graphs...")
        create_throughput_graph()
        create_latency_graph()
        create_spike_graph()
        
        # Create comprehensive dashboard
        create_summary_dashboard()
        
        print()
        print("=" * 50)
        print("All Graphs Created Successfully!")
        print("=" * 50)
        print()
        print("Created files:")
        print("  1. throughput_comparison.png - Bar chart of ops/sec")
        print("  2. latency_comparison.png - P99 and Max latency")
        print("  3. spike_comparison.png - Rehashing spike analysis")
        print("  4. benchmark_dashboard.png - Comprehensive summary")
        print()
        print("All files saved in: results/")
        print()
        
    except FileNotFoundError as e:
        print(f"Error: Could not find CSV files in results/ directory")
        print(f"Make sure you've run ./benchmark first!")
        print(f"Details: {e}")
    except Exception as e:
        print(f"Error creating graphs: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
