// Copyright 2022 <Maros Varchola - mvarchdev>

#include "flappybird/game_stats.h"

#include "flappybird/confparser.h"
#include <stddef.h>

static int read_or_zero(const char *key) {
  int value = get_int_key_value(GAME_STATS_FILE, key);
  return value < 0 ? 0 : value;
}

game_stats game_stats_load(void) {
  game_stats stats = {0};
  stats.total_runs = read_or_zero("total_runs");
  stats.total_score = read_or_zero("total_score");
  stats.total_jumps = read_or_zero("total_jumps");
  stats.total_collisions = read_or_zero("total_collisions");
  stats.total_pipes_passed = read_or_zero("total_pipes_passed");
  stats.best_score = read_or_zero("best_score");
  stats.best_streak = read_or_zero("best_streak");
  return stats;
}

int game_stats_save(const game_stats *stats) {
  if (stats == NULL) {
    return -1;
  }

  int status = 0;
  status |= set_int_key_value("./stats_tmpfile", GAME_STATS_FILE, "total_runs", stats->total_runs);
  status |=
      set_int_key_value("./stats_tmpfile", GAME_STATS_FILE, "total_score", stats->total_score);
  status |=
      set_int_key_value("./stats_tmpfile", GAME_STATS_FILE, "total_jumps", stats->total_jumps);
  status |= set_int_key_value("./stats_tmpfile", GAME_STATS_FILE, "total_collisions",
                              stats->total_collisions);
  status |= set_int_key_value("./stats_tmpfile", GAME_STATS_FILE, "total_pipes_passed",
                              stats->total_pipes_passed);
  status |= set_int_key_value("./stats_tmpfile", GAME_STATS_FILE, "best_score", stats->best_score);
  status |=
      set_int_key_value("./stats_tmpfile", GAME_STATS_FILE, "best_streak", stats->best_streak);

  return status == 0 ? 0 : -1;
}

void game_stats_record_run(game_stats *stats, int run_score, const run_metrics *metrics) {
  if (stats == NULL || metrics == NULL) {
    return;
  }

  stats->total_runs++;
  stats->total_score += run_score;
  stats->total_jumps += metrics->jumps;
  stats->total_collisions += metrics->collisions;
  stats->total_pipes_passed += metrics->pipes_passed;

  if (run_score > stats->best_score) {
    stats->best_score = run_score;
  }
  if (metrics->highest_streak > stats->best_streak) {
    stats->best_streak = metrics->highest_streak;
  }
}
