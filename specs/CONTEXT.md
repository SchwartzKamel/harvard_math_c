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

## Avoid (non-canonical aliases)

- “average” -> use **mean** and `hm_mean`.
- “standard deviation” in an API name -> use **stddev** with an explicit population/sample suffix.
- “error code” -> use **status** and `hm_status`.
- “input list” -> use **values** and **count**.

## Open questions

Streaming, weighted, and inferential statistics are later extensions. The performance pass compares the checked batch API against a reference and optional peer implementations.
