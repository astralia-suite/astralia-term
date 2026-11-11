# Rendering

Frame pacing, CPU drawing, glyphs and scale changes. Sources: `src/render/render.c`, `src/render/render.h`, `src/render/boxdraw.c`, `src/render/boxdraw.h`, `src/render/font.c`, `src/render/font.h`, `src/main.c`.

## Frame pacing

- Draw only when `frame_ready && render_needed()`. That caps output at one frame per display refresh regardless of pty throughput.
- `get_buffer()` returning `NULL` just skips the frame. All buffers busy is normal, and the next wakeup retries.
- While mode `2026` is set, `render_needed()` returns false; a 150 ms timer ends it. A client that never resets it would otherwise freeze the display.

## Drawing

- Only the default background is translucent, and fills are premultiplied. Explicit backgrounds stay opaque, and unpremultiplied colors glow on `a8r8g8b8` buffers.
- Draw `U+2500..U+259F` procedurally via `boxdraw`, never from font glyphs. Font glyphs stop short of the cell edge, so stacked rows show gaps.
- Every visual-bell frame and the one after it must be a full redraw. The overlay is baked into the buffer, and the next frame copies it.
- Glyph alpha of light-on-dark text is remapped in `blit_glyph`, not blended straight. `ASTRALIA_GLYPH_GAMMA`, default `1.4`, thickens strokes; color glyphs and `boxdraw` masks skip it.

## Scale changes

- Rescale `app->width/height` before reloading fonts on a scale change. Otherwise the grid reflows to the new cell size and back, losing alternate-screen content.
