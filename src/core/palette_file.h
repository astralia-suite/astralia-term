#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PALETTE_FILE_COLORS 16

struct palette_file {
    uint32_t colors[PALETTE_FILE_COLORS];
    uint32_t fg, bg, cursor;
    bool has_color[PALETTE_FILE_COLORS];
    bool has_fg, has_bg, has_cursor;
};

// `key=RRGGBB` lines (fg, bg, cursor, color0..color15); unknown or malformed lines are skipped. True when at least one key was read.
bool palette_file_parse(const char *text, struct palette_file *out);
bool palette_file_load(const char *path, struct palette_file *out);

// $XDG_STATE_HOME/astralia/palette, else $HOME/.local/state/astralia/palette. False when neither is set.
bool palette_file_path(char *buf, size_t size);
