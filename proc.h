#ifndef PROC_INFO_H_
#define PROC_INFO_H_

#include <sys/resource.h>
#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"

typedef enum {
  EXIT_PROC_INFO_READ_FAIL = 200,
  EXIT_PROC_MEM_CORRUPTED,
  EXIT_PROC_MEM_EXCEEDED,
} proc_info_cd;

struct proc_info_t {
  pid_t  pid;
  size_t virt;  // total virtual memory, swap, shared, etc
  size_t ram;   // mem in ram, including shared libs
  size_t heap;  // heap allocs, malloc, calloc, etc
  size_t stack; // total stack memory
  size_t text;  // code/text segment information
};

typedef enum {
  PROC_INFO_FLAG_PID   = 1 << 0, // 0x00000001
  PROC_INFO_FLAG_VIRT  = 1 << 1, // 0x00000010
  PROC_INFO_FLAG_RAM   = 1 << 2,
  PROC_INFO_FLAG_HEAP  = 1 << 3,
  PROC_INFO_FLAG_STACK = 1 << 4,
  PROC_INFO_FLAG_TEXT  = 1 << 5,
  PROC_INFO_FLAG_FPS   = 1 << 6,
  PROC_INFO_FLAG_ALL   = 0xFF,
} PROC_INFO_FLAGS;

void PROC_INFO_BOOTSTRAP(void);
void PROC_INFO_CAP_MEM(size_t);
void PROC_INFO_DRAW(PROC_INFO_FLAGS);

#ifdef LOAD_PROC_INFO
#define DEBUG_FONT_SZ 16

static struct rlimit MEM_CAP;
static FILE *PROC_FILE;

struct proc_info_t PROC_INFO;

static void open_proc_file(void) {
  char path[256];
  PROC_INFO.pid = getpid();
  snprintf(path, sizeof(path), "/proc/%d/status", PROC_INFO.pid);
  PROC_FILE = fopen(path, "r");
  if (PROC_FILE == NULL) exit(EXIT_PROC_INFO_READ_FAIL);
}

static void populate_proc_info(void) {
  #define MAX_LINE_LEN 256
  char line[MAX_LINE_LEN];
  // TODO: inotify is a better idea
  open_proc_file();
  while (fgets(line, sizeof(line), PROC_FILE)) {
    if (strncmp(line, "VmSize:", 7) == 0)
      sscanf(line + 7, "%lu kB", &PROC_INFO.virt);
    else if (strncmp(line, "VmRSS:", 6) == 0)
      sscanf(line + 6, "%lu kB", &PROC_INFO.ram);
    else if (strncmp(line, "VmData:", 7) == 0)
      sscanf(line + 7, "%lu kB", &PROC_INFO.heap);
    else if (strncmp(line, "VmStk:", 6) == 0)
      sscanf(line + 6, "%lu kB", &PROC_INFO.stack);
    else if (strncmp(line, "VmExe:", 6) == 0)
      sscanf(line + 6, "%lu kB", &PROC_INFO.text);
  }
  fclose(PROC_FILE);
  PROC_FILE = NULL;
}

static void exit_proc_info(int code, void *args) {
  (void) args;
  if (code >= EXIT_PROC_INFO_READ_FAIL && code < 300) {
    switch (code) {
    case EXIT_PROC_INFO_READ_FAIL:
      fprintf(stderr, "[ERROR]: could not read from proc file.\n");
      break;
    case EXIT_PROC_MEM_CORRUPTED:
      fprintf(stderr, "[ERROR]: rlimit syscall lead to corrupt mem.\n");
      break;
    case EXIT_PROC_MEM_EXCEEDED:
      fprintf(stderr, "[ERROR]: memory limit exceeded.\n");
      break;
    default:
      fprintf(stderr, "[ERROR]: could not read from proc file.\n");
      break;
    }
  }
  if (PROC_FILE != NULL) fclose(PROC_FILE);
  fprintf(stdout, "[INFO]: proc.h resources deallocated ...\n");
}

void PROC_INFO_BOOTSTRAP(void) {
  on_exit(exit_proc_info, NULL);
}

void PROC_INFO_CAP_MEM(size_t num_bytes) {
  MEM_CAP.rlim_cur = num_bytes;
  MEM_CAP.rlim_max = num_bytes;
  if (setrlimit(RLIMIT_AS, &MEM_CAP) != 0) exit(EXIT_PROC_MEM_CORRUPTED);
}

static char *proc_info_text_from_flags(PROC_INFO_FLAGS fs)
{
  #define MAX_TXT_LEN 512
  char *buf = (char *) malloc(MAX_TXT_LEN * sizeof(char));
  size_t buf_offset = 0;
  if (fs & PROC_INFO_FLAG_PID)
    buf_offset += snprintf(buf + buf_offset, MAX_TXT_LEN - buf_offset,
                           "(pid: %d) ", PROC_INFO.pid);   
  if (fs & PROC_INFO_FLAG_VIRT)
    buf_offset += snprintf(buf + buf_offset, MAX_TXT_LEN - buf_offset,
                           "(virt: %zu) ",  PROC_INFO.virt);
  if (fs & PROC_INFO_FLAG_RAM)
    buf_offset += snprintf(buf + buf_offset, MAX_TXT_LEN - buf_offset,
                           "(ram: %zu) ", PROC_INFO.ram);   
  if (fs & PROC_INFO_FLAG_HEAP)
    buf_offset += snprintf(buf + buf_offset, MAX_TXT_LEN - buf_offset,
                           "(heap: %zu) ", PROC_INFO.heap);
  if (fs & PROC_INFO_FLAG_STACK)
    buf_offset +=  snprintf(buf + buf_offset, MAX_TXT_LEN - buf_offset,
                            "(stack: %zu) ", PROC_INFO.stack);   
  if (fs & PROC_INFO_FLAG_TEXT)
    buf_offset += snprintf(buf + buf_offset, MAX_TXT_LEN - buf_offset,
                           "(text: %zu) ", PROC_INFO.text);
  if (fs & PROC_INFO_FLAG_FPS)
    buf_offset += snprintf(buf + buf_offset, MAX_TXT_LEN - buf_offset,
                           "(fps: %d) ", GetFPS());
  return buf;
}

static void proc_info_draw_text(PROC_INFO_FLAGS fs, Color clr) {
  char *txt = proc_info_text_from_flags(fs);
  int txt_width = MeasureText(txt, DEBUG_FONT_SZ) + 4;
  DrawText(txt, SCREEN_WIDTH - txt_width, 1, DEBUG_FONT_SZ, clr);
  free(txt);
}

void PROC_INFO_DRAW(PROC_INFO_FLAGS fs) {
  populate_proc_info();

  size_t mem_consumed = PROC_INFO.virt;
  float mem_usage_pct = ((float) mem_consumed /
                         (float) MEM_CAP.rlim_max) * 1000.0f;

  Color clr = {
    .r = (unsigned char)(255.0f * mem_usage_pct),
    .g = (unsigned char)(255.0f * (1.0f - mem_usage_pct)),
    .b = 0,
    .a = 255,
  };

  DrawRectangle(0, 0, SCREEN_WIDTH, DEBUG_FONT_SZ, BLACK);
  proc_info_draw_text(fs, clr);
}

#endif // LOAD_PROC_INFO
#endif // PROC_INFO_H_
