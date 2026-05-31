#include "control/lowpass_filter.h"

#include <math.h>
#include <corecrt_math_defines.h>

void LPF_Init(
        LowPassFilter_t *filter,
        float cutoff_frequency,
        float sample_time)
{
    filter->cutoff_frequency =
            cutoff_frequency;

    filter->sample_time =
            sample_time;

    float tau =
            1.0f /
            (2.0f * (float)M_PI * cutoff_frequency);

    filter->alpha =
            sample_time /
            (tau + sample_time);

    filter->state = 0.0f;
}

float LPF_Update(
        LowPassFilter_t *filter,
        float input)
{
    filter->state =
            filter->state +
            filter->alpha *
            (input - filter->state);

    return filter->state;
}

void LPF_Reset(
        LowPassFilter_t *filter,
        float value)
{
    filter->state = value;
}