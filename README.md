# velotrax-isa-check

Public cross-architecture determinism check for the Velotrax V113R1 substrate
(test kernel tier). Same C code, same input, same math — one identical
SHA-256 hash across processor architectures.

**Verified so far:** x86_64 and ARM64 on physical hardware (see the green
runs under Actions — GitHub's `ubuntu-24.04` and `ubuntu-24.04-arm` runners),
plus RISC-V64 under QEMU emulation.

## Reproduce it yourself (2 commands)

    gcc -O2 -ffp-contract=off a1_orbital_kernel.c -lm -o k
    ./k | sha256sum

Expected (Tier B, stdout text):
`932aed1711971e98c4a26992eb94c4a13e35e3c8308e3bb3e3bf1aac1699eb77`

Binary state baseline (Tier A, raw 48-byte final state — for bare-metal
targets where printf formatting differs):

    gcc -O2 -ffp-contract=off -DDUMP_STATE a1_orbital_kernel.c -lm -o kd
    ./kd | sha256sum

Expected:
`03450a5360e2bcec3f8577c357c79f704b9f224a5e0b0449da370ecf4fe94e42`

## What this does and doesn't show

- ✅ Bit-identical floating-point orbital propagation (RK4, 100,000 steps)
  across x86_64 / ARM64 / RISC-V under documented compiler-flag discipline
  (`-ffp-contract=off`, no `-march=native`).
- ❌ Not yet covered: cross-compiler (Clang/MSVC), RISC-V physical silicon,
  and kernels using transcendental libm functions — scoped for the next
  validation cycle.

Rule this repo exists to demonstrate: **if you can't independently rebuild
the hash, it doesn't ship.**

Velotrax — deterministic compute verification for space software.
Contact: Shiva Allu · allu.siva@gmail.com
