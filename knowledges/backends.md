# Backends

Window, buffer, protocol and raw input handling for each display backend. Sources: `src/backend/backend.h`, `src/backend/wayland.c`, `src/backend/x11.c`.

## Frames and buffers

- Backends may free buffers only on a size change. `render_frame()` copies the previous frame into a switched buffer and needs it.
- Backend `configure` sizes and pointer coordinates are buffer pixels, not logical. Wayland multiplies logical values by the fractional scale.
- [wayland] `frame_ready` must also be set on `xdg_surface.configure` when no frame callback is pending. Otherwise the resized frame after an ack never draws.
- [wayland] A pending `wl_callback` must be released with `wl_callback_destroy`, never just nulled. `wl_surface_destroy` doesn't free it, so a late `done` fires against reset state.
- [wayland] Never attach a buffer before the first `xdg_surface.configure` is acked. Compositors kill the client with "xdg_surface has never been configured".
- [x11] There is no frame callback, so `commit` arms a 60 Hz `timerfd`. MIT-SHM completion events are what mark buffers free again.

## Protocols

- [wayland] Optional protocol events need sane fallback defaults, not zero. Some compositors never send `repeat_info`, and `0/0` silently disables key repeat.
- [wayland] `wl_data_device_manager` is bound at v1, not the newest version. v3+ sends drag-and-drop negotiation events this backend never implements.

## Raw input

- [wayland] `wl_pointer.axis`, `axis_discrete` and `axis_value120` must be reconciled once per `frame()`, not dispatched individually. A compositor sends only one of the three per scroll, so per-event handling miscounts notches.
- [wayland] `axis_value120` needs the `wl_seat` bind capped at v8, not v7. Sub-objects inherit the bound version, and a lower cap falls back to deprecated `axis_discrete`.
- [x11] A held key sends release+press pairs, since detectable autorepeat is not enabled. Collapse them into one `KEY_REPEAT` by peeking the next event's timestamp, not with the XKB extension.
