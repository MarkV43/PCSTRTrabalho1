#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "udpclient.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#define NSEC_PER_SEC 1000000000

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: trabalho1 <address> <port>");
        exit(1);
    }
    int port = atoi(argv[2]);
    setup_sockets(port, argv[1]);

    char response[1000];
//    exchange_message("st-0", response);

    struct timespec t;
    long interval = 30000000;
    clock_gettime(CLOCK_MONOTONIC, &t);
    t.tv_sec++;

    //constants
    const float R = 0.001f,
                S = 4184,
                B = 4,
                P = 1000,
                Href = 2.5f,
                Tref = 40,
                Vref = Href * B;

    // PID constants
    const float cpNa = 1,
                ciNa = 300,
                cdNa = 1;
    const float cpNi = -1,
                ciNi = -300,
                cdNi = -1;
    const float cpN = 1,
                ciN = 10,
                cdN = 0.5f;
    const float cpQ = 100,
                ciQ = 300;

    // errors
    float Terr;
    float TintNa = 0;
    float TintNi = 0;
    float TintQ = 0;
    float Verr;
    float Vint = 0;
    float Vpre = 0;
    float Vder;

    // sensors
    float T, Ta, Ti, No, H;
    // actuators
    float Q, Ni, Na, Nf;
    // other
    float V, N, Nar, Nir, Nas, Qar;

    bool satNa = false,
         satNi = false,
         satQ = false;

    char buffer[20] = "a__";

    int i = 0;
    while (1) {
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

        exchange_message("sta0", response);
        Ta = strtof(response + 3, NULL);
        exchange_message("st-0", response);
        T = strtof(response + 3, NULL);
        exchange_message("sti0", response);
        Ti = strtof(response + 3, NULL);
        exchange_message("sno0", response);
        No = strtof(response + 3, NULL);
        exchange_message("sh-0", response);
        H = strtof(response + 3, NULL);

        Terr = Tref - T;
        if (!satNa) {
            TintNa += Terr * 0.03f; // 30ms
        }
        if (!satNi) {
            TintNi += Terr * 0.03f; // 30ms
        }
        if (!satQ) {
            TintQ += Terr * 0.03f;
        }

        V = H * B;
        Verr = Vref - V; // m³
        Vint += Verr * 0.03f;
        Vder = (V - Vpre) / 0.03f;
        Vpre = V;

        if (++i == 33) {
            i = 0;
            printf("%f\n%f\nNa: %f\n", TintNa, TintNi, (Ta + Ti * No * R * S - Tref * (No * R * S + 1)) / ((Ti - 80) * R * S));
        }

        /* PID control */

        N = Verr * cpN * 6;

        float prop = (80 - Tref) / (Tref - Ti);
        // Ni = Na * prop
        // N = Na + Ni
        // N = Na + Na * prop
        // N = Na (1 + prop)
        // Na = N / (1 + prop)
        // Ni = N - Na

        Nar = (Ta + Ti * No * R * S - Tref * (No * R * S + 1)) / ((Ti - 80) * R * S);
        Nas = (N + No) / (1 + prop);
        Na = Nar + cpNa * Terr + Nas;
        if (!satNa) {
            Na += TintNa / ciN;
        }

        satNa = false;
        if (Na > 10) {
            Na = 10;
            satNa = true;
        } else if (Na < 0) {
            Na = 0;
            satNa = true;
        }

        Nir = No - Na;
        Ni = Nir + cpNi * Terr + N + No - Nas;
        if (!satNi) {
            Ni += TintNi / ciNi;
        }

        satNi = false;
        if (Ni > 100) {
            Ni = 100;
            satNi = true;
        } else if (Ni < 0) {
            Ni = 0;
            satNi = true;
        }

//        Nf = Ni + Na - No;

        Qar = S * Ni * (Tref - Ti) - S * Na * (80 - Tref) + (Tref - Ta) / R;
        Q = cpQ * Terr + Qar;
        if (!satQ) {
            Q += TintQ / ciQ;
        }

        satQ = false;
        if (Q > 1000000) {
            Q = 1000000;
            satQ = true;
        } else if (Q < 0) {
            Q = 0;
            satQ = true;
        }

        buffer[1] = 'n';
        buffer[2] = 'a';
        gcvt(Na, 6, buffer + 3);
        exchange_message(buffer, response);
        buffer[2] = 'i';
        gcvt(Ni, 6, buffer + 3);
        exchange_message(buffer, response);
//        buffer[2] = 'f';
//        gcvt(Nf, 6, buffer + 3);
//        exchange_message(buffer, response);
        buffer[1] = 'q';
        buffer[2] = '-';
        gcvt(Q, 6, buffer + 3);
        exchange_message(buffer, response);

        t.tv_nsec += interval;
        while (t.tv_nsec >= NSEC_PER_SEC) {
            t.tv_nsec -= NSEC_PER_SEC;
            t.tv_sec++;
        }
    }
}
#pragma clang diagnostic pop