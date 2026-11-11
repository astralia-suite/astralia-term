# Terminal model

Cells, the grid, scrollback, selection, escape sequence replies and links. Sources: `src/term/term.c`, `src/term/term.h`, `src/term/grid.c`, `src/term/grid.h`, `src/term/composed.c`, `src/term/composed.h`, `src/term/selection.c`, `src/term/selection.h`, `src/term/hyperlink.c`, `src/term/hyperlink.h`, `src/term/url.c`, `src/term/url.h`, `src/term/vt_csi.c`, `src/term/vt_osc.c`, `src/core/width_table.h`.

## Cells

- An all-zero `struct cell` is blank with default colors. Rows can be `calloc`ed lazily, so never give blank a nonzero encoding.
- `struct cell` must stay 20 bytes. Grid memory and scroll cost scale with it, so reuse fields behind an attr bit.
- `cp_width()` reads the generated `width_table.h`, never libc `wcwidth()`. Widths must not depend on the user's locale or glibc's Unicode version.
- Bold palette colors 0–7 draw bright, and colors resolve at draw time. Resolving at write time would break `DECSCNM` and palette changes.
- A cell `cp` at or above `COMPOSED_BASE` is a `composed_table` index, not a codepoint. Anything reading text must resolve it through `composed_get()` first.

## Screen and scrollback

- Only the normal screen reflows on resize; the alternate screen truncates. Full-screen apps redraw on `SIGWINCH`, and reflowing would corrupt their layout.
- Scrolls that push lines into history must bump `view_offset` when scrolled back. Otherwise the viewed text slides away while output keeps arriving.
- `grid_resize()`'s `clamp_points()` clamps every tracked point into `[0, rows-1]`. A selection endpoint threaded through it loses its scrollback row when reflow pushes it above the window.

## Escape sequences

- OSC replies must end with the request's terminator, BEL or ST. Some applications only parse the terminator they sent.
- The `XTVERSION` reply name must contain `kitty`. `snacks.nvim` enables image support only by matching that name, else opens images as binary.

## Links

- The open OSC 8 link lives in `term.link`, never in `struct pen`. `SGR 0` resets the whole pen and would silently close the link.
- A link handle in `tile_row`/`tile_col` is valid only with `ATTR_LINK` and without `ATTR_IMAGE`. Image cells reuse those bytes, so read them through `cell_link()`.
