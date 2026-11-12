# Contributing rules

These rules define the project. A change that breaks them is rejected regardless of
how much it gets working.

## Write new code; never translate machine code

For every function:

1. Work out what it is **for**: inputs, outputs, side effects, its role in the game.
2. Write fresh C++ that does that job, the way an engineer would write it from a spec.
3. Use the disassembly only to understand behaviour, never as text to convert.

Red flags that mean the code is a translation, not a reimplementation:
register-style variable names, emulated stacks or memory, `goto` webs, one statement
per instruction, magic offsets into structs instead of named fields. Renaming the
variables of a translation does not make it new code.

## Never depend on the original binary

- No `NATIVE(address, name)` macros or anything that forwards to original code.
- No original addresses used as pointers. Addresses appear **only in comments**, for
  traceability.
- No code carried over from earlier recompilation/lifting efforts.
- No game assets, binaries, metadata dumps or method lists in the repository.

## Unfinished functions

Leave a TODO with the original address and full name, plus the smallest stub that
makes it obvious it isn't done:

```cpp
// TODO(0x7be2af0, Quantum.FPMathUtility.OrientOnNormal): needs FPQuaternion.
```

If a stub must exist so callers compile, its body is empty or asserts — it never
fakes a plausible result.

## Style

- Keep original class/method names where known (`Quantum::FPMathUtility::Map`);
  choose descriptive names where not.
- Reconstruct structs, enums and signatures from how data is used; name every field.
- Put the original address above each implemented function as a comment.
- Flag uncertainty in a comment instead of guessing silently.
- Platform/engine services go through interfaces in `src/platform/` (or future engine
  modules), never directly to an OS API from game code.
- The build must stay green: every commit configures, builds and passes `ctest`.
- Format with the repository's `.clang-format`.

## Determinism

The gameplay simulation is lockstep-deterministic. Simulation code uses `rr::det::FP`
fixed-point maths only — no `float`/`double` — and must preserve the original's
rounding behaviour (see comments in `src/core/include/rr/core/fp.h`).
