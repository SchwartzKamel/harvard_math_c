# Spec: importable descriptive-statistics library

## JTBD (job to be done)
As a C programmer studying or teaching introductory statistics, I want to include a small installed library and call descriptive statistics with a pointer, count, and output variable so that I can use correct formulas without writing setup or memory-management boilerplate.

## User-visible success criteria
- A fresh checkout configures, builds, and runs its tests with CMake and no third-party dependencies.
- A downstream CMake project can use `find_package(harvard_math CONFIG REQUIRED)`, include `<harvard_math/harvard_math.h>`, and link `harvard_math::harvard_math`.
- A caller can implement a custom statistic using the same `hm_statistic_fn` signature as the built-ins.
- The initial API provides sum, mean, minimum, maximum, range, median, population/sample variance, and population/sample standard deviation with explicit error statuses.

## Magic moment
- **The whoa:** a consumer can write `hm_mean(values, count, &mean);` and link the same way whether the library is in-tree or installed.
- **Demo path:** configure and build the library, install it to a temporary prefix, configure a tiny external CMake consumer against that prefix, run the consumer on `{2, 4, 4, 8}`, and observe the documented mean, median, and sample standard deviation.
- **Smallest end-to-end slice:** installed package discovery, public umbrella-header inclusion, one linked statistic call, and a successful executable run.
- **Merely functional vs magical:** merely functional code builds from the repository; the useful experience is a copy-pasteable consumer with predictable names, explicit errors, and no hand-written link/include paths.

## Acceptance criteria → backpressure

| Criterion (EARS) | How it's verified (command/check) |
|---|---|
| When a fresh checkout is configured with CMake and Ninja, the project shall generate a build and CTest shall report all tests passing. | `cmake -S . -B build -G Ninja -DHM_BUILD_TESTS=ON -DHM_BUILD_EXAMPLES=ON && cmake --build build && ctest --test-dir build --output-on-failure` |
| When a caller supplies a non-empty finite array and valid output pointer, each initial descriptive statistic shall return `HM_STATUS_OK` and the mathematically correct result within the test tolerance. | `ctest --test-dir build --output-on-failure` |
| If a caller supplies an empty array, a null output pointer, a null values pointer with a positive count, insufficient values for sample statistics, or a non-finite value, the library shall return the documented non-success status without dereferencing invalid memory. | Error-path assertions in `tests/test_stats.c`, run by CTest. |
| When median is requested, the library shall leave the caller's input array unchanged. | Median immutability assertion in `tests/test_stats.c`. |
| When a downstream project calls `find_package(harvard_math CONFIG REQUIRED)`, the installed package shall expose `harvard_math::harvard_math`, its public headers, and the required math linkage. | Temporary-prefix install and external consumer build/run in `tests/package_consumer/`. |
| When a caller defines a function matching `hm_statistic_fn`, the function shall be usable as a custom statistic without private-library types. | Compile-and-run custom midrange example in `examples/basic.c` and package consumer. |
| When documentation is read, it shall state the population/sample divisor distinction, input/error contract, build commands, and extension layout. | README and `docs/design.md` review plus the demo command. |

## Holdout scenarios
- **Files:** `scenarios/import-and-use.yaml`
- **Holdout rule:** authored here, but the Implement step must not read it; Validate/Review judges the external consumer journey against it.

## Assumptions
- The project name and installed package name are `harvard_math`; the public include prefix is `harvard_math` and the C symbol prefix is `hm_`.
- C99 is the compatibility floor. CMake is the canonical build system and Ninja is the recommended generator; no second build definition is required.
- Statistics operate on immutable finite `double` arrays. Empty input and non-finite values are errors rather than silently propagated sentinels.
- Public functions use an output pointer plus `hm_status` so a valid numeric result is never confused with an error sentinel.
- Initial scope is descriptive batch statistics only; no streaming, weighted data, quantiles, probability distributions, CLI, or ABI-stability promise is required yet.

## Out of scope (this pass)
- Streaming/stateless accumulator objects.
- Weighted statistics, quantiles/percentiles, covariance/correlation, regression, and probability distributions.
- Integer/generic numeric containers and user-defined allocators.
- Publishing to a system package manager or remote registry.
- A command-line application built on top of the library.
