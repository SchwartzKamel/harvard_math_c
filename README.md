# harvard_math

A small, importable C99 library for introductory descriptive statistics.

The normal caller experience is intentionally short:

```c
#include <harvard_math/harvard_math.h>

const double values[] = {2.0, 4.0, 4.0, 8.0};
double mean;

if (hm_mean(values, 4, &mean) == HM_STATUS_OK) {
    /* mean == 4.5 */
}
```

Every built-in statistic is a plain function with the same shape as `hm_statistic_fn`, so a custom statistic is just another C function—no registration or library-owned data structure is needed.

## Requirements

- C99 compiler
- CMake 3.16 or newer
- Ninja (recommended; another CMake generator also works)

The library has no third-party runtime dependencies. CMake links the platform math library when required.

## Build and test

```bash
cmake -S . -B build -G Ninja -DHM_BUILD_TESTS=ON -DHM_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/examples/harvard_math_example
```

The example prints:

```text
mean=4.50 q1=3.50 median=4.00 q3=5.00 sample_stddev=2.516611 zscore=1.390759 slope=2.00 ztest=2.00 p=0.045500 ci=[96.080072,103.919928] intercept=0.00 correlation=1.00 covariance=3.33 prediction=10.00 multiple_prediction=-1.00 midrange=5.00
```

Use `BUILD_SHARED_LIBS=ON` for a shared build; the default is a static library.

### Make convenience wrapper

The `Makefile` is intentionally thin: it delegates all compilation and installation to CMake, using Ninja by default. This keeps one source of build truth while making the common commands short:

```bash
make test
make example
make package-test
make install PREFIX="$HOME/.local"
```

Override the generator or build directory when needed:

```bash
make BUILD_DIR=build-shared BUILD_SHARED_LIBS=ON test
make BUILD_DIR=build-make GENERATOR="Unix Makefiles" test
```

### Benchmark

Run the dependency-free benchmark comparison with:

```bash
make benchmark
```

This builds an optimized Release benchmark, generates one deterministic finite `double` dataset, checks that the library and handwritten C reference produce equivalent results, then prints timings and relative speedups. Machine-readable results are written to `build/benchmark.csv`.

The benchmark is a local measurement, not a universal performance guarantee. It records the compiler, build type, input size, warmups, and repetitions so results can be compared responsibly across machines. With GNU Scientific Library 2.8+ available through pkg-config, add `GSL=ON` to compare semantically matching GSL operations:

```bash
PKG_CONFIG_PATH=/path/to/gsl/lib/pkgconfig make benchmark GSL=ON
```

GSL is not required for the default benchmark; missing GSL is reported as an optional peer limitation. Paired covariance timings are emitted by the companion `pair-benchmark.csv` artifact.

#### Performance snapshot

The following snapshot was recorded on 2026-09-06 from the optimized implementation at `7a48028`:

```text
CPU:       Intel Core i9-14900HX, WSL2
Compiler:  GCC 11.4
Build:     CMake Release, -O3 -DNDEBUG, Ninja
Dataset:   1,048,576 deterministic finite doubles
Timing:    CPU 2 pinned with taskset, 10 warmups, 50 repetitions, median of 5 process runs
```

The handwritten reference uses the same checked-call shape and input data. Speedup means
`reference time / harvard_math time`; values above `1.0x` mean `harvard_math` was faster.

| Operation | Handwritten C (µs) | `harvard_math` (µs) | Speedup |
|---|---:|---:|---:|
| sum | 691.750 | 387.809 | 1.78x |
| mean | 683.025 | 382.085 | 1.76x |
| min | 1,081.074 | 764.715 | 1.40x |
| max | 1,056.311 | 759.473 | 1.39x |
| range | 1,059.358 | 762.258 | 1.37x |
| median | 58,302.725 | 3,468.434 | 16.89x |
| population variance | 1,107.122 | 1,095.203 | 1.04x |
| sample variance | 1,078.041 | 1,127.175 | 1.01x |
| population stddev | 1,093.357 | 1,095.025 | 1.00x |
| sample stddev | 1,074.453 | 1,119.122 | 0.96x |

A GSL 2.8 peer build using the same `-O3 -DNDEBUG` flags produced these matching-operation results:

| Operation | `harvard_math` (µs) | GSL (µs) | `harvard_math` / GSL |
|---|---:|---:|---:|
| mean | 383.589 | 4,606.966 | 0.08x |
| min | 778.465 | 1,087.150 | 0.72x |
| max | 762.521 | 1,055.470 | 0.73x |
| range | 786.550 | 1,054.188 | 0.75x |
| median | 3,445.003 | 2,720.537 | 1.29x |
| sample variance | 1,092.129 | 8,846.405 | 0.12x |
| sample stddev | 1,072.336 | 8,886.263 | 0.12x |

These numbers are evidence for this CPU/compiler/build, not a universal guarantee. GSL has different stride, validation, and median-mutation semantics; the adapter reconciles those differences and intentionally leaves sum and population variance out of the GSL table.

To reproduce the default snapshot's shape on Linux:

```bash
make clean
make benchmark
taskset -c 2 build/bench/harvard_math_benchmark \
    --size 1048576 --warmups 10 --repetitions 50 \
    --csv /tmp/harvard_math-benchmark.csv
```

## Install and import from another CMake project

Install to a prefix:

```bash
cmake -S . -B build -G Ninja -DHM_BUILD_TESTS=ON
cmake --build build
cmake --install build --prefix "$HOME/.local"
```

A downstream project's `CMakeLists.txt` can then use the exported target:

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_statistics_app LANGUAGES C)

find_package(harvard_math CONFIG REQUIRED)

add_executable(my_statistics_app main.c)
target_link_libraries(my_statistics_app PRIVATE harvard_math::harvard_math)
```

Configure that project with `-DCMAKE_PREFIX_PATH="$HOME/.local"`. Its source only needs:

```c
#include <harvard_math/harvard_math.h>
```

For non-CMake consumers, installation also provides `harvard_math.pc`:

```bash
PKG_CONFIG_PATH="$HOME/.local/lib/pkgconfig" \
    pkg-config --cflags --libs harvard_math
```

## Initial API

All statistic functions return `hm_status` and write their result to `double *out` only on success. `hm_status_message` converts a status to readable text.

| Function | Meaning | Minimum input |
|---|---|---:|
| `hm_sum` | Sum of all values | 1 |
| `hm_mean` | Arithmetic mean | 1 |
| `hm_zscore` | Standardized value `(value - mean) / stddev` | scalar |
| `hm_min` / `hm_max` | Smallest / largest value | 1 |
| `hm_range` | `max - min` | 1 |
| `hm_median` | Middle value, averaging the two middle values when needed | 1 |
| `hm_quantile` | Linearly interpolated order statistic for `q` in `[0,1]` | 1 |
| `hm_variance_population` | Squared deviations divided by `n` | 1 |
| `hm_variance_sample` | Squared deviations divided by `n - 1` | 2 |
| `hm_stddev_population` | Square root of population variance | 1 |
| `hm_stddev_sample` | Square root of sample variance | 2 |
| `hm_slope` | Ordinary least-squares slope for paired `x`/`y` values | 2 |

Population and sample functions are deliberately named separately. There is no hidden default divisor.

### Status contract

- `HM_STATUS_OK` — result was written.
- `HM_STATUS_INVALID_ARGUMENT` — null output, or null values with a positive count.
- `HM_STATUS_EMPTY_INPUT` — `count` is zero.
- `HM_STATUS_INSUFFICIENT_DATA` — a sample statistic was requested with fewer than two values.
- `HM_STATUS_NONFINITE_INPUT` — at least one value is `NaN` or infinite.
- `HM_STATUS_ALLOCATION_FAILED` — the median's temporary copy could not be allocated.
- `HM_STATUS_UNDEFINED` — the requested mathematical result is undefined for the supplied values, such as zero z-score standard deviation or constant regression `x` values.

Use `hm_status_message(status)` for a stable human-readable description.

## School-core formulas

The five school-level operations are available through the umbrella header:

- `hm_mean(values, count, &mean)`
- `hm_stddev_population(values, count, &stddev)`
- `hm_stddev_sample(values, count, &stddev)`
- `hm_zscore(value, mean, stddev, &zscore)`
- `hm_slope(x, y, count, &slope)`

The z-score is `(value - mean) / stddev`. The slope is the ordinary least-squares slope:

```text
slope = sum((xᵢ - x̄) * (yᵢ - ȳ)) / sum((xᵢ - x̄)^2)
```

A zero standard deviation makes a z-score undefined. A constant predictor array makes a slope undefined. Both cases return `HM_STATUS_UNDEFINED` without writing the output.

## Quantiles and quartiles

`hm_quantile` uses `h=(n-1)q` linear interpolation. Use `q=0.25`, `0.50`, and `0.75` for Q1, the median convention, and Q3:

```c
double q1;
double median;
double q3;
hm_quantile(values, count, 0.25, &q1);
hm_quantile(values, count, 0.50, &median);
hm_quantile(values, count, 0.75, &q3);
```

The input remains unchanged. Pass a fraction rather than a percentile number: use `0.75`, not `75`.

## Inferential and regression extensions

Known-population-standard-deviation z-tests are available for one or two sample means:

```c
hm_ztest_result result;
hm_ztest_one_sample(
    104.0, 100.0, 10.0, 25,
    HM_ALTERNATIVE_TWO_SIDED,
    &result
);
/* result.z_score == 2.0; result.p_value ~= 0.0455 */
```

Known-σ confidence intervals use a confidence fraction such as `0.95`:

```c
hm_confidence_interval interval;
hm_mean_confidence_interval(100.0, 10.0, 25, 0.95, &interval);
```

Simple paired-data analysis exposes `hm_slope`, `hm_intercept`, `hm_correlation`, `hm_covariance_population`, `hm_covariance_sample`, and `hm_predict`. Multiple regression accepts a row-major predictor matrix with an automatic intercept:

```c
/* predictors contains rows * predictor_count values. */
double coefficients[3];
hm_multiple_regression_fit(predictors, rows, 2, responses, coefficients);
hm_multiple_predict(coefficients, 2, new_predictors, &prediction);
```

Multiple-regression coefficients are ordered as `[intercept, beta_1, ..., beta_p]`. Rank-deficient matrices and mathematically undefined results return `HM_STATUS_UNDEFINED`. This is an educational small-matrix API, not a replacement for a general numerical modeling package.

## Defining a custom statistic

Use the public callback type and compose existing statistics when useful:

```c
static hm_status midrange(const double *values, size_t count, double *out)
{
    double minimum;
    double maximum;
    hm_status status = hm_min(values, count, &minimum);

    if (status != HM_STATUS_OK) {
        return status;
    }
    status = hm_max(values, count, &maximum);
    if (status != HM_STATUS_OK) {
        return status;
    }
    *out = minimum / 2.0 + maximum / 2.0;
    return HM_STATUS_OK;
}

hm_statistic_fn statistic = midrange;
double result;
if (statistic(values, count, &result) == HM_STATUS_OK) {
    /* use result */
}
```

The custom function owns no library memory and can live in the consuming application or in a future module under `src/`.

## Directory layout

```text
include/harvard_math/       public, installed headers
    harvard_math.h           one-header entry point
    stats.h                  descriptive-statistics declarations
src/                         implementation only
tests/                       CTest unit tests
    package_consumer/        separate installed-package consumer
examples/                    runnable usage examples
cmake/                       CMake package and pkg-config templates
Makefile                     thin CMake/Ninja convenience wrapper
docs/                        design, extension notes, and audit trail
specs/                       project requirements and engineering principles
scenarios/                   black-box acceptance journeys
```

Repository engineering references:

- [`AGENTS.md`](AGENTS.md) — build, validation, and codebase conventions.
- [`IMPLEMENTATION_PLAN.md`](IMPLEMENTATION_PLAN.md) — task state for the current scaffold.
- [`docs/README.md`](docs/README.md) — documentation index.
- [`docs/design.md`](docs/design.md) — API rationale, algorithms, extension rules, and benchmark boundaries.
- [`specs/`](specs/) and [`scenarios/`](scenarios/) — requirements and holdout acceptance journeys.

See [`docs/design.md`](docs/design.md) for the API rationale, error boundaries, algorithms, and rules for adding future statistical modules.
