// Copyright 2022 <Maros Varchola - mvarchdev>

#include "flappybird/audio.h"

#include <ncurses.h>
#include <stdio.h>
#include <strings.h>

static bool g_audio_enabled = true;
static bool g_use_beep = true;

void audio_set_enabled(bool enabled) { g_audio_enabled = enabled; }

void audio_set_mode(const char *mode) {
  if (mode == NULL) {
    return;
  }

  if (strcasecmp(mode, "bell") == 0) {
    g_use_beep = false;
  } else if (strcasecmp(mode, "beep") == 0) {
    g_use_beep = true;
  }
}

static void play_tick(void) {
  if (!g_audio_enabled) {
    return;
  }

  if (g_use_beep) {
    beep();
  } else {
    putchar('\a');
    fflush(stdout);
  }
}

void audio_play(audio_event event) {
  if (!g_audio_enabled) {
    return;
  }

  switch (event) {
    case AUDIO_EVENT_COLLISION:
      play_tick();
      play_tick();
      break;
    case AUDIO_EVENT_HIGH_SCORE:
      play_tick();
      play_tick();
      play_tick();
      break;
    case AUDIO_EVENT_MENU_MOVE:
    case AUDIO_EVENT_JUMP:
    case AUDIO_EVENT_PIPE_PASSED:
    case AUDIO_EVENT_COUNTDOWN:
      play_tick();
      break;
    default:
      break;
  }
}
