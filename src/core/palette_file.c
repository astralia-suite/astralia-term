#include "core/palette_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool
parse_hex(const char *s, uint32_t *out) {
    if (*s == '#')
        s++;
    uint32_t v = 0;
    int n = 0;
    for (; n < 6; n++) {
        char c = s[n];
        uint32_t d;
        if (c >= '0' && c <= '9')
            d = c - '0';
        else if (c >= 'a' && c <= 'f')
            d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            d = c - 'A' + 10;
        else
            return false;
        v = v << 4 | d;
    }
    if (s[n] != '\0' && s[n] != '\n' && s[n] != '\r')
        return false;
    *out = v;
    return true;
}

static bool
parse_line(const char *line, size_t key_len, struct palette_file *out) {
    char key[16];
    if (key_len >= sizeof(key))
        return false;
    memcpy(key, line, key_len);
    key[key_len] = '\0';

    uint32_t rgb;
    if (!parse_hex(line + key_len + 1, &rgb))
        return false;

    if (strcmp(key, "fg") == 0) {
        out->fg = rgb;
        out->has_fg = true;
    } else if (strcmp(key, "bg") == 0) {
        out->bg = rgb;
        out->has_bg = true;
    } else if (strcmp(key, "cursor") == 0) {
        out->cursor = rgb;
        out->has_cursor = true;
    } else if (strncmp(key, "color", 5) == 0 && key[5] != '\0') {
        char *end;
        long idx = strtol(key + 5, &end, 10);
        if (*end != '\0' || idx < 0 || idx >= PALETTE_FILE_COLORS)
            return false;
        out->colors[idx] = rgb;
        out->has_color[idx] = true;
    } else {
        return false;
    }
    return true;
}

bool palette_file_parse(const char *text, struct palette_file *out) {
    *out = (struct palette_file){0};
    bool any = false;
    while (*text != '\0') {
        const char *eol = strchr(text, '\n');
        const char *eq = memchr(text, '=', eol != NULL ? (size_t)(eol - text) : strlen(text));
        if (eq != NULL && parse_line(text, (size_t)(eq - text), out))
            any = true;
        if (eol == NULL)
            break;
        text = eol + 1;
    }
    return any;
}

bool palette_file_load(const char *path, struct palette_file *out) {
    FILE *f = fopen(path, "r");
    if (f == NULL)
        return false;
    char buf[1024];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    buf[n] = '\0';
    return palette_file_parse(buf, out);
}

bool palette_file_path(char *buf, size_t size) {
    const char *state = getenv("XDG_STATE_HOME");
    const char *home = getenv("HOME");
    int n;
    if (state != NULL && state[0] == '/')
        n = snprintf(buf, size, "%s/astralia/palette", state);
    else if (home != NULL && home[0] != '\0')
        n = snprintf(buf, size, "%s/.local/state/astralia/palette", home);
    else
        return false;
    return n >= 0 && (size_t)n < size;
}
