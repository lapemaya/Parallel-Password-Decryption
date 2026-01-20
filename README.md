# Password Decryption - Parallel Brute Force Attack

A high-performance parallel password cracking system that demonstrates the effectiveness of parallel computing using OpenMP. This project implements and compares sequential and parallel brute-force attacks on DES-encrypted passwords.

**GitHub Repository**: [https://github.com/lapemaya/Parallel-Password-Decryption](https://github.com/lapemaya/Parallel-Password-Decryption)

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Algorithms](#algorithms)
- [Performance Optimizations](#performance-optimizations)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Benchmarking](#benchmarking)
- [Results](#results)
- [Project Structure](#project-structure)
- [Technical Details](#technical-details)
- [License](#license)

## 🎯 Overview

This project implements a brute-force password cracking system that targets 8-character date-formatted passwords (DDMMYYYY) encrypted using the DES algorithm with crypt(). The implementation includes:

- **Sequential version**: Single-threaded baseline implementation
- **Parallel version**: OpenMP-based multi-threaded implementation with cancellation points
- **Parallel NOWAIT version**: Optimized parallel version without implicit barriers

### Key Achievements

- **Maximum speedup**: 12.04× with 25 threads (NOWAIT variant)
- **Peak throughput**: ~4.42 million passwords per second
- **Optimal efficiency**: 90-100% at 2-4 threads
- **Overthreading analysis**: Performance characterization up to 32 threads, demonstrating degradation beyond physical core counts

The project demonstrates significant performance improvements through parallelization while also revealing the limits of overthreading when thread counts exceed available hardware resources.

## ✨ Features

### Core Functionality
- **Brute-force attack** on DES-encrypted passwords
- **Random password generation** for testing (500 iterations)
- **Real-time progress tracking** with visual progress bars
- **Performance metrics**: execution time, passwords/second, speedup, efficiency
- **Correctness verification**: validates found passwords against targets

### Parallel Computing Features
- **OpenMP parallelization** with dynamic thread configuration
- **Loop cancellation**: early termination when password is found
- **Thread-safe operations**: atomic flags and critical sections
- **Reentrant cryptography**: uses `crypt_r()` for thread safety
- **Two parallel strategies**:
  - Standard version with implicit synchronization
  - NOWAIT version for reduced barrier overhead

### Performance Analysis
- **Comprehensive benchmarking suite** (bash script)
- **Automated testing** across multiple thread counts (1, 2, 4, 8, 16, 20)
- **Visualization tools** (Python scripts for plotting)
- **Metrics tracked**:
  - Execution time
  - Speedup vs sequential baseline
  - Parallel efficiency
  - Scalability
  - Throughput (passwords/second)

## 🏗️ Architecture

### Password Search Space
- **Format**: 8-character date strings (DDMMYYYY)
- **Day range**: 00-31 (32 values)
- **Month range**: 00-12 (13 values)
- **Year range**: 0000-2025 (2026 values)
- **Total combinations**: 32 × 13 × 2026 = 842,816 per iteration
- **Test iterations**: 500 random passwords per benchmark run

### Parallelization Strategy
```
┌─────────────────────────────────────────┐
│          Master Thread                  │
│  - Generates random target password     │
│  - Initializes search space             │
└───────────────┬─────────────────────────┘
                │
        ┌───────▼───────┐
        │  Fork Threads │
        └───────┬───────┘
                │
    ┌───────────┴───────────┐
    │  Parallel Search      │
    │  - collapse(3) loops  │
    │  - static scheduling  │
    │  - cancellation point │
    └───────────┬───────────┘
                │
        ┌───────▼───────┐
        │  Join Threads │
        └───────────────┘
```

## 🔬 Algorithms

### Sequential Algorithm
```cpp
for each random target password:
    for day in 0..31:
        for month in 0..12:
            for year in 0..2025:
                generate candidate password
                hash = crypt(candidate, salt)
                if hash matches target:
                    found = true
                    break all loops
```

### Parallel Algorithm (with Cancellation)
```cpp
#pragma omp parallel
{
    for each random target password:
        #pragma omp for collapse(3) schedule(static)
        for day in 0..31:
            for month in 0..12:
                for year in 0..2025:
                    #pragma omp cancellation point for
                    if found_flag:
                        #pragma omp cancel for
                    
                    generate candidate password
                    hash = crypt_r(candidate, salt, &thread_data)
                    
                    if hash matches target:
                        #pragma omp critical
                        {
                            found = candidate
                            found_flag = true
                        }
}
```

### Key Differences: Parallel vs Parallel NOWAIT

1. **Standard Parallel Version**:
   - Uses `#pragma omp cancel for` which requires waiting for all threads
   - Implicit barrier at the end of parallel for
   - Safer but slightly slower

2. **NOWAIT Version**:
   - Uses `if (found_flag) continue` without formal cancellation
   - No implicit barriers with `nowait` clause
   - Faster but requires careful synchronization

## ⚡ Performance Optimizations

### 1. Manual Date String Construction
- **Optimization**: Construct 8-character candidate passwords using arithmetic on integers (`'0' + digit`) instead of high-level string formatting
- **Effect**: Lower CPU time per candidate, reduced heap activity, less allocator-related jitter

### 2. Per-Thread Cryptographic Data
- **Optimization**: Each thread uses separate `crypt_data` struct with reentrant `crypt_r()` instead of non-reentrant `crypt()`
- **Effect**: Removes shared-state contention, avoids sequentialization, improves parallel scalability

### 3. Quick-Reject Hash Comparison
- **Optimization**: Check two representative bytes (positions 3 and 4) before performing full `strcmp()` on DES hash
- **Effect**: Most candidates fail the cheap check, reducing expensive string comparisons and saving CPU cycles

### 4. Atomic Early-Termination Flag
- **Optimization**: `std::atomic<bool>` with relaxed loads for frequent checks and release store when setting flag
- **Effect**: Fast early exit with minimal synchronization cost, balances correctness with low-overhead polling

### 5. OpenMP Cancellation Points
- **Optimization**: Cancellation points and `#pragma omp cancel for` allow threads to stop iterating when match found
- **Effect**: Faster termination when password found (note: cannot combine with `nowait` clause)

### 6. Static Work Distribution
- **Optimization**: `collapse(3)` directive with `schedule(static)` for deterministic, low-overhead partitioning
- **Effect**: Reduced runtime scheduling overhead, good cache locality for contiguous iteration chunks

### 7. Reduction for Counting
- **Optimization**: OpenMP `reduction(+:total_passwords_tested)` avoids atomic increments on hot path
- **Effect**: Lower contention on global counter while maintaining accurate aggregated metrics

### 8. Minimal Per-Iteration Allocations
- **Optimization**: Avoid dynamic allocations inside hot loop (no temporary `std::string` per candidate)
- **Effect**: Prevents allocator contention, reduces cache pollution, lower latency per candidate

### 9. Compiler Optimizations
```bash
-O3                # Maximum optimization
-march=native      # CPU-specific instructions
-ffast-math        # Aggressive math optimizations
-DNDEBUG          # Remove debug assertions
```

## 📦 Requirements

### System Requirements
- **OS**: Linux (tested on Ubuntu-based systems)
- **CPU**: Multi-core processor (tested on Intel Core i9-13900H with 14 cores, 20 threads via Hyper-Threading)
- **RAM**: Minimum 2GB
- **Compiler**: GCC 9.0+ or Clang 10.0+ with OpenMP 4.0+ support

### Software Dependencies
- **CMake** 3.12 or higher
- **OpenMP** (typically included with GCC)
- **Python 3.6+** (for visualization)
  - matplotlib
  - pandas
  - numpy

### Libraries
- `libcrypt` (DES encryption, standard on Linux)
- Standard C++ libraries (C++11)

## 🚀 Installation

### 1. Clone the Repository
```bash
git clone <repository-url>
cd Password-Decryption
```

### 2. Install Python Dependencies
```bash
pip3 install matplotlib pandas numpy
```

### 3. Build the Project
```bash
mkdir -p cmake-build-debug
cd cmake-build-debug
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ..
```

### 4. Verify Installation
```bash
ls cmake-build-debug/
# Should show: Password8Sequenziale, Password8ParallelRandomPassword, 
#              Password8ParallelRandomPasswordNOWAIT
```

## 💻 Usage

### Running Individual Programs

#### Sequential Version
```bash
./cmake-build-debug/Password8Sequenziale
```
- Runs single-threaded baseline
- No parameters needed
- Outputs execution time and throughput

#### Parallel Version (Interactive)
```bash
./cmake-build-debug/Password8ParallelRandomPassword
```
- Prompts for number of threads
- Displays available cores
- Validates thread count

#### Parallel Version (Command Line)
```bash
./cmake-build-debug/Password8ParallelRandomPassword 8
```
- Directly specifies 8 threads
- Useful for scripting and benchmarking

#### Parallel NOWAIT Version
```bash
./cmake-build-debug/Password8ParallelRandomPasswordNOWAIT 8
```
- Optimized version without implicit barriers
- Same interface as standard parallel version

### Environment Variables

Enable OpenMP cancellation (required for parallel versions):
```bash
export OMP_CANCELLATION=true
```

Control thread affinity:
```bash
export OMP_PROC_BIND=true
export OMP_PLACES=cores
```

## 📊 Benchmarking

### Running the Benchmark Suite

The project includes a comprehensive benchmarking system:

```bash
./benchmark.sh
```

This script:
1. Runs sequential version to establish baseline
2. Tests parallel versions with 1, 2, 4, 8, 16, 20 threads
3. Tests NOWAIT version with additional 25 and 32 threads (overthreading analysis)
4. Collects execution time, speedup, efficiency metrics
5. Saves results to `benchmark_results/benchmark_TIMESTAMP.csv`

### Generating Visualizations

```bash
python3 plot_benchmark.py benchmark_results/benchmark_TIMESTAMP.csv
```

Generates individual plots:
- `*_execution_time.png`: Execution time vs threads
- `*_speedup.png`: Speedup vs threads (with ideal reference)
- `*_efficiency.png`: Parallel efficiency percentage
- `*_scalability.png`: Strong scaling analysis
- `*_throughput.png`: Passwords cracked per second
- `*_combined.png`: Summary dashboard with all metrics
- `*_summary_table.png`: Performance statistics table

### Custom Benchmark Configuration

Edit `benchmark.sh` to customize:
```bash
THREAD_COUNTS=(1 2 4 8 16 20)  # Thread counts to test
NUM_RUNS=3                      # Repetitions per test (for averaging)
```

## 📈 Results

### Performance Metrics (Intel Core i9-13900H, 14 cores, 20 threads)

#### Execution Time Results

| Threads | Sequential (s) | Parallel (s) | NOWAIT (s) |
|---------|---------------|--------------|------------|
| 1       | 571.04        | 590.55       | 591.77     |
| 2       | ---           | 283.27       | 301.39     |
| 4       | ---           | 161.25       | 158.35     |
| 8       | ---           | 97.44        | 101.75     |
| 16      | ---           | 64.41        | 67.00      |
| 20      | ---           | 54.28        | 55.57      |
| 25      | ---           | ---          | **47.43**  |
| 32      | ---           | ---          | 50.07      |

#### Speedup Results

| Threads | Parallel | NOWAIT       | Ideal   |
|---------|----------|--------------|---------|
| 1       | 0.97×    | 0.96×        | 1.00×   |
| 2       | 2.01×    | 1.89×        | 2.00×   |
| 4       | 3.54×    | 3.60×        | 4.00×   |
| 8       | 5.86×    | 5.61×        | 8.00×   |
| 16      | 8.86×    | 8.52×        | 16.00×  |
| 20      | 10.52×   | 10.27×       | 20.00×  |
| 25      | ---      | **12.04×**   | 25.00×  |
| 32      | ---      | 11.40×       | 32.00×  |

#### Efficiency Results

| Threads | Parallel Eff | NOWAIT Eff | Quality    |
|---------|--------------|------------|------------|
| 1       | 96.7%        | 96.5%      | Excellent  |
| 2       | 100.5%       | 94.5%      | Excellent  |
| 4       | 88.5%        | 90.0%      | Very Good  |
| 8       | 73.3%        | 70.1%      | Good       |
| 16      | 55.4%        | 53.3%      | Moderate   |
| 20      | 52.6%        | 51.4%      | Moderate   |
| 25      | ---          | **48.2%**  | Fair       |
| 32      | ---          | 35.6%      | Poor       |

#### Throughput Results

| Threads | Sequential   | Parallel      | NOWAIT        |
|---------|--------------|---------------|---------------|
| 1       | 737,972      | 374,606       | 374,518       |
| 2       | ---          | 746,267       | 745,892       |
| 4       | ---          | 1,390,429     | 1,388,791     |
| 8       | ---          | 2,173,329     | 2,182,808     |
| 16      | ---          | 3,351,151     | 3,350,576     |
| 20      | ---          | 3,895,931     | 3,841,099     |
| 25      | ---          | ---           | **4,423,183** |
| 32      | ---          | ---           | 4,512,225     |

*Throughput measured in passwords per second*

### Key Observations

1. **Near-linear speedup** up to 2 threads (~100% efficiency)
2. **Good scaling** up to 8 threads with 70-73% efficiency
3. **Peak performance at 25 threads**: Maximum speedup of 12.04× and throughput of 4.42M passwords/sec
4. **Overthreading penalty**: Performance degrades at 32 threads despite exceeding hardware thread capacity (20 logical threads)
5. **NOWAIT advantage**: 1-5% improvement over standard parallel version by eliminating implicit barriers
6. **Efficiency decline**: From 90-100% at 2-4 threads to 35.6% at 32 threads

### Overthreading Analysis

**Why 25 threads improve performance:**
- Workload balancing compensates for irregular early-termination behavior
- I/O and memory latency hiding keeps computational units busy
- Hyper-Threading headroom allows slight oversubscription

**Why 32 threads degrade performance:**
- Context switching overhead (32 threads on 20 hardware threads)
- Cache thrashing with more threads competing for L1/L2/L3 cache
- Memory bandwidth saturation
- Synchronization contention on atomic operations
- TLB pressure with more concurrent address spaces

**Optimal thread count**: 20-25 threads represent the sweet spot for this workload on this hardware, balancing parallelism with hardware constraints.

## 📁 Project Structure

```
Password-Decryption/
├── README.md                                    # This file
├── CMakeLists.txt                              # Build configuration
├── benchmark.sh                                # Automated benchmarking
├── plot_benchmark.py                           # Visualization generator
│
├── Password8Sequenziale.cpp                    # Sequential implementation
├── Password8ParallelRandomPassword.cpp         # Parallel with cancellation
├── Password8ParallelRandomPasswordNOWAIT.cpp   # Parallel NOWAIT version
│
├── parallel_password_report.tex                # Technical report (LaTeX)
├── beamerthemesintef.sty                       # Presentation theme
├── sintefcolor.sty                             # Color definitions
│
├── cmake-build-debug/                          # Build artifacts
│   ├── Password8Sequenziale                    # Sequential binary
│   ├── Password8ParallelRandomPassword         # Parallel binary
│   └── Password8ParallelRandomPasswordNOWAIT   # Parallel NOWAIT binary
│
├── benchmark_results/                          # Benchmark outputs
│   ├── benchmark_*.csv                         # Raw data
│   └── benchmark_*_[plot_type].png            # Visualizations
│
├── assets/                                     # Presentation assets
└── unifi_latex_overleaf/                      # Presentation template
```

## 🔧 Technical Details

### DES Encryption
- **Algorithm**: Data Encryption Standard (DES)
- **Function**: `crypt()` / `crypt_r()` (POSIX standard)
- **Salt**: Fixed 2-character salt "AB"
- **Output**: 13-character hash string

### OpenMP Directives Used
- `#pragma omp parallel`: Create parallel region
- `#pragma omp for collapse(3)`: Parallelize 3 nested loops
- `#pragma omp critical`: Protect shared data updates
- `#pragma omp cancellation point for`: Check for cancellation
- `#pragma omp cancel for`: Request loop cancellation
- `reduction(+:var)`: Parallel sum accumulation
- `schedule(static)`: Static work distribution

### Memory Safety
- **Thread-local storage**: Each thread has independent `crypt_data` structure
- **Atomic operations**: Lock-free synchronization for found flag
- **Critical sections**: Minimal locking for result storage

### Why Ideal Speedup Is Not Achieved

1. **Amdahl's Law**: Sequential portions limit parallelization
   - Random password generation (sequential)
   - Progress reporting (sequential)
   - Result verification (sequential)

2. **Early Termination & Load Imbalance**:
   - Password found at random iteration in search space
   - Some threads find password early in their chunks
   - Other threads waste work on their assigned portions
   - Static scheduling cannot rebalance work dynamically
   - Load imbalance increases with more threads

3. **Synchronization Overhead**:
   - Atomic flag checks at every iteration
   - Critical section contention when password found
   - Thread creation/destruction costs
   - Memory ordering operations

4. **Hardware Limitations**:
   - Memory bandwidth bottleneck (crypt_r() is memory-intensive)
   - Cache contention (thread-local crypt_data structures compete for cache)
   - Cache coherence traffic between cores
   - NUMA effects on multi-socket systems
   - TLB pressure with many concurrent threads

5. **Overthreading Effects** (beyond 20-25 threads):
   - Excessive context switching
   - Cache thrashing
   - Resource contention
   - Diminishing returns on parallelism

## 🎓 Educational Value

This project demonstrates:
- **Parallel algorithm design**: Converting sequential to parallel code with OpenMP
- **Thread synchronization**: Atomic operations, critical sections, memory ordering
- **Performance analysis**: Speedup, efficiency, scalability metrics and visualization
- **Optimization techniques**: Early termination, cache optimization, reentrant functions
- **Benchmarking methodology**: Systematic performance evaluation across thread counts
- **OpenMP programming**: Practical use of parallel constructs (collapse, reduction, cancellation, nowait)
- **Hardware constraints**: Understanding overthreading, Hyper-Threading, and resource contention
- **Trade-offs**: Balancing synchronization overhead vs. early termination benefits

## 🤝 Contributing

Contributions are welcome! Areas for improvement:
- GPU acceleration (CUDA implementation)
- Distributed computing (MPI version)
- Alternative hash algorithms (SHA, bcrypt)
- Rainbow table generation
- Dictionary attack implementation

## 📝 License

This project is for educational purposes only. Use responsibly and ethically.

## 👤 Author

**Lapo Chiostrini**  
University of Florence - Parallel Computing Course  
Email: lapo.chiostrini1@edu.unifi.it

## 🙏 Acknowledgments

- OpenMP community for parallel computing standards
- Python matplotlib/pandas for visualization tools
- POSIX crypt library for encryption functions

---

**Note**: This software is intended for educational purposes and ethical security research only. Do not use for unauthorized access to systems or data.

