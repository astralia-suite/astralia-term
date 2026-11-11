# Graphics

Kitty graphics commands, image placements and Unicode placeholders. Sources: `src/term/graphics.c`, `src/term/graphics.h`, `src/term/kitty_placeholder.c`, `src/term/kitty_placeholder.h`, `src/term/kitty_diacritics.h`, `src/term/sixel.c`, `src/term/sixel.h`.

## Replies

- A command with no image id (`id=0`) must never get a reply, success or error. The client isn't reading, so a reply leaks into the next shell prompt.
- Every failure on a file or shared memory medium must produce the same reply. A distinct error lets a remote program probe which files exist and their size.

## Placements

- `yazi` sends `a=T,U=1,t=s` with no `c=` or `r=`. A sizeless virtual placement must default from image and cell size, or its preview is blank.
- A non-virtual image lives in one `ATTR_IMAGE` anchor cell; erasing that cell removes the whole image. The anchor moves with text, whereas tile cells tore images on reflow.
- `render_frame()` redraws in full whenever the visible placement hash changes. Image pixels span rows the anchor's row doesn't dirty, so they would go stale.
