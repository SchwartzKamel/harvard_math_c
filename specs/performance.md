# Spec: fast checked batch statistics

## JTBD (job to be done)
As a C programmer choosing a small statistics library, I want the safe public batch API to approach or beat a careful handwritten C implementation so that I get validation and numerical semantics without paying avoidable runtime overhead.

## User-visible success criteria
- `make benchmark` runs without external dependencies and compares `harvard_math` with a handwritten C reference using identical inputs and call semantics.
- The benchmark reports correctness, per-operation timing, input size, compiler/build mode, and a relative speedup in both human-readable and machine-readable forms.
- The checked public API keeps its existing error, finite-input, population/sample, and non-mutating-input contracts.
- Mean, sum, min, max, range, variance, and standard deviation do not perform avoidable extra full-array passes; median is compared using equivalent copy, unsorted-input, and immutability semantics.
- The README contains reproducible benchmark instructions and clearly labels machine-specific results rather than making a universal speed claim.
- When GSL is available, an optional adapter compares matching operations and documents semantic differences; GSL is not a required runtime or build dependency.

## Magic moment
- **The whoa:** one command prints a trustworthy comparison such as `handwritten C`, `harvard_math`, and optional `GSL`, while proving the results agree before showing timings.
- **Demo path:** build with optimized settings, run `make benchmark`, inspect the table for mean, variance, and median, then follow the README command on another machine.
- **Smallest end-to-end slice:** a no-dependency benchmark generates one deterministic dataset, invokes the safe library and a same-contract reference, verifies results, and prints timing/speedup rows.
- **Merely functional vs magical:** merely functional code has a benchmark binary; the useful experience makes the comparison repeatable, honest about semantics, and easy to extend with another implementation.

## Acceptance criteria → backpressure

| Criterion (EARS) | How it's verified (command/check) |
|---|---|
| When `make benchmark` is run on a finite deterministic dataset, the benchmark shall build/run without third-party dependencies and report rows for sum, mean, min, max, range, population/sample variance, population/sample standard deviation, and median. | `make clean && make benchmark` |
| When a benchmark row is emitted, the runner shall verify the library and reference results within the documented tolerance before reporting timing or speedup. | Benchmark exits non-zero on mismatch; `make benchmark` exits 0. |
| When the safe API and handwritten reference are compiled with the same optimized C settings, the library's linear statistics shall be no slower than the reference beyond a documented 10% tolerance on the benchmark host, and median shall compare the same copy, unsorted-input, and immutability workload semantics. | `make benchmark` machine-readable summary and correctness gate. |
| If the benchmark is run without GSL, the default target shall still pass and identify the optional peer as unavailable rather than failing the build. | `make benchmark` on the current host, where `gsl-config` is unavailable. |
| When GSL is available, the optional adapter shall compare only semantically matching operations and shall identify stride, sample-variance, status/error, and sorted-median differences in its report. | `make benchmark GSL=ON` on a GSL-enabled host, or compile/configuration check of the adapter. |
| When invalid, empty, non-finite, and sample-size boundary inputs are tested, the optimization shall preserve the existing status contract and never dereference invalid memory. | `make test` / CTest, including `tests/test_stats.c`. |
| When the README performance section is followed, it shall state the benchmark command, compiler flags, dataset/repetition methodology, machine-specific scope, and comparison limitations. | Documentation review plus `make benchmark`. |
| When benchmark code is built by default, the normal library/test build shall remain free of benchmark-only dependencies and targets. | `make test` and default `cmake --build build`. |

## Holdout scenarios
- **Files:** `scenarios/performance-benchmark.yaml`
- **Holdout rule:** authored here, but the Implement step must not read it; Validate/Review judges the benchmark's user-facing output and README path.

## Assumptions
- “Fastest” means fastest checked implementation under the current public contract, not an unsafe API that assumes valid input.
- The primary baseline is a same-signature handwritten C implementation with the same validation, unsorted input, and non-mutating median semantics; its qsort algorithm is intentionally a straightforward comparison point rather than an identical implementation.
- A benchmark run uses finite deterministic `double` data, `-O3` (and records whether `-march=native` is enabled), warmups, repeated measurements, and an escaped checksum to prevent dead-code elimination.
- Results are relative evidence for a named machine/compiler/build, not a promise across all CPUs or compilers.
- GSL is an optional comparison peer, not a required dependency. Higher-level runtimes such as Python/NumPy and R are not kernel-level peers for this pass.
- The checked public API remains the compatibility surface; no unchecked or architecture-specific public API is added in this pass.

## Out of scope (this pass)
- Changing the public error or numeric semantics to win a benchmark.
- `-ffast-math` as a default release setting.
- Mandatory GSL, BLAS, SIMD runtime dispatch, OpenMP, or platform-specific assembly.
- Comparing language runtimes as if they were equivalent C kernels.
- Streaming accumulators, weighted statistics, quantiles, or new statistical families.
