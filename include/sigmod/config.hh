#ifndef CONFIG_HH
#define CONFIG_HH

#include <cstdint>

typedef float float32_t;
typedef double score_t;

const uint32_t k_nearest_neighbors = 100;
const uint32_t vector_num_dimension = 100;
const uint32_t batch_size = 10000;

#define TOT_ELEMENTS 1000
#define STOP_AFTER_TOT_ELEMENTS
// #define DISATTEND_CHECKS
// #define ENABLE_KD_FOREST
// #define ENABLE_BALL_FOREST
#define ENABLE_EXAUSTIVE

#endif
