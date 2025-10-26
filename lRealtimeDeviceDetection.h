#ifndef LREALTIMEDEVICEDETECTION_H
#define LREALTIMEDEVICEDETECTION_H

// ⛓️ 시스템 헤더
#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <signal.h>

// 📁 파일 경로 상수
#define LOG_FILE "USBLog.txt"

// 🧠 전역 변수
extern int running;                  // 프로그램 실행 상태
extern char last_label[128];        // 마지막 감지된 USB 라벨

// 🧩 함수 선언

// CSV 파일 초기화 (헤더 생성)
void init_csv_if_needed(const char *filename);

// 로그 파일에 이벤트 기록
void log_event(const char *message);

// CSV 파일에 USB 정보 기록
void write_csv(const char *label, const char *summary, const char *size,
               const char *serial, const char *vendor, const char *model,
               const char *action);

// USB 이벤트 감지 및 처리 루프
void monitor_usb();

// SIGINT (Ctrl+C) 처리 함수
void handle_sigint(int sig);

#endif
