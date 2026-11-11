# Event loop

The single `epoll` loop, its hooks and its timers. Sources: `src/core/loop.c`, `src/core/loop.h`.

## Dispatch

- The `epoll` loop must never block, even briefly. One loop drives the pty, display and timers, so blocking freezes the terminal.
- Hooks run `HIGH`, `NORMAL`, then `LOW` before every `epoll_wait`; a new backend dispatches in `HIGH` and flushes in `LOW`. Otherwise queued display events stall until an unrelated fd wakes the loop.
- `loop_del()` is safe inside a callback. Removed handlers are freed after the dispatch pass, so add no extra deferral.
- The main loop must `continue` on `EINTR`, never break. Breaking silently exits the process, masquerading as an unexplained crash.

## Timers

- A `timerfd` callback must call `loop_timer_ack()`. Otherwise the fd stays readable and the loop spins on it forever.
