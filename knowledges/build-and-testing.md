# Build and testing

Meson quirks, linking the tests and safe manual testing. Sources: `meson.build`, `meson_options.txt`, `build.sh`, `test/`.

## Build

- The "Pkg-config error with 'fcft'" configure warning is expected. `fcft.pc` requires header-only `tllist`, so `meson.build` falls back to linking `libfcft`.
- `HAVE_WAYLAND` and `HAVE_X11` come from generated `config-build.h`, which `backend.h` includes. Test with `#if`, not `#ifdef`, since both are always defined.
- `cursor-shape-v1.xml` needs `tablet-v2.xml` generated alongside it. It references `tablet-v2` types, so dropping it breaks the link.
- A `static_library()`'s consumers need every dependency its linked-in objects reference. `term.c` calls `graphics.c`, so every test calling `term_init` must list `zlib` and `libspng`.

## Testing

- When a dependency is missing, stop and tell the user. `vttest` is not installed on the dev machine; `cage` is.
- Test Wayland with `cage -- build/astralia-term`, and capture with `import -window root`. Cage's X11 window is hard to find by name; cage shows black behind translucency.
- Never `pkill astralia-term`, by name or `-f`, to clean up test instances. It kills the user's instance too; launch with `&`, keep `$!`, kill that PID.
- A hex escape in a C string literal consumes every following hex digit. `"\x1bc"` is one out-of-range escape, so split it: `"\x1b" "c"`.
