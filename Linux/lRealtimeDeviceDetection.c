#include "lRealtimeDeviceDetection.h"

int running = 1;
char last_label[128] = "";

void handle_sigint(int sig) {
    printf("\n🛑 프로그램 종료 중...\n");
    running = 0;
}

void init_csv_if_needed(const char *filename) {
    struct stat st;
    if (stat(filename, &st) != 0) {
        FILE *fp = fopen(filename, "w");
        if (fp) {
            fprintf(fp, "USB명,USB 내용요약정리,현재 일자,수정일자,오전/오후 수정 시간,USB 용량,시리얼,제조사,모델명\n");
            fclose(fp);
        }
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

void monitor_usb() {
    struct udev *udev;
    struct udev_monitor *mon;
    struct udev_device *dev;

    signal(SIGINT, handle_sigint);

    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "udev 초기화 실패\n");
        return;
    }

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", "partition");
    udev_monitor_enable_receiving(mon);

    int fd = udev_monitor_get_fd(mon);
    printf("🖥️ USB 감지 대기 중...\n");

    while (running) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        if (select(fd + 1, &fds, NULL, NULL, NULL) > 0) {
            dev = udev_monitor_receive_device(mon);
            if (dev) {
                const char *action = udev_device_get_action(dev);
                const char *label = udev_device_get_property_value(dev, "ID_FS_LABEL");
                const char *size = udev_device_get_property_value(dev, "ID_FS_SIZE");
                const char *serial = udev_device_get_property_value(dev, "ID_SERIAL");
                const char *vendor = udev_device_get_property_value(dev, "ID_VENDOR");
                const char *model = udev_device_get_property_value(dev, "ID_MODEL");

                if (action && label) {
                    if (strcmp(action, "add") == 0) {
                        if (strcmp(label, last_label) != 0) {
                            strncpy(last_label, label, sizeof(last_label));
                            write_csv(label, "USB 내용요약 정리", size, serial, vendor, model, "추가");
                        }
                    } else if (strcmp(action, "remove") == 0) {
                        write_csv(label, "USB 제거됨", size, serial, vendor, model, "제거");
                        last_label[0] = '\0';
                    }
                }

                udev_device_unref(dev);
            }
        }
    }

    udev_unref(udev);
    printf("✅ 안전하게 종료되었습니다.\n");
}
