# DETI Coins Miner

This project focuses on mining DETI coins using various optimization techniques, including **AVX**, **AVX2**, **AVX512**, **SIMD instructions**, **OpenMP**, and **CUDA**. It also supports a **Client-Server AVX OpenMP** mining approach.

The mining process is based on **MD5 hash computations** optimized for different hardware architectures.

---

## 📂 Project Structure

- **`deti_coins.c`** – Main source file.  
- **`*.h`** – Headers for CPU, AVX, OpenMP, and CUDA implementations.  
- **`*.cu`** – CUDA kernel files for GPU-accelerated mining.  
- **`Makefile`** – Build instructions for different mining approaches.  

---

## ⚙️ Requirements

### **Dependencies**
- **C Compiler** (`gcc` or `clang`)
- **OpenMP** (for parallel CPU mining)
- **AVX/AVX2/AVX512 support** (depending on CPU)
- **CUDA Toolkit** (for GPU mining)
- **Linux or macOS** (Apple Silicon supported in CPU-only mode)

### **Hardware Requirements**
- Intel/AMD CPU with AVX/AVX2/AVX512 support  
- (Optional) NVIDIA GPU with CUDA support (Compute Capability ≥ 7.5 recommended)

---

## 🔨 Compilation

Use the provided `Makefile` to compile the miner for different modes:

### **1. CPU Mining (Intel/AMD, No CUDA)**

```bash
make deti_coins_intel
```

This builds the binary **`deti_coins_intel`**, optimized for:
- **AVX**
- **AVX2**
- **AVX + OpenMP**
- **AVX2 + OpenMP**
- **AVX512** (if supported by the CPU)

---

### **2. CPU Mining (Apple Silicon, No CUDA)**

```bash
make deti_coins_apple
```

This builds the binary **`deti_coins_apple`**, optimized for ARM-based CPUs (no AVX/OpenMP support).

---

### **3. CPU + CUDA Mining**

```bash
make deti_coins_intel_cuda
```

This builds the binary **`deti_coins_intel_cuda`**, which uses:
- **CPU (AVX/AVX2/OpenMP)** for initial work distribution
- **CUDA kernels** for GPU-accelerated mining

The `Makefile` will also compile:
- **`md5_cuda_kernel.cubin`** – MD5 hash kernel  
- **`deti_coins_cuda_kernel_search.cubin`** – Mining kernel for DETI coins

If you need to manually build these kernels:

```bash
make md5_cuda_kernel.cubin
make deti_coins_cuda_kernel_search.cubin
```

---

# 🚀 Running the Miner

After compilation, run the miner according to the desired mode.  
Replace `[seconds]`, `[n_random_words]`, `[n_threads]`, `[port]`, or `[special_text]` as needed.

---

## ✅ **1. Run MD5 Tests**

```bash
./deti_coins_intel -t
```

Runs internal MD5 correctness tests.

---

## 🔥 **2. Search for DETI Coins**

The general syntax is:

```bash
./deti_coins_intel -s[mode] [seconds] [n_random_words] [n_threads|port|special_text]
```

- **`seconds`** → duration of the search (min: 120, max: 7200)  
- **`n_random_words`** → number of random 4-byte words (default: 1, max: 9)  
- **`n_threads`** → number of threads for OpenMP modes (default: 8)  
- **`port`** → port for server or client modes  
- **`special_text`** → text inserted into the DETI coin  

### **Modes Table**

| Mode | Command Example | Description |
|------|----------------|-------------|
| **0** | `./deti_coins_intel -s0 1800` | CPU-only MD5 search |
| **1** | `./deti_coins_intel -s1 1800 4` | AVX (single-threaded) search |
| **2** | `./deti_coins_intel -s2 1800 4 8` | AVX + OpenMP (multi-threaded) search |
| **3** | `./deti_coins_intel -s3 1800 4` | AVX2 (single-threaded) search |
| **4** | `./deti_coins_intel -s4 1800 4 8` | AVX2 + OpenMP (multi-threaded) search |
| **5** | `./deti_coins_intel -s5 1800 4` | AVX512 (single-threaded) search *(if supported)* |
| **6** | `./deti_coins_intel -s6 5000` | Starts a server on port 5000 |
| **7** | `./deti_coins_intel -s7 1800 5000` | Client mode: connects to server on port 5000 |
| **8** | `./deti_coins_intel -s8 1800 4` | NEON (ARM-based CPUs, single-threaded) |
| **9** | `./deti_coins_intel -s9 1800 4` | CUDA GPU search *(requires CUDA build)* |
| **a** | `./deti_coins_intel -sa 1800 "SPECIAL_TEXT"` | Special search inserting `SPECIAL_TEXT` |

---

## ⚙️ **Additional Notes**

- **Minimum & Maximum Duration**:  
  - Less than 120 seconds → forced to 120 seconds  
  - More than 7200 seconds → forced to 7200 seconds

- **Defaults**:  
  - `n_random_words` = 1  
  - `n_threads` = 8  
  - `special_text` = "DEFAULT"

- **Building CUDA Version**:  
  If CUDA mode is enabled, build required binaries first:  
  ```bash
  make clean
  make md5_cuda_kernel.cubin
  make deti_coins_cuda_kernel_search.cubin
  make deti_coins_intel_cuda
  ```

---

## 🧪 **Example Full Command Set**

```bash
# Basic CPU search for 30 minutes
./deti_coins_intel -s0 1800

# AVX2 multi-threaded with 6 random words and 12 threads
./deti_coins_intel -s4 3600 6 12

# Server running on port 7000
./deti_coins_intel -s6 7000

# Client connecting to port 7000 for 20 minutes
./deti_coins_intel -s7 1200 7000

# Special search with custom text
./deti_coins_intel -sa 600 "HELLO_DETI"
```

---
**Author:** *Tomás Oliveira e Silva*  
*Arquiteturas de Alto Desempenho 2024/2025*  


---

## 🏆 Implemented Mining Techniques

- **✅ AVX (Single & Multi-threaded)**  
- **✅ AVX2 (Single & Multi-threaded)**  
- **✅ AVX512 (Single-threaded)**  
- **✅ SIMD Instructions (via AVX/AVX2/AVX512)**  
- **✅ OpenMP (CPU parallelism)**  
- **✅ CUDA (GPU mining)**  
- **✅ Client-Server AVX OpenMP**  

---

## 📄 License

This project was developed for academic purposes as part of the **DETI Practical Assignment A1**.

---
