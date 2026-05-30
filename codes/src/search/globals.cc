#include "globals.h"

#include "axioms.h"
#include "causal_graph.h"
#include "domain_transition_graph.h"
#include "heuristic.h"
#include "int_packer.h"
#include "legacy_causal_graph.h"
#include "operator.h"
#include "rng.h"
#include "state.h"
#include "state_registry.h"
#include "successor_generator.h"
#include "timer.h"
#include "utilities.h"
#include "policy-repair/policy.h"
#include "policy-repair/regression.h"
#include "policy-repair/deadend.h"
#include "fond_plus/fp_ter.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

#include <ext/hash_map>
using namespace __gnu_cxx;

#include "state_registry.h"

static const int PRE_FILE_VERSION = 3;


// TODO: This needs a proper type and should be moved to a separate
//       mutexes.cc file or similar, accessed via something called
//       g_mutexes. (Right now, the interface is via global function
//       are_mutex, which is at least better than exposing the data
//       structure globally.)

static vector<vector<set<pair<int, int> > > > g_inconsistent_facts;

DeadendTuple::~DeadendTuple() { ; }

bool test_goal(const State &state) {
    if (g_policy && !(g_policy->empty()))
        return test_policy(state);

    for (int i = 0; i < g_goal.size(); i++) {
        if (state[g_goal[i].first] != g_goal[i].second) {
            return false;
        }
    }

    g_matched_distance = 0;

    std::shared_ptr<PartialState> gs = make_shared<PartialState> ();
    for (int i = 0; i < g_goal.size(); i++)
        (*gs)[g_goal[i].first] = g_goal[i].second;
    std::shared_ptr<RegressionStep> grs = make_shared<RegressionStep>(gs, 0);

    g_matched_policy = grs;

    return true;
}

bool test_policy(const State &state) {

    std::shared_ptr<RegressionStep>  best_step = g_policy->get_best_step(state);

    if ((best_step && g_plan_with_policy) || (best_step && best_step->is_goal)) {

        g_matched_policy = best_step;
        g_matched_distance = best_step->distance;

        return true;
    } else {
        return false;
    }
}

int calculate_plan_cost(const vector<const Operator *> &plan) {
    // TODO: Refactor: this is only used by save_plan (see below)
    //       and the SearchEngine classes and hence should maybe
    //       be moved into the SearchEngine (along with save_plan).
    int plan_cost = 0;
    for (int i = 0; i < plan.size(); i++) {
        plan_cost += plan[i]->get_cost();
    }
    return plan_cost;
}

void save_plan(const vector<const Operator *> &plan, int iter) {
    // TODO: Refactor: this is only used by the SearchEngine classes
    //       and hence should maybe be moved into the SearchEngine.
    ofstream outfile;
    if (iter == 0) {
        outfile.open(g_plan_filename.c_str(), ios::out);
    } else {
        ostringstream out;
        out << g_plan_filename << "." << iter;
        outfile.open(out.str().c_str(), ios::out);
    }
    for (int i = 0; i < plan.size(); i++) {
        if(g_findsc_info){
            cout << plan[i]->get_name() << " (" << plan[i]->get_cost() << ")" << endl;
        }
        outfile << "(" << plan[i]->get_name() << ")" << endl;
    }
    outfile.close();
    int plan_cost = calculate_plan_cost(plan);
    ofstream statusfile;
    statusfile.open("plan_numbers_and_cost", ios::out | ios::app);
    statusfile << iter << " " << plan_cost << endl;
    statusfile.close();
    if(g_findsc_info){
        cout << "Plan length: " << plan.size() << " step(s)." << endl;
        cout << "Plan cost: " << plan_cost << endl;
    }
}

bool peek_magic(istream &in, string magic) {
    string word;
    in >> word;
    bool result = (word == magic);
    for (int i = word.size() - 1; i >= 0; i--)
        in.putback(word[i]);
    return result;
}

void check_magic(istream &in, string magic) {
    string word;
    in >> word;
    if (word != magic) {
        cout << "Failed to match magic word '" << magic << "'." << endl;
        cout << "Got '" << word << "'." << endl;
        if (magic == "begin_version") {
            cerr << "Possible cause: you are running the planner "
                 << "on a preprocessor file from " << endl
                 << "an older version." << endl;
        }
        exit_with(EXIT_INPUT_ERROR);
    }
}

void read_and_verify_version(istream &in) {
    int version;
    check_magic(in, "begin_version");
    in >> version;
    check_magic(in, "end_version");
    if (version != PRE_FILE_VERSION) {
        cerr << "Expected preprocessor file version " << PRE_FILE_VERSION
             << ", got " << version << "." << endl;
        cerr << "Exiting." << endl;
        exit_with(EXIT_INPUT_ERROR);
    }
}

void read_metric(istream &in) {
    check_magic(in, "begin_metric");
    in >> g_use_metric;
    check_magic(in, "end_metric");
}

void read_variables(istream &in) {
    int count;
    in >> count;
    for (int i = 0; i < count; i++) {
        check_magic(in, "begin_variable");
        string name;
        in >> name;
        g_variable_name.push_back(name);
        int layer;
        in >> layer;
        g_axiom_layers.push_back(layer);
        int range;
        in >> range;
        g_variable_domain.push_back(range);
        in >> ws;
        vector<string> fact_names(range);
        for (size_t i = 0; i < fact_names.size(); i++)
            getline(in, fact_names[i]);
        g_fact_names.push_back(fact_names);
        check_magic(in, "end_variable");
    }
}

void read_mutexes(istream &in) {
    g_inconsistent_facts.resize(g_variable_domain.size());
    for (size_t i = 0; i < g_variable_domain.size(); ++i)
        g_inconsistent_facts[i].resize(g_variable_domain[i]);

    int num_mutex_groups;
    in >> num_mutex_groups;

    /* NOTE: Mutex groups can overlap, in which case the same mutex
       should not be represented multiple times. The current
       representation takes care of that automatically by using sets.
       If we ever change this representation, this is something to be
       aware of. */

    for (size_t i = 0; i < num_mutex_groups; ++i) {
        check_magic(in, "begin_mutex_group");
        int num_facts;
        in >> num_facts;
        vector<pair<int, int> > invariant_group;
        invariant_group.reserve(num_facts);
        for (size_t j = 0; j < num_facts; ++j) {
            int var, val;
            in >> var >> val;
            invariant_group.push_back(make_pair(var, val));
        }
        check_magic(in, "end_mutex_group");
        for (size_t j = 0; j < invariant_group.size(); ++j) {
            const pair<int, int> &fact1 = invariant_group[j];
            int var1 = fact1.first, val1 = fact1.second;
            for (size_t k = 0; k < invariant_group.size(); ++k) {
                const pair<int, int> &fact2 = invariant_group[k];
                int var2 = fact2.first;
                if (var1 != var2) {
                    /* The "different variable" test makes sure we
                       don't mark a fact as mutex with itself
                       (important for correctness) and don't include
                       redundant mutexes (important to conserve
                       memory). Note that the preprocessor removes
                       mutex groups that contain *only* redundant
                       mutexes, but it can of course generate mutex
                       groups which lead to *some* redundant mutexes,
                       where some but not all facts talk about the
                       same variable. */
                    g_inconsistent_facts[var1][val1].insert(fact2);
                }
            }
        }
    }
}

void read_goal(istream &in) {
    check_magic(in, "begin_goal");
    int count;
    in >> count;
    if (count < 1) {
        cerr << "Task has no goal condition!" << endl;
        exit_with(EXIT_INPUT_ERROR);
    }
    for (int i = 0; i < count; i++) {
        int var, val;
        in >> var >> val;
        g_goal.push_back(make_pair(var, val));
        g_goal_orig.push_back(make_pair(var, val));
    }
    check_magic(in, "end_goal");
}

void dump_goal() {
    cout << "Goal Conditions:" << endl;
    for (int i = 0; i < g_goal.size(); i++)
        cout << "  " << g_variable_name[g_goal[i].first] << ": "
             << g_goal[i].second << endl;
}

void read_operators(istream &in) {
    int count;
    in >> count;
    for (int i = 0; i < count; i++)
        g_operators.push_back(Operator(in, false, i));
}

void read_axioms(istream &in) {
    int count;
    in >> count;

    // HAZ: Make sure axioms are /not/ considered
    if (count > 0) {
        cout << "\n\nError: Axioms are not permitted.\n" << endl;
        exit(1);
    }
    for (int i = 0; i < count; i++)
        g_axioms.push_back(Operator(in, true, i));

    g_axiom_evaluator = new AxiomEvaluator;
}

void read_fairnesses(istream & in){
    int count;
    in >> count;

//    for(auto kv : opname_fpaction_map){
//        cout << kv.first << " " << kv.second << endl;
//    }

    string action_name;
    for(int i = 0; i < count; ++i){
        check_magic(in,"begin_fairness");
        int num_A = 0, num_B = 0;
        in >> num_A;
        in >> num_B;
        set<FpAction*> * A = new set<FpAction*>;
        set<FpAction*> * B = new set<FpAction*>;
        ABPair * ABp = new ABPair(A,B);
        FpAction* cur_action = nullptr;
        for(int j = 0; j < num_A; ++j){
            in >> ws;
            std::getline(in,action_name);
            if(g_message_debug)
                cout << ":a " << action_name << endl;
            if(g_nondet_index_mapping.find(action_name) == g_nondet_index_mapping.end()){ // TODO: count -> find
//                if(g_message_debug)
                cout << "Warning! Action " << "(" << action_name << ")" << " in fairness assumption cannot be found in g_nondet_mapping!" << endl;
//                exit(1);
                continue;
            }
            cur_action = g_fp_actions[g_nondet_index_mapping[action_name]];
            A->insert(cur_action);
            // associate the action with AB
            cur_action->belongsToA(ABp);
        }
        for(int j = 0; j < num_B; ++j){
            std::getline(in,action_name);
            if(g_message_debug)
                cout << ":b " << action_name << endl;
            if(g_nondet_index_mapping.find(action_name) == g_nondet_index_mapping.end()){ // TODO: count -> find
//                if(g_message_debug)
                cout << "Warning! Action " << "(" << action_name << ")" << " in fairness assumption cannot be found in g_nondet_mapping!" << endl;
//                exit(1);
                continue;
            }
            cur_action = g_fp_actions[g_nondet_index_mapping[action_name]];
            B->insert(cur_action);
            // associate the action with AB
            cur_action->belongsToB(ABp);
        }
        g_fp_ABs.push_back(ABp);
        check_magic(in,"end_fairness");
    }
}

void read_everything(istream &in) {
    read_and_verify_version(in);
    read_metric(in);
    read_variables(in);
    read_mutexes(in);
    g_initial_state_data.resize(g_variable_domain.size());
    check_magic(in, "begin_state");
    for (int i = 0; i < g_variable_domain.size(); i++) {
        in >> g_initial_state_data[i];
    }
    check_magic(in, "end_state");
    g_default_axiom_values = g_initial_state_data;

    read_goal(in);
    if(g_var_action){
        g_variable_domain.push_back(-1);
        g_variable_name.push_back("var_action");
        g_axiom_layers.push_back(-1); // TODO ?
    }
    read_operators(in);
    if(g_var_action){
        int range = g_operators.size();
        g_variable_domain[g_variable_domain.size() - 1] = range;
        vector<string> fact_names(range);
        for (size_t i = 0; i < fact_names.size(); i++)
            fact_names[i] = g_operators[i].get_name();
        g_fact_names.push_back(fact_names);
    }
    read_axioms(in);
    check_magic(in, "begin_SG");
    g_successor_generator_orig = read_successor_generator(in);
    check_magic(in, "end_SG");
    DomainTransitionGraph::read_all(in);
    g_legacy_causal_graph = new LegacyCausalGraph(in);

    // NOTE: causal graph is computed from the problem specification,
    // so must be built after the problem has been read in.
    g_causal_graph = new CausalGraph;

    assert(!g_variable_domain.empty());
    g_state_packer = new IntPacker(g_variable_domain);
    cout << "Variables: " << g_variable_domain.size() << endl;
    cout << "Bytes per state: "
         << g_state_packer->get_num_bins() *
            g_state_packer->get_bin_size_in_bytes() << endl;

    // NOTE: state registry stores the sizes of the state, so must be
    // built after the problem has been read in.
    g_state_registry = new StateRegistry;

    /* Build the data structures required for mapping between the
     * deterministic actions and their non-deterministic equivalents. */
    int cur_nondet = 0;
    for (int i = 0; i < g_operators.size(); i++) {

        int nondet_index = -1;

        if (g_nondet_index_mapping.find(g_operators[i].get_nondet_name()) == g_nondet_index_mapping.end()) {

            nondet_index = cur_nondet;
            g_nondet_index_mapping[g_operators[i].get_nondet_name()] = cur_nondet;

            g_nondet_mapping.push_back(new vector<Operator *>());
            g_nondet_conditional_mask.push_back(new vector<int>());

            cur_nondet++;

        } else {
            nondet_index = g_nondet_index_mapping[g_operators[i].get_nondet_name()];
        }

        g_operators[i].nondet_index = nondet_index;
        g_nondet_mapping[nondet_index]->push_back(&g_operators[i]);

        for (int j = 0; j < g_operators[i].get_pre_post().size(); j++) {
            for (int k = 0; k < g_operators[i].get_pre_post()[j].cond.size(); k++) {
                int var = g_operators[i].get_pre_post()[j].cond[k].var;
                vector<int> *var_list = g_nondet_conditional_mask[nondet_index];
                if (find(var_list->begin(), var_list->end(), var) == var_list->end())
                    g_nondet_conditional_mask[nondet_index]->push_back(var);
            }
        }
    }

    for(size_t i = 0;i < g_nondet_mapping.size();++i){
        string op_name = (*(g_nondet_mapping[i]))[0]->get_nondet_name();
        FpAction* new_fp_action = new FpAction(op_name,/*action_index = */i);
        g_fp_actions.push_back(new_fp_action);
    }

    if(!g_strong_planning && !g_sc_planning)
        read_fairnesses(in);
}

void dump_everything() {
    cout << "Use metric? " << g_use_metric << endl;
    cout << "Min Action Cost: " << g_min_action_cost << endl;
    cout << "Max Action Cost: " << g_max_action_cost << endl;
    // TODO: Dump the actual fact names.
    cout << "Variables (" << g_variable_name.size() << "):" << endl;
    for (int i = 0; i < g_variable_name.size(); i++)
        cout << "  " << g_variable_name[i]
             << " (range " << g_variable_domain[i] << ")" << endl;
    State initial_state = g_initial_state();
    cout << "Initial State (PDDL):" << endl;
    initial_state.dump_pddl();
    cout << "Initial State (FDR):" << endl;
    initial_state.dump_fdr();
    dump_goal();
    /*
    cout << "Successor Generator:" << endl;
    g_successor_generator->dump();
    for(int i = 0; i < g_variable_domain.size(); i++)
      g_transition_graphs[i]->dump();
    */
}

bool has_axioms() {
    return !g_axioms.empty();
}

void verify_no_axioms() {
    if (has_axioms()) {
        cerr << "Heuristic does not support axioms!" << endl << "Terminating."
             << endl;
        exit_with(EXIT_UNSUPPORTED);
    }
}

int get_first_cond_effects_op_id() {
    for (int i = 0; i < g_operators.size(); i++) {
        const vector<PrePost> &pre_post = g_operators[i].get_pre_post();
        for (int j = 0; j < pre_post.size(); j++) {
            const vector<Prevail> &cond = pre_post[j].cond;
            if (cond.empty())
                continue;
            // Accept conditions that are redundant, but nothing else.
            // In a better world, these would never be included in the
            // input in the first place.
            int var = pre_post[j].var;
            int pre = pre_post[j].pre;
            int post = pre_post[j].post;
            if (pre == -1 && cond.size() == 1 && cond[0].var == var
                && cond[0].prev != post && g_variable_domain[var] == 2)
                continue;
            return i;
        }
    }
    return -1;
}

bool has_cond_effects() {
    return get_first_cond_effects_op_id() != -1;
}

void verify_no_cond_effects() {
    int op_id = get_first_cond_effects_op_id();
    if (op_id != -1) {
            cerr << "Heuristic does not support conditional effects "
                 << "(operator " << g_operators[op_id].get_name() << ")" << endl
                 << "Terminating." << endl;
            exit_with(EXIT_UNSUPPORTED);
    }
}

void verify_no_axioms_no_cond_effects() {
    verify_no_axioms();
    verify_no_cond_effects();
}

bool are_mutex(const pair<int, int> &a, const pair<int, int> &b) {
    if (a.first == b.first) // same variable: mutex iff different value
        return a.second != b.second;
    return bool(g_inconsistent_facts[a.first][a.second].count(b));
}

const State &g_initial_state() {
    return g_state_registry->get_initial_state();
}


void dumpActionInd(const string &file_name)
{
    fstream file;
    file.open(file_name.c_str(),ios::out);

    for(size_t di = 0;di < g_operators.size();++di){

        file << g_operators[di].get_name() << ";";
        std::set<std::string>pres;
        std::set<std::string>effs;
        vector<PrePost> pps = g_operators[di].get_pre_post();
        vector<Prevail> pvs = g_operators[di].get_prevail();

        for(size_t i = 0;i < pps.size();++i){
            std::vector<std::string> pred_var = g_fact_names[pps[i].var];
            if(pps[i].pre != -1){
                pres.insert(pred_var[pps[i].pre]);
            }
            effs.insert(pred_var[pps[i].post]);
        }

        for(size_t i = 0;i < pvs.size();++i){
            std::string pre_atom = g_fact_names[pvs[i].var][pvs[i].prev];
            pres.insert(pre_atom);
        }

        for(std::set<std::string >::iterator it = pres.begin();it!=pres.end();){
            file << *it;
            if((++it) != pres.end()){
                file << ",";
            }
        }
        file << ";";
        for(std::set<std::string >::iterator it = effs.begin();it!=effs.end();){
            file << *it;
            if((++it) != effs.end()){
                file << ",";
            }
        }

        file << "\n";
    }
    file.close();
}

void savePolicyFiles(Policy* policy,const string & full_fodler_path)
{
    if(!(mkdir(full_fodler_path.c_str(),0777))){
        cerr << "Fail to mkdir folder: " << full_fodler_path;
//        exit(1);
    }
    dumpActionInd(full_fodler_path + "/action.ind");
    ofstream file_policy(full_fodler_path + "/policy");
    ofstream file_graph(full_fodler_path + "/graph.out");
    ofstream file_state(full_fodler_path + "/state.ind");
    policy->dumpPolicyStatesGraph(file_policy,file_state,file_graph,false);
}

bool g_use_metric;
int g_min_action_cost = numeric_limits<int>::max();
int g_max_action_cost = 0;
vector<string> g_variable_name;
vector<int> g_variable_domain;
vector<vector<string> > g_fact_names;
vector<int> g_axiom_layers;
vector<int> g_default_axiom_values;
IntPacker *g_state_packer;
vector<int> g_initial_state_data;
vector<pair<int, int> > g_goal;
vector<Operator> g_operators;
vector<Operator> g_axioms;
AxiomEvaluator *g_axiom_evaluator;
vector<DomainTransitionGraph *> g_transition_graphs;
CausalGraph *g_causal_graph;
LegacyCausalGraph *g_legacy_causal_graph;

SuccessorGenerator *g_successor_generator_orig; // Renamed so the ops can be pruned based on deadends
DeadendAwareSuccessorGenerator *g_successor_generator;

map<string, int> g_nondet_index_mapping; // Maps a non-deterministic action name to its id
vector<vector<Operator *> *> g_nondet_mapping; // Maps a non-deterministic action id to a list of ground operators
vector<vector<int> *> g_nondet_conditional_mask; // Maps a non-deterministic action id to the variables that must be defined when doing context-sensitive regression

vector<FpAction*> g_fp_actions;
vector<ABPair* > g_fp_ABs;
string* g_search_option;

std::shared_ptr<RegressionStep>  g_matched_policy; // Contains the condition that matched when our policy recognized the state
int g_matched_distance; // Containts the distance to the goal for the matched policy
Policy *g_policy; // The policy to check while searching
Policy *g_regressable_ops; // The policy to check what operators are applicable
Policy *g_regressable_cond_ops; // The policy to check what operators with conditional effects are applicable
Policy *g_deadend_policy; // Policy that returns the set of names for nondet operators that should be avoided
Policy *g_deadend_states; // Policy that returns an item if the given state is a deadend
Policy *g_temporary_deadends; // Policy that stores deadends as we find them online (to avoid repeated ones)
Policy *g_best_policy; // The best policy we've found so far
vector< DeadendTuple * > g_found_deadends; // Vector of deadends / contexts found while planning
double g_best_policy_score = 0.0; // Score for the best policy we've seen so far
int g_failed_open_states = 0; // Number of failed open states in the most recent jic run
bool g_updated_deadends = false; // True if updating the policy created new deadends
bool g_replan_detected_deadends = false; // True if the weak planning procedure created a new deadend
int g_silent_planning = 1;
bool g_forgetpolicy = false; // Forget the global policy after every simulation run
bool g_replan_during_simulation = true; // True if we want to allow the system to replan
bool g_fullstate = false; // Use the full state for regression
bool g_plan_locally = false; // Plan for the expected state rather than replanning to the goal // Yuan: GTF may not need this.
bool g_plan_locally_limited = false; // Limit the local planning to a small number of search nodes // Yuan: GTF may not need this.
bool g_limit_states = false; // Used to limit the search when replanning for a local goal
int g_limit_states_max = 100; // Maximum number of states to expand in the subsequent search
bool g_force_limit_states = false; // Forces the search to stop based on the g_limit_states_max
bool g_plan_with_policy = true; // Stop planning when the policy matches
bool g_partial_planlocal = false; // Plan locally to the partial state that would have matched our expected state // Yuan: GTF may not need this.
bool g_detect_deadends = true; // Decide whether or not deadends should be detected and avoided
bool g_check_with_forbidden = true; // We set this when a strong cyclic policy is failed to be found without using forbidden ops in the heuristic // Yuan: GTF need this to be true.
bool g_generalize_deadends = true; // Try to find minimal sized deadends from the full state (based on relaxed reachability)
bool g_record_online_deadends = true; // Record the deadends as they occur online, and add them to the deadend policy after solving
bool g_sample_for_depth1_deadends = true; // Analyze the non-deterministic alternate states from the generated weak plans for deadends
bool g_combine_deadends = false; // Combine FSAP conditions for a new deadend when there are no applicable actions
bool g_repeat_fsap_backwards = false; // Keep making FSAPs as long as states where they hold have no applicable actions
bool g_regress_only_relevant_deadends = false; // Only regresses a deadend for an FSAP if the action triggers the deadend
int g_combined_count = 0; // Number of times a deadend was generated from combining FSAPs
int g_repeat_fsap_count = 0; // Number of times we applied the repeated FSAP technique
bool g_repeat_strengthening = false; // Continue to strengthen pairs back to the initial state.
bool g_optimized_scd = false; // Do optimized strong cyclic detection // Yuan: GTF may not need this.
bool g_final_fsap_free_round = false; // Do a final JIC pass with deadends disabled
bool g_seeded = false; // Only want to seed once
int g_trial_depth = 1000; // Used to limit the number of simulation steps
int g_num_trials = 1; // Number of trials that should be used for the simulation
double g_jic_limit = 1800.0; // Limit for the just-in-case repairs
vector<pair<int, int> > g_goal_orig;
Heuristic *g_heuristic_for_reachability;
bool g_dump_policy = true;
bool g_show_policy = false;
bool g_dump_graph = false;
int g_monotonicity_violations = 0; // Count on the number of times we need to add a deadend because of a bad policy loop
int g_num_regsteps = 0; // Used to give each regstep an id based on when it was created
int g_num_epochs = 1; // Forced number of times to run the jic loop

bool g_optimize_final_policy = false; // Only keep the final pairs and FSAPs that are needed
bool g_record_relevant_pairs = false; // If true, used pairs will be kept

bool g_debug = false; // Flag for debugging parts of the code
int g_debug_count = 1; // Index that allows to locate spots in the output

bool g_safetybelt_optimized_scd = true; // Gradually disable the optimized SCD setting when it proves useless
bool g_running_debug = false;
//std::map<std::string, int> g_op_redundant_weight_map;
std::vector<int> g_action_redundant_weight;
int g_op_redundant_coefficient = 0;

bool g_message_debug = false;
bool g_from_scratch = true;
int g_bqnp = 0; // 0: QNP, 1:BQNP, 2:SmallBQNP
std::string g_main_path;
int g_debug_nont = 0;
std::string g_ntgraph_path;
std::string g_planner_path;
//bool g_running_debug=false;
int g_policy_steps_type = 0; // 0 for all items from new Policy; 1 for all items in NonT cycles
std::vector<std::string> g_inits_values;
std::string g_ff_solver = "ff";
std::string g_working_src_path;
int g_num_SCs = 0;
int g_num_solutions = 0;
int g_num_unique_solutions = 0;
bool g_detect_nont_p = false;
bool g_sort_ban_items = true;
bool g_sort_scc = false;
bool g_early_return_nont = true;
bool g_compact_nont_scc = true;
int g_online_keyact_weight = 0;

bool g_cycle_items_first = true;
bool g_save_all_solutions = false;
bool g_save_all_NTSC_files = false;
bool g_abort_when_sc_duplicates = false;
bool g_monitoring_sc = false;

bool g_sc_fsaps = true;
bool g_adv_fsaps = true;
bool g_skip_key = true;
//int g_pick_mode = 1; // 0 for not adding the rest actions to fsap, 1 for adding the rest, 2 for adding the applicable only.
bool g_strengthen_key = true;

int g_var_action = 0; // 0 for not Variableization, 1 for all in one var.

bool g_preprocess_online = false;

bool g_findsc_info = false;
int g_num_solution2find = 0;

SolvingMode g_solving_mode = SolvingMode::Others;

bool g_sc_planning = false;
bool g_strong_planning = false;

Timer g_timer_regression;
Timer g_timer_simulator;
Timer g_timer_engine_init;
Timer g_timer_search;
Timer g_timer_policy_build;
Timer g_timer_policy_eval;
Timer g_timer_policy_use;
Timer g_timer_jit;
Timer g_timer_solving;

int g_stat_pruning_key = 0;

int g_argc;
const char **g_argv;

Timer g_timer;
string g_plan_filename = "sas_plan";
RandomNumberGenerator g_rng(2011); // Use an arbitrary default seed.
StateRegistry *g_state_registry = 0;
