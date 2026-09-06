#if !defined(_WIN32)
#    define _POSIX_C_SOURCE 200809L
#endif

#include <harvard_math/regression.h>

#ifdef HM_HAVE_GSL
#    include "gsl_adapter.h"
#endif

#include <math.h>
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

typedef hm_status (*pair_statistic_fn)(const double *, const double *, size_t, double *);

struct pair_case {
    const char *name;
    pair_statistic_fn library_statistic;
    pair_statistic_fn reference_statistic;
#ifdef HM_HAVE_GSL
    pair_statistic_fn peer_statistic;
#endif
};

static double seconds_now(void)
{
#if defined(_WIN32)
    static LARGE_INTEGER frequency;
    static int initialized;
    LARGE_INTEGER counter;

    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = 1;
    }
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return (double)timestamp.tv_sec + (double)timestamp.tv_nsec / 1e9;
#endif
}

static hm_status reference_covariance(const double *x, const double *y,
                                      size_t count, int sample, double *out)
{
    double mean_x = 0.0;
    double mean_y = 0.0;
    double cross = 0.0;

    if (out == NULL || x == NULL || y == NULL) {
        return HM_STATUS_INVALID_ARGUMENT;
    }
    if (count == 0) {
        return HM_STATUS_EMPTY_INPUT;
    }
    for (size_t i = 0; i < count; ++i) {
        if (!isfinite(x[i]) || !isfinite(y[i])) {
            return HM_STATUS_NONFINITE_INPUT;
        }
        mean_x += x[i];
        mean_y += y[i];
    }
    if (sample && count < 2) {
        return HM_STATUS_INSUFFICIENT_DATA;
    }
    mean_x /= (double)count;
    mean_y /= (double)count;
    for (size_t i = 0; i < count; ++i) {
        cross += (x[i] - mean_x) * (y[i] - mean_y);
    }
    *out = cross / (double)(sample ? count - 1 : count);
    return HM_STATUS_OK;
}

static hm_status reference_covariance_population(const double *x, const double *y,
                                                 size_t count, double *out)
{
    return reference_covariance(x, y, count, 0, out);
}

static hm_status reference_covariance_sample(const double *x, const double *y,
                                             size_t count, double *out)
{
    return reference_covariance(x, y, count, 1, out);
}

static double run_pair(pair_statistic_fn statistic, const double *x, const double *y,
                       size_t count, size_t warmups, size_t repetitions, double *last)
{
    volatile double checksum = 0.0;
    double result = 0.0;
    double start;

    for (size_t i = 0; i < warmups; ++i) {
        if (statistic(x, y, count, &result) != HM_STATUS_OK) {
            fprintf(stderr, "paired statistic failed during warmup\n");
            exit(EXIT_FAILURE);
        }
    }
    start = seconds_now();
    for (size_t i = 0; i < repetitions; ++i) {
        if (statistic(x, y, count, &result) != HM_STATUS_OK) {
            fprintf(stderr, "paired statistic failed during timing\n");
            exit(EXIT_FAILURE);
        }
        checksum += result;
    }
    if (checksum == -1.0) {
        fprintf(stderr, "unreachable checksum\n");
    }
    *last = result;
    return (seconds_now() - start) / (double)repetitions;
}

static void run_case(const struct pair_case *test_case, const double *x, const double *y,
                     size_t count, size_t warmups, size_t repetitions, FILE *csv)
{
    double reference_value;
    double library_value;
    double reference_seconds;
    double library_seconds;
    double tolerance;
#ifdef HM_HAVE_GSL
    double peer_value;
    double peer_seconds;
#endif

    if (test_case->library_statistic(x, y, count, &library_value) != HM_STATUS_OK ||
        test_case->reference_statistic(x, y, count, &reference_value) != HM_STATUS_OK) {
        fprintf(stderr, "%s initial result failed\n", test_case->name);
        exit(EXIT_FAILURE);
    }
    tolerance = 1e-9 * fmax(1.0, fabs(reference_value));
    if (fabs(library_value - reference_value) > tolerance) {
        fprintf(stderr, "%s mismatch: library=%.17g reference=%.17g\n",
                test_case->name, library_value, reference_value);
        exit(EXIT_FAILURE);
    }
    reference_seconds = run_pair(test_case->reference_statistic, x, y, count,
                                 warmups, repetitions, &reference_value);
    library_seconds = run_pair(test_case->library_statistic, x, y, count,
                               warmups, repetitions, &library_value);
#ifdef HM_HAVE_GSL
    if (test_case->peer_statistic != NULL) {
        if (test_case->peer_statistic(x, y, count, &peer_value) != HM_STATUS_OK ||
            fabs(peer_value - reference_value) > tolerance) {
            fprintf(stderr, "%s GSL result mismatch\n", test_case->name);
            exit(EXIT_FAILURE);
        }
        peer_seconds = run_pair(test_case->peer_statistic, x, y, count,
                                warmups, repetitions, &peer_value);
    }
#endif

#ifdef HM_HAVE_GSL
    if (test_case->peer_statistic != NULL) {
        printf("%-24s %12.3f %15.3f %8.2fx %12.3f %8.2fx\n",
               test_case->name, reference_seconds * 1e6, library_seconds * 1e6,
               reference_seconds / library_seconds, peer_seconds * 1e6,
               library_seconds / peer_seconds);
    } else
#endif
    {
        printf("%-24s %12.3f %15.3f %8.2fx\n", test_case->name,
               reference_seconds * 1e6, library_seconds * 1e6,
               reference_seconds / library_seconds);
    }
    fprintf(csv, "%s,%.9g,%.9g,%.9g", test_case->name,
            reference_seconds, library_seconds,
            reference_seconds / library_seconds);
#ifdef HM_HAVE_GSL
    if (test_case->peer_statistic != NULL) {
        fprintf(csv, ",%.9g,%.9g", peer_seconds, library_seconds / peer_seconds);
    } else
#endif
    {
        fprintf(csv, ",,");
    }
    fputc('\n', csv);
}

int main(int argc, char **argv)
{
    const struct pair_case cases[] = {
#ifdef HM_HAVE_GSL
        {"covariance_population", hm_covariance_population,
         reference_covariance_population, hm_gsl_covariance_population},
        {"covariance_sample", hm_covariance_sample,
         reference_covariance_sample, hm_gsl_covariance_sample}
#else
        {"covariance_population", hm_covariance_population,
         reference_covariance_population},
        {"covariance_sample", hm_covariance_sample,
         reference_covariance_sample}
#endif
    };
    const size_t count = 1U << 20;
    const size_t warmups = 3;
    const size_t repetitions = 7;
    double *x = malloc(count * sizeof *x);
    double *y = malloc(count * sizeof *y);
    const char *csv_path = argc > 2 && strcmp(argv[1], "--csv") == 0 ? argv[2] : "pair-benchmark.csv";
    FILE *csv;

    if (x == NULL || y == NULL) {
        free(x);
        free(y);
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < count; ++i) {
        x[i] = sin((double)i * 0.001) + (double)(i % 17) * 0.01;
        y[i] = 2.0 * x[i] + cos((double)i * 0.0007);
    }
    csv = fopen(csv_path, "w");
    if (csv == NULL) {
        free(x);
        free(y);
        perror(csv_path);
        return EXIT_FAILURE;
    }
    fprintf(csv, "operation,reference_seconds,library_seconds,speedup,gsl_seconds,library_over_gsl\n");
    printf("harvard_math paired covariance benchmark\n");
    printf("build_type: %s, count: %zu, warmups: %zu, repetitions: %zu\n",
           HM_BENCHMARK_BUILD_TYPE, count, warmups, repetitions);
#ifdef HM_HAVE_GSL
    printf("peer: GNU Scientific Library (GSL)\n");
    printf("%-24s %12s %15s %8s %12s %8s\n",
           "operation", "reference us", "harvard_math us", "hm/ref", "GSL us", "hm/GSL");
#else
    printf("peer: GSL unavailable; rerun with GSL=ON on a GSL-enabled host\n");
    printf("%-24s %12s %15s %8s\n", "operation", "reference us", "harvard_math us", "hm/ref");
#endif
    printf("--------------------------------------------------------------------------\n");
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        run_case(&cases[i], x, y, count, warmups, repetitions, csv);
    }
    fclose(csv);
    free(x);
    free(y);
    printf("CSV results: %s\n", csv_path);
    return EXIT_SUCCESS;
}
