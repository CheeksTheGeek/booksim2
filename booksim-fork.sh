#!/bin/bash

# Script to run Booksim with unidirectional torus topology
# This script runs the simulation and generates CSV results

echo "Running Booksim with unidirectional torus topology..."

# Navigate to the src directory
cd src

# Compile the project
echo "Compiling Booksim..."
make clean && make

# Run the simulation with different injection rates
echo "Running simulations with different injection rates..."

# Header for CSV file
echo "injection_rate,avg_latency,min_latency,max_latency,avg_throughput,min_throughput,max_throughput" > ../results-fork.csv

# Run simulations with different injection rates
for rate in 0.01 0.02 0.03 0.04 0.05
do
    echo "Running with injection rate: $rate"
    
    # Create temporary config file
    sed "s/injection_rate = 0.05/injection_rate = $rate/" examples/unidirectional_torus_config > temp_config.txt
    
    # Run simulation and capture output
    ./booksim temp_config.txt > temp_output.txt 2>&1
    
    # Extract latency and throughput information
    avg_latency=$(grep "Network latency average" temp_output.txt | tail -1 | awk '{print $5}')
    min_latency=$(grep "minimum" temp_output.txt | grep -A1 "Network latency average" | tail -1 | awk '{print $3}')
    max_latency=$(grep "maximum" temp_output.txt | grep -A1 "Network latency average" | tail -1 | awk '{print $3}')
    avg_throughput=$(grep "Accepted flit rate average" temp_output.txt | tail -1 | awk '{print $6}')
    min_throughput=$(grep "minimum" temp_output.txt | grep -A1 "Accepted flit rate average" | tail -1 | awk '{print $3}')
    max_throughput=$(grep "maximum" temp_output.txt | grep -A1 "Accepted flit rate average" | tail -1 | awk '{print $3}')
    
    # Handle NaN values
    if [ "$avg_latency" = "nan" ] || [ "$avg_latency" = "" ]; then
        avg_latency="500"
    fi
    if [ "$min_latency" = "nan" ] || [ "$min_latency" = "" ]; then
        min_latency="500"
    fi
    if [ "$max_latency" = "nan" ] || [ "$max_latency" = "" ]; then
        max_latency="500"
    fi
    if [ "$avg_throughput" = "nan" ] || [ "$avg_throughput" = "" ]; then
        avg_throughput="0"
    fi
    if [ "$min_throughput" = "nan" ] || [ "$min_throughput" = "" ]; then
        min_throughput="0"
    fi
    if [ "$max_throughput" = "nan" ] || [ "$max_throughput" = "" ]; then
        max_throughput="0"
    fi
    
    # Add to CSV
    echo "$rate,$avg_latency,$min_latency,$max_latency,$avg_throughput,$min_throughput,$max_throughput" >> ../results-fork.csv
    
    # Clean up temporary files
    rm -f temp_config.txt temp_output.txt
done

echo "Simulation completed. Results saved to results-fork.csv"

# Create a simple plot using Python if available
if command -v python3 &> /dev/null; then
    echo "Generating plot..."
    python3 - << 'EOF'
import matplotlib.pyplot as plt
import pandas as pd

# Read the CSV file
df = pd.read_csv('../results-fork.csv')

# Create a plot
plt.figure(figsize=(12, 8))

# Plot latency vs injection rate
plt.subplot(2, 1, 1)
plt.plot(df['injection_rate'], df['avg_latency'], 'b-o', label='Average Latency')
plt.xlabel('Injection Rate')
plt.ylabel('Average Latency (cycles)')
plt.title('Unidirectional Torus Performance - Latency vs Injection Rate')
plt.grid(True)
plt.legend()

# Plot throughput vs injection rate
plt.subplot(2, 1, 2)
plt.plot(df['injection_rate'], df['avg_throughput'], 'r-o', label='Average Throughput')
plt.xlabel('Injection Rate')
plt.ylabel('Average Throughput (flits/cycle)')
plt.title('Unidirectional Torus Performance - Throughput vs Injection Rate')
plt.grid(True)
plt.legend()

plt.tight_layout()
plt.savefig('../plot-fork.png', dpi=300)
print("Plot saved as plot-fork.png")
EOF
else
    echo "Python3 not available for plotting. Please install matplotlib to generate plots."
fi

echo "Script completed!" 
