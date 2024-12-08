# Building LLVM with ThreadSanitizer (TSan) Support

This guide outlines the steps to build the LLVM project with ThreadSanitizer (TSan) enabled and use it to compile and detect threading issues in your code.

---

## System Requirements

- **RAM:** 32 GB  
- **CPU:** 8 cores  (more the cores, faster the compilation)
- **Architecture:** x86_64  
- **OS:** Ubuntu 22.04  or higher
- **CMake Version:** 3.22.1 or higher  
- **Clang Version:** 14.0.0  or higher

---

## Steps to Build LLVM and Enable TSan

### 1. Clone LLVM Project
Ensure the LLVM source code is available. If not already cloned:
```bash
git clone https://github.com/NikamSD/TSAN_Logging.git
```

### 2. Configure Build Using CMake

To configure the LLVM project build with ThreadSanitizer (TSan) support, execute the following command:

```bash
CC=clang CXX=clang++ cmake -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DCOMPILER_RT_BUILD_SANITIZERS=ON \
    -DCOMPILER_RT_BUILD_TSAN=ON \
    -DCOMPILER_RT_DEBUG=ON \
    ../llvm-project/llvm
```
#### Explanation of Flags:  

* `CC=clang` and `CXX=clang++`: Specifies the Clang compiler for building LLVM.  
* `-G "Unix Makefiles"`: Generates Unix Makefiles for the build system.  
* `-DCMAKE_BUILD_TYPE=Debug`: Sets the build type to Debug for detailed debugging information.  
* `-DLLVM_ENABLE_PROJECTS="clang;compiler-rt"`: Includes the Clang and compiler-rt sub-projects in the build.  
* `-DLLVM_TARGETS_TO_BUILD="X86"`: Specifies the target architecture for the build (x86 in this case).  
* `-DCOMPILER_RT_BUILD_SANITIZERS=ON`: Enables the build of sanitizers in the compiler-rt project.  
* `-DCOMPILER_RT_BUILD_TSAN=ON`: Specifically enables ThreadSanitizer (TSan).  
* `-DCOMPILER_RT_DEBUG=ON`: Enables debug symbols for the runtime libraries.

### 3. Build LLVM Project
Build the clang and compiler-rt sub-projects:

```bash
make -j8 clang compiler-rt
```

### 4. Install TSan Runtime Libraries  

Copy the necessary runtime libraries to the system directory for ThreadSanitizer:  

```bash  
sudo cp <ProjectDir>/build/lib/clang/<available version>/lib/x86_64-unknown-linux-gnu/libclang_rt.tsan-x86_64.a /usr/lib/clang/<available version>/lib/linux/  
sudo cp <ProjectDir>/build/lib/clang/<available version>/lib/x86_64-unknown-linux-gnu/libclang_rt.tsan_cxx-x86_64.a /usr/lib/clang/<available version>/lib/linux/
```
* `libclang_rt.tsan-x86_64.a`: Provides runtime support for ThreadSanitizer in C programs.
* `libclang_rt.tsan_cxx-x86_64.a`: Provides runtime support for ThreadSanitizer in C++ programs.


### 5. Compile and Link with TSan Enabled #### (Using ThreadSanitizer to Detect Threading Issues)
Compile your C++ file (e.g., example.cpp) with ThreadSanitizer enabled:

```bash
clang++ -fsanitize=thread -fPIE -pie -g -O1 -o example example.cpp
```

* `-fsanitize=thread`: Enables ThreadSanitizer.
* `-fPIE`: Generates position-independent code (PIC) for executables.
* `-pie`: Creates a position-independent executable.
* `-g`: Generates debug information for better diagnostics.
* `-O1`: Sets optimization level to 1.

### 6. Run the Executable
Execute the binary to identify threading issues:

```bash
./example
```
ThreadSanitizer will analyze runtime behavior and report data races, synchronization issues, and other threading bugs.

---

# Tsan Logging
Tsan Logging outlines the implementation of logging functionality in ThreadSanitizer (TSan). This feature is designed to capture and log events where instrumentation hooks are defined. The logs provide insights into thread interactions, memory addresses, and file locations, assisting in debugging and analysis.

## Overview
The TSan logging mechanism captures details about thread events and writes them to a log file (instrumentationHookLog.txt). It provides the following features:

* Logs thread IDs.
* Logs specific memory addresses and caller program counters (PC).
* Retrieves source file names and line numbers using symbolization.
* Handles thread-safe file operations.

## Key Components
## 1. Logging Functions
All logging functions are encapsulated in the __tsan namespace, ensuring that they are scoped to TSan's internal implementation.

### a. tsanInterceptorsAndMemoryAccessOperationsLogging
* Purpose: Logs a message with details about thread state, memory address, and caller information.
* Parameters:
    * `logMessage`: Message to log.
    * `addr`: Memory address related to the event (optional).
    * `thr`: Thread state pointer (optional).
    * `callerpc`: Caller program counter (optional).
    * `tid`: Thread ID (optional).
* Flow:
  * Opens the log file in append mode after initial creation.
  * Writes the thread ID (thr->tid) if available.
  * Logs the message and memory address (in hexadecimal format) if provided.
  * Uses a symbolizer to log file names and line numbers based on callerpc.
### b. convertHexadecimalToString
* Purpose: Converts an unsigned integer to its hexadecimal string representation.
* Usage: Converts memory addresses for logging.
### c. convertIntegerToDecimalString
* Purpose: Converts an unsigned integer to its decimal string representation.
* Usage: Converts thread IDs and other numerical data for logging.

## 2. Thread Safety
A Mutex object `(mtx)` ensures that file operations are thread-safe, preventing race conditions during concurrent logging.

## 3. Symbolization
The implementation integrates with the Symbolizer to:

* Retrieve the file name and line number associated with a given program counter (callerpc).
* Provide detailed contextual information for debugging.

## 4. Log file handling

The instrumentationHookLogMessage function in TSan Logging is responsible for writing detailed log entries to a file (instrumentationHookLog.txt). The following details explain how this process is handled based on the uploaded files and its integration within the runtime instrumentation.

### a. Initialization of Log File
The log file is initially opened in write-only mode (WrOnly) during the first invocation. A static variable, logClear, ensures the file is cleared only once at startup to avoid overwriting during runtime.
Subsequent log entries append to the file using the append mode (Append).

```bash

if (!logClear) {
    tsanLogFlag = OpenFile("tsanLogFile.txt", WrOnly);
    logClear = true;
} else {
    tsanLogFlag = OpenFile("tsanLogFile.txt", Append);
}

```
### b. Thread-Safe Operations
A Mutex object `(mtx)` ensures thread safety while multiple threads write to the log simultaneously. The lock `(Lock l(&mtx);)` guarantees atomicity during file access, preventing race conditions.

### c. Content Formatting
The log entries are constructed dynamically based on the information available in each call:

* `Thread ID`: If the ThreadState (thr) contains a valid thread ID (thr->tid), it is converted to a decimal string using intToDecStr and logged as part of the message.
* `Message Content`: The provided logMessage parameter is written to the file.
#### Additional Context:
* `Thread Associations`: If a tid is passed, it logs the association (e.g., "(Thread X)").
* `Address Information`: Memory addresses are logged in a hexadecimal format using hexadecimalString.
* `Caller Symbolization`: If a valid program counter (callerpc) is provided, the Symbolizer retrieves the source file and line number, which are included in the log.
#### Note: The file name and line number are recorded only during memory access for reading or writing, as these operations occur on the Stack.

### d. File Operations
The log file is closed after each write operation using CloseFile to ensure file system consistency and prevent resource leaks.

### e. Integration with Interceptors and Memory Access Operations
The function instrumentationHookLogMessage is used in multiple interceptors to log significant runtime events:

#### Thread Creation and Joining:
Logs when a thread is created or joined, capturing the thread ID and associated resources.
Example: pthread_create and pthread_join use this function to log their events.
```bash
__tsan::tsanInterceptorsAndMemoryAccessOperationsLogging("|Thread Create", (void*)th, thr, 0, p.tid);
```
#### Mutex Lock/Unlock:
Logs mutex acquisition (pthread_mutex_lock) and release (pthread_mutex_unlock) operations.
```bash
__tsan::tsanInterceptorsAndMemoryAccessOperationsLogging("|ACQ ", m, thr, pc);
__tsan::tsanInterceptorsAndMemoryAccessOperationsLogging("|REL ", m, thr, PC);
```

#### Memory Access Operation (Read/Write)
`tsanInterceptorsAndMemoryAccessOperationsLogging` is used within tsan_read and tsan_write instrumentation hooks to log information during memory read and write operations. This log helps in identifying threads that access shared memory simultaneously.

#### Note: Integration with interceptor is in `tsan_posix_interceptor.cpp` and memory access operation in `tsan_interface.inc`

### f. Log entries example:
#### Thread Creation:
```bash
Thread 3|Thread Create(Thread 4)
```
#### Mutex Lock:
```bash
Thread 2|ACQ |0x7ffd123abc| at line 34 in file mutex_test.cpp
```
#### Join Operation:
```bash
Thread 2|Join|0x7ffd123abc(Thread 3)
```
#### Memory Access Operations:
```bash
// Log the read operation
    __tsan::tsanInterceptorsAndMemoryAccessOperationsLogging("|READ ", addr, thr, callerpc, thr->tid);

// Log the write operation.
    __tsan::tsanInterceptorsAndMemoryAccessOperationsLogging("|WRITE", addr, thr, callerpc, thr->tid);
```

