#ifndef SCFINDER_H
#define SCFINDER_H

#include "../globals.h"
#include "../global_man.h"
#include "../operator.h"
#include "../option_parser.h"
#include "../timer.h"
#include "../utilities.h"
#include "../search_engine.h"
#include "../policy-repair/regression.h"
#include "../policy-repair/simulator.h"
#include "../policy-repair/policy.h"
#include "../policy-repair/jit.h"
#include "../policy-repair/partial_state.h"
#include "fp_ter.h"

class ScFinder
{
    SearchEngine * engine;
    Simulator * sim;
    void reset();
    void buildPlan();
public:
    ScFinder();
    bool solve(list<std::shared_ptr<PolicyItem> > fsaps);
    void clear();

    bool is_sc;
    std::shared_ptr<list<std::shared_ptr<PolicyItem>>> full_steps;
    std::shared_ptr<map<PartialState,int>> state_idx_map;
    std::shared_ptr<Graph> solution_graph;


};

#endif // SCFINDER_H
