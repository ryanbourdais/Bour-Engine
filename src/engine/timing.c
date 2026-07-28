#include "timing.h"

void frame_clock_init(FrameClock *clock, double initial_time)
{
    clock->previous_time = initial_time;
    clock->delta_time = 0.0;
}
void frame_clock_update(FrameClock *clock, double current_time)
{
    clock->delta_time = current_time - clock->previous_time;
    clock->previous_time = current_time;
}
double frame_clock_delta_time(const FrameClock *clock)
{
    return clock->delta_time;
}
