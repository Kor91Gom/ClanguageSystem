#ifndef WREALTIMEDEVICEDETECTION_H
#define WREALTIMEDEVICEDETECTION_H

// ✅ Windows API 헤더
#include <windows.h>
#include <dbt.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ✅ 전역 상수
#define LOG_FILE "USBLog.txt"

// ✅ 전역 변수
extern int running;
extern char last_label[128];

// ✅ 함수 선언
void init_csv_if_needed(const char *filename);
void log_event(const char *message);
void write_csv(const char *label, const char *summary, const char *size,
               const char *serial, const char *vendor, const char *model,
               const char *action);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void monitor_usb();

#endif
