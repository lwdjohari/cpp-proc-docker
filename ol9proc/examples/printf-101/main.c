/*
 * Real-World printf/snprintf Reference Demos
 *
 * Build (C99+):  gcc -std=c99 -Wall -Wextra -O2 demo_printf_snprintf.c -o demo
 * Run:           ./demo
 *
 * Print-101 is quick crash course for handling print in C:
 * - How to use snprintf with an advancing cursor (pos) safely
 * - How to detect truncation and avoid overflows
 * - How to cap %s with precision to avoid log spam
 * - How to parse with sscanf + %n to advance a read cursor
 * - How to two-pass size with snprintf(NULL,0,...) and then allocate
 * - How to format binary data as hex for debugging/telemetry
 *
 * NOTE: Currency & digit grouping (e.g., "' flag or strfmon) are NOT ISO C portable.
 *       Keep them out of portable codebases or hide behind feature flags.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

/* ----------------------------------------------------------------------
 * 1) Structured log line (safe & machine-parseable)
 *
 * Real-world use:
 *   You need a single log line that backends (ELK/Splunk/Grafana) can parse,
 *   while being robust against long user fields. This uses:
 *     - one buffer + cursor (pos),
 *     - snprintf return checks for truncation,
 *     - precision on %s to cap untrusted/long strings.
 *
 * Purpose:
 *   - Incremental building with bounds
 *   - Defensive %s with precision (%.Ns)
 *   - Clear key=value style for machine parsing
 * ---------------------------------------------------------------------- */
void log_line_structured_demo(void)
{
    char line[256];
    size_t pos = 0, cap = sizeof line;
    int n;

    // Timestamp: in real systems you’d format ISO-8601; here we keep it simple.
    n = snprintf(line + pos, cap - pos, "ts=%ld ", (long)time(NULL));
    if (n < 0 || (size_t)n >= cap - pos)
        goto trunc;
    pos += (size_t)n;

    n = snprintf(line + pos, cap - pos, "level=%s ", "INFO");
    if (n < 0 || (size_t)n >= cap - pos)
        goto trunc;
    pos += (size_t)n;

    // User might be long; cap it to 64 chars to avoid giant log lines.
    const char *user = "alice@example.com";
    n = snprintf(line + pos, cap - pos, "user=%.64s ", user);
    if (n < 0 || (size_t)n >= cap - pos)
        goto trunc;
    pos += (size_t)n;

    // Code = numeric for easy machine queries.
    n = snprintf(line + pos, cap - pos, "code=%d", 200);
    if (n < 0 || (size_t)n >= cap - pos)
        goto trunc;
    pos += (size_t)n;

    printf("[structured_log] %s\n", line);
    return;

trunc:
    // In production: emit a clear diagnostic or switch to a bigger buffer.
    puts("[structured_log] TRUNCATED");
}

/* ----------------------------------------------------------------------
 * 2) HTTP request header builder (client-side)
 *
 * Real-world use:
 *   Build an HTTP request on an embedded device or a low-level client without
 *   higher-level libs. Headers must be exact and CRLF-terminated.
 *
 * Purpose:
 *   - Safe multi-line construction
 *   - Knowing where to put CRLF
 *   - Keeping a single cursor and checking bounds
 * ---------------------------------------------------------------------- */
void http_request_builder_demo(void)
{
    char req[512];
    size_t pos = 0, cap = sizeof req;
    int n;

    const char *host = "api.example.com";
    const char *path = "/v1/items";
    const char *token = "abc123";

    n = snprintf(req + pos, cap - pos, "GET %s HTTP/1.1\r\n", path);
    if (n < 0 || (size_t)n >= cap - pos)
        goto trunc;
    pos += (size_t)n;

    n = snprintf(req + pos, cap - pos, "Host: %s\r\n", host);
    if (n < 0 || (size_t)n >= cap - pos)
        goto trunc;
    pos += (size_t)n;

    // Optional auth header (e.g., bearer token)
    n = snprintf(req + pos, cap - pos, "Authorization: Bearer %s\r\n", token);
    if (n < 0 || (size_t)n >= cap - pos)
        goto trunc;
    pos += (size_t)n;

    // Terminate headers with a blank line
    n = snprintf(req + pos, cap - pos, "\r\n");
    if (n < 0 || (size_t)n >= cap - pos)
        goto trunc;
    pos += (size_t)n;

    printf("[http_request]\n%s", req);
    return;

trunc:
    puts("[http_request] TRUNCATED");
}

/* ----------------------------------------------------------------------
 * 3) Binary hex dump (debug/telemetry/output framing)
 *
 * Real-world use:
 *   You want to log or display raw bytes (packets, keys, hashes) in hex form
 *   for debugging or audit trails. The %02hhX pattern is the standard.
 *
 * Purpose:
 *   - %02hhX for fixed-width upper-hex bytes
 *   - Buffer cursor pattern for repeated appends
 * ---------------------------------------------------------------------- */
void binary_hexdump_demo(void)
{
    uint8_t buf[12] = {0x48, 0x54, 0x54, 0x50, 0x2F, 0x31, 0x2E, 0x31, 0x00, 0xDE, 0xAD, 0xBE};
    char out[128];
    size_t pos = 0, cap = sizeof out;
    int n;

    for (size_t i = 0; i < sizeof buf; i++)
    {
        n = snprintf(out + pos, cap - pos, "%02hhX%s", buf[i], (i + 1 < sizeof buf) ? " " : "");
        if (n < 0 || (size_t)n >= cap - pos)
        {
            puts("[hexdump] TRUNCATED");
            return;
        }
        pos += (size_t)n;
    }
    printf("[hexdump] %s\n", out);
}

/* ----------------------------------------------------------------------
 * 4) sscanf with %n for parsing (cursor advance)
 *
 * Real-world use:
 *   You have a structured text record (log line, config file, protocol line)
 *   and you want to parse field-by-field while keeping track of how much
 *   you’ve consumed. %n tells you the offset so you can resume from there.
 *
 * Purpose:
 *   - Using %n in *scanf to know how much was read
 *   - Building simple tokenizers without unsafe pointer math
 * ---------------------------------------------------------------------- */
void sscanf_with_position_demo(void)
{
    const char *src = "ID=42 AMT=3.50 CURRENCY=USD END";
    int id, used = 0, used2 = 0;
    double amt;
    char currency[8];

    // Read "ID=<int> AMT=<double> " and capture consumed length in 'used'
    if (sscanf(src, "ID=%d AMT=%lf %n", &id, &amt, &used) == 2)
    {
        // Continue parsing from src + used
        if (sscanf(src + used, "CURRENCY=%7s %n", currency, &used2) == 1)
        {
            printf("[parse] id=%d amt=%.2f currency=%s next='%s'\n",
                   id, amt, currency, src + used + used2);
        }
        else
        {
            puts("[parse] failed reading currency");
        }
    }
    else
    {
        puts("[parse] failed reading id/amt");
    }
}

/* ----------------------------------------------------------------------
 * 5) Two-pass dynamic allocation with snprintf(NULL,0,...)
 *
 * Real-world use:
 *   When you don’t know the final length (user input, variable lists),
 *   call snprintf(NULL,0,...) to compute the required size, then allocate
 *   exactly once, then print. No guessing, no reallocation loops.
 *
 * Purpose:
 *   - Correct, portable, allocation-sized-by-format pattern
 *   - No manual '\0' needed; snprintf handles it (n>0)
 * ---------------------------------------------------------------------- */
void snprintf_dynamic_alloc_demo(void)
{
    const char *name = "widget-pro-v2";
    int qty = 7;
    double price = 1234.5;

    // First pass: compute size (excluding '\0')
    int need = snprintf(NULL, 0, "name=%s qty=%d price=%.2f", name, qty, price);
    if (need < 0)
    {
        puts("[alloc] size calc failed");
        return;
    }

    char *msg = (char *)malloc((size_t)need + 1);
    if (!msg)
    {
        puts("[alloc] OOM");
        return;
    }

    // Second pass: generate string
    (void)snprintf(msg, (size_t)need + 1, "name=%s qty=%d price=%.2f", name, qty, price);

    printf("[alloc] %s\n", msg);
    free(msg);
}

/* ----------------------------------------------------------------------
 * 6) CSV line builder (quote + sanitize)
 *
 * Real-world use:
 *   You emit CSV for downstream processing. CSV fields with commas or quotes
 *   must be quoted and internal quotes doubled. This is a minimal demo; in
 *   production, wrap quoting in a helper.
 *
 * Purpose:
 *   - Safe CSV output basics
 *   - Buffer cursor + truncation checks
 * ---------------------------------------------------------------------- */
static int csv_quote_into(char *dst, size_t cap, const char *src)
{
    // Writes a CSV-quoted field into dst (no trailing comma).
    // Returns number of characters written, or -1 on truncation.
    size_t pos = 0;
    int n;

    if (cap == 0)
        return -1;
    dst[pos++] = '"';
    for (const char *p = src; *p; ++p)
    {
        if (*p == '"')
        {
            n = snprintf(dst + pos, cap - pos, "\"\""); // double quotes
        }
        else
        {
            n = snprintf(dst + pos, cap - pos, "%c", *p);
        }
        if (n < 0 || (size_t)n >= cap - pos)
            return -1;
        pos += (size_t)n;
    }
    if (pos >= cap)
        return -1;
    dst[pos++] = '"';
    if (pos >= cap)
        return -1;
    dst[pos] = '\0';
    return (int)pos;
}

void csv_line_builder_demo(void)
{
    char line[256];
    size_t pos = 0, cap = sizeof line;
    int n;

    const char *name = "Alice";
    const char *city = "New York, USA"; // contains comma -> must quote
    int age = 30;

    // name
    n = csv_quote_into(line + pos, cap - pos, name);
    if (n < 0)
    {
        puts("[csv] TRUNCATED");
        return;
    }
    pos += (size_t)n;

    // comma
    n = snprintf(line + pos, cap - pos, ",");
    if (n < 0 || (size_t)n >= cap - pos)
    {
        puts("[csv] TRUNCATED");
        return;
    }
    pos += (size_t)n;

    // city
    n = csv_quote_into(line + pos, cap - pos, city);
    if (n < 0)
    {
        puts("[csv] TRUNCATED");
        return;
    }
    pos += (size_t)n;

    // comma + age
    n = snprintf(line + pos, cap - pos, ",%d", age);
    if (n < 0 || (size_t)n >= cap - pos)
    {
        puts("[csv] TRUNCATED");
        return;
    }
    pos += (size_t)n;

    printf("[csv] %s\n", line);
}

/* ----------------------------------------------------------------------
 * MAIN: run all demos
 * ---------------------------------------------------------------------- */
int main(void)
{
    puts("== real-world printf/snprintf demos ==");
    log_line_structured_demo();
    http_request_builder_demo();
    binary_hexdump_demo();
    sscanf_with_position_demo();
    snprintf_dynamic_alloc_demo();
    csv_line_builder_demo();
    return 0;
}
