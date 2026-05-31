#ifndef LOWPASS_FILTER_H
#define LOWPASS_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float cutoff_frequency;
    float sample_time;

    float alpha;

    float state;

} LowPassFilter_t;

void LPF_Init(
        LowPassFilter_t *filter,
        float cutoff_frequency,
        float sample_time);

float LPF_Update(
        LowPassFilter_t *filter,
        float input);

void LPF_Reset(
        LowPassFilter_t *filter,
        float value);

#ifdef __cplusplus
}
#endif

#endif