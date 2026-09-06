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
mean=4.50 median=4.00 sample_stddev=2.516611 midrange=5.00
```

Use `BUILD_SHARED_LIBS=ON` for a shared build; the default is a static library.

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

All functions return `hm_status` and write their result to `double *out` only on success.

| Function | Meaning | Minimum input |
|---|---|---:|
| `hm_sum` | Sum of all values | 1 |
| `hm_mean` | Arithmetic mean | 1 |
| `hm_min` / `hm_max` | Smallest / largest value | 1 |
| `hm_range` | `max - min` | 1 |
| `hm_median` | Middle value, averaging the two middle values when needed | 1 |
| `hm_variance_population` | Squared deviations divided by `n` | 1 |
| `hm_variance_sample` | Squared deviations divided by `n - 1` | 2 |
| `hm_stddev_population` | Square root of population variance | 1 |
| `hm_stddev_sample` | Square root of sample variance | 2 |

Population and sample functions are deliberately named separately. There is no hidden default divisor.

### Status contract

- `HM_STATUS_OK` — result was written.
- `HM_STATUS_INVALID_ARGUMENT` — null output, or null values with a positive count.
- `HM_STATUS_EMPTY_INPUT` — `count` is zero.
- `HM_STATUS_INSUFFICIENT_DATA` — a sample statistic was requested with fewer than two values.
- `HM_STATUS_NONFINITE_INPUT` — at least one value is `NaN` or infinite.
- `HM_STATUS_ALLOCATION_FAILED` — the median's temporary copy could not be allocated.

Use `hm_status_message(status)` for a stable human-readable description.

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
docs/                        design and extension notes
```

See [`docs/design.md`](docs/design.md) for the API rationale, error boundaries, algorithms, and rules for adding future statistical modules.
