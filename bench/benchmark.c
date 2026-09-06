#if !defined(_WIN32)
#    define _POSIX_C_SOURCE 200809L
#endif

#include "reference_stats.h"

#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#    include <windows.h>
#endif

#ifndef HM_BENCHMARK_BUILD_TYPE
#    define HM_BENCHMARK_BUILD_TYPE "unknown"
#endif

struct benchmark_case {
    const char *name;
    hm_statistic_fn library_statistic;
    hm_statistic_fn reference_statistic;
};

struct benchmark_result {
    double reference_seconds;
    double library_seconds;
    double reference_value;
    double library_value;
};

static const char *compiler_name(void)
{
#if defined(__clang__)
    return "Clang " __clang_version__;
#elif defined(__GNUC__)
    return "GCC " __VERSION__;
#elif defined(_MSC_VER)
    return "MSVC";
#else
    return "unknown compiler";
#endif
}

static double monotonic_seconds(void)
{
#if defined(_WIN32)
    static LARGE_INTEGER frequency;
    static int initialized;
    LARGE_INTEGER counter;

    if (!initialized) {
        if (!QueryPerformanceFrequency(&frequency)) {
            fprintf(stderr, "QueryPerformanceFrequency failed\n");
            exit(EXIT_FAILURE);
        }
        initialized = 1;
    }
    if (!QueryPerformanceCounter(&counter)) {
        fprintf(stderr, "QueryPerformanceCounter failed\n");
        exit(EXIT_FAILURE);
    }
    return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec timestamp;

    if (clock_gettime(CLOCK_MONOTONIC, &timestamp) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (double)timestamp.tv_sec + (double)timestamp.tv_nsec / 1e9;
#endif
}

static int parse_size(const char *text, size_t *out)
{
    unsigned long long value;
    char *end;

    errno = 0;
    value = strtoull(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value == 0 ||
        value > (unsigned long long)(size_t)-1) {
        return 0;
    }
    *out = (size_t)value;
    return 1;
}

static void print_usage(const char *program)
{
    printf("Usage: %s [--size N] [--repetitions N] [--warmups N] [--csv PATH]\n", program);
}

static double tolerance_for(double expected)
{
    return 1e-9 * fmax(1.0, fabs(expected));
}

static double run_statistic(hm_statistic_fn statistic, const double *values,
                            size_t count, size_t warmups, size_t repetitions,
                            double *last_value)
{
    volatile double checksum = 0.0;
    double result = 0.0;
    double start;

    for (size_t i = 0; i < warmups; ++i) {
        if (statistic(values, count, &result) != HM_STATUS_OK) {
            fprintf(stderr, "benchmark statistic returned an error\n");
            exit(EXIT_FAILURE);
        }
    }
    start = monotonic_seconds();
    for (size_t i = 0; i < repetitions; ++i) {
        if (statistic(values, count, &result) != HM_STATUS_OK) {
            fprintf(stderr, "benchmark statistic returned an error\n");
            exit(EXIT_FAILURE);
        }
        checksum += result;
    }
    if (checksum == -1.0) {
        fprintf(stderr, "unreachable checksum\n");
    }
    *last_value = result;
    return (monotonic_seconds() - start) / (double)repetitions;
}

static struct benchmark_result run_case(const struct benchmark_case *test_case,
                                         const double *values, size_t count,
                                         size_t warmups, size_t repetitions)
{
    struct benchmark_result result;
    hm_status library_status;
    hm_status reference_status;
    double difference;

    library_status = test_case->library_statistic(values, count, &result.library_value);
    reference_status = test_case->reference_statistic(values, count, &result.reference_value);
    if (library_status != HM_STATUS_OK || reference_status != HM_STATUS_OK) {
        fprintf(stderr, "%s returned status library=%s reference=%s\n",
                test_case->name, hm_status_message(library_status),
                hm_status_message(reference_status));
        exit(EXIT_FAILURE);
    }
    difference = fabs(result.library_value - result.reference_value);
    if (difference > tolerance_for(result.reference_value)) {
        fprintf(stderr, "%s result mismatch: library=%.17g reference=%.17g\n",
                test_case->name, result.library_value, result.reference_value);
        exit(EXIT_FAILURE);
    }

    result.reference_seconds = run_statistic(
        test_case->reference_statistic, values, count, warmups, repetitions,
        &result.reference_value);
    result.library_seconds = run_statistic(
        test_case->library_statistic, values, count, warmups, repetitions,
        &result.library_value);
    return result;
}

static void print_human_result(const struct benchmark_case *test_case,
                               size_t count, const struct benchmark_result *result)
{
    const double reference_microseconds = result->reference_seconds * 1e6;
    const double library_microseconds = result->library_seconds * 1e6;

    printf("%-24s %-10zu %12.3f %15.3f %8.2fx\n",
           test_case->name, count, reference_microseconds, library_microseconds,
           result->reference_seconds / result->library_seconds);
}

static void write_csv_result(FILE *csv, const struct benchmark_case *test_case,
                             size_t count, size_t warmups, size_t repetitions,
                             const struct benchmark_result *result)
{
    fprintf(csv, "%s,%zu,%zu,%zu,%.9g,%.9g,%.9g,%.17g,%.17g\n",
            test_case->name, count, warmups, repetitions,
            result->reference_seconds, result->library_seconds,
            result->reference_seconds / result->library_seconds,
            result->reference_value, result->library_value);
}

int main(int argc, char **argv)
{
    const struct benchmark_case cases[] = {
        {"sum", hm_sum, hm_reference_sum},
        {"mean", hm_mean, hm_reference_mean},
        {"min", hm_min, hm_reference_min},
        {"max", hm_max, hm_reference_max},
        {"range", hm_range, hm_reference_range},
        {"median", hm_median, hm_reference_median},
        {"variance_population", hm_variance_population, hm_reference_variance_population},
        {"variance_sample", hm_variance_sample, hm_reference_variance_sample},
        {"stddev_population", hm_stddev_population, hm_reference_stddev_population},
        {"stddev_sample", hm_stddev_sample, hm_reference_stddev_sample}
    };
    const size_t case_count = sizeof cases / sizeof cases[0];
    size_t count = 1U << 20;
    size_t repetitions = 7;
    size_t warmups = 3;
    const char *csv_path = NULL;
    double *values;
    FILE *csv = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            if (!parse_size(argv[++i], &count)) {
                fprintf(stderr, "invalid --size value\n");
                return EXIT_FAILURE;
            }
            continue;
        }
        if (strcmp(argv[i], "--repetitions") == 0 && i + 1 < argc) {
            if (!parse_size(argv[++i], &repetitions)) {
                fprintf(stderr, "invalid --repetitions value\n");
                return EXIT_FAILURE;
            }
            continue;
        }
        if (strcmp(argv[i], "--warmups") == 0 && i + 1 < argc) {
            if (!parse_size(argv[++i], &warmups)) {
                fprintf(stderr, "invalid --warmups value\n");
                return EXIT_FAILURE;
            }
            continue;
        }
        if (strcmp(argv[i], "--csv") == 0 && i + 1 < argc) {
            csv_path = argv[++i];
            continue;
        }
        fprintf(stderr, "unknown or incomplete argument: %s\n", argv[i]);
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (count > (size_t)-1 / sizeof *values) {
        fprintf(stderr, "dataset is too large\n");
        return EXIT_FAILURE;
    }
    values = malloc(count * sizeof *values);
    if (values == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < count; ++i) {
        values[i] = sin((double)i * 0.001) + (double)(i % 17) * 0.01;
    }

    if (csv_path != NULL) {
        csv = fopen(csv_path, "w");
        if (csv == NULL) {
            perror(csv_path);
            free(values);
            return EXIT_FAILURE;
        }
        fprintf(csv, "operation,count,warmups,repetitions,reference_seconds,library_seconds,speedup,reference_result,library_result\n");
    }

    printf("harvard_math benchmark\n");
    printf("compiler: %s\n", compiler_name());
    printf("build_type: %s\n", HM_BENCHMARK_BUILD_TYPE);
    printf("count: %zu, warmups: %zu, repetitions: %zu\n", count, warmups, repetitions);
    printf("Results are checked against the reference before timing.\n\n");
    printf("%-24s %-10s %12s %15s %8s\n",
           "operation", "count", "reference us", "harvard_math us", "speedup");
    printf("--------------------------------------------------------------------------\n");

    for (size_t i = 0; i < case_count; ++i) {
        const struct benchmark_result result = run_case(
            &cases[i], values, count, warmups, repetitions);
        print_human_result(&cases[i], count, &result);
        if (csv != NULL) {
            write_csv_result(csv, &cases[i], count, warmups, repetitions, &result);
        }
    }

    if (csv != NULL) {
        fclose(csv);
        printf("\nCSV results: %s\n", csv_path);
    }
    free(values);
    return EXIT_SUCCESS;
}
