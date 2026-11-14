"""Compile the demote path with and without the test-only power-loss hook.

LT_AMB1_POWERLOSS_TEST is a bench-only instrumentation flag. This test builds
the actual ``image2_flash_backend.c`` twice from the same harness:

* with ``-DLT_AMB1_POWERLOSS_TEST=1`` it asserts the marker is emitted exactly
  once, before the demote operation runs; that the demote operation still runs
  exactly once and still gates success; and that a policy without target
  publication evidence still cannot demote (or arm the hook);
* without the flag it asserts the hook is completely compiled out.

``lt_wdt_feed()`` and ``lt_amb1_log_puts()`` are replaced by counters, and the
weak ``lt_amb1_powerloss_delay_ms()`` is overridden so nothing ever waits.
No hardware, network, credentials or flash access is involved.
"""
import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
OTA = ROOT / "base/ota"

HARNESS = r'''
#include "image2_flash_backend.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

/* --- test instrumentation (overrides the weak target delay) ---------------- */
static int marker_count, wdt_feeds, delay_calls, demote_calls;
static char events[16];
static int event_count;

static void event(char kind) {
    assert(event_count < (int)sizeof(events));
    events[event_count++] = kind;
}

void lt_amb1_log_puts(const char *s) {
    assert(!strcmp(s, "\r\nAMB1-POWERLOSS-DEMOTE-ARMED\r\n"));
    marker_count++;
    event('M');
}
void lt_wdt_feed(void) { wdt_feeds++; }
/* Strong definition beats the backend's weak one: the suite must never wait. */
void lt_amb1_powerloss_delay_ms(uint32_t ms) {
    assert(ms == 10u);
    delay_calls++;
}

/* --- fake NOR ------------------------------------------------------------ */
typedef struct {
    unsigned char mem[4096];
    const void *owner;
    int reads, writes, erases, locks, unlocks;
    int demote_ret, demote_present;
} sim;
static sim s;
static image2_flash_backend b;
static image2_flash_config c;
static image2_flash_sdkops ops;

static int lock(void *p, const void *owner) {
    sim *x = p;
    x->locks++;
    if (x->owner) return 0;
    x->owner = owner;
    return 1;
}
static void unlock(void *p, const void *owner) {
    sim *x = p;
    assert(x->owner == owner);
    x->owner = 0;
    x->unlocks++;
}
static int read_bytes(void *p, uint32_t a, void *out, size_t n) {
    sim *x = p;
    assert(x->owner == &b);
    assert(n && a < 4096 && n <= 4096 - a);
    x->reads++;
    memcpy(out, x->mem + a, n);
    return 1;
}
static int program(void *p, uint32_t a, const void *in, size_t n) {
    sim *x = p;
    const unsigned char *v = in;
    size_t i;
    assert(x->owner == &b);
    assert(n && a < 4096 && n <= 4096 - a);
    x->writes++;
    for (i = 0; i < n; i++) x->mem[a + i] &= v[i];
    return 1;
}
static int erase(void *p, uint32_t a) {
    sim *x = p;
    assert(x->owner == &b);
    x->erases++;
    memset(x->mem + a, 255, 256);
    return 1;
}
/* Dedicated demote capability, exactly one ASCII '8' -> '0'. */
static int demote(void *p) {
    sim *x = p;
    demote_calls++;
    event('D');
    x->mem[264] = '0';
    return x->demote_ret;
}

static void setup(void) {
    memset(&s, 0, sizeof(s));
    memset(&b, 0, sizeof(b));
    memset(&c, 0, sizeof(c));
    memset(events, 0, sizeof(events));
    event_count = marker_count = wdt_feeds = delay_calls = demote_calls = 0;
    memset(s.mem, 255, sizeof(s.mem));
    s.demote_ret = 1;
    c.layout.flash_size = 4096;
    c.layout.sector_size = 256;
    c.layout.slots[0] = (image2_range){256, 1280};
    c.layout.slots[1] = (image2_range){2048, 3072};
    c.layout.reviewed = c.layout.slots[1];
    c.layout.reviewed_layout = c.layout.normal_selector = 1;
    c.layout.protected_count = 1;
    c.layout.protected_ranges[0] = (image2_range){0, 256};
    c.image_size = 700;
    c.page_size = 64;
    c.extent = (image2_range){2048, 2816};
    c.allow_source_demote = 1;
    ops = (image2_flash_sdkops){&s, lock, unlock, read_bytes, program, erase, demote};
}

/* Every precondition, including target publication and full source signature. */
static void ready_to_demote(void) {
    setup();
    memcpy(s.mem + 264, "81958711", 8);   /* running slot source signature */
    assert(image2_flash_open(&b, &c, &ops));
    assert(image2_flash_phase(&b, 2));
    assert(image2_flash_phase(&b, 3));
    /* Publishing the target signature flips publication_attempted for real. */
    assert(image2_flash_write(&b, 2056, "81958711", 8));
    assert(b.publication_attempted);
    assert(!memcmp(s.mem + 2056, "81958711", 8));
}

static void check_marker_only_with_flag(int expected) {
    assert(marker_count == expected);
    assert(event_count == (expected ? 2 : 1));
    if (expected) {
        /* Marker strictly before the demote operation. */
        assert(events[0] == 'M' && events[1] == 'D');
        assert(wdt_feeds > 0 && delay_calls > 0);
    } else {
        assert(events[0] == 'D');
        assert(wdt_feeds == 0 && delay_calls == 0);
    }
}

int main(void) {
    /* 1. Happy path: demote runs exactly once, result gates success. */
    ready_to_demote();
    assert(image2_flash_demote(&b));
    assert(demote_calls == 1);
    assert(!memcmp(s.mem + 264, "01958711", 8));
    check_marker_only_with_flag(
#ifdef LT_AMB1_POWERLOSS_TEST
        1
#else
        0
#endif
    );
    assert(s.writes == 1);              /* publication only; demote used ops.demote */

    /* 2. A failing demote still runs (and was still announced) but fails closed. */
    ready_to_demote();
    s.demote_ret = 0;
    assert(!image2_flash_demote(&b));
    assert(demote_calls == 1);
#ifdef LT_AMB1_POWERLOSS_TEST
    assert(marker_count == 1);
#else
    assert(marker_count == 0);
#endif
    assert(b.failed);

    /* 3. No target publication evidence: the hook must NOT arm and no demote. */
    setup();
    memcpy(s.mem + 264, "81958711", 8);
    memcpy(s.mem + 2056, "01958711", 8);   /* target holds the old/demoted form */
    assert(image2_flash_open(&b, &c, &ops));
    assert(image2_flash_phase(&b, 2));
    assert(image2_flash_phase(&b, 3));
    b.publication_attempted = 1;           /* isolate the evidence read itself */
    assert(!image2_flash_demote(&b));
    assert(marker_count == 0 && demote_calls == 0 && wdt_feeds == 0);

    /* 4. Publication never attempted: same fail-closed result, hook silent. */
    setup();
    memcpy(s.mem + 264, "81958711", 8);
    memcpy(s.mem + 2056, "81958711", 8);
    assert(image2_flash_open(&b, &c, &ops));
    assert(image2_flash_phase(&b, 2));
    assert(image2_flash_phase(&b, 3));
    assert(!image2_flash_demote(&b));
    assert(marker_count == 0 && demote_calls == 0 && wdt_feeds == 0);

    /* 5. Source signature missing: hook silent even with target evidence. */
    setup();
    memcpy(s.mem + 2056, "81958711", 8);
    assert(image2_flash_open(&b, &c, &ops));
    assert(image2_flash_phase(&b, 2));
    assert(image2_flash_phase(&b, 3));
    b.publication_attempted = 1;
    assert(!image2_flash_demote(&b));
    assert(marker_count == 0 && demote_calls == 0 && wdt_feeds == 0);

    /* 6. Demote not allowed by policy: hook silent. */
    ready_to_demote();
    c.allow_source_demote = 0;
    b.config.allow_source_demote = 0;
    assert(!image2_flash_demote(&b));
    assert(marker_count == 0 && demote_calls == 0 && wdt_feeds == 0);
    return 0;
}
'''


class Image2PowerlossHookTests(unittest.TestCase):
    def _build_and_run(self, define_hook):
        preferred_tmp = pathlib.Path(
            "/private/var/folders/4q/bvwkg_397v102xfwcpxgdymc0000gn/T/opencode"
        )
        with tempfile.TemporaryDirectory(
            prefix="image2-powerloss-",
            dir=preferred_tmp if preferred_tmp.is_dir() else None,
        ) as directory:
            path = pathlib.Path(directory)
            source = path / "powerloss.c"
            source.write_text(HARNESS)
            executable = path / "powerloss"
            command = ["cc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                       "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                       "-I", str(OTA)]
            if define_hook:
                command.append("-DLT_AMB1_POWERLOSS_TEST=1")
            command += [str(source), str(OTA / "image2_flash_backend.c"),
                        "-o", str(executable)]
            subprocess.run(command, check=True)
            subprocess.run([str(executable)], check=True)

    def test_marker_then_single_demote_with_flag(self):
        self._build_and_run(define_hook=True)

    def test_hook_compiled_out_without_flag(self):
        self._build_and_run(define_hook=False)

    def test_flag_is_never_defined_by_the_build(self):
        # The hook must be absent from every normal build: nothing may default
        # the macro on, and the OTA sources may only test it, never define it.
        offenders = []
        for relative in ("lt_defs.h", "base/lt_defs.h"):
            candidate = ROOT / relative
            if candidate.is_file() and "LT_AMB1_POWERLOSS_TEST" in candidate.read_text():
                offenders.append(str(candidate))
        builder = ROOT.parents[1] / "builder/family/realtek-amb1.py"
        if builder.is_file() and "LT_AMB1_POWERLOSS_TEST" in builder.read_text():
            offenders.append(str(builder))
        for source in OTA.glob("*.c"):
            for line in source.read_text().splitlines():
                stripped = line.strip()
                if stripped.startswith("#define") and "LT_AMB1_POWERLOSS_TEST" in stripped:
                    offenders.append("%s: %s" % (source, stripped))
        self.assertEqual(offenders, [])


if __name__ == "__main__":
    unittest.main()
