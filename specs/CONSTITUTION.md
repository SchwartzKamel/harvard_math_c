# Constitution: harvard_math

> Project-wide principles for the importable C library.

## Principles
1. **Code quality** — Keep the public surface small and explicit. Use C99, `hm_`-prefixed symbols, const-correct inputs, documented ownership, and status-returning APIs for recoverable errors. Keep implementation details private to `src/`.
2. **Testing** — Every public statistic has normal and boundary/error coverage. CMake/CTest is the canonical validation path, and at least one test consumes the library through its installed package interface.
3. **Security & privacy** — Perform pointer/count validation before dereferencing, never read or write outside caller-provided ranges, never use global mutable state, and never perform I/O or log caller data.
4. **UX & consistency** — A caller can include `<harvard_math/harvard_math.h>`, pass a `const double` array and its length, and receive a clear `hm_status`. All custom statistics use the same function signature as built-ins.
5. **Performance** — Common descriptive statistics are O(n) and use O(1) additional storage. Median may use an O(n) copy and O(n log n) sort because it must not mutate caller data. Do not add external runtime dependencies.

## Non-negotiables
- The public API must not invoke undefined behavior for documented invalid inputs; it returns an error status instead.
- Sample and population formulas must remain separate and explicitly named.
- Any new public symbol must be documented and covered by a deterministic test.

## Tech constraints
- **Must use:** portable C99, CMake (with Ninja recommended), the C standard library, and the platform math library where required. The Makefile may only wrap CMake commands.
- **Must avoid:** third-party runtime dependencies, hidden allocation in O(1)-space statistics, and a command-line interface in the library core.
- **Deployment target:** installed static or shared library consumed through CMake `find_package` or pkg-config.

## Recording deviations

| Date | Principle | Why we deviated | Scope |
|---|---|---|---|
| None | None | No deviations recorded. | None |

**Version**: 1.0 | **Ratified**: 2026-09-06 | **Last Amended**: 2026-09-06
