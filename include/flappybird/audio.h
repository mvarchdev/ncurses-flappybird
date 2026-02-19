// Copyright 2022 <Maros Varchola - mvarchdev>

#ifndef FLAPPYBIRD_AUDIO_H
#define FLAPPYBIRD_AUDIO_H

#include <stdbool.h>

typedef enum audio_event {
  AUDIO_EVENT_MENU_MOVE,
  AUDIO_EVENT_JUMP,
  AUDIO_EVENT_PIPE_PASSED,
  AUDIO_EVENT_COLLISION,
  AUDIO_EVENT_COUNTDOWN,
  AUDIO_EVENT_HIGH_SCORE
} audio_event;

void audio_set_enabled(bool enabled);
void audio_set_mode(const char *mode);
void audio_play(audio_event event);

#endif  // FLAPPYBIRD_AUDIO_H
