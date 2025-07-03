#!/usr/bin/env python3
"""
CKKS Performance Analysis Script for Google Benchmark Results
Analyzes Google Benchmark JSON output and generates publication-quality plots
for comparing Standard CKKS vs Binary CKKS performance.
"""

import json
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import argparse
from pathlib import Path
import re

# Set publication-quality plot style
plt.style.use('seaborn-v0_8-paper')
sns.set_palette("husl")

class CKKSBenchmarkAnalyzer:
    def __init__(self, json_file):
        self.json_file = Path(json_file)
        self.results = {}
        self.load_benchmark_results()
        
    def load_benchmark_results(self):
        """Load Google Benchmark JSON results"""
        if not self.json_file.exists():
            raise FileNotFoundError(f"Benchmark results file not found: {self.json_file}")
        
        with open(self.json_file, 'r') as f:
            data = json.load(f)
        
        # Extract benchmark results
        benchmarks = data.get('benchmarks', [])
        
        for benchmark in benchmarks:
            name = benchmark['name']
            time_ns = benchmark['real_time']  # nanoseconds
            time_us = time_ns / 1000.0  # convert to microseconds
            
            # Parse benchmark name to extract information
            # Format: operation_type/parameter_set
            parts = name.split('/')
            if len(parts) >= 1:
                base_name = parts[0]
                
                # Extract operation type and variant
                if 'standard_ckks_' in base_name:
                    variant = 'Standard CKKS'
                    operation = base_name.replace('standard_ckks_', '').title()
                elif 'binary_ckks_' in base_name:
                    variant = 'Binary CKKS'
                    operation = base_name.replace('binary_ckks_', '').title()
                else:
                    continue
                
                # Extract parameter information
                param_match = re.search(r'(small|medium|large)_(\d+)', base_name)
                if param_match:
                    size_category = param_match.group(1)
                    ring_dim = int(param_match.group(2))
                else:
                    size_category = 'unknown'
                    ring_dim = 1024  # default
                
                key = (operation, ring_dim, size_category)
                if key not in self.results:
                    self.results[key] = {}
                
                self.results[key][variant] = {
                    'time_us': time_us,
                    'iterations': benchmark.get('iterations', 1),
                    'cpu_time': benchmark.get('cpu_time', time_ns) / 1000.0
                }
    
    def create_summary_dataframe(self):
        """Create a summary dataframe with all results"""
        summary_data = []
        
        for (operation, ring_dim, size_category), variants in self.results.items():
            if 'Standard CKKS' in variants and 'Binary CKKS' in variants:
                std_time = variants['Standard CKKS']['time_us']
                bin_time = variants['Binary CKKS']['time_us']
                speedup = std_time / bin_time if bin_time > 0 else 0
                efficiency = (speedup - 1) / speedup * 100 if speedup > 1 else 0
                
                summary_data.append({
                    'Operation': operation,
                    'RingDim': ring_dim,
                    'SizeCategory': size_category,
                    'StandardCKKS_Time': std_time,
                    'BinaryCKKS_Time': bin_time,
                    'Speedup': speedup,
                    'Efficiency': efficiency
                })
        
        return pd.DataFrame(summary_data)
    
    def plot_performance_comparison(self, save_path="ckks_performance_comparison.pdf"):
        """Create a comprehensive performance comparison plot"""
        df = self.create_summary_dataframe()
        
        if df.empty:
            print("No data available for plotting")
            return
        
        fig, axes = plt.subplots(2, 2, figsize=(14, 10))
        fig.suptitle('CKKS Standard vs Binary Variant Performance Comparison', 
                    fontsize=16, fontweight='bold')
        
        # 1. Runtime comparison by operation (use largest ring dimension)
        ax1 = axes[0, 0]
        max_ring_dim = df['RingDim'].max()
        main_data = df[df['RingDim'] == max_ring_dim]
        
        if not main_data.empty:
            operations = main_data['Operation'].unique()
            x = np.arange(len(operations))
            width = 0.35
            
            std_times = [main_data[main_data['Operation'] == op]['StandardCKKS_Time'].iloc[0] for op in operations]
            bin_times = [main_data[main_data['Operation'] == op]['BinaryCKKS_Time'].iloc[0] for op in operations]
            
            bars1 = ax1.bar(x - width/2, std_times, width, label='Standard CKKS', alpha=0.8)
            bars2 = ax1.bar(x + width/2, bin_times, width, label='Binary CKKS', alpha=0.8)
            
            ax1.set_xlabel('Operations')
            ax1.set_ylabel('Runtime (μs)')
            ax1.set_title(f'Runtime Comparison (Ring Dimension: {max_ring_dim})')
            ax1.set_yscale('log')
            ax1.set_xticks(x)
            ax1.set_xticklabels(operations, rotation=45, ha='right')
            ax1.legend()
            ax1.grid(True, alpha=0.3)
        
        # 2. Speedup analysis
        ax2 = axes[0, 1]
        if not main_data.empty:
            speedups = [main_data[main_data['Operation'] == op]['Speedup'].iloc[0] for op in operations]
            colors = ['green' if s > 1 else 'red' for s in speedups]
            
            bars = ax2.bar(operations, speedups, color=colors, alpha=0.7)
            ax2.axhline(y=1, color='black', linestyle='--', alpha=0.5)
            ax2.set_xlabel('Operations')
            ax2.set_ylabel('Speedup (Standard/Binary)')
            ax2.set_title('Speedup Analysis')
            ax2.set_xticklabels(operations, rotation=45, ha='right')
            ax2.grid(True, alpha=0.3)
            
            # Add speedup values on bars
            for bar, speedup in zip(bars, speedups):
                height = bar.get_height()
                ax2.text(bar.get_x() + bar.get_width()/2., height + 0.05,
                        f'{speedup:.2f}x', ha='center', va='bottom', fontweight='bold')
        
        # 3. Scaling with ring dimension
        ax3 = axes[1, 0]
        key_operations = ['Encrypt', 'Add', 'Multiply']
        
        for operation in key_operations:
            if operation in df['Operation'].values:
                op_data = df[df['Operation'] == operation]
                ring_dims = sorted(op_data['RingDim'].unique())
                
                std_times = []
                bin_times = []
                for rd in ring_dims:
                    rd_data = op_data[op_data['RingDim'] == rd]
                    if not rd_data.empty:
                        std_times.append(rd_data['StandardCKKS_Time'].iloc[0])
                        bin_times.append(rd_data['BinaryCKKS_Time'].iloc[0])
                
                if std_times and bin_times:
                    ax3.plot(ring_dims, std_times, 'o-', label=f'Standard {operation}', 
                            linewidth=2, markersize=6)
                    ax3.plot(ring_dims, bin_times, 's--', label=f'Binary {operation}', 
                            linewidth=2, markersize=6)
        
        ax3.set_xlabel('Ring Dimension')
        ax3.set_ylabel('Runtime (μs)')
        ax3.set_title('Scaling with Ring Dimension')
        ax3.set_yscale('log')
        ax3.set_xscale('log')
        ax3.legend()
        ax3.grid(True, alpha=0.3)
        
        # 4. Efficiency comparison
        ax4 = axes[1, 1]
        if not main_data.empty:
            efficiencies = [main_data[main_data['Operation'] == op]['Efficiency'].iloc[0] for op in operations]
            colors = plt.cm.RdYlGn([e/100 for e in efficiencies])
            
            bars = ax4.bar(operations, efficiencies, color=colors, alpha=0.8)
            ax4.set_xlabel('Operations')
            ax4.set_ylabel('Efficiency (%)')
            ax4.set_title('Binary CKKS Efficiency Gain')
            ax4.set_xticklabels(operations, rotation=45, ha='right')
            ax4.grid(True, alpha=0.3)
            
            # Add efficiency values on bars
            for bar, eff in zip(bars, efficiencies):
                height = bar.get_height()
                ax4.text(bar.get_x() + bar.get_width()/2., height + 1,
                        f'{eff:.1f}%', ha='center', va='bottom', fontweight='bold')
        
        plt.tight_layout()
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
        print(f"Performance comparison plot saved to {save_path}")
    
    def generate_latex_table(self, save_path="ckks_results_table.tex"):
        """Generate a LaTeX table for the paper"""
        df = self.create_summary_dataframe()
        
        if df.empty:
            print("No data available for table generation")
            return
        
        # Use largest ring dimension for main table
        max_ring_dim = df['RingDim'].max()
        main_data = df[df['RingDim'] == max_ring_dim]
        
        latex_content = "\\begin{table}[htbp]\n"
        latex_content += "\\centering\n"
        latex_content += "\\caption{Performance Comparison: Standard CKKS vs Binary CKKS}\n"
        latex_content += "\\label{tab:ckks_comparison}\n"
        latex_content += "\\begin{tabular}{|l|r|r|r|r|}\n"
        latex_content += "\\hline\n"
        latex_content += "Operation & Standard CKKS & Binary CKKS & Speedup & Efficiency \\\\\n"
        latex_content += " & ($\\mu$s) & ($\\mu$s) & & (\\%) \\\\\n"
        latex_content += "\\hline\n"
        
        for _, row in main_data.iterrows():
            latex_content += f"{row['Operation']} & "
            latex_content += f"{row['StandardCKKS_Time']:.2f} & "
            latex_content += f"{row['BinaryCKKS_Time']:.2f} & "
            latex_content += f"{row['Speedup']:.2f}x & "
            latex_content += f"{row['Efficiency']:.1f}\\% \\\\\n"
            latex_content += "\\hline\n"
        
        latex_content += "\\end{tabular}\n"
        latex_content += f"\\end{{table}}\n"
        latex_content += f"% Results for ring dimension {max_ring_dim}\n"
        
        with open(save_path, 'w') as f:
            f.write(latex_content)
        
        print(f"LaTeX table saved to {save_path}")
    
    def print_summary_statistics(self):
        """Print summary statistics"""
        df = self.create_summary_dataframe()
        
        if df.empty:
            print("No data available for summary")
            return
        
        print("\n=== CKKS Performance Comparison Summary ===")
        print(f"Total benchmarks analyzed: {len(self.results)}")
        print(f"Ring dimensions tested: {sorted(df['RingDim'].unique())}")
        print(f"Operations benchmarked: {list(df['Operation'].unique())}")
        
        # Statistics for largest ring dimension
        max_ring_dim = df['RingDim'].max()
        main_data = df[df['RingDim'] == max_ring_dim]
        
        print(f"\n=== Results for Ring Dimension {max_ring_dim} ===")
        print("Operation           | Standard CKKS | Binary CKKS | Speedup | Efficiency")
        print("-" * 75)
        
        for _, row in main_data.iterrows():
            print(f"{row['Operation']:<18} | {row['StandardCKKS_Time']:>10.2f} μs | "
                  f"{row['BinaryCKKS_Time']:>8.2f} μs | {row['Speedup']:>6.2f}x | "
                  f"{row['Efficiency']:>8.1f}%")
        
        if not main_data.empty:
            avg_speedup = main_data['Speedup'].mean()
            max_speedup = main_data['Speedup'].max()
            min_speedup = main_data['Speedup'].min()
            
            print(f"\nSpeedup Statistics:")
            print(f"  Average: {avg_speedup:.2f}x")
            print(f"  Maximum: {max_speedup:.2f}x")
            print(f"  Minimum: {min_speedup:.2f}x")
            
            # Best and worst performing operations
            best_op = main_data.loc[main_data['Speedup'].idxmax(), 'Operation']
            worst_op = main_data.loc[main_data['Speedup'].idxmin(), 'Operation']
            
            print(f"\nBest performing operation: {best_op} ({max_speedup:.2f}x speedup)")
            print(f"Worst performing operation: {worst_op} ({min_speedup:.2f}x speedup)")

def main():
    parser = argparse.ArgumentParser(description='Analyze CKKS benchmark results from Google Benchmark JSON')
    parser.add_argument('json_file', help='Google Benchmark JSON results file')
    parser.add_argument('--output-dir', default='.', help='Directory to save output files')
    
    args = parser.parse_args()
    
    # Create output directory if it doesn't exist
    output_dir = Path(args.output_dir)
    output_dir.mkdir(exist_ok=True)
    
    try:
        # Analyze results
        analyzer = CKKSBenchmarkAnalyzer(args.json_file)
        
        # Print summary
        analyzer.print_summary_statistics()
        
        # Generate plots
        print("\nGenerating plots...")
        analyzer.plot_performance_comparison(output_dir / "ckks_performance_comparison.pdf")
        
        # Generate LaTeX table
        print("\nGenerating LaTeX table...")
        analyzer.generate_latex_table(output_dir / "ckks_results_table.tex")
        
        print(f"\nAll outputs saved to {output_dir}")
        print("\nFor your conference paper, use:")
        print("1. ckks_performance_comparison.pdf - Main comparison figure")
        print("2. ckks_results_table.tex - LaTeX table for inclusion in paper")
        
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main())