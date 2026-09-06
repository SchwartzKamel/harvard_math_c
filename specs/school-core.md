# Spec: school-statistics core

## JTBD (job to be done)
As a student or teacher using introductory statistics, I want the small set of formulas I learned in class to be callable through the same checked C API so that examples and exercises do not each reimplement edge-case handling.

## User-visible success criteria
- The public library exposes a scalar z-score operation and a paired-array simple-regression slope operation.
- Existing mean and population/sample standard-deviation operations remain unchanged.
- A z-score with a zero standard deviation and a slope with constant `x` return an explicit undefined-result status instead of `NaN` or undefined behavior.
- A caller can include the umbrella header and use all five school-core concepts without private headers.
- Tests and README examples show the formulas, input boundaries, and status behavior.

## Magic moment
- **The whoa:** a student can compute a mean, sample standard deviation, z-score, and slope with the same `hm_status` pattern and no formula boilerplate.
- **Demo path:** build the example, run it on a small class-data set, and see the documented z-score and line slope alongside the existing descriptive statistics.
- **Smallest end-to-end slice:** public declarations, linked implementations, deterministic tests, and one runnable example using z-score and slope.
- **Merely functional vs magical:** merely functional functions return numbers; the useful experience makes undefined mathematical cases explicit and keeps the school formulas easy to recognize.

## Acceptance criteria → backpressure

| Criterion (EARS) | How it's verified (command/check) |
|---|---|
| When finite `value`, `mean`, and positive finite `stddev` are supplied, `hm_zscore` shall write `(value - mean) / stddev` and return `HM_STATUS_OK`. | `tests/test_stats.c` assertions run by `make test`. |
| If `hm_zscore` receives zero standard deviation, it shall return `HM_STATUS_UNDEFINED` without writing the output. | `tests/test_stats.c` boundary assertion. |
| If `hm_zscore` receives a negative standard deviation, a non-finite input, or a null output, it shall return the documented non-success status without undefined behavior. | `tests/test_stats.c` error assertions. |
| When finite paired arrays `x` and `y` contain at least two observations and `x` varies, `hm_slope` shall write the least-squares slope `Σ((xᵢ-x̄)(yᵢ-ȳ)) / Σ((xᵢ-x̄)²)` and return `HM_STATUS_OK`. | `tests/test_regression.c` formula assertions run by CTest. |
| If `hm_slope` receives fewer than two observations or constant `x`, it shall return `HM_STATUS_INSUFFICIENT_DATA` or `HM_STATUS_UNDEFINED` respectively without writing the output. | `tests/test_regression.c` boundary assertions. |
| If either regression input contains a non-finite value or a required pointer is null, the library shall return a status before dereferencing invalid memory. | `tests/test_regression.c` error assertions. |
| When z-score or slope is called, caller-owned input arrays shall remain unchanged. | Immutability assertions in `tests/test_regression.c`. |
| When the public example is built, it shall demonstrate z-score and slope through the umbrella header. | `cmake --build build && ./build/examples/harvard_math_example` and README review. |

## Holdout scenarios
- **Files:** `scenarios/school-core.yaml`
- **Holdout rule:** authored here, but the Implement step must not read it; Validate/Review judges the user-facing example and installed API path.

## Assumptions
- “Z” means an individual z-score, not a z-test statistic.
- Slope means the ordinary least-squares slope for one predictor and one response; intercept, correlation, and prediction are separate future APIs.
- `HM_STATUS_UNDEFINED` is a mathematical-result status, appended to preserve existing status numeric values.
- A positive standard deviation is required for a z-score; zero is undefined and negative is an invalid argument.
- A slope requires finite paired arrays of equal logical length, at least two observations, and nonzero centered `x` sum of squares.
- The school-core APIs remain batch-oriented and do not allocate or mutate caller-owned input.

## Out of scope (this pass)
- Z-tests, p-values, confidence intervals, t-tests, and hypothesis-test distributions.
- Regression intercept, correlation coefficient, residuals, prediction, and multiple regression.
- Weighted or streaming variants of z-score or slope.
