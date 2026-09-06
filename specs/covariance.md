# Spec: paired covariance

## JTBD (job to be done)
As a statistics student or analyst, I want population and sample covariance for paired arrays so that I can quantify joint variation before using correlation or regression.

## User-visible success criteria
- The public API exposes population and sample covariance for finite paired arrays.
- Population uses divisor `n`; sample uses divisor `n-1` and requires at least two observations.
- The operation preserves paired input arrays, handles large common offsets, and returns explicit statuses for invalid/constant/insufficient cases.
- A paired benchmark compares both covariance functions with a same-semantics handwritten reference and optional GSL peer behavior.
- README/design/example/package consumer document and exercise covariance.

## Magic moment
- **The whoa:** a caller can compute covariance, correlation, and slope on the same paired data without rewriting centering logic.
- **Demo path:** build the paired benchmark, inspect correctness-checked covariance rows, then run the installed consumer/example on a linear paired dataset.
- **Smallest end-to-end slice:** public declarations, implementation using the centered paired summary, deterministic population/sample tests, paired benchmark, and installed example.
- **Merely functional vs magical:** merely functional returns a covariance; the useful experience makes the divisor choice explicit and keeps covariance/correlation/slope consistent.

## Acceptance criteria → backpressure

| Criterion (EARS) | How it's verified (command/check) |
|---|---|
| When finite paired arrays with at least one observation are supplied, population covariance shall return centered cross-deviation sum divided by `n`. | `tests/test_regression.c` formula assertion via `make test`. |
| When finite paired arrays with at least two observations are supplied, sample covariance shall return centered cross-deviation sum divided by `n-1`. | `tests/test_regression.c`. |
| If sample covariance receives fewer than two observations, it shall return `HM_STATUS_INSUFFICIENT_DATA` without writing output. | Boundary test. |
| If paired pointers are null, data are non-finite, or count is zero, the library shall return the documented status without dereferencing invalid memory. | Error tests. |
| When covariance is computed, caller-owned paired arrays shall remain unchanged. | Immutability test. |
| When the paired benchmark runs, library/reference results shall be checked before timing and CSV rows shall be emitted for population and sample covariance. | `make benchmark` plus paired CSV inspection. |
| When the installed consumer/example runs, it shall demonstrate covariance alongside correlation and slope through public headers. | `make package-test` and example execution. |

## Holdout scenarios
- **Files:** `scenarios/covariance.yaml`
- **Holdout rule:** authored here, but the Implement step must not read it; Validate/Review judges paired benchmark and example output.

## Assumptions
- Covariance belongs with paired regression APIs and reuses the existing centered summary.
- Constant `x` or constant `y` does not make covariance undefined; a zero covariance is valid. Correlation remains undefined when either variance is zero.
- GSL comparison is optional; default builds stay dependency-free.

## Out of scope (this slice)
- Weighted covariance, streaming covariance, covariance matrices, and missing-data handling.
