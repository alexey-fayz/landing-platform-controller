#include "control/trajectory_generator.h"

#include <math.h>

static float Sign(float x)
{
    if(x > 0.0f)
        return 1.0f;

    if(x < 0.0f)
        return -1.0f;

    return 0.0f;
}

void TrajectoryGenerator_Init(
        TrajectoryGenerator_t *traj,
        float max_velocity,
        float max_acceleration)
{
    traj->target_position = 0.0f;

    traj->current_position = 0.0f;

    traj->current_velocity = 0.0f;

    traj->max_velocity =
            max_velocity;

    traj->max_acceleration =
            max_acceleration;

    traj->active = false;
}

void TrajectoryGenerator_SetTarget(
        TrajectoryGenerator_t *traj,
        float target_position)
{
    traj->target_position =
            target_position;

    traj->active = true;
}

void TrajectoryGenerator_Update(
        TrajectoryGenerator_t *traj,
        float dt)
{
    if(!traj->active)
        return;

    float error =
            traj->target_position -
            traj->current_position;

    if(fabsf(error) < 0.01f)
    {
        traj->current_position =
                traj->target_position;

        traj->current_velocity = 0.0f;

        traj->active = false;

        return;
    }

    float desired_velocity =
            Sign(error) *
            traj->max_velocity;

    float velocity_error =
            desired_velocity -
            traj->current_velocity;

    float acceleration =
            velocity_error / dt;

    if(acceleration >
       traj->max_acceleration)
    {
        acceleration =
                traj->max_acceleration;
    }

    if(acceleration <
       -traj->max_acceleration)
    {
        acceleration =
                -traj->max_acceleration;
    }

    traj->current_velocity +=
            acceleration * dt;

    if(traj->current_velocity >
       traj->max_velocity)
    {
        traj->current_velocity =
                traj->max_velocity;
    }

    if(traj->current_velocity <
       -traj->max_velocity)
    {
        traj->current_velocity =
                -traj->max_velocity;
    }

    traj->current_position +=
            traj->current_velocity * dt;
}

float TrajectoryGenerator_GetPosition(
        TrajectoryGenerator_t *traj)
{
    return traj->current_position;
}

float TrajectoryGenerator_GetVelocity(
        TrajectoryGenerator_t *traj)
{
    return traj->current_velocity;
}