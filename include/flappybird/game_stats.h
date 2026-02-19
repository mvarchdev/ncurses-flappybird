// Copyright 2022 <Maros Varchola - mvarchdev>

#ifndef FLAPPYBIRD_GAME_STATS_H
#define FLAPPYBIRD_GAME_STATS_H

#include "flappybird/game_metrics.h"

#define GAME_STATS_FILE "./assets/game_stats.conf"

typedef struct game_stats {
  int total_runs;
  int total_score;
  int total_jumps;
  int total_collisions;
  int total_pipes_passed;
  int best_score;
  int best_streak;
} game_stats;

game_stats game_stats_load(void);
int game_stats_save(const game_stats *stats);
void game_stats_record_run(game_stats *stats, int run_score, const run_metrics *metrics);

#endif  // FLAPPYBIRD_GAME_STATS_H
