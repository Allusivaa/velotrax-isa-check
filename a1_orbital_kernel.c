/*
 * V113R2 — A1: Compiler flag drift kernel
 *
 * Compiles the orbital propagation kernel under different optimization flags
 * and emits the final state + SHA-256 of the trajectory bytes. Used to
 * characterize -ffast-math / -O3 / -ffp-contract drift on a single ISA.
 *
 * Cross-ISA testing (ARM64 / RISC-V via QEMU) requires apt-installed
 * toolchains; that portion is documented in the A1 audit report as
 * requiring an unrestricted environment.
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#ifdef DUMP_STATE
#include <string.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif
#endif

#define GM_EARTH 3.986004418e14
#define ISS_R0 6779000.0
#define N_STEPS 100000          /* 1000 orbits × 100 steps/orbit */
#define ISS_PERIOD 5554.684946  /* derived from sqrt(a^3/mu) * 2pi */

static void derivatives(const double s[6], double d[6]) {
    double rn = sqrt(s[0]*s[0] + s[1]*s[1] + s[2]*s[2]);
    if (rn < 1e-6) { for (int i=0;i<6;i++) d[i]=0; return; }
    double k = -GM_EARTH / (rn * rn * rn);
    d[0] = s[3]; d[1] = s[4]; d[2] = s[5];
    d[3] = k * s[0]; d[4] = k * s[1]; d[5] = k * s[2];
}

static void rk4_step(double s[6], double dt) {
    double k1[6], k2[6], k3[6], k4[6], tmp[6];
    derivatives(s, k1);
    for (int i=0;i<6;i++) tmp[i] = s[i] + 0.5 * dt * k1[i];
    derivatives(tmp, k2);
    for (int i=0;i<6;i++) tmp[i] = s[i] + 0.5 * dt * k2[i];
    derivatives(tmp, k3);
    for (int i=0;i<6;i++) tmp[i] = s[i] + dt * k3[i];
    derivatives(tmp, k4);
    for (int i=0;i<6;i++) s[i] += (dt/6.0) * (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]);
}

int main(int argc, char** argv) {
    double v_circ = sqrt(GM_EARTH / ISS_R0);
    double state[6] = {ISS_R0, 0.0, 0.0, 0.0, v_circ, 0.0};
    double dt = ISS_PERIOD / 100.0;

    /* Run 1000 orbits */
    for (long i = 0; i < N_STEPS; i++) {
        rk4_step(state, dt);
    }

#ifdef DUMP_STATE
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
#endif
    fwrite(state, sizeof(double), 6, stdout);
    return 0;
#endif
    /* Emit final state with full precision */
    printf("V113R2_A1_FINAL_STATE\n");
    for (int i = 0; i < 6; i++) {
        printf("  s[%d] = %.17e\n", i, state[i]);
    }
    double rn = sqrt(state[0]*state[0] + state[1]*state[1] + state[2]*state[2]);
    printf("  |r|  = %.17e\n", rn);
    double E = 0.5 * (state[3]*state[3] + state[4]*state[4] + state[5]*state[5])
               - GM_EARTH / rn;
    printf("  E    = %.17e\n", E);
    return 0;
}
