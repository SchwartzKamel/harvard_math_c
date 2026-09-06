# CONTEXT — harvard_math vocabulary

## Terms

| Term | Means | Canonical name (code / UI) | Not to be confused with |
|---|---|---|---|
| Statistic | A function that maps a finite batch of `double` values to one `double` result. | `hm_statistic_fn` | A probability distribution or a stored value. |
| Batch API | An API that receives an immutable array and element count in one call. | `values`, `count` | A streaming accumulator. |
| Population variance | Variance using divisor `n`. | `hm_variance_population` | Sample variance using divisor `n - 1`. |
| Sample variance | Unbiased sample estimate using divisor `n - 1`; requires at least two values. | `hm_variance_sample` | Population variance. |
| Status | The explicit result code returned by a public operation. | `hm_status` | A floating-point sentinel value. |
| Public header | An installed header intended for consumers. | `include/harvard_math/*.h` | A private header under `src/`. |
| Install tree | The files produced by `cmake --install` for downstream consumers. | `CMAKE_INSTALL_PREFIX` | The in-tree build directory. |
| Reference implementation | A deliberately straightforward same-contract implementation used as a performance and result baseline. | `reference` | A universal mathematical oracle. |
| Benchmark | A repeatable timed measurement that first checks result equivalence. | `benchmark` | A correctness test or a universal performance guarantee. |
| Speedup | Reference time divided by library time; values above 1.0 mean the library was faster in that run. | `speedup` | A causal claim about every machine. |
| Peer implementation | An established external library with an adapter that reconciles API semantics before comparison. | `peer` | The handwritten reference. |
| Z-score | A standardized scalar value computed as `(value - mean) / stddev`. | `hm_zscore` | A z-test statistic. |
| Slope | The ordinary least-squares slope of paired response values against predictor values. | `hm_slope` | An intercept, correlation, or prediction. |
| Undefined result | A mathematically requested result with no finite definition for the supplied inputs. | `HM_STATUS_UNDEFINED` | A malformed pointer or non-finite input. |
| Alternative | The tail direction used by a hypothesis test. | `hm_alternative` | A confidence level. |
| P-value | The normal-tail probability associated with a z statistic and alternative. | `p_value` | A confidence interval bound. |
| Confidence interval | An estimated lower/upper range using a known population standard deviation and confidence fraction. | `hm_confidence_interval` | A prediction interval. |
| Intercept | The fitted response value when the predictor is zero. | `hm_intercept` | A slope. |
| Correlation | Pearson's standardized linear association for paired arrays. | `hm_correlation` | A regression slope. |
| Predictor matrix | Row-major multiple-regression input with one row per observation and no intercept column. | `predictors` | A column-major matrix or coefficient vector. |

## Avoid (non-canonical aliases)

- “average” -> use **mean** and `hm_mean`.
- “standard deviation” in an API name -> use **stddev** with an explicit population/sample suffix.
- “error code” -> use **status** and `hm_status`.
- “input list” -> use **values** and **count**.

## Open questions

Streaming, weighted, and inferential statistics are later extensions. The performance pass compares the checked batch API against a reference and optional peer implementations. The school-core slice adds z-score and ordinary least-squares slope without adding hypothesis tests or a full regression model. The inferential-regression slice adds known-σ z-tests/intervals and educational simple/multiple regression APIs, not a general statistical modeling system.
