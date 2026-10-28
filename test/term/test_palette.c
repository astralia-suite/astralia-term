// Palette file parsing and applying it to a live term.
#include <stdio.h>
#include <string.h>

#include "core/palette_file.h"
#include "term/term.h"

static int failures;

#define CHECK(cond)                                                                  \
    do {                                                                             \
        if (!(cond)) {                                                               \
            fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #cond); \
            failures++;                                                              \
        }                                                                            \
    } while (0)

static void
feed(struct term *t, const char *s) {
    term_feed(t, (const uint8_t *)s, strlen(s));
}

static void
test_parse(void) {
    struct palette_file p;
    CHECK(palette_file_parse("fg=ddeeff\nbg=#102030\ncursor=9b57f4\ncolor0=000001\ncolor15=fffffe\n", &p));
    CHECK(p.has_fg && p.fg == 0xddeeff);
    CHECK(p.has_bg && p.bg == 0x102030);
    CHECK(p.has_cursor && p.cursor == 0x9b57f4);
    CHECK(p.has_color[0] && p.colors[0] == 0x000001);
    CHECK(p.has_color[15] && p.colors[15] == 0xfffffe);
    CHECK(!p.has_color[7]);

    CHECK(!palette_file_parse("", &p));
    CHECK(!palette_file_parse("garbage\nfg=12345\nfg=zzzzzz\ncolor16=ffffff\ncolor-1=ffffff\nbogus=ffffff\n", &p));
    CHECK(!p.has_fg && !p.has_bg && !p.has_cursor);

    // A bad line does not hide the good ones around it
    CHECK(palette_file_parse("bg=nothex\nfg=010203\ncolorx=ffffff\ncolor3=040506", &p));
    CHECK(p.has_fg && p.fg == 0x010203 && !p.has_bg);
    CHECK(p.has_color[3] && p.colors[3] == 0x040506);
}

static void
test_apply(void) {
    struct term t;
    term_init(&t, 10, 2, &(struct term_host){0}, NULL);

    struct palette_file p;
    palette_file_parse("fg=aaaaaa\nbg=111111\ncursor=00ff00\ncolor1=ff0000\ncolor2=00ff00\ncolor5=9b57f5\n", &p);
    t.colors_changed = false;
    term_apply_palette(&t, &p);
    CHECK(t.colors_changed);
    CHECK(t.default_fg == 0xaaaaaa && t.default_bg == 0x111111);
    CHECK(t.palette[1] == 0xff0000 && t.initial_palette[1] == 0xff0000);
    CHECK(t.cursor_color_set && t.cursor_color == 0x00ff00);
    CHECK(t.link_fg == 0x9b57f5);

    // A program override survives a reload, but the reset target moves
    feed(&t, "\x1b]4;1;#123456\a\x1b]11;#222222\a\x1b]12;#333333\a");
    palette_file_parse("fg=bbbbbb\nbg=000000\ncursor=ffffff\ncolor1=0000ff\ncolor2=00ffff\n", &p);
    term_apply_palette(&t, &p);
    CHECK(t.palette[1] == 0x123456 && t.initial_palette[1] == 0x0000ff);
    CHECK(t.palette[2] == 0x00ffff);
    CHECK(t.default_fg == 0xbbbbbb);
    CHECK(t.default_bg == 0x222222 && t.initial_bg == 0x000000);
    CHECK(t.cursor_color == 0x333333);
    CHECK(t.link_fg == 0x9b57f5);

    feed(&t, "\x1b]104\a\x1b]111\a\x1b]112\a");
    CHECK(t.palette[1] == 0x0000ff);
    CHECK(t.default_bg == 0x000000);
    CHECK(t.cursor_color_set && t.cursor_color == 0xffffff);

    term_destroy(&t);
}

int main(void) {
    test_parse();
    test_apply();

    if (failures == 0)
        printf("test_palette: all tests passed\n");
    return failures == 0 ? 0 : 1;
}
