# Spec: quantiles and quartiles

## JTBD (job to be done)
As a student or analyst using descriptive statistics, I want a checked quantile operation so that quartiles and percentiles use one documented convention instead of repeated ad-hoc sorting code.

## User-visible success criteria
- The public API exposes `hm_quantile(values, count, q, out)` for `q` in `[0, 1]`.
- The operation uses the linear interpolation convention `h=(n-1)q`, matching the documented quartile examples and GSL's sorted-data quantile convention.
- The operation validates finite input, preserves caller data, and returns explicit statuses for empty input, invalid `q`, allocation failure, and non-finite values.
- The no-dependency benchmark compares unsorted input against a same-semantics handwritten reference before timing; README documents q25, q50, and q75 usage.

## Magic moment
- **The whoa:** `hm_quantile(values, count, 0.75, &q3)` gives the third quartile with the same status/output contract as mean and standard deviation.
- **Demo path:** build, run the quantile benchmark, then use the installed umbrella header to compute Q1, median, and Q3 without mutating the input.
- **Smallest end-to-end slice:** public declaration, copy/selection implementation, boundary tests, benchmark row, docs, and installed consumer compile.
- **Merely functional vs magical:** merely functional returns an order statistic; the useful experience states the interpolation convention and makes quartile calls obvious.

## Acceptance criteria → backpressure

| Criterion (EARS) | How it's verified (command/check) |
|---|---|
| When finite values and `q` in `[0,1]` are supplied, `hm_quantile` shall return the linearly interpolated order statistic using `h=(n-1)q`. | `tests/test_stats.c` formula assertions via `make test`. |
| When q is `0`, `0.5`, or `1`, the operation shall return the minimum, median convention, or maximum without mutating input. | Boundary and immutability assertions. |
| If q is non-finite or outside `[0,1]`, the operation shall return `HM_STATUS_INVALID_ARGUMENT` without writing output. | Error assertions. |
| If values are empty, null, or contain a non-finite element, the operation shall return the documented status without undefined behavior. | Error assertions. |
| When the benchmark is run, quantile rows shall compare correctness before timing and report machine-readable output. | `make benchmark` and CSV inspection. |
| When the installed package consumer is built, it shall compile a Q1/median/Q3 call through the umbrella header. | `make package-test`. |
| When documentation is read, it shall name the interpolation convention and show quartile calls. | README/design review plus example execution. |

## Holdout scenarios
- **Files:** `scenarios/quantile.yaml`
- **Holdout rule:** authored here, but the Implement step must not read it; Validate/Review judges the benchmark, example, and installed API path.

## Assumptions
- Quantiles use the inclusive linear interpolation convention used by GSL's sorted-data quantile formula: `h=(n-1)q`.
- The API accepts a fraction `q` from 0 through 1, not a 0–100 percentile number.
- The implementation may copy and select; caller input is never reordered.
- GSL remains optional; the default build has no external dependency.

## Out of scope (this slice)
- Alternate quantile definitions, weighted quantiles, streaming quantiles, and approximate sketches.
- A separate percentile-number API; callers can pass `percent / 100.0`.
