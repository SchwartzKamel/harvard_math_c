# Implementation plan

> Standard-track plan for the importable `harvard_math` C library scaffold. One task advances per iteration.

## Convergence
- **Satisfaction threshold:** 95 — overall holdout-scenario score required before handoff.
- **Stratified order:** tier 1 installed-consumer journey first; deterministic checks remain the hard gate.
- **Scenarios:** `scenarios/import-and-use.yaml` (holdout; do not read during implementation).

## Now (next up)

### T1 — Establish the CMake/CTest validation signal and minimal library target
- **files/areas:** `CMakeLists.txt`, `include/harvard_math/`, `src/`, `tests/`, `.gitignore`
- **validation:** `cmake -S . -B build -DHM_BUILD_TESTS=ON && cmake --build build && ctest --test-dir build --output-on-failure`
- **acceptance:** A C99 library target, public umbrella header, implementation translation unit, and executable smoke test configure/build/test successfully with no external dependencies.
- **scenarios/tier:** Enables the tier-1 installed-consumer journey.
- **status:** done
- **notes:** `cmake -S . -B build -DHM_BUILD_TESTS=ON && cmake --build build && ctest --test-dir build --output-on-failure` passed; one status smoke test passed.

### T2 — Implement the initial descriptive batch-statistics API
- **files/areas:** `CMakeLists.txt`, `include/harvard_math/stats.h`, `src/stats.c`, `tests/test_stats.c`
- **validation:** `cmake --build build && ctest --test-dir build --output-on-failure`
- **acceptance:** Sum, mean, min, max, range, median, population/sample variance, and population/sample stddev implement the documented status contract, numerical formulas, finite-input checks, and non-mutating median behavior.
- **status:** done
- **notes:** Build and CTest pass with coverage for all ten public operations, invalid input, sample-size boundaries, custom function signature, and median immutability. Corrected the initial test expectation after checking the deviations for `{8, 2, 4, 4}` (population variance is `4.75`).

### T3 — Add install/export/pkg-config integration and a separate consumer
- **files/areas:** `cmake/`, `tests/package_consumer/`, `tests/`, `CMakeLists.txt`
- **validation:** `cmake --install build --prefix /tmp/harvard_math-prefix && cmake -S tests/package_consumer -B /tmp/harvard_math-consumer -DCMAKE_PREFIX_PATH=/tmp/harvard_math-prefix && cmake --build /tmp/harvard_math-consumer && ctest --test-dir /tmp/harvard_math-consumer --output-on-failure`
- **acceptance:** The install tree contains headers, library, CMake package files, and pkg-config metadata; a separate project discovers, links, and runs the library through the exported target.
- **scenarios/tier:** `import-and-use`, tier 1.
- **status:** pending
- **notes:**

### T4 — Document the quick-start API and extension/layout conventions
- **files/areas:** `README.md`, `docs/design.md`, `examples/basic.c`, `examples/CMakeLists.txt`, `Makefile`
- **validation:** `cmake --build build && ./build/examples/harvard_math_example`
- **acceptance:** A copy-pasteable example demonstrates built-in and custom statistics; documentation explains build/install/use, error handling, formula naming, and where future modules belong.
- **status:** pending
- **notes:**

## Later (backlog)
- Add streaming accumulators as a separate API only when a real caller needs data that cannot fit in a batch.
- Add weighted/quantile/correlation modules behind new public headers with their own formula and edge-case contracts.

### TZ — Demo validation (required before Ship/Handoff)
- **files/areas:** installed package and `tests/package_consumer/`
- **validation:** `rm -rf /tmp/harvard_math-prefix /tmp/harvard_math-consumer && cmake --install build --prefix /tmp/harvard_math-prefix && cmake -S tests/package_consumer -B /tmp/harvard_math-consumer -DCMAKE_PREFIX_PATH=/tmp/harvard_math-prefix && cmake --build /tmp/harvard_math-consumer && ctest --test-dir /tmp/harvard_math-consumer --output-on-failure`
- **acceptance:** The external consumer completes the spec's smallest end-to-end demo path and exits 0 with expected output.
- **status:** pending
- **notes:**

## Done
- Plan and requirements artifacts — goal, API defaults, scope, acceptance checks, and holdout journey recorded.
- T1 — CMake/CTest signal and minimal public library target — configure, build, and status smoke test passed.
- T2 — Initial descriptive batch-statistics API — clean build and CTest pass; all declared statistics and error paths covered.

---

A task may be marked `done` only after its exact validation command exits 0. Keep this file fresh enough for a new agent to resume without transcript context.
