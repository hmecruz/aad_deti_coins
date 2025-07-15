# DETI Coins Miner

This project is the first practical assignment (**A1**) and focuses on mining DETI coins using various optimization techniques, including **AVX**, **AVX2**, **AVX512**, **SIMD instructions**, **OpenMP**, and **CUDA**. It also supports a **Client-Server AVX OpenMP** mining approach.

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

## 🚀 Running the Miner

After compilation, run the miner according to the desired mode:

### **1. AVX / AVX2 / AVX512 (Single-threaded)**

```bash
./deti_coins_intel --mode avx
./deti_coins_intel --mode avx2
./deti_coins_intel --mode avx512
```

(Replace `--mode` with the desired instruction set; if no flag is provided, the default is AVX.)

---

### **2. AVX / AVX2 + OpenMP (Multi-threaded)**

```bash
./deti_coins_intel --mode avx_openmp --threads 8
./deti_coins_intel --mode avx2_openmp --threads 8
```

Increase `--threads` according to your CPU cores.

---

### **3. Client-Server AVX OpenMP**

The project supports a client-server model where mining tasks are distributed among multiple machines.

#### Start the Server:
```bash
./deti_coins_intel --server --mode avx_openmp --threads 8
```

#### Start a Client:
```bash
./deti_coins_intel --client --server-addr <SERVER_IP> --threads 4
```

---

### **4. CUDA Mining**

Run the CUDA-accelerated miner:

```bash
./deti_coins_intel_cuda --mode cuda --gpu 0
```

You can specify the GPU index (`--gpu`) if multiple GPUs are available.

---

## 🧹 Cleaning the Build

To remove all generated binaries:

```bash
make clean
```

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
