#pragma once

#include <stdbool.h>
#include <float.h>
#include <stdio.h>

typedef struct ProcessTimer
{
    const char *name;
    double start_time;
    double last_ms;
    double total_ms;
    double max_ms;
    double min_ms;
    unsigned long sample_count;
    bool running;
} ProcessTimer;

typedef struct ProcessTimerLogConfig {
    FILE *average_log_file;
    FILE *warning_log_file;

    double warning_threshold_ms;
    unsigned long report_interval_samples;

    bool log_average_reports;
    bool log_spikes;
} ProcessTimerLogConfig;

void process_timer_init(ProcessTimer *timer, const char *name);
void process_timer_begin(ProcessTimer *timer, double current_time);
void process_timer_end(ProcessTimer *timer, double current_time);

double process_timer_last_ms(const ProcessTimer *timer);
double process_timer_average_ms(const ProcessTimer *timer);
double process_timer_min_ms(const ProcessTimer *timer);
double process_timer_max_ms(const ProcessTimer *timer);
void process_timer_reset(ProcessTimer *timer);

bool process_timer_log_config_open(
    ProcessTimerLogConfig *config,
    const char *average_log_path,
    const char *warning_log_path,
    double warning_threshold_ms,
    unsigned long report_interval_samples,
    bool log_average_reports,
    bool log_spikes
);
void process_timer_log_config_close(ProcessTimerLogConfig *config);
void process_timer_log_report(const ProcessTimer *timer, const ProcessTimerLogConfig *config);
