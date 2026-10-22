#include "core/util.h"

#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/width_table.h"

static enum log_level max_level = LOG_LEVEL_INFO;

static void
oom(void) {
    fputs("astralia-term: out of memory\n", stderr);
    abort();
}

void *
xmalloc(size_t size) {
    void *p = malloc(size);
    if (unlikely(p == NULL && size != 0))
        oom();
    return p;
}

void *
xcalloc(size_t nmemb, size_t size) {
    void *p = calloc(nmemb, size);
    if (unlikely(p == NULL && nmemb != 0 && size != 0))
        oom();
    return p;
}

void *
xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (unlikely(p == NULL && size != 0))
        oom();
    return p;
}

char *
xstrdup(const char *s) {
    char *p = strdup(s);
    if (unlikely(p == NULL))
        oom();
    return p;
}

void log_set_level(enum log_level lvl) {
    max_level = lvl;
}

bool log_redirect_to_state_file(void) {
    const char *state = getenv("XDG_STATE_HOME");
    const char *home = getenv("HOME");
    char dir[PATH_MAX], path[PATH_MAX];

    int n;
    if (state != NULL && state[0] == '/')
        n = snprintf(dir, sizeof(dir), "%s/astralia-term", state);
    else if (home != NULL && home[0] != '\0')
        n = snprintf(dir, sizeof(dir), "%s/.local/state/astralia-term", home);
    else
        return false;
    if (n < 0 || (size_t)n >= sizeof(dir))
        return false;

    // mkdir -p
    for (char *p = dir + 1;; p++) {
        if (*p != '/' && *p != '\0')
            continue;
        char saved = *p;
        *p = '\0';
        if (mkdir(dir, 0755) < 0 && errno != EEXIST)
            return false;
        *p = saved;
        if (saved == '\0')
            break;
    }

    n = snprintf(path, sizeof(path), "%s/astralia-term.log", dir);
    if (n < 0 || (size_t)n >= sizeof(path))
        return false;
    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0600);
    if (fd < 0)
        return false;
    bool ok = dup2(fd, STDERR_FILENO) >= 0;
    close(fd);
    return ok;
}

void log_msg(enum log_level lvl, const char *module, int errnum, const char *fmt, ...) {
    if (lvl > max_level)
        return;

    static const char *const prefix[] = {
        [LOG_LEVEL_ERR] = "err",
        [LOG_LEVEL_WARN] = "warn",
        [LOG_LEVEL_INFO] = "info",
        [LOG_LEVEL_DBG] = "dbg",
    };

    fprintf(stderr, "%s: %s: ", prefix[lvl], module);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (errnum != 0)
        fprintf(stderr, ": %s", strerror(errnum));
    fputc('\n', stderr);
}

int cp_width(uint32_t cp) {
    if (likely(cp < 0x7f))
        return cp >= 0x20 ? 1 : 0;
    if (unlikely(cp >= 0x110000))
        return 1;

    // 2-bit codes: 0 = width 1, 1 = width 0, 2 = width 2
    uint8_t byte = width_stage2[width_stage1[cp >> 8]][(cp & 0xff) >> 2];
    unsigned code = (byte >> ((cp & 3) * 2)) & 3;
    return code == 0 ? 1 : code == 1 ? 0
                                     : 2;
}
