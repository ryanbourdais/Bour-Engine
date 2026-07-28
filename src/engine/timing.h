#pragma once

typedef struct FrameClock {
    double previous_time;
    double delta_time;
} FrameClock;

void frame_clock_init(FrameClock *clock, double initial_time);
void frame_clock_update(FrameClock *clock, double current_time);
double frame_clock_delta_time(const FrameClock *clock);
