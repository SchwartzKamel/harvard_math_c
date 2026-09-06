# AGENTS.md

## Build & run

Configure the canonical CMake + Ninja build from the repository root:

```bash
cmake -S . -B build -G Ninja -DHM_BUILD_TESTS=ON -DHM_BUILD_EXAMPLES=ON
cmake --build build
./build/examples/harvard_math_example
```

Install to a temporary prefix when checking downstream consumption:

```bash
cmake --install build --prefix /tmp/harvard_math-prefix
cmake -S tests/package_consumer -B /tmp/harvard_math-consumer -G Ninja \
  -DCMAKE_PREFIX_PATH=/tmp/harvard_math-prefix
cmake --build /tmp/harvard_math-consumer
ctest --test-dir /tmp/harvard_math-consumer --output-on-failure
```

## Validate (backpressure)

```bash
cmake -S . -B build -G Ninja -DHM_BUILD_TESTS=ON -DHM_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The installed-package consumer is part of the end-to-end validation and must also build and run.

## Operational notes

- The library targets portable C99 and links the platform math library through CMake.
- `BUILD_SHARED_LIBS=ON` enables a shared build; the default static build is equally supported.
- Empty input, null output pointers, null values with a positive count, non-finite values, and insufficient sample data return `hm_status` errors.

## Codebase patterns

- Public headers live in `include/harvard_math/`; private implementation belongs in `src/`.
- Every public C symbol starts with `hm_`; every public statistic has the shape `hm_status fn(const double *, size_t, double *)`.
- Keep common descriptive statistics single-pass and O(1) extra space. Median copies before sorting so caller data remains unchanged.
- Add public API documentation and deterministic tests together.
