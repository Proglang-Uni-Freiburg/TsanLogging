#include "tsan_logging.h"

using namespace __tsan;
using namespace __sanitizer;

fd_t logFileFd = kInvalidFd;
static Mutex mtx;

static LogEntry *logHead = nullptr;
static LogEntry *logTail = nullptr;

void __tsan::convertHexadecimalToString(unsigned long long valueToConvert, char *bufferString)
{
  const char *hex = "0123456789abcdef";
  for(int i = 15; i >=0; i--)
  {
    bufferString[i] = hex[valueToConvert & 0xF];
    valueToConvert >>= 4;
  }
  bufferString[16] = '\0';
}

void __tsan::convertIntegerToDecimalString(unsigned long long valueToConvert, char *bufferString)
{
  int i=0;
    
  if(valueToConvert == 0)
  {
    bufferString[i++] = '0';   
  }
  else{
    while (valueToConvert > 0)
    {
      bufferString[i++] = '0' + valueToConvert % 10;
      valueToConvert /= 10;
    }

    for (int j = 0; j < i/2; ++j)
    {
      char temp = bufferString[j];
      bufferString[j] = bufferString[i -j -1];
      bufferString[i-j-1] = temp;
    }
  }
  bufferString[i] = '\0';
}


void __tsan::tsanInterceptorsAndMemoryAccessOperationsLogging(const char* logMessage, void *addr, ThreadState *thr, uptr callerpc, Tid tid)
{
   Lock l(&mtx);
   LogEntry* newEntry = (LogEntry*)InternalAlloc(sizeof(LogEntry));

   newEntry->message = logMessage;
   newEntry->addr = addr;
   newEntry->thr = thr;
   newEntry->callerpc = callerpc;
   newEntry->tid = tid;
   newEntry->next = nullptr;

   if (logHead == nullptr)
   {
     logHead = newEntry;    
   }
   else
   {
     logTail->next = newEntry;
   }
   logTail = newEntry;
}

void __tsan::dumpLogToFile() {

  static bool logClear = false;
 
  if (!logClear)
  { 
    logFileFd = OpenFile("tsanLogFile.txt", WrOnly);
    logClear = true;
  }
  else
  {
    logFileFd = OpenFile("tsanLogFile.txt", Append);
  }

  if (logFileFd != kInvalidFd) {
    LogEntry* current = logHead;
    while (current) {
      if (current->thr && current->thr->tid >= 0) {
        char threadIdBuf[30];
        convertIntegerToDecimalString((unsigned long long)current->thr->tid, threadIdBuf);
        WriteToFile(logFileFd, "Thread ", 7);
        WriteToFile(logFileFd, threadIdBuf, internal_strlen(threadIdBuf));
        WriteToFile(logFileFd, ": ", 2);
      }
      WriteToFile(logFileFd, current->message, internal_strlen(current->message));
      if (current->tid > 0) {
        WriteToFile(logFileFd, " (Thread ", 9);
        char threadIdBuf1[30];
        convertIntegerToDecimalString((unsigned long long)current->tid, threadIdBuf1);
        WriteToFile(logFileFd, threadIdBuf1, internal_strlen(threadIdBuf1));
        WriteToFile(logFileFd, ")", 1);
      }
      if (current->addr != nullptr) {
        WriteToFile(logFileFd, "|", 1);
        char addrBuf[18] = "0x";
        convertHexadecimalToString((unsigned long long)current->addr, addrBuf + 2);
        WriteToFile(logFileFd, addrBuf, internal_strlen(addrBuf));
        WriteToFile(logFileFd, "|", 1);
      }
      if (current->callerpc != 0) {
        Symbolizer *symbolizer = Symbolizer::GetOrInit();
        if (symbolizer) {
          SymbolizedStack *frames = symbolizer->SymbolizePC(current->callerpc);
          if (frames) {
            const AddressInfo &info = frames->info;
            char lineBuf1[20];
            convertIntegerToDecimalString(info.line, lineBuf1);
            WriteToFile(logFileFd, " at line ", 9);
            WriteToFile(logFileFd, lineBuf1, internal_strlen(lineBuf1));
            WriteToFile(logFileFd, " in file ", 9);
            WriteToFile(logFileFd, info.file, internal_strlen(info.file));
          }
        }
      }
      WriteToFile(logFileFd, "\n", 1);
      LogEntry* temp = current;
      current = current->next;
      InternalFree(temp);
    }
    CloseFile(logFileFd);
  }
}

