#ifndef SEEK_HH
#define SEEK_HH

#include <sigmod/record.hh>
#include <iostream>

/* Seek(accessor, start, end, value) {
 *  - accessor is a callable Index -> Value such that accessor(i) is the i-th element of a collection
 *  - the indexes of the sub-collection to explore are assumed to be [start, end)
 *    - **note the exclusion of end**
 *  - value is the value to seek
 * }
 *
 * If the value isn't in the collection {
 *  - SeekLow finds the position immediately before it
 *  - SeekHigh finds the prositiom immediately after it
 * }
 * */

template <typename Accessor>
uint32_t SeekLow(const Accessor accessor,
                 const uint32_t start,
                 const uint32_t end,
                 const float32_t value) {
    uint32_t l = start;
    uint32_t r = end - 1;
    uint32_t m = (l + r) / 2;

    while(l < r) {
        const float32_t center = accessor(m);
        if (center > value && m > start) {
            r = m - 1;
            m = (l + r) / 2;
        } else if (center < value && m < end - 1) {
            l = m + 1;
            m = (l + r) / 2;
        } else {
            while(m < end - 1 && accessor(m + 1) == value)
              m++;
            return m;
        }
    }

    return r;
}

template <typename Accessor>
uint32_t SeekHigh(const Accessor accessor,
                  const uint32_t start,
                  const uint32_t end,
                  const float32_t value) {
    uint32_t l = start;
    uint32_t r = end - 1;
    uint32_t m = (l + r) / 2;

    while(l < r) {
        const float32_t center = accessor(m);
        if (center > value && m > start) {
            r = m - 1;
            m = (l + r) / 2;
        } else if (center < value && m < end - 1) {
            l = m + 1;
            m = (l + r) / 2;
        } else {
            while(m > start && accessor(m - 1) == value)
              m--;
            return m;
        }
    }

    return l;
}

#endif
