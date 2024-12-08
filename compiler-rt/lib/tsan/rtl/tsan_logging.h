/**
 * @file tsan_logging.h
 * @brief Header file for ThreadSanitizer logging functions.
 *
 * This header file declares logging functions for ThreadSanitizer (TSan).
 * It should not include any other headers. All functions declared in this
 * header are extern "C" and start with __tsan_.
 *
 * @note This header should NOT include any other headers.
 */
#ifndef TSAN_LOGGING_H
#define TSAN_LOGGING_H

#include "sanitizer_common/sanitizer_ptrauth.h"
#include "sanitizer_common/sanitizer_stacktrace_printer.h"
#include "sanitizer_common/sanitizer_symbolizer.h"

#include "sanitizer_common/sanitizer_stackdepot.h"

#include "tsan_rtl.h"
#include "tsan_interface.h"
#include "sanitizer_common/sanitizer_file.h"

namespace __tsan {


void convertHexadecimalToString(unsigned long long valueToConvert, char *bufferString);

void convertIntegerToDecimalString(unsigned long long valueToConvert, char *bufferString);

void tsanInterceptorsAndMemoryAccessOperationsLogging(const char* logMessage, void *addr = nullptr, ThreadState *thr = nullptr, uptr callerpc = 0, Tid tid = (int)0); 

}  // namespace __tsan

#endif  // TSAN_LOGGING_H
