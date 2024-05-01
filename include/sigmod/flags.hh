#ifndef FLAGS_HH
#define FLAGS_HH

/* FLAGS */

#define USE_FIRST_METRIC
// #define USE_SECOND_METRIC

/* RULES */

#ifdef USE_FIRST_METRIC
  #ifdef USE_SECOND_METRIC
    #error "unable at most one metric"
  #endif
#else
  #ifndef USE_SECOND_METRIC
    #error "unable at least one metric"
  #endif
#endif

#endif
