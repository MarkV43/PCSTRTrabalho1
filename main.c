#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
                Tref = 40;
    // sensors
    float T, Ta, Ti, No, H;
    // actuators
    float Q, Ni, Na, Nf;

    char buffer[20] = "a__";

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

        /* PID control */

        Na = (Ta + Ti * No * R * S - Tref * (No * R * S + 1)) / ((Ti - 80) * R * S);

        if (Na > 10) {
            Na = 10;
        } else if (Na < 0) {
            Na = 0;
        }

        Ni = No - Na;

        if (Ni > 100) {
            Ni = 100;
        } else if (Ni < 0) {
            Ni = 0;
        }

        Nf = Ni + Na - No;

        Q = S * Ni * (Tref - Ti) - S * Na * (80 - Tref) + (Tref - Ta) / R;

        if (Q > 1000000) {
            Q = 1000000;
        } else if (Q < 0) {
            Q = 0;
        }

        buffer[1] = 'n';
        buffer[2] = 'a';
        gcvt(Na, 6, buffer + 3);
        exchange_message(buffer, response);
        buffer[2] = 'i';
        gcvt(Ni, 6, buffer + 3);
        exchange_message(buffer, response);
        buffer[2] = 'f';
        gcvt(Nf, 6, buffer + 3);
        exchange_message(buffer, response);
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