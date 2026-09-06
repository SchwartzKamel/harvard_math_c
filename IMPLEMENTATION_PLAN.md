# Implementation plan

> Full-track performance plan for the completed importable `harvard_math` C library. One task advances per iteration.

## Convergence
- **Satisfaction threshold:** 95 — overall holdout-scenario score required before handoff.
- **Stratified order:** tier 1 benchmark journey first; deterministic correctness and benchmark checks remain hard gates.
- **Scenarios:** `scenarios/performance-benchmark.yaml`, `scenarios/school-core.yaml`, `scenarios/inferential-regression.yaml`, `scenarios/quantile.yaml`, and `scenarios/covariance.yaml` (holdouts; do not read during implementation).
- **Performance contract:** optimize the checked public API; do not trade away validation, numerical semantics, or input immutability.
- **School-core contract:** add z-score and ordinary least-squares slope without expanding into hypothesis tests or a full regression model.
- **Inferential/regression contract:** add known-σ z-tests/intervals and educational simple/multiple regression with explicit singular-case statuses; do not promise a general modeling solver.

## Now (next up)

### T1 — Establish the CMake/CTest validation signal and minimal library target
- **files/areas:** `CMakeLists.txt`, `include/harvard_math/`, `src/`, `tests/`, `.gitignore`
- **validation:** `cmake -S . -B build -G Ninja -DHM_BUILD_TESTS=ON && cmake --build build && ctest --test-dir build --output-on-failure`
- **acceptance:** A C99 library target, public umbrella header, implementation translation unit, and executable smoke test configure/build/test successfully with no external dependencies.
- **scenarios/tier:** Enables the tier-1 installed-consumer journey.
- **status:** done
- **notes:** `cmake -S . -B build -G Ninja -DHM_BUILD_TESTS=ON && cmake --build build && ctest --test-dir build --output-on-failure` passed; one status smoke test passed.

### T2 — Implement the initial descriptive batch-statistics API
- **files/areas:** `CMakeLists.txt`, `include/harvard_math/stats.h`, `src/stats.c`, `tests/test_stats.c`
- **validation:** `cmake --build build && ctest --test-dir build --output-on-failure`
- **acceptance:** Sum, mean, min, max, range, median, population/sample variance, and population/sample stddev implement the documented status contract, numerical formulas, finite-input checks, and non-mutating median behavior.
- **status:** done
- **notes:** Build and CTest pass with coverage for all ten public operations, invalid input, sample-size boundaries, custom function signature, and median immutability. Corrected the initial test expectation after checking the deviations for `{8, 2, 4, 4}` (population variance is `4.75`).

### T3 — Add install/export/pkg-config integration and a separate consumer
- **files/areas:** `cmake/`, `tests/package_consumer/`, `tests/`, `CMakeLists.txt`
- **validation:** `cmake --install build --prefix /tmp/harvard_math-prefix && cmake -S tests/package_consumer -B /tmp/harvard_math-consumer -G Ninja -DCMAKE_PREFIX_PATH=/tmp/harvard_math-prefix && cmake --build /tmp/harvard_math-consumer && ctest --test-dir /tmp/harvard_math-consumer --output-on-failure`
- **acceptance:** The install tree contains headers, library, CMake package files, and pkg-config metadata; a separate project discovers, links, and runs the library through the exported target.
- **scenarios/tier:** `import-and-use`, tier 1.
- **status:** done
- **notes:** Installed static archive, public headers, CMake config/version/targets, and pkg-config metadata. A separate Ninja consumer discovered `harvard_math::harvard_math`, built, passed CTest, and printed the expected results. Review also compiled a non-CMake consumer from the generated pkg-config flags, including the required `-lm` dependency.

### T4 — Document the quick-start API and extension/layout conventions
- **files/areas:** `README.md`, `docs/design.md`, `examples/basic.c`, `examples/CMakeLists.txt`
- **validation:** `cmake -S . -B build -G Ninja -DHM_BUILD_TESTS=ON -DHM_BUILD_EXAMPLES=ON && cmake --build build && ./build/examples/harvard_math_example`
- **acceptance:** A copy-pasteable example demonstrates built-in and custom statistics; documentation explains build/install/use, error handling, formula naming, and where future modules belong.
- **status:** done
- **notes:** Added the Ninja-oriented quick start, API/error/formula documentation, layout/extension guide, and runnable custom-statistic example. The example build and execution passed.

### T5 — Add the requested Make convenience wrapper
- **files/areas:** `Makefile`, `.gitignore`, `README.md`, `AGENTS.md`, `docs/design.md`, `specs/`
- **validation:** `make clean && make test && make example && make package-test`
- **acceptance:** The Makefile delegates to CMake/Ninja and provides build, test, example, install, and external-package validation targets without duplicating compiler or linker rules.
- **status:** done
- **notes:** Make targets passed with Ninja, including an installed external consumer; build-tree patterns now remain ignored.

### T6 — Establish the correctness-checked benchmark harness and handwritten baseline
- **files/areas:** `bench/`, `CMakeLists.txt`, `Makefile`, `README.md`
- **validation:** `make clean && make benchmark`
- **acceptance:** A no-dependency benchmark builds with CMake/Ninja, runs deterministic finite datasets, compares the checked library API with a same-contract handwritten C reference before timing, reports operation/input/build metadata and relative speedup, and exits non-zero on a result mismatch.
- **scenarios/tier:** `performance-benchmark`, tier 1.
- **status:** done
- **notes:** `make clean && make benchmark` passed in Release mode, `make help` exposes the benchmark target, and `build/benchmark.csv` contains one header plus ten statistic rows. The harness verifies results before timing and keeps GSL optional. Baseline measurements show the current mean/variance implementation is materially slower than the handwritten reference, giving T7 a concrete target.

### T7 — Remove avoidable passes and expensive arithmetic from linear checked statistics
- **files/areas:** `src/stats.c`, `tests/test_stats.c`, `bench/`, `docs/design.md`
- **validation:** `make test && make benchmark`
- **acceptance:** Sum, mean, min, max, range, variance, and standard deviation preserve the existing status/numeric contract while their optimized finite-input paths meet the benchmark's documented 10% reference tolerance on the benchmark host.
- **status:** done
- **notes:** Fused finite validation with sum/min/max/range and replaced per-element long-double/Welford work with a centered two-pass double path plus an extended-precision overflow fallback. Added release-active checks for invalid pointers, non-finite input after an overflow trigger, and large common offsets. Latest benchmark run met the 10% target for every linear operation; median remains for T8.

### T8 — Benchmark and optimize median with equivalent input semantics
- **files/areas:** `src/stats.c`, `tests/test_stats.c`, `bench/`, `include/harvard_math/stats.h`, `docs/design.md`
- **validation:** `make test && make benchmark`
- **acceptance:** Median remains non-mutating and numerically correct; the chosen implementation is measured against a reference with the same validation, unsorted input, and copy semantics, and any algorithm change is kept only if it improves the documented benchmark cases without an adversarial collapse.
- **status:** done
- **notes:** Fused validation into the copy, then replaced large-input qsort with median-of-three three-way introselect plus a depth-limited local qsort fallback; small inputs retain qsort. Fuzz, sorted/reverse/duplicate/organ-pipe cases, CTest, and benchmarks passed. Median improved from near parity to roughly 16x on the generated 1,048,576-value workload and avoided the organ-pipe quickselect worst case.

### T9 — Add an optional GSL peer adapter and semantic comparison
- **files/areas:** `bench/`, `CMakeLists.txt`, `Makefile`, `README.md`, `docs/design.md`
- **validation:** `make benchmark` without GSL, plus `PKG_CONFIG_PATH=/tmp/gsl-install/lib/pkgconfig make benchmark GSL=ON` on a host with GSL installed
- **acceptance:** The default build remains dependency-free and identifies GSL as unavailable when absent; when enabled, the adapter compares only matching operations and records GSL's stride, sample-variance, direct-return, and sorted-median semantics.
- **status:** done
- **notes:** Default mode reported GSL unavailable and passed. Built GSL 2.8 in `/tmp/gsl-install`, then the GSL-enabled Ninja build and correctness-checked peer comparison passed for mean/min/max/range/median/sample variance/sample stddev. Sum and population variance remain intentionally reference-only.

### T10 — Publish reproducible performance methodology and results
- **files/areas:** `README.md`, `docs/design.md`, `bench/`
- **validation:** `make benchmark && /home/lafia/.agents/skills/wgm/scripts/check-docs.sh .`
- **acceptance:** README shows the benchmark command, machine/compiler/flags, dataset sizes, repetitions/warmups, correctness tolerance, handwritten/GSL comparison limits, and a clearly labeled result snapshot.
- **status:** done
- **notes:** README now includes the default handwritten-reference snapshot, optimized GSL 2.8 peer snapshot, reproduction command, speedup definition, semantic caveats, and machine-specific scope. Benchmark and docs structural checks passed.

### T11 — Add checked z-score support
- **files/areas:** `include/harvard_math/stats.h`, `src/stats.c`, `tests/test_stats.c`, `README.md`
- **validation:** `make test`
- **acceptance:** `hm_zscore` computes the documented standardized value, rejects non-finite/invalid inputs, returns `HM_STATUS_UNDEFINED` for zero standard deviation, and leaves output untouched on failure.
- **scenarios/tier:** `school-core`, tier 1.
- **status:** done
- **notes:** Added `HM_STATUS_UNDEFINED` after existing enum values, implemented checked z-score, and covered positive, zero, negative, non-finite, and output-preservation cases. `make test` passed.

### T12 — Add ordinary least-squares slope support
- **files/areas:** `include/harvard_math/regression.h`, `include/harvard_math/harvard_math.h`, `src/regression.c`, `CMakeLists.txt`, `tests/CMakeLists.txt`, `tests/test_regression.c`
- **validation:** `make test`
- **acceptance:** `hm_slope` computes centered least-squares slope for finite paired arrays, rejects insufficient/constant-x/invalid input with explicit statuses, and does not mutate caller arrays.
- **scenarios/tier:** `school-core`, tier 1.
- **status:** done
- **notes:** Added a separate public regression header/module, centered finite-input implementation with extended fallback, umbrella-header export, and deterministic tests for exact lines, large offsets, constant predictors, insufficient data, invalid pointers, non-finite inputs, and immutability. `make test` passed.

### T13 — Document and demonstrate the school core
- **files/areas:** `README.md`, `docs/design.md`, `examples/basic.c`, `examples/CMakeLists.txt`, `tests/package_consumer/`, `specs/library-scaffold.md`
- **validation:** `make test && make package-test && cmake --build build --target harvard_math_example && ./build/examples/harvard_math_example`
- **acceptance:** Public documentation and the runnable example show mean, sample stddev, z-score, and slope with formulas and undefined-case semantics; installed consumers continue to build.
- **status:** done
- **notes:** Updated the public API table, school-core formula section, example output, design guidance, historical scaffold scope, and installed consumer. The exact validation command passed with `zscore=1.390759` and `slope=2.00`.

### T14 — Add known-σ z-tests, p-values, and mean confidence intervals
- **files/areas:** `include/harvard_math/analysis.h`, `include/harvard_math/harvard_math.h`, `src/analysis.c`, `CMakeLists.txt`, `tests/CMakeLists.txt`, `tests/test_analysis.c`
- **validation:** `make test`
- **acceptance:** One- and two-sample z-tests return z/p results for all alternatives; known-σ mean intervals support valid confidence fractions; invalid, zero-count, and non-positive-σ inputs preserve status/output contracts.
- **scenarios/tier:** `inferential-regression`, tier 1.
- **status:** done
- **notes:** Added public analysis types, one/two-sample known-σ z-tests, normal-tail p-values, inverse-normal confidence intervals, and boundary tests. `make test` passed.

### T15 — Add simple-regression intercept, correlation, and prediction
- **files/areas:** `include/harvard_math/regression.h`, `src/regression.c`, `tests/test_regression.c`
- **validation:** `make test`
- **acceptance:** Intercept, Pearson correlation, and simple prediction match ordinary least-squares formulas; constant/insufficient/non-finite cases return explicit statuses and inputs remain unchanged.
- **scenarios/tier:** `inferential-regression`, tier 1.
- **status:** done
- **notes:** Reused the centered paired-data summary for intercept, slope, and correlation; added checked simple prediction and constant-response boundaries. `make test` passed.

### T16 — Add educational multiple-regression fit and prediction
- **files/areas:** `include/harvard_math/regression.h`, `src/regression.c`, `tests/test_regression.c`, `docs/design.md`
- **validation:** `make test`
- **acceptance:** Row-major predictors fit an intercept-first coefficient vector for full-rank data, predict correctly, and return `HM_STATUS_UNDEFINED` for rank-deficient matrices without mutating inputs.
- **scenarios/tier:** `inferential-regression`, tier 1.
- **status:** done
- **notes:** Added checked row-major multiple-regression fitting with an automatic intercept, partial-pivot elimination, coefficient prediction, rank-deficiency detection, allocation checks, and immutability tests. `make test` passed.

### T17 — Document and demonstrate inferential/regression extensions
- **files/areas:** `README.md`, `docs/design.md`, `examples/basic.c`, `tests/package_consumer/main.c`, `tests/package_consumer/`
- **validation:** `make test && make package-test && cmake --build build --target harvard_math_example && ./build/examples/harvard_math_example`
- **acceptance:** Public docs and installed/example consumers show z-test/p-value, confidence interval, intercept/correlation/prediction, and multiple-regression prediction with assumptions and edge statuses.
- **status:** done
- **notes:** README/design docs, example, and installed consumer now cover all added school inference/regression APIs. The exact validation command passed with the documented z-test, p-value, confidence interval, simple prediction, and multiple prediction output.

### T18 — Add checked quantile support
- **files/areas:** `include/harvard_math/stats.h`, `src/stats.c`, `tests/test_stats.c`, `bench/`
- **validation:** `make test && make benchmark`
- **acceptance:** `hm_quantile` implements the documented `h=(n-1)q` interpolation, preserves input, rejects invalid q/non-finite/empty input explicitly, and benchmark/reference results agree before timing.
- **scenarios/tier:** `quantile`, tier 1.
- **status:** done
- **notes:** Added linearly interpolated q-fraction quantiles using guarded copy/selection, Q1/Q3 benchmark rows, boundary/immutability tests, and correctness-before-timing CSV output. The first benchmark run measured approximately 10x Q1 and 4.7x Q3 versus full-sort reference on 1,048,576 values.

### T19 — Document and install the quantile capability
- **files/areas:** `README.md`, `docs/design.md`, `examples/basic.c`, `tests/package_consumer/main.c`, `tests/package_consumer/`
- **validation:** `make test && make package-test && cmake --build build --target harvard_math_example && ./build/examples/harvard_math_example`
- **acceptance:** README/design explain q fractions and quartile calls; example and installed consumer exercise Q1/median/Q3; benchmark output includes quantile rows.
- **status:** done
- **notes:** Added q-fraction/interpolation docs, Q1/Q3 example and installed-consumer checks, and quantile output to the school demo. Exact validation passed without compiler warnings.

### T20 — Add population/sample covariance
- **files/areas:** `include/harvard_math/regression.h`, `src/regression.c`, `tests/test_regression.c`
- **validation:** `make test`
- **acceptance:** Population/sample covariance reuse centered paired data, preserve inputs, distinguish `n` from `n-1`, and return explicit invalid/insufficient/non-finite statuses.
- **scenarios/tier:** `covariance`, tier 1.
- **status:** done
- **notes:** Added population/sample covariance beside slope/correlation, including constant-input zero covariance and sample-size boundaries. `make test` passed.

### T21 — Add paired covariance benchmark and documentation
- **files/areas:** `bench/`, `CMakeLists.txt`, `Makefile`, `README.md`, `docs/design.md`, `examples/basic.c`, `tests/package_consumer/main.c`
- **validation:** `make clean && make test && make benchmark && make package-test && cmake --build build --target harvard_math_example && ./build/examples/harvard_math_example`
- **acceptance:** Paired benchmark/reference rows verify covariance before timing; docs/example/installed consumer show covariance beside correlation/slope; default and optional GSL modes remain honest about semantics.
- **status:** pending
- **notes:** Use a dedicated paired benchmark rather than disguising two-array operations as one-array statistics.

## Later (backlog)
- Add weighted statistics only after choosing weighted status and normalization contracts.
- Add streaming accumulators as a separate API only when a real caller needs data that cannot fit in a batch.
- Add weighted/quantile/correlation modules behind new public headers with their own formula and edge-case contracts.
- Add architecture-specific SIMD dispatch only after scalar benchmark limits and portability policy are settled.

### TZ — Demo validation (completed scaffold)
- **files/areas:** installed package and `tests/package_consumer/`
- **validation:** `rm -rf /tmp/harvard_math-prefix /tmp/harvard_math-consumer && cmake --install build --prefix /tmp/harvard_math-prefix && cmake -S tests/package_consumer -B /tmp/harvard_math-consumer -G Ninja -DCMAKE_PREFIX_PATH=/tmp/harvard_math-prefix && cmake --build /tmp/harvard_math-consumer && ctest --test-dir /tmp/harvard_math-consumer --output-on-failure`
- **acceptance:** The external consumer completes the scaffold spec's smallest end-to-end demo path and exits 0 with expected output.
- **status:** done
- **notes:** Final tier-1 install demo passed before this performance pass.

### PZ — Performance demo validation (completed)
- **files/areas:** `bench/`, `README.md`, `Makefile`, optimized library, optional GSL adapter
- **validation:** `make clean && make test && make benchmark`
- **acceptance:** Correctness-checked benchmark output is reproducible, the safe library meets the performance contract against the handwritten reference, and the README explains how to reproduce and interpret the comparison.
- **status:** done
- **notes:** Exact command passed: CTest green, benchmark CSV generated with ten rows, all rows correctness-checked before timing. Optional GSL-enabled mode also passed separately with GSL 2.8 built using `-O3 -DNDEBUG`.

### SZ — School-core demo validation (completed)
- **files/areas:** `include/harvard_math/`, `src/`, `tests/`, `examples/basic.c`, `README.md`, installed package
- **validation:** `make test && make package-test && cmake --build build --target harvard_math_example && ./build/examples/harvard_math_example`
- **acceptance:** The public example and installed consumer exercise the new z-score and slope APIs, all deterministic tests pass, and the tier-1 school-core journey is satisfied.
- **status:** done
- **notes:** CTest, installed package consumer, and example execution passed; output includes the expected z-score and slope.

### IZ — Inferential/regression demo validation (completed)
- **files/areas:** `include/harvard_math/`, `src/analysis.c`, `src/regression.c`, `tests/`, `examples/basic.c`, `README.md`, installed package
- **validation:** `make test && make package-test && cmake --build build --target harvard_math_example && ./build/examples/harvard_math_example`
- **acceptance:** The example and installed consumer exercise z-tests, p-values, confidence intervals, simple regression, and multiple-regression prediction; all deterministic tests pass; the tier-1 inferential/regression journey is satisfied.
- **status:** done
- **notes:** CTest, installed consumer, and example output all passed; the example prints the documented z-test, p-value, confidence interval, simple regression, and multiple-regression values.

### QZ — Quantile demo validation (completed)
- **files/areas:** `include/harvard_math/stats.h`, `src/stats.c`, `tests/`, `bench/`, `examples/basic.c`, `README.md`, installed package
- **validation:** `make clean && make test && make benchmark && make package-test && cmake --build build --target harvard_math_example && ./build/examples/harvard_math_example`
- **acceptance:** Quantile correctness and benchmark rows pass, Q1/median/Q3 are visible in docs/example/consumer, and the tier-1 quantile journey is satisfied.
- **status:** done
- **notes:** CTest, Q1/Q3 benchmark rows, installed consumer, and example output passed; measured Q1/Q3 speedups were recorded in the plan and CSV.

## Done
- Plan and requirements artifacts — goal, API defaults, scope, acceptance checks, and holdout journey recorded.
- T6 — Correctness-checked benchmark harness and handwritten baseline — Release benchmark, CSV output, metadata, and pre-timing result checks passed.
- T7 — Fast checked linear statistics — safe one-pass/fused paths and centered variance passed CTest and met the current benchmark target.
- T8 — Fast checked median — fused copy/validation and guarded introselect passed adversarial-shape tests and benchmarked well above qsort on the generated large workload.
- T9 — Optional GSL peer adapter — dependency-free and GSL-enabled benchmark modes both passed with semantic differences documented.
- T10 — Reproducible performance methodology/results — README and design docs updated; benchmark and docs checks passed.
- PZ — Performance demo — exact `make clean && make test && make benchmark` gate passed with CSV output.
- T11 — Checked z-score — public API, undefined status, and boundary tests passed.
- T12 — Checked ordinary least-squares slope — public regression module and CTest coverage passed.
- T13 — School-core documentation/example — README, design docs, installed consumer, and example updated and validated.
- T14 — Known-σ z-tests and confidence intervals — public analysis module and CTest coverage passed.
- T15 — Simple regression intercept/correlation/prediction — centered formulas and boundary tests passed.
- T16 — Educational multiple regression — intercept-first fit, prediction, singular-matrix, and input-boundary tests passed.
- T17 — Inferential/regression documentation/example — README, design docs, installed consumer, and example validated.
- T18 — Checked quantiles — interpolation semantics, boundary tests, and Q1/Q3 benchmark rows passed.
- T19 — Quantile documentation/example — Q-fraction docs, installed consumer, and warning-free example passed.
- T20 — Paired covariance — population/sample formulas and centered boundary tests passed.
- QZ — Quantile demo — tier-1 Q1/median/Q3 journey passed with satisfaction 100/100.
- IZ — Inferential/regression demo — tier-1 z-test/interval/regression journey passed with satisfaction 100/100.
- SZ — School-core demo — tier-1 z-score/slope journey passed with satisfaction 100/100.
- Performance plan and baseline docs audit — checked API performance goal, reference semantics, optional peer policy, and Full-track baseline recorded.
- T1 — CMake/CTest signal and minimal public library target — configure, build, and status smoke test passed.
- T2 — Initial descriptive batch-statistics API — clean build and CTest pass; all declared statistics and error paths covered.
- T3 — Install/export/pkg-config integration and external consumer — temporary-prefix install and separate Ninja consumer passed.
- T4 — Documentation, layout guide, and runnable example — Ninja configure/build and example execution passed.
- T5 — Make convenience wrapper — `make clean && make test && make example && make package-test` passed.
- TZ — External installed-consumer demo — tier-1 scenario scored 100/100; deterministic CTest and executable output passed.
- Ship review — docs audit GREEN; pkg-config static-link dependency verified with a real consumer.

---

A task may be marked `done` only after its exact validation command exits 0. Keep this file fresh enough for a new agent to resume without transcript context.
