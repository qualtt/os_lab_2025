#include "utils.h"

// Реализация функции MultModulo
uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
    uint64_t result = 0;
    a = a % mod;
    while (b > 0) {
        if (b % 2 == 1)
            result = (result + a) % mod;
        a = (a * 2) % mod;
        b /= 2;
    }
    return result % mod;
}

// Реализация функции ConvertStringToUI64
bool ConvertStringToUI64(const char *str, uint64_t *val) {
    char *endptr;
    *val = strtoull(str, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid number format: %s\n", str);
        return false;
    }
    return true;
}

uint64_t Factorial(const struct FactorialArgs *args) {
    uint64_t res = 1;
    for (uint64_t i = args->begin; i <= args->end; ++i) {
        res = MultModulo(res, i, args->mod);
    }
    return res;
}