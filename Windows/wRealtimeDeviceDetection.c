#include "wRealtimeDeviceDetection.h"

int running = 1;
char last_label[128] = "";

void init_csv_if_needed(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fp = fopen(filename, "w");
        if (fp) {
            fprintf(fp, "USB명,USB 내용요약정리,현재 일자,수정일자,오전/오후 수정 시간,USB 용량,시리얼,제조사,모델명\n");
            fclose(fp);
        }
    } else {
        fclose(fp);
    }
}

void log_event(const char *message) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (fp) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
        fprintf(fp, "[%s] %s\n", timestamp, message);
        fclose(fp);
    }
}

void write_csv(const char *label, const char *summary, const char *size,
               const char *serial, const char *vendor, const char *model,
               const char *action) {
    char date[20], time_str[20], am_pm[3], filename[64];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(date, sizeof(date), "%Y-%m-%d", t);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", t);
    strcpy(am_pm, (t->tm_hour < 12) ? "A" : "P");

    snprintf(filename, sizeof(filename), "USB_%s.csv", date);
    init_csv_if_needed(filename);

    FILE *fp = fopen(filename, "a");
    if (fp == NULL) {
        perror("CSV 파일 열기 실패");
        return;
    }

    fprintf(fp, "%s,%s,%s,%s,%s %s,%s,%s,%s,%s\n",
            label, summary, date, date, am_pm, time_str,
            size ? size : "Unknown",
            serial ? serial : "Unknown",
            vendor ? vendor : "Unknown",
            model ? model : "Unknown");

    fclose(fp);

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "%s 이벤트 기록됨: %s (%s)", action, label, size ? size : "Unknown");
    log_event(log_msg);
    printf("📁 %s: %s (%s)\n", action, label, size ? size : "Unknown");
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DEVICECHANGE) {
        if (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE) {
            PDEV_BROADCAST_HDR hdr = (PDEV_BROADCAST_HDR)lParam;
            if (hdr && hdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                const char *action = (wParam == DBT_DEVICEARRIVAL) ? "추가" : "제거";
                write_csv("USB장치", "USB 내용요약 정리", "Unknown", "Unknown", "Unknown", "Unknown", action);
            }
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void monitor_usb() {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = "USBMonitorClass";

    RegisterClass(&wc);
    HWND hwnd = CreateWindow("USBMonitorClass", "USB Monitor", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

    printf("🖥️ USB 감지 대기 중...\n");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
