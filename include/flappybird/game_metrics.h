// Copyright 2022 <Maros Varchola - mvarchdev>

#ifndef FLAPPYBIRD_GAME_METRICS_H
#define FLAPPYBIRD_GAME_METRICS_H

typedef struct run_metrics {
  int jumps;
  int collisions;
  int pipes_passed;
  int pauses;
  int highest_streak;
  int highest_multiplier;
} run_metrics;

#endif  // FLAPPYBIRD_GAME_METRICS_H
