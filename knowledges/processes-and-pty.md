# Processes and the pty

Spawning the shell, reaping it and writing to the pty. Sources: `src/core/pty.c`, `src/core/pty.h`.

## Child process

- Never allocate memory in a forked child before `exec()`. `fork()` can copy a lock held by another thread, deadlocking the child.
- Ignoring `SIGCHLD` and calling `waitpid()` cannot coexist. Ignoring lets the kernel auto-reap and breaks exit codes, so `SIGCHLD` goes through `signalfd`.
- `child_exec()` must `scrub_host_env()` terminal and editor identification variables such as `TERM_PROGRAM`, `NVIM` and `KITTY_`. Otherwise they leak from the launcher into every shell and break its checks.

## Pty

- `pty_write` must buffer and wait for `EPOLLOUT` when the pty is full. A blocking write deadlocks when the child is blocked writing output back.
