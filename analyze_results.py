#!/usr/bin/env python3
"""
CKKS Performance Analysis Script
Analyzes benchmark results and generates publication-quality plots
for comparing Standard CKKS vs Binary CKKS performance.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import glob
import os
from pathlib import Path
import argparse

# Set publication-quality plot style
plt.style.use('seaborn-v0_8-paper')
sns.set_palette("husl")

class CKKSResultsAnalyzer:
    def __init__(self, data_directory="."):
        self.data_dir = Path(data_directory)
        self.results = {}
        self.load_all_results()
        
    def load_all_results(self):
        """Load all CSV files matching the pattern ckks_comparison_*.csv"""
        csv_files = list(self.data_dir.glob("ckks_comparison_*.csv"))
        
        if not csv_files:
            print(f"No benchmark result files found in {self.data_dir}")
            print("Expected files matching pattern: ckks_comparison_*.csv")
            return
        
        print(f"Found {len(csv_files)} result files:")
        for file in csv_files:
            print(f"  - {file.name}")
        
        for csv_file in csv_files:
            # Extract parameters from filename: ckks_comparison_<ring_dim>_<security>.csv
            parts = csv_file.stem.split('_')
            if len(parts) >= 4:
                ring_dim = int(parts[2])
                security = int(parts[3])
                
                try:
                    df = pd.read_csv(csv_file)
                    self.results[(ring_dim, security)] = df
                    print(f"Loaded results for ring_dim={ring_dim}, security={security}")
                except Exception as e:
                    print(f"Error loading {csv_file}: {e}")
    
    def create_summary_dataframe(self):
        """Create a summary dataframe with all results"""
        summary_data = []
        
        for (ring_dim, security), df in self.results.items():
            for _, row in df.iterrows():
                summary_data.append({
                    'RingDim': ring_dim,
                    'Security': security,
                    'Operation': row['Operation'],
                    'StandardCKKS_Mean': row['Standard_CKKS_Mean'],
                    'StandardCKKS_Stddev': row['Standard_CKKS_Stddev'],
                    'BinaryCKKS_Mean': row['Binary_CKKS_Mean'],
                    'BinaryCKKS_Stddev': row['Binary_CKKS_Stddev'],
                    'Speedup': row['Speedup'],
                    'Efficiency': (row['Speedup'] - 1) / row['Speedup'] * 100 if row['Speedup'] > 1 else 0
                })
        
        return pd.DataFrame(summary_data)
    
    def plot_performance_comparison(self, save_path="performance_comparison.pdf"):
        """Create a comprehensive performance comparison plot"""
        summary_df = self.create_summary_dataframe()
        
        if summary_df.empty:
            print("No data available for plotting")
            return
        
        fig, axes = plt.subplots(2, 2, figsize=(14, 10))
        fig.suptitle('CKKS Standard vs Binary Variant Performance Comparison', 
                    fontsize=16, fontweight='bold')
        
        # 1. Runtime comparison (log scale)
        ax1 = axes[0, 0]
        operations = summary_df['Operation'].unique()
        ring_dims = sorted(summary_df['RingDim'].unique())
        
        x = np.arange(len(operations))
        width = 0.35
        
        # Take results for the largest ring dimension for main comparison
        main_data = summary_df[summary_df['RingDim'] == max(ring_dims)]
        
        std_means = [main_data[main_data['Operation'] == op]['StandardCKKS_Mean'].iloc[0] for op in operations]
        bin_means = [main_data[main_data['Operation'] == op]['BinaryCKKS_Mean'].iloc[0] for op in operations]
        std_stds = [main_data[main_data['Operation'] == op]['StandardCKKS_Stddev'].iloc[0] for op in operations]
        bin_stds = [main_data[main_data['Operation'] == op]['BinaryCKKS_Stddev'].iloc[0] for op in operations]
        
        bars1 = ax1.bar(x - width/2, std_means, width, yerr=std_stds, label='Standard CKKS', 
                       alpha=0.8, capsize=5)
        bars2 = ax1.bar(x + width/2, bin_means, width, yerr=bin_stds, label='Binary CKKS', 
                       alpha=0.8, capsize=5)
        
        ax1.set_xlabel('Operations')
        ax1.set_ylabel('Runtime (μs)')
        ax1.set_title(f'Runtime Comparison (Ring Dimension: {max(ring_dims)})')
        ax1.set_yscale('log')
        ax1.set_xticks(x)
        ax1.set_xticklabels(operations, rotation=45, ha='right')
        ax1.legend()
        ax1.grid(True, alpha=0.3)
        
        # 2. Speedup analysis
        ax2 = axes[0, 1]
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
        
        for operation in ['Encryption', 'Addition', 'Multiplication']:
            if operation in summary_df['Operation'].values:
                op_data = summary_df[summary_df['Operation'] == operation]
                ring_dims_sorted = sorted(op_data['RingDim'].unique())
                std_times = [op_data[op_data['RingDim'] == rd]['StandardCKKS_Mean'].iloc[0] 
                           for rd in ring_dims_sorted]
                bin_times = [op_data[op_data['RingDim'] == rd]['BinaryCKKS_Mean'].iloc[0] 
                           for rd in ring_dims_sorted]
                
                ax3.plot(ring_dims_sorted, std_times, 'o-', label=f'Standard {operation}', 
                        linewidth=2, markersize=6)
                ax3.plot(ring_dims_sorted, bin_times, 's--', label=f'Binary {operation}', 
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
        
    def plot_detailed_analysis(self, save_path="detailed_analysis.pdf"):
        """Create detailed analysis plots"""
        summary_df = self.create_summary_dataframe()
        
        if summary_df.empty:
            print("No data available for plotting")
            return
        
        fig, axes = plt.subplots(2, 3, figsize=(18, 12))
        fig.suptitle('Detailed CKKS Performance Analysis', fontsize=16, fontweight='bold')
        
        operations = summary_df['Operation'].unique()
        
        # Individual operation comparisons
        for i, operation in enumerate(operations[:6]):  # Limit to 6 operations
            if i >= 6:
                break
                
            row = i // 3
            col = i % 3
            ax = axes[row, col]
            
            op_data = summary_df[summary_df['Operation'] == operation]
            ring_dims = sorted(op_data['RingDim'].unique())
            
            std_times = []
            bin_times = []
            std_errs = []
            bin_errs = []
            
            for rd in ring_dims:
                rd_data = op_data[op_data['RingDim'] == rd]
                if not rd_data.empty:
                    std_times.append(rd_data['StandardCKKS_Mean'].iloc[0])
                    bin_times.append(rd_data['BinaryCKKS_Mean'].iloc[0])
                    std_errs.append(rd_data['StandardCKKS_Stddev'].iloc[0])
                    bin_errs.append(rd_data['BinaryCKKS_Stddev'].iloc[0])
            
            if std_times and bin_times:
                ax.errorbar(ring_dims, std_times, yerr=std_errs, 
                           label='Standard CKKS', marker='o', linewidth=2, capsize=5)
                ax.errorbar(ring_dims, bin_times, yerr=bin_errs, 
                           label='Binary CKKS', marker='s', linewidth=2, capsize=5)
                
                ax.set_xlabel('Ring Dimension')
                ax.set_ylabel('Runtime (μs)')
                ax.set_title(f'{operation} Performance')
                ax.set_yscale('log')
                ax.legend()
                ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
        print(f"Detailed analysis plot saved to {save_path}")
    
    def generate_latex_table(self, save_path="results_table.tex"):
        """Generate a LaTeX table for the paper"""
        summary_df = self.create_summary_dataframe()
        
        if summary_df.empty:
            print("No data available for table generation")
            return
        
        # Take results for the largest ring dimension
        main_data = summary_df[summary_df['RingDim'] == max(summary_df['RingDim'].unique())]
        
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
            latex_content += f"{row['StandardCKKS_Mean']:.2f} & "
            latex_content += f"{row['BinaryCKKS_Mean']:.2f} & "
            latex_content += f"{row['Speedup']:.2f}x & "
            latex_content += f"{row['Efficiency']:.1f}\\% \\\\\n"
            latex_content += "\\hline\n"
        
        latex_content += "\\end{tabular}\n"
        latex_content += "\\end{table}\n"
        
        with open(save_path, 'w') as f:
            f.write(latex_content)
        
        print(f"LaTeX table saved to {save_path}")
    
    def print_summary_statistics(self):
        """Print summary statistics"""
        summary_df = self.create_summary_dataframe()
        
        if summary_df.empty:
            print("No data available for summary")
            return
        
        print("\n=== CKKS Performance Comparison Summary ===")
        print(f"Analyzed {len(self.results)} parameter configurations")
        print(f"Ring dimensions tested: {sorted(summary_df['RingDim'].unique())}")
        print(f"Security levels tested: {sorted(summary_df['Security'].unique())}")
        print(f"Operations benchmarked: {list(summary_df['Operation'].unique())}")
        
        # Overall statistics
        main_data = summary_df[summary_df['RingDim'] == max(summary_df['RingDim'].unique())]
        
        print(f"\n=== Results for Ring Dimension {max(summary_df['RingDim'].unique())} ===")
        print("Operation           | Standard CKKS | Binary CKKS | Speedup | Efficiency")
        print("-" * 75)
        
        for _, row in main_data.iterrows():
            print(f"{row['Operation']:<18} | {row['StandardCKKS_Mean']:>10.2f} μs | "
                  f"{row['BinaryCKKS_Mean']:>8.2f} μs | {row['Speedup']:>6.2f}x | "
                  f"{row['Efficiency']:>8.1f}%")
        
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
    parser = argparse.ArgumentParser(description='Analyze CKKS benchmark results')
    parser.add_argument('--data-dir', default='.', 
                       help='Directory containing benchmark CSV files')
    parser.add_argument('--output-dir', default='.', 
                       help='Directory to save output files')
    
    args = parser.parse_args()
    
    # Create output directory if it doesn't exist
    output_dir = Path(args.output_dir)
    output_dir.mkdir(exist_ok=True)
    
    # Analyze results
    analyzer = CKKSResultsAnalyzer(args.data_dir)
    
    if not analyzer.results:
        print("No benchmark results found. Please run the benchmark first.")
        return
    
    # Print summary
    analyzer.print_summary_statistics()
    
    # Generate plots
    print("\nGenerating plots...")
    analyzer.plot_performance_comparison(output_dir / "performance_comparison.pdf")
    analyzer.plot_detailed_analysis(output_dir / "detailed_analysis.pdf")
    
    # Generate LaTeX table
    print("\nGenerating LaTeX table...")
    analyzer.generate_latex_table(output_dir / "results_table.tex")
    
    print(f"\nAll outputs saved to {output_dir}")
    print("\nFor your conference paper, use:")
    print("1. performance_comparison.pdf - Main comparison figure")
    print("2. detailed_analysis.pdf - Detailed operation analysis")  
    print("3. results_table.tex - LaTeX table for inclusion in paper")

if __name__ == "__main__":
    main()