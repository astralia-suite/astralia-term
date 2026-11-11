# Input encoding

Turning keys and mouse events into pty bytes. Sources: `src/input/input.c`, `src/input/input.h`, `src/input/kitty_keys.h`.

## Keyboard

- `kitty_keymap[]` must stay sorted on `sym`, with no exceptions. One out-of-order entry silently breaks binary search for unrelated keys.
- `ev->utf8` has Ctrl's transformation applied; the keysym doesn't. Derive CSI-u code points from `xkb_keysym_to_utf32(ev->sym)`, or every Ctrl combo corrupts.
- A `Delete` or `BackSpace` keysym maps to a control code point in `xkb_keysym_to_utf32`. Kitty alternate-key reporting must skip functional keys, or `yazi` drops `Delete`.

## Mouse

- xterm's wire button numbering is 0-based, while `input_mouse_encode()` takes 1-based buttons. Encoding `button` instead of `button - 1` reports the wrong button to every app.
