#ifndef SIGMOD_METRICS_HH
#define SIGMOD_METRICS_HH
#include <sigmod/config.hh>
#include <cmath>

template <typename WithFields>
score_t first_metric(WithFields& with_fields) {
    score_t sum = 0.0;
    for (uint32_t i = 0; i < vector_num_dimension; i++) {
        sum += with_fields.fields[i] * with_fields.fields[i];
    }
    return std::sqrt(sum);
}

template <typename WithFields>
score_t second_metric(WithFields& with_fields) {
    score_t gamma = 0.0;
    for (uint32_t i = 0; i < vector_num_dimension; i++) {
        gamma += with_fields.fields[i];
    }
    gamma /= vector_num_dimension;

    score_t sum = 0.0;
    score_t val = 0.0;

    for (uint32_t i = 0; i < vector_num_dimension; i++) {
        val = with_fields.fields[i] - gamma;
        sum += val * val;
    }
    return std::sqrt(sum);
}

#endif
