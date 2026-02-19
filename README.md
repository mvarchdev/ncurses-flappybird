# Ncurses Flappy Bird

Terminal Flappy Bird clone written in C with `ncurses`.

## Features

- Real-time gameplay loop with gravity/jump physics.
- Config-driven rendering and level tuning.
- Persistent save state and hall-of-fame scores by nickname.
- Persistent global gameplay statistics across runs.
- Dynamic streak-based score multiplier system.
- Configurable terminal sound cues (beep or bell).
- Portable build with dependency tracking (`Makefile`).

## Repository layout

```text
.
├── assets/                     # level files, settings, banners, save/hof data
├── include/flappybird/         # public headers
├── src/                        # source implementation
├── .taskfiles/                 # modular Taskfile tasks
├── Makefile                    # build, run, lint, format
└── Taskfile.yaml               # task orchestration
```

## Build and run

### Requirements

- C compiler with C11 support (`gcc`/`clang`)
- `ncurses` development package
- `make`

### Commands

```bash
make all
./flappy_bird
```

or via Task:

```bash
task build:all
task build:run
```

## Configuration

### Screen and render settings

Edit `/assets/settings/settings.conf`:

- header and border colors
- header dimensions
- FPS
- sound settings (`sound_enabled`, `sound_mode`)
- optional gravity defaults

### Gameplay controls

- `Space`: jump
- `P`: pause/resume
- `E`: end run
- `H`: in-game quick hint
- `Left/Right`: menu navigation
- `Enter`: select
- `B`: exit information pages

### Map size

Edit constants in `/include/flappybird/rendering.h`:

- `MAPSIZEX`
- `MAPSIZEY`
- margins and border width constants

### Levels

Levels are defined in `/assets/levels/level_<n>.conf`.

To add a level:

1. Copy an existing file (for example `level_3.conf`).
2. Rename it to the next numeric level.
3. Adjust speed, gravity, spacing, width and color values.

## Developer workflow

### Make targets

- `make all`: build binary
- `make run`: build and run
- `make debug`: debug build
- `make release`: optimized build
- `make lint`: static analysis via `cppcheck`
- `make format`: format C files via `clang-format`
- `make clean`: remove artifacts

### Task targets

- `task setup`: install pre-commit hooks and run baseline checks
- `task ci`: clean build + lint checks
- `task quality:pre-commit`: run all pre-commit hooks

### Pre-commit hooks

Configured in `/.pre-commit-config.yaml`:

- file hygiene checks (`pre-commit-hooks`)
- YAML and Markdown linting
- `clang-format` auto-formatting for `.c/.h`
- build verification on `pre-push`

## Notes

- Runtime save files are:
  - `/assets/saves.conf`
  - `/assets/hall_of_fame.conf`
  - `/assets/game_stats.conf`
- All are git-ignored through `/assets/.gitignore`.
