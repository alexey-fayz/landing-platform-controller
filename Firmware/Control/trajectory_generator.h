#ifndef TRAJECTORY_GENERATOR_H
#define TRAJECTORY_GENERATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct
{
    float target_position;

    float current_position;

    float current_velocity;

    float max_velocity;

    float max_acceleration;

    bool active;

} TrajectoryGenerator_t;

void TrajectoryGenerator_Init(
        TrajectoryGenerator_t *traj,
        float max_velocity,
        float max_acceleration);

void TrajectoryGenerator_SetTarget(
        TrajectoryGenerator_t *traj,
        float target_position);

void TrajectoryGenerator_Update(
        TrajectoryGenerator_t *traj,
        float dt);

float TrajectoryGenerator_GetPosition(
        TrajectoryGenerator_t *traj);

float TrajectoryGenerator_GetVelocity(
        TrajectoryGenerator_t *traj);

#ifdef __cplusplus
}
#endif

#endif