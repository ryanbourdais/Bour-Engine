#include "profiler.h"
#include <stddef.h>

static double seconds_to_ms(double seconds)
{
    return seconds * 1000.0;
}

void process_timer_init(ProcessTimer *timer, const char *name)
{
    timer->name = name;
    timer->start_time = 0.0;
    timer->last_ms = 0.0;
    timer->total_ms = 0.0;
    timer->min_ms = DBL_MAX;
    timer->max_ms = 0.0;
    timer->sample_count = 0;
    timer->running = false;
}

void process_timer_begin(ProcessTimer *timer, double current_time)
{
    timer->start_time = current_time;
    timer->running = true;
}

void process_timer_end(ProcessTimer *timer, double current_time)
{
    if (!timer->running)
    {
        return;
    }

    timer->last_ms = seconds_to_ms(current_time - timer->start_time);
    timer->total_ms += timer->last_ms;

    if (timer->last_ms < timer->min_ms)
    {
        timer->min_ms = timer->last_ms;
    }

    if (timer->last_ms > timer->max_ms)
    {
        timer->max_ms = timer->last_ms;
    }

    timer->sample_count++;
    timer->running = false;
}


double process_timer_last_ms(const ProcessTimer *timer)
{
    return timer->last_ms;
}

double process_timer_average_ms(const ProcessTimer *timer)
{
    if (timer->sample_count == 0)
    {
        return 0.0;
    }

    return timer->total_ms / (double)timer->sample_count;
}

double process_timer_min_ms(const ProcessTimer *timer)
{
    if (timer->sample_count == 0)
    {
        return 0.0;
    }
    return timer->min_ms;
}

double process_timer_max_ms(const ProcessTimer *timer)
{
    return timer->max_ms;
}

void process_timer_reset(ProcessTimer *timer)
{    
    timer->start_time = 0.0;
    timer->last_ms = 0.0;
    timer->total_ms = 0.0;
    timer->min_ms = DBL_MAX;
    timer->max_ms = 0.0;
    timer->sample_count = 0;
    timer->running = false;

}

bool process_timer_log_config_open(
    ProcessTimerLogConfig *config,
    const char *average_log_path,
    const char *warning_log_path,
    double warning_threshold_ms,
    unsigned long report_interval_samples,
    bool log_average_reports,
    bool log_spikes
)
{
    config->average_log_file = NULL;
    config->warning_log_file = NULL;
    config->warning_threshold_ms = warning_threshold_ms;
    config->report_interval_samples = report_interval_samples;
    config->log_average_reports = log_average_reports;
    config->log_spikes = log_spikes;

    if (log_average_reports)
    {
        config->average_log_file = fopen(average_log_path, "w");
        if (config->average_log_file == NULL)
        {
            return false;
        }

        fprintf(config->average_log_file, "process,samples,last_ms,average_ms,min_ms,max_ms\n");
    }

    if (log_spikes)
    {
        config->warning_log_file = fopen(warning_log_path, "w");
        if (config->warning_log_file == NULL)
        {
            if (config->average_log_file != NULL)
            {
                fclose(config->average_log_file);
                config->average_log_file = NULL;
            }

            return false;
        }
        fprintf(config->warning_log_file, "process,samples,last_ms,threshold_ms\n");
    }
    return true;
}

void process_timer_log_config_close(ProcessTimerLogConfig *config)
{
    if (config->average_log_file != NULL)
    {
        fclose(config->average_log_file);
        config->average_log_file = NULL;
    }

    if (config->warning_log_file != NULL)
    {
        fclose(config->warning_log_file);
        config->warning_log_file = NULL;
    }
}

void process_timer_log_report(const ProcessTimer *timer, const ProcessTimerLogConfig *config)
{
    if (timer == NULL || config == NULL || timer->sample_count == 0)
    {
        return;
    }

    if (
        config->log_spikes 
        && config->warning_log_file != NULL 
        && timer->last_ms > config->warning_threshold_ms
    )
    {
        fprintf(
            config->warning_log_file,
            "%s,%lu,%.3f,%.3f\n",
            timer->name,
            timer->sample_count,
            timer->last_ms,
            config->warning_threshold_ms
        );

        fflush(config->warning_log_file);
    }

    if (
        config->log_average_reports 
        && config->average_log_file != NULL 
        && config->report_interval_samples > 0 
        && timer->sample_count % config->report_interval_samples == 0
    )
    {
        fprintf(
            config->average_log_file,
            "%s,%lu,%.3f,%.3f,%.3f,%.3f\n",
            timer->name,
            timer->sample_count,
            process_timer_last_ms(timer),
            process_timer_average_ms(timer),
            process_timer_min_ms(timer),
            process_timer_max_ms(timer)
        );

        fflush(config->average_log_file);
    }
}