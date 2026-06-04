#include "core/application.h"
#include "rtc.h"
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

static bool app_time_is_valid(const t_time *current_time) {
    if (current_time == NULL) return false;

    return current_time->month >= 1 && current_time->month <= 12 &&
           current_time->day >= 1 && current_time->day <= 31 &&
           current_time->hours <= 23 &&
           current_time->minutes <= 59 &&
           current_time->seconds <= 59;
}

static uint8_t days_in_month(uint8_t month, uint8_t year) {
    static const uint8_t days[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (month < 1 || month > 12) return 31;
    if (month == 2 && year % 4 == 0) return 29;

    return days[month - 1];
}

int app_update_real_time(t_ctx *ctx) {
    if (ctx == NULL) return 1;

    hw_rtc_t hw_time;

    if (hw_rtc_get_time(&hw_time) != 0) {
        time_t now = time(NULL);
        struct tm *tm_now = localtime(&now);

        if (tm_now == NULL) return 1;

        ctx->real_time.year = (uint8_t)(tm_now->tm_year % 100);
        ctx->real_time.month = (uint8_t)(tm_now->tm_mon + 1);
        ctx->real_time.day = (uint8_t)tm_now->tm_mday;
        ctx->real_time.hours = (uint8_t)tm_now->tm_hour;
        ctx->real_time.minutes = (uint8_t)tm_now->tm_min;
        ctx->real_time.seconds = (uint8_t)tm_now->tm_sec;

        return app_time_is_valid(&ctx->real_time) ? 0 : 1;
    }

    ctx->real_time.year = hw_time.year;
    ctx->real_time.month = hw_time.month;
    ctx->real_time.day = hw_time.day;
    ctx->real_time.hours = hw_time.hours;
    ctx->real_time.minutes = hw_time.minutes;
    ctx->real_time.seconds = hw_time.seconds;

    return app_time_is_valid(&ctx->real_time) ? 0 : 1;
}

void app_tick_real_time(t_ctx *ctx) {
    if (ctx == NULL || !app_time_is_valid(&ctx->real_time)) {
        app_update_real_time(ctx);
        return;
    }

    ctx->real_time.seconds++;

    if (ctx->real_time.seconds < 60) return;

    ctx->real_time.seconds = 0;
    ctx->real_time.minutes++;

    if (ctx->real_time.minutes < 60) return;

    ctx->real_time.minutes = 0;
    ctx->real_time.hours++;

    if (ctx->real_time.hours < 24) return;

    ctx->real_time.hours = 0;
    ctx->real_time.day++;

    if (ctx->real_time.day <= days_in_month(ctx->real_time.month, ctx->real_time.year)) return;

    ctx->real_time.day = 1;
    ctx->real_time.month++;

    if (ctx->real_time.month <= 12) return;

    ctx->real_time.month = 1;
    ctx->real_time.year++;
}
