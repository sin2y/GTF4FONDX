#ifndef FP_TEST_H
#define FP_TEST_H

#include "fp_ter.h"

class SieveStar {
public:
    SieveStar() {
    }

    void reset() {
        cycle_state_idx.clear();
    }

    const std::set<int> &getCycleStateIdx() {
        return cycle_state_idx;
    }

    bool operator()(Graph &graph);

private:
    std::set<int> cycle_state_idx;
};

#endif
