#include "utils.hpp"

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include "arch/arch.hpp"

void generic_crash_handler(int signum) {
    if (signum == SIGSEGV) {
        crash("Segmentation fault.");
    } else {
        crash("Unexpected signal: %d\n", signum);
    }
}

void ignore_crash_handler(int signum) {}

void install_generic_crash_handler() {
    struct sigaction action;
    bzero(&action, sizeof(action));
    action.sa_handler = generic_crash_handler;
    int res = sigaction(SIGSEGV, &action, NULL);
    guarantee_err(res == 0, "Could not install SEGV handler");

    bzero(&action, sizeof(action));
    action.sa_handler = ignore_crash_handler;
    res = sigaction(SIGPIPE, &action, NULL);
    guarantee_err(res == 0, "Could not install PIPE handler");
}

// fast non-null terminated string comparison
int sized_strcmp(const char *str1, int len1, const char *str2, int len2) {
    int min_len = std::min(len1, len2);
    int res = memcmp(str1, str2, min_len);
    if (res == 0)
        res = len1-len2;
    return res;
}

void print_hd(const void *vbuf, size_t offset, size_t ulength) {
    flockfile(stderr);

    const char *buf = (const char *)vbuf;
    ssize_t length = ulength;

    char bd_sample[16] = { 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 
                           0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD, 0xBD };
    char zero_sample[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    char ff_sample[16] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
                           0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    bool skipped_last = false;
    while (length > 0) {
        bool skip = memcmp(buf, bd_sample, 16) == 0 ||
                    memcmp(buf, zero_sample, 16) == 0 ||
                    memcmp(buf, ff_sample, 16) == 0;
        if (skip) {
            if (!skipped_last) fprintf(stderr, "*\n");
        } else {
            fprintf(stderr, "%.8x  ", (unsigned int)offset);
            for (int i = 0; i < 16; i++) {
                if (i < (int)length) fprintf(stderr, "%.2hhx ", buf[i]);
                else fprintf(stderr, "   ");
            }
            fprintf(stderr, "| ");
            for (int i = 0; i < 16; i++) {
                if (i < (int)length) {
                    if (isprint(buf[i])) fputc(buf[i], stderr);
                    else fputc('.', stderr);
                } else {
                    fputc(' ', stderr);
                }
            }
            fprintf(stderr, "\n");
        }
        skipped_last = skip;

        offset += 16;
        buf += 16;
        length -= 16;
    }

    funlockfile(stderr);
}

// Precise time functions

// These two fields are initialized with current clock values (roughly) at the same moment.
// Since monotonic clock represents time since some arbitrary moment, we need to correlate it
// to some other clock to print time more or less precisely.
//
// Of course that doesn't solve the problem of clocks having different rates.
static struct {
    timespec hi_res_clock;
    time_t low_res_clock;
} time_sync_data;

void initialize_precise_time() {
    int res = clock_gettime(CLOCK_MONOTONIC, &time_sync_data.hi_res_clock);
    guarantee(res == 0, "Failed to get initial monotonic clock value");
    (void) time(&time_sync_data.low_res_clock);
}

precise_time_t get_precise_time() {
    precise_time_t result;
    timespec now;

    int err = clock_gettime(CLOCK_MONOTONIC, &now);
    assert_err(err == 0, "Failed to get monotonic clock value");
    if (err == 0) {
        // Compute time difference between now and origin of time
        time_t dsec = now.tv_sec - time_sync_data.hi_res_clock.tv_sec;
        long dnsec = now.tv_nsec - time_sync_data.hi_res_clock.tv_nsec;
        if (dnsec < 0) {
            dnsec += 1e9;
            dsec--;
        }

        time_t now_lowres = time_sync_data.low_res_clock + dsec;

        (void) gmtime_r(&now_lowres, &result.low_res);
        result.nanosec = dnsec;
        return result;
    } else {
        // fallback: we can't get nanoseconds value, so we fake it
        time_t now_lowres = time(NULL);
        (void) gmtime_r(&now_lowres, &result.low_res);
        result.nanosec = 0;
        return result;
    }
}

void format_precise_time(const precise_time_t& time, char* buf, size_t max_chars) {
    int res = snprintf(buf, max_chars,
        "%04d-%02d-%02d %02d:%02d:%02d.%06d",
        time.low_res.tm_year+1900,
        time.low_res.tm_mon+1,
        time.low_res.tm_mday,
        time.low_res.tm_hour,
        time.low_res.tm_min,
        time.low_res.tm_sec,
        (int) (time.nanosec/1e3));
    (void)res;
    assert(0 <= res);
}
