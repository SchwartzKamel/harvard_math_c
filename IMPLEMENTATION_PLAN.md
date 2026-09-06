# Implementation plan

> Full-track performance plan for the completed importable `harvard_math` C library. One task advances per iteration.

## Convergence
- **Satisfaction threshold:** 95 — overall holdout-scenario score required before handoff.
- **Stratified order:** tier 1 benchmark journey first; deterministic correctness and benchmark checks remain hard gates.
- **Scenarios:** `scenarios/performance-benchmark.yaml` (holdout; do not read during implementation).
- **Performance contract:** optimize the checked public API; do not trade away validation, numerical semantics, or input immutability.

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
- **files/areas:** `bench/`, `CMakeLists.txt`, `Makefile`, `docs/design.md`
- **validation:** `make benchmark` without GSL, plus `make benchmark GSL=ON` on a host with GSL installed
- **acceptance:** The default build remains dependency-free and identifies GSL as unavailable when absent; when enabled, the adapter compares only matching operations and records GSL's stride, sample-variance, direct-return, and sorted-median semantics.
- **status:** pending
- **notes:** Current host does not expose `gsl-config`; optional peer validation must be explicit rather than silently skipped.

### T10 — Publish reproducible performance methodology and results
- **files/areas:** `README.md`, `docs/design.md`, `bench/`
- **validation:** `make benchmark && /home/lafia/.agents/skills/wgm/scripts/check-docs.sh .`
- **acceptance:** README shows the benchmark command, machine/compiler/flags, dataset sizes, repetitions/warmups, correctness tolerance, handwritten/GSL comparison limits, and a clearly labeled result snapshot.
- **status:** pending
- **notes:** Results must be regenerated rather than presented as universal claims.

## Later (backlog)
- Add streaming accumulators as a separate API only when a real caller needs data that cannot fit in a batch.
- Add weighted/quantile/correlation modules behind new public headers with their own formula and edge-case contracts.
- Add architecture-specific SIMD dispatch only after scalar benchmark limits and portability policy are settled.

### TZ — Demo validation (completed scaffold)
- **files/areas:** installed package and `tests/package_consumer/`
- **validation:** `rm -rf /tmp/harvard_math-prefix /tmp/harvard_math-consumer && cmake --install build --prefix /tmp/harvard_math-prefix && cmake -S tests/package_consumer -B /tmp/harvard_math-consumer -G Ninja -DCMAKE_PREFIX_PATH=/tmp/harvard_math-prefix && cmake --build /tmp/harvard_math-consumer && ctest --test-dir /tmp/harvard_math-consumer --output-on-failure`
- **acceptance:** The external consumer completes the scaffold spec's smallest end-to-end demo path and exits 0 with expected output.
- **status:** done
- **notes:** Final tier-1 install demo passed before this performance pass.

### PZ — Performance demo validation (required before Ship/Handoff)
- **files/areas:** `bench/`, `README.md`, `Makefile`, optimized library, optional GSL adapter
- **validation:** `make clean && make test && make benchmark`
- **acceptance:** Correctness-checked benchmark output is reproducible, the safe library meets the performance contract against the handwritten reference, and the README explains how to reproduce and interpret the comparison.
- **status:** pending
- **notes:** Run optional GSL comparison separately when the dependency is available; do not turn its absence into a default-build failure.

## Done
- Plan and requirements artifacts — goal, API defaults, scope, acceptance checks, and holdout journey recorded.
- T6 — Correctness-checked benchmark harness and handwritten baseline — Release benchmark, CSV output, metadata, and pre-timing result checks passed.
- T7 — Fast checked linear statistics — safe one-pass/fused paths and centered variance passed CTest and met the current benchmark target.
- T8 — Fast checked median — fused copy/validation and guarded introselect passed adversarial-shape tests and benchmarked well above qsort on the generated large workload.
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
