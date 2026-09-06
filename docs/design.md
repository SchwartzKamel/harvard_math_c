# Library design

## Goals

`harvard_math` is a small, dependency-free C99 library for descriptive statistics. Its main design goal is a short path from a C array to a checked numeric result:

```c
double mean;
hm_status status = hm_mean(values, count, &mean);
```

The library is intentionally batch-oriented. It does not own the caller's values, and it does not maintain process-wide state.

## Directory layout

```text
include/harvard_math/       installed public headers
    harvard_math.h           umbrella include for the normal case
    stats.h                  descriptive-statistics API
src/                         private implementation translation units
tests/                       in-tree deterministic tests
    package_consumer/        separate downstream-package consumer
examples/                    small copy-pasteable programs
cmake/                       install-package and pkg-config templates
Makefile                     thin CMake/Ninja convenience wrapper
docs/                        design notes and audit trail
specs/                       requirements and project engineering decisions
scenarios/                   black-box acceptance journeys
```

The source tree and install tree deliberately have different jobs: only `include/` is exposed to consumers, while `src/` can be reorganized without requiring callers to change include paths.

## API contract

Every batch statistic has the same shape:

```c
typedef hm_status (*hm_statistic_fn)(const double *values,
                                     size_t count,
                                     double *out);
```

That makes a custom statistic an ordinary C function—no registration, macros, opaque objects, or allocator are needed. Built-ins and custom functions can be passed around using the same function-pointer type.

On every public operation:

- `out` must be non-null.
- `count == 0` returns `HM_STATUS_EMPTY_INPUT` without reading `values`.
- A positive count requires a non-null `values` pointer.
- All values must be finite; `NaN`, positive infinity, and negative infinity return `HM_STATUS_NONFINITE_INPUT`.
- The output is written only on success.
- The caller owns `values`; the library never frees or changes it.

`hm_median` makes a temporary copy before selection. It uses introselect for large inputs and a small-input sort; the caller's values are never reordered. The other initial statistics use constant additional storage.

## Formula naming

Population and sample estimators are separate API names so a caller does not need to remember an implicit default:

- population variance: `sum((x - mean)^2) / n`
- sample variance: `sum((x - mean)^2) / (n - 1)`; at least two values are required
- population/sample standard deviation: the square root of the corresponding variance
- quantile: with `h=(n-1)q`, interpolate between the order statistics at `floor(h)` and `ceil(h)`; `q` is a fraction from 0 through 1

Variance uses a centered two-pass double fast path: the first pass validates and accumulates offsets from the first value, and the second pass accumulates centered squares. Centering avoids the large-common-offset failure mode of a naïve raw-sum formula while keeping the hot path vectorization-friendly. If a shifted value or offset sum overflows `double`, the implementation falls back to an extended-precision recurrence.

## Adding a future module

Keep future areas independent and discoverable:

1. Add a focused public header under `include/harvard_math/`, such as `probability.h` or `distribution.h`; ordinary regression slope now lives in `regression.h`.
2. Add its implementation under `src/` and keep private helpers there.
3. Use `hm_status` and document all boundary conditions in the public header.
4. Add a matching test source under `tests/` and register it with CTest.
5. Include the header from `harvard_math.h` only when it is part of the supported umbrella API.
6. Update README/API documentation and the install/export rules if a new public artifact is introduced.

The school core currently includes z-score and ordinary least-squares slope. The inferential extension adds known-σ one/two-sample z-tests, p-values, known-σ mean intervals, simple-regression intercept/correlation/prediction, and a small row-major multiple-regression solver. It deliberately does not promise t-tests, unknown-σ intervals, coefficient standard errors, or robust/high-condition-number modeling.

## Inferential and regression modules

- `analysis.h` owns alternative hypotheses, z-test result structs, p-values, and known-σ confidence intervals.
- `regression.h` owns simple paired-data statistics, population/sample covariance, and intercept-first multiple-regression fit/prediction.
- `HM_STATUS_UNDEFINED` identifies zero denominators, constant predictors, singular matrices, and non-finite computed results; malformed arguments retain `HM_STATUS_INVALID_ARGUMENT`.
- Multiple regression adds an intercept automatically and accepts row-major predictor data. Its educational partial-pivot solver is intentionally bounded in scope.

Streaming accumulators should be a separate module and type when needed; they should not complicate the simple batch calls.

## Build and packaging

CMake is the only build definition. Ninja is the recommended generator. The root `Makefile` is only a convenience wrapper that forwards to CMake, so it cannot drift into a second compiler/linker definition:

```bash
cmake -S . -B build -G Ninja -DHM_BUILD_TESTS=ON -DHM_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The same common path is available as:

```bash
make test
```

Installation exports the `harvard_math::harvard_math` target, headers, a CMake config package, and pkg-config metadata. A downstream project should use the exported target rather than manually guessing library or include paths.

## Benchmark peers

The no-dependency benchmark compares the checked API with a straightforward handwritten C reference. GNU Scientific Library (GSL) is an optional peer enabled with `make benchmark GSL=ON` when `pkg-config` can find `gsl`. The adapter validates finite input before calling GSL, uses stride 1, compares GSL's sample variance and standard deviation with the matching `hm_*` functions, and copies input before calling GSL's in-place quickselect median. Sum and population variance have no direct GSL row in this initial comparison.

A GSL comparison is a peer measurement, not a proof that either implementation is universally faster. Keep compiler flags, input data, allocation semantics, and sample/population definitions visible when interpreting it.
