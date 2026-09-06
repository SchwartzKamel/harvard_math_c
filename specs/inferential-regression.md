# Spec: inferential and regression school extensions

## JTBD (job to be done)
As a student or teacher completing a statistics course, I want the common z-test, confidence-interval, and regression formulas available through the same checked C library so that I can move from descriptive summaries to standard classroom inference without reimplementing numerical edge cases.

## User-visible success criteria
- A caller can run one- and two-sample z-tests and receive both a z statistic and p-value for two-sided, lower-tail, or upper-tail alternatives.
- A caller can compute a known-population-standard-deviation confidence interval for a mean.
- A caller can compute simple-regression intercept, slope, correlation, and prediction from paired arrays.
- A caller can fit a multiple linear regression with an automatically added intercept from a row-major predictor matrix and predict from its coefficients.
- Invalid, insufficient, singular, non-finite, and mathematically undefined inputs return explicit statuses without mutating inputs or output on failure.
- The installed umbrella header and example expose the new APIs without adding runtime dependencies.

## Magic moment
- **The whoa:** one checked API can take a class dataset from z-test and confidence interval through a fitted line and multiple-regression prediction.
- **Demo path:** build the example, run it on a known-σ sample and paired predictor/response data, and observe the z statistic, p-value, interval, fitted intercept/slope, correlation, prediction, and multiple-regression prediction.
- **Smallest end-to-end slice:** public inferential/regression headers, linked implementations, deterministic formula tests, and an installed consumer using the umbrella header.
- **Merely functional vs magical:** merely functional APIs return numbers; the useful experience distinguishes tail alternatives, known-σ assumptions, undefined singular cases, and row-major matrix layout.

## Acceptance criteria → backpressure

| Criterion (EARS) | How it's verified (command/check) |
|---|---|
| When a valid one-sample z-test is supplied, the library shall return the documented z statistic and normal-tail p-value for each supported alternative. | `tests/test_analysis.c` formula assertions via `make test`. |
| When a valid two-sample z-test is supplied, the library shall use the combined known-σ standard error and return a checked statistic/p-value result. | `tests/test_analysis.c` via CTest. |
| When a valid known-σ confidence request is supplied, the library shall return finite lower/upper bounds for the requested confidence level. | `tests/test_analysis.c` 95% interval assertion. |
| If a z-test or interval receives zero observations, non-positive known standard deviation, invalid confidence/alternative, or non-finite input, it shall return an explicit non-success status without writing output. | `tests/test_analysis.c` boundary assertions. |
| When finite paired arrays with varying `x` are supplied, the library shall return simple-regression slope, intercept, correlation, and prediction matching the least-squares formulas. | `tests/test_regression.c` formula assertions via CTest. |
| If simple regression receives constant `x`, constant `y` for correlation, insufficient observations, or non-finite input, it shall return `HM_STATUS_UNDEFINED`, `HM_STATUS_INSUFFICIENT_DATA`, or `HM_STATUS_NONFINITE_INPUT` as documented. | `tests/test_regression.c` boundary assertions. |
| When a row-major finite predictor matrix has full column rank, multiple regression shall write an intercept-first coefficient vector and a prediction shall match the known linear model. | `tests/test_regression.c` multiple-fit/predict assertions. |
| If a multiple-regression matrix is rank deficient, too short, invalid, or allocation fails, the library shall return an explicit status and leave outputs unchanged. | `tests/test_regression.c` singular/boundary assertions. |
| When regression functions run, caller-owned predictors, responses, and coefficient inputs shall remain unchanged. | Input immutability assertions in `tests/test_regression.c`. |
| When the installed package consumer and example are built, they shall use the new APIs through public headers and print the documented school formulas. | `make package-test && cmake --build build --target harvard_math_example && ./build/examples/harvard_math_example`. |

## Holdout scenarios
- **Files:** `scenarios/inferential-regression.yaml`
- **Holdout rule:** authored here, but the Implement step must not read it; Validate/Review judges the example and installed consumer journey.

## Assumptions
- A z-test uses known population standard deviation. One- and two-sample mean tests are supported; proportion tests and unknown-σ t-tests are not.
- `hm_alternative` supports two-sided, lower-tail, and upper-tail tests.
- A confidence interval uses a known population standard deviation and a confidence fraction in `(0, 1)`; inverse-normal critical values are computed internally.
- Simple regression APIs operate on paired finite arrays and use ordinary least squares.
- Multiple regression accepts row-major predictors with shape `rows × predictor_count`, automatically adds an intercept, and writes coefficients as `[intercept, beta_1, ..., beta_p]`.
- Rank-deficient or numerically singular normal equations return `HM_STATUS_UNDEFINED`; this is a small educational API, not a general-purpose high-condition-number solver.
- `HM_STATUS_UNDEFINED` distinguishes mathematical singularity from malformed arguments.

## Out of scope (this pass)
- Unknown-σ t-tests, t-distributions, logistic regression, ANOVA, chi-square tests, and non-normal distributions.
- Proportion z-tests and continuity corrections.
- Robust regression, regularization, weighted/missing-data regression, and streaming fitting.
- Formal covariance-matrix or standard-error output for regression coefficients.
