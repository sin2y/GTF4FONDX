#ifndef JIT_H
#define JIT_H

#include <stack>
#include <list>
#include <set>

#include "../globals.h"
#include "../operator.h"
#include "../search_engine.h"
#include "../option_parser.h"

#include "policy.h"
#include "regression.h"
#include "simulator.h"
#include "deadend.h"
#include "partial_state.h"

class Simulator;

struct UnhandledState {
    std::shared_ptr<PartialState> state;
    int cost;

    UnhandledState(std::shared_ptr<PartialState> s, int c) : state(s), cost(c) {}
    ~UnhandledState() { /*delete state;*/ }

    bool operator<(const UnhandledState& other) const { return (cost < other.cost); }

    void dump() const;
};

bool perform_jit_repairs(Simulator *sim);

struct SCNode {
    std::shared_ptr<PartialState>  full_state;
    std::shared_ptr<PartialState>  expected_state;
    std::shared_ptr<PartialState>  previous_state;
    std::shared_ptr<RegressionStep>  prev_regstep;
    const Operator * prev_op;
    SCNode(std::shared_ptr<PartialState>  fs, std::shared_ptr<PartialState>  es, std::shared_ptr<PartialState>  ps, std::shared_ptr<RegressionStep>  pr, const Operator * op) :
       full_state(fs), expected_state(es), previous_state(ps), prev_regstep(pr), prev_op(op) {}
};

#endif
