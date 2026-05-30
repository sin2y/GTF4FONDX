#ifndef GTF_H
#define GTF_H

#include "../policy-repair/partial_state.h"
#include "../policy-repair/regression.h"
#include "fp_ter.h"
#include "fp_test.h"
#include "sc_finder.h"
#include "../qnp/num_search.h"


class Controller{
public:
    Controller(const list<std::shared_ptr<PolicyItem>> & policy_steps,const set<pair<PartialState,int> > & last_KeySteps,
               const list<std::shared_ptr<PolicyItem>> & last_Fsaps);
    std::shared_ptr<PolicyItem> readStep();
    void addKey(std::shared_ptr<PolicyItem>);
    bool isInKeys(std::shared_ptr<PolicyItem>);
    const set<pair<PartialState,int> > & getKeySteps() { return KeySteps; }
    list<std::shared_ptr<PolicyItem>> * getFsaps() { return &Fsaps; }

    list<std::shared_ptr<PolicyItem>> steps;
    list<std::shared_ptr<PolicyItem>>::iterator cur_step;
    set<pair<PartialState,int> >  KeySteps;
    list<std::shared_ptr<PolicyItem>> Fsaps;
private:
    Controller(){};

};

class ScSolution{
public:
    ScSolution(const list<std::shared_ptr<PolicyItem>> & pi, const Graph & graph, const list<std::shared_ptr<PolicyItem>> & fsaps)
        : pi(std::move(pi))
        , graph(std::move(graph))
        , fsaps(std::move(fsaps))
    {};
    ~ScSolution() {}
    list<std::shared_ptr<PolicyItem>> pi;
    Graph graph;
    list<std::shared_ptr<PolicyItem>> fsaps;
};

class CompareWeights{
public:
    CompareWeights() {}

    bool operator()(std::shared_ptr<PolicyItem> a,std::shared_ptr<PolicyItem> b) const {
        return g_action_redundant_weight[((NondetDeadend*)a.get())->get_index()]
                >
                g_action_redundant_weight[((NondetDeadend*)b.get())->get_index()];
    }
};

class GTF
{
public:
    GTF(ScFinder * sc_finder, bool strengthen_key,bool test_key);

    void AddAdversarialFsap(int types);

    void saveKey(std::shared_ptr<PolicyItem> item);
    void strengthenKey(std::shared_ptr<PolicyItem> item);
    bool testKey(const set<pair<PartialState,int> > & key_steps);
    void BackTrack();
    ScSolution FindSC();
    void Rearrange(list<std::shared_ptr<PolicyItem>> & pi);
    bool run(SieveStar *);

    bool pruning;
    list<std::shared_ptr<PolicyItem>> * fsaps;
    stack<Controller*> ctls;
    Controller* cur_ctl;

    bool strengthen_key;
    bool test_key;

    SieveStar * test_algo;
    CompareWeights weights_compare;

    ScFinder * sc_finder;
    map<PartialState,int> state_ind_map;
};

#endif
