#include "../operator.h"
#include "../globals.h"
#include "policy.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
using namespace std;

/* NOTE on possible optimizations:

   * Sharing "GeneratorEmpty" instances might help quite a bit with
     reducing memory usage and possibly even speed, because there are
     bound to be many instance of those. However, it complicates
     deleting the successor generator, and memory doesn't seem to be
     an issue. Speed appears to be fine now, too. So this is probably
     an unnecessary complication.

   * Using slist instead of list led to a further 10% speedup on the
     largest Logistics instance, logistics-98/prob28.pddl. It would of
     course also reduce memory usage. However, it would make the code
     g++-specific, so it's probably not worth it.

*/


bool GeneratorSwitch::check_match(const PartialState &curr, bool keep_all) {
    if (immediate_items.size() > 0)
        return true;

    if ((curr[switch_var] != -1) && (generator_for_value[curr[switch_var]]->check_match(curr, keep_all)))
        return true;

    if ((curr[switch_var] == -1) && keep_all) {
        for (int i = 0; i < g_variable_domain[switch_var]; i++) {
            if (generator_for_value[i]->check_match(curr, keep_all))
                return true;
        }
    }

    if (default_generator->check_match(curr, keep_all))
        return true;

    return false;
}

void GeneratorSwitch::generate_applicable_items(const PartialState &curr, vector<std::shared_ptr<PolicyItem>> &reg_items, bool keep_all, bool only_if_relevant) { // Anno 【 ??? 】
//    cout << "in GeneratorSwitch::generate_applicable_items" << "\n" << "immediate_items" << "\n";
    for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = immediate_items.begin(); op_iter != immediate_items.end(); ++op_iter) {
//        cout << "fsap" << "\n";
//        Policy::cout_item(op_iter,true);
//        cout << "not fsap" << "\n";
//        Policy::cout_item(op_iter,false);

        if (!only_if_relevant || (*op_iter)->check_relevance(curr))
            reg_items.push_back(*op_iter);
    }

    if (curr[switch_var] != -1)
        generator_for_value[curr[switch_var]]->generate_applicable_items(curr, reg_items, keep_all, only_if_relevant);
    else if (keep_all) {
        for (int i = 0; i < g_variable_domain[switch_var]; i++) {
            generator_for_value[i]->generate_applicable_items(curr, reg_items, keep_all, only_if_relevant);
        }
    }

    default_generator->generate_applicable_items(curr, reg_items, keep_all, only_if_relevant);
}

void GeneratorSwitch::generate_applicable_items(const PartialState &curr, vector<std::shared_ptr<PolicyItem>> &reg_items, int bound) {
    int best_val = 9999999;
    std::shared_ptr<PolicyItem> best_rs = NULL;
    for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = immediate_items.begin(); op_iter != immediate_items.end(); ++op_iter)
    {
        int cur_val = ((RegressionStep *)(*op_iter).get())->distance;
        if (cur_val < best_val) {
            best_val = cur_val;
            best_rs = *op_iter;
        }
    }
    if (best_val <= bound)
        reg_items.push_back(best_rs);

    if (curr[switch_var] == -1) {
        for (int i = 0; i < g_variable_domain[switch_var]; i++) {
            generator_for_value[i]->generate_applicable_items(curr, reg_items, bound);
        }
    } else {
        generator_for_value[curr[switch_var]]->generate_applicable_items(curr, reg_items, bound);
    }
    default_generator->generate_applicable_items(curr, reg_items, bound);
}

bool GeneratorLeaf::check_match(const PartialState &, bool) {
    if (applicable_items.size() > 0)
        return true;
    else
        return false;
}

void GeneratorLeaf::generate_applicable_items(const PartialState &curr, vector<std::shared_ptr<PolicyItem>> &reg_items, bool, bool only_if_relevant) {
//    cout << "in GeneratorLeaf::generate_applicable_items bool bool" << "\n";
    for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = applicable_items.begin(); op_iter != applicable_items.end(); ++op_iter) {
//        cout << "fsap" << "\n";
//        Policy::cout_item(op_iter,true);
//        cout << "not fsap" << "\n";
//        Policy::cout_item(op_iter,false);
        if (!only_if_relevant || (*op_iter)->check_relevance(curr))
            reg_items.push_back(*op_iter);
    }
}

void GeneratorLeaf::generate_applicable_items(const PartialState &, vector<std::shared_ptr<PolicyItem>> &reg_items, int bound) {
    int best_val = 9999999;
    std::shared_ptr<PolicyItem> best_rs = NULL;
//    cout << "in GeneratorLeaf::generate_applicable_items int" << "\n";
    for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = applicable_items.begin(); op_iter != applicable_items.end(); ++op_iter)
    {
//        cout << "fsap" << "\n";
//        Policy::cout_item(op_iter,true);
//        cout << "not fsap" << "\n";
//        Policy::cout_item(op_iter,false);
        int cur_val = ((RegressionStep *)(*op_iter).get())->distance;
        if (cur_val < best_val) {
            best_val = cur_val;
            best_rs = *op_iter;
        }
    }
    if (best_val <= bound)
        reg_items.push_back(best_rs);
}



GeneratorSwitch::GeneratorSwitch(int switch_variable,
                                 list<std::shared_ptr<PolicyItem>> &reg_items,
                                 const vector<GeneratorBase *> &gen_for_val,
                                 GeneratorBase *default_gen)
    : switch_var(switch_variable),
      generator_for_value(gen_for_val),
      default_generator(default_gen) {

    immediate_items.swap(reg_items);
}

GeneratorSwitch::~GeneratorSwitch() {
    for (int i = 0; i < generator_for_value.size(); i++)
        delete generator_for_value[i];
    delete default_generator;
}

void GeneratorSwitch::dump(string indent) const {
    cout << indent << "switch on " << g_variable_name[switch_var] << endl;
    cout << indent << "immediately:" << endl;
    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = immediate_items.begin();
         op_iter != immediate_items.end(); ++op_iter)
        cout << indent << (*op_iter)->get_name() << endl;
    for (int i = 0; i < g_variable_domain[switch_var]; i++) {
        cout << indent << "case " << i << ":" << endl;
        generator_for_value[i]->dump(indent + "  ");
    }
    cout << indent << "always:" << endl;
    default_generator->dump(indent + "  ");
}

void GeneratorSwitch::generate_cpp_input(ofstream &outfile) const {
    outfile << "switch " << switch_var << endl;
    outfile << "check " << immediate_items.size() << endl;
    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = immediate_items.begin();
         op_iter != immediate_items.end(); ++op_iter)
        outfile << (*op_iter)->get_name() << endl;
    for (int i = 0; i < g_variable_domain[switch_var]; i++) {
        //cout << "case "<<switch_var->get_name()<<" (Level " <<switch_var->get_level() <<
        //  ") has value " << i << ":" << endl;
        generator_for_value[i]->generate_cpp_input(outfile);
    }
    //cout << "always:" << endl;
    default_generator->generate_cpp_input(outfile);
}

GeneratorLeaf::GeneratorLeaf(list<std::shared_ptr<PolicyItem>> &items) {
    applicable_items.swap(items);
}

void GeneratorLeaf::dump(string indent) const {
    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = applicable_items.begin();
         op_iter != applicable_items.end(); ++op_iter)
        cout << indent << (*op_iter)->get_name() << endl;
}

void GeneratorLeaf::generate_cpp_input(ofstream &outfile) const {
    outfile << "check " << applicable_items.size() << endl;
    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = applicable_items.begin();
         op_iter != applicable_items.end(); ++op_iter)
        outfile << (*op_iter)->get_name() << endl;
}

void GeneratorEmpty::dump(string indent) const {
    cout << indent << "<empty>" << endl;
}

void GeneratorEmpty::generate_cpp_input(ofstream &outfile) const {
    outfile << "check 0" << endl;
}


int GeneratorBase::get_best_var(list<std::shared_ptr<PolicyItem>> &reg_items, set<int> &vars_seen) {
    vector< pair<int,int> > var_count = vector< pair<int,int> >(g_variable_name.size());

    for (int i = 0; i < g_variable_name.size(); i++) {
        var_count[i] = pair<int,int>(0, i);
    }

    for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = reg_items.begin(); op_iter != reg_items.end(); ++op_iter) {
        for (int i = 0; i < g_variable_name.size(); i++) {
            if (-1 != (*((*op_iter)->state))[i]) {
                var_count[i].first++;
            }
        }
    }

    sort(var_count.begin(), var_count.end());

    for (int i = var_count.size() - 1; i >= 0; i--) {
        if (vars_seen.count(var_count[i].second) <= 0) {
            //cout << "Best var " << var_count[i].second << " with a count of " << var_count[i].first << endl;
            return var_count[i].second;
        }
    }

    assert(false);
    return -1;
}

bool GeneratorBase::reg_item_done(std::shared_ptr<PolicyItem>op_iter, set<int> &vars_seen) {
    for (int i = 0; i < g_variable_name.size(); i++) {
        if ((-1 != (*(op_iter->state))[i]) && (vars_seen.count(i) <= 0))
            return false;
    }

    return true;
}

GeneratorBase *GeneratorBase::create_generator(list<std::shared_ptr<PolicyItem>> &reg_items, set<int> &vars_seen) {
    if (reg_items.empty())
        return new GeneratorEmpty;

    // If every item is done, then we create a leaf node
    bool all_done = true;
    for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = reg_items.begin(); all_done && (op_iter != reg_items.end()); ++op_iter) {
        if (!(reg_item_done(*op_iter, vars_seen))) {
            all_done = false;
        }
    }

    if (all_done) {
        return new GeneratorLeaf(reg_items);
    } else {
        return new GeneratorSwitch(reg_items, vars_seen);
    }
}

GeneratorSwitch::GeneratorSwitch(list<std::shared_ptr<PolicyItem>> &reg_items, set<int> &vars_seen) {
    switch_var = get_best_var(reg_items, vars_seen);

    vector< list<std::shared_ptr<PolicyItem>> > value_items;
    list<std::shared_ptr<PolicyItem>> default_items;

    // Initialize the value_items
    for (int i = 0; i < g_variable_domain[switch_var]; i++)
        value_items.push_back(/*std::move*/(list<std::shared_ptr<PolicyItem>>()));

    // Sort out the regression items
    for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = reg_items.begin(); op_iter != reg_items.end(); ++op_iter) {
        if (reg_item_done(*op_iter, vars_seen)) {
            immediate_items.push_back(*op_iter);
        } else if (-1 != (*((*op_iter)->state))[switch_var]) {
            value_items[(*((*op_iter)->state))[switch_var]].push_back(*op_iter);
        } else { // == -1
            default_items.push_back(*op_iter);
        }
    }

    vars_seen.insert(switch_var);

    // Create the switch generators
    for (int i = 0; i < value_items.size(); i++) {
        generator_for_value.push_back(create_generator(value_items[i], vars_seen));
    }

    // Create the default generator
    default_generator = create_generator(default_items, vars_seen);

    vars_seen.erase(switch_var);
}

GeneratorBase *GeneratorSwitch::update_policy(list<std::shared_ptr<PolicyItem>> &reg_items, set<int> &vars_seen) {
    vector< list<std::shared_ptr<PolicyItem>> > value_items;
    list<std::shared_ptr<PolicyItem>> default_items;

    // Initialize the value_items
    for (int i = 0; i < g_variable_domain[switch_var]; i++)
        value_items.push_back(list<std::shared_ptr<PolicyItem>>());

    // Sort out the regression items
    for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = reg_items.begin(); op_iter != reg_items.end(); ++op_iter) {
        if (reg_item_done(*op_iter, vars_seen)) {
            immediate_items.push_back(*op_iter);
        } else if (-1 != (*((*op_iter)->state))[switch_var]) {
            value_items[(*((*op_iter)->state))[switch_var]].push_back(*op_iter);
        } else { // == -1
            default_items.push_back(*op_iter);
        }
    }

    vars_seen.insert(switch_var);

    // Update the switch generators
    for (int i = 0; i < value_items.size(); i++) {
        GeneratorBase *newGen = generator_for_value[i]->update_policy(value_items[i], vars_seen);
        if (NULL != newGen) {
            delete generator_for_value[i];
            generator_for_value[i] = newGen;
        }
    }

    // Update the default generator
    GeneratorBase *newGen = default_generator->update_policy(default_items, vars_seen);
    if (NULL != newGen) {
        delete default_generator;
        default_generator = newGen;
    }

    vars_seen.erase(switch_var);

    return NULL;
}

GeneratorBase *GeneratorLeaf::update_policy(list<std::shared_ptr<PolicyItem>> &reg_items, set<int> &vars_seen) {
    if (reg_items.empty())
        return NULL;

    bool all_done = true;
    for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = reg_items.begin(); op_iter != reg_items.end(); ++op_iter) {
        if (!reg_item_done(*op_iter, vars_seen)) {
            all_done = false;
            break;
        }
    }

    if (all_done) {
        applicable_items.splice(applicable_items.end(), reg_items);
        return NULL;
    } else {
        reg_items.splice(reg_items.end(), applicable_items);
        return new GeneratorSwitch(reg_items, vars_seen);
    }
}

GeneratorBase *GeneratorEmpty::update_policy(list<std::shared_ptr<PolicyItem>> &reg_items, set<int> &vars_seen) {
    if (reg_items.empty())
        return NULL;

    bool all_done = true;
    for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = reg_items.begin(); op_iter != reg_items.end(); ++op_iter) {
        if (!reg_item_done(*op_iter, vars_seen)) {
            all_done = false;
            break;
        }
    }

    if (all_done)
        return new GeneratorLeaf(reg_items);
    else
        return new GeneratorSwitch(reg_items, vars_seen);
}

void Policy::add_item(std::shared_ptr<PolicyItem> item) {
    list<std::shared_ptr<PolicyItem>> reg_items;
    reg_items.push_back(item);
    update_policy(reg_items);
}


void Policy::update_policy(list<std::shared_ptr<PolicyItem>> &reg_items) {
    g_timer_policy_build.resume();

    // Reset the score since a change is being made to the policy
    score = 0.0;

    set<int> vars_seen;
    if (root)
        root->update_policy(reg_items, vars_seen);
    else
        root = new GeneratorSwitch(reg_items, vars_seen);
    all_items.insert(all_items.end(), reg_items.begin(), reg_items.end());

    g_timer_policy_build.stop();
}

void Policy::copy_relevant_items(list<std::shared_ptr<PolicyItem>> &items, bool checksc) {
    for(const auto & op : all_items){
        if (op->relevant || (checksc && ((RegressionStep*)op.get())->is_sc)) {
            items.push_back(op);
        }
    }
//    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = all_items.begin(); op_iter != all_items.end(); ++op_iter) {
//        if ((*op_iter)->relevant || (checksc && ((RegressionStep*)(*op_iter))->is_sc)) {
//            items.push_back(*op_iter);
//        }
//    }
}

list<std::shared_ptr<PolicyItem>> Policy::_getAllItems()
{
    return all_items;
}

void Policy::generate_applicable_items(const PartialState &curr, vector<std::shared_ptr<PolicyItem>> &reg_items, bool keep_all, bool only_if_relevant) {

    assert (!only_if_relevant || keep_all);

    if (root)
        root->generate_applicable_items(curr, reg_items, keep_all, only_if_relevant); // Anno 【 Deadend_policy好像也是turn to GeneratorSwitch 】
}

bool Policy::check_match(const PartialState &curr, bool keep_all) {
    if (root)
        return root->check_match(curr, keep_all);
    else
        return false;
}

std::shared_ptr<RegressionStep> Policy::get_best_step(const PartialState &curr) {// Anno Important function !!!

    if (0 == root)
        return 0;

    g_timer_policy_use.resume();
    vector<std::shared_ptr<PolicyItem>> current_steps;
    generate_applicable_items(curr, current_steps, false, false);

    if (0 == current_steps.size()) {
        g_timer_policy_use.stop();
        return 0;
    }

    // We should only return steps that aren't forbidden
    vector<std::shared_ptr<PolicyItem>> forbidden_items;
    set<int> forbidden;
//    g_deadend_policy->cout_human_policy(true);
    if (!complete)
        g_deadend_policy->generate_applicable_items(curr, forbidden_items, false, false); // Anno 【 Generate applicable actions based on fasp from deadends. 】 // Anno 【 here turn to GeneratorSwitch 】
    for (int i = 0; i < forbidden_items.size(); i++)
        forbidden.insert(((NondetDeadend*)(forbidden_items[i].get()))->op_index);

    // If we are keeping track of the relevant deadends, then we mark the
    //  most compact ones that trigger a forbidden action as being relevant. // Anno 【 ? 】
    map<int, vector<NondetDeadend *> *> ind_to_fsaps;
    map<int, NondetDeadend*> ind_to_fsap;
    if (g_record_relevant_pairs) {
        // First get all of the fsaps sorted
        for (int i = 0; i < forbidden_items.size(); i++) {
            int ind = ((NondetDeadend*)(forbidden_items[i].get()))->op_index;
            if (ind_to_fsaps.find(ind) == ind_to_fsaps.end())
                ind_to_fsaps[ind] = new vector<NondetDeadend *>();
            ind_to_fsaps[ind]->push_back(((NondetDeadend*)(forbidden_items[i].get())));
        }
        // Next find the best one for each index
        for (map<int, vector<NondetDeadend *> *>::iterator it=ind_to_fsaps.begin(); it!=ind_to_fsaps.end(); ++it) {
            int nondet_index = it->first;
            vector<NondetDeadend *> fsaps = *(it->second);
            ind_to_fsap[nondet_index] = fsaps[0];
            for (int i = 0; i < fsaps.size(); i++) {
                if (fsaps[i]->generality() > ind_to_fsap[nondet_index]->generality())
                    ind_to_fsap[nondet_index] = fsaps[i];
            }
        }
    }


    int best_index = -1;
    int best_sc_index = -1;

    for (int i = 0; i < current_steps.size(); i++) {
        if (((RegressionStep*)(current_steps[i].get()))->is_goal ||
            (0 == forbidden.count(((RegressionStep*)(current_steps[i]).get())->op->nondet_index))) {

            if ((-1 == best_index) || (*((RegressionStep*)(current_steps[i].get())) < *((RegressionStep*)(current_steps[best_index].get()))))
                best_index = i;

            if (g_optimized_scd && ((RegressionStep*)(current_steps[i].get()))->is_sc &&
                ((-1 == best_sc_index) || (*((RegressionStep*)(current_steps[i].get())) < *((RegressionStep*)(current_steps[best_sc_index].get())))))
                best_sc_index = i;

        } else if (g_record_relevant_pairs && (0 != forbidden.count(((RegressionStep*)(current_steps[i].get()))->op->nondet_index))) {
            //cout << "Marking relevant fsap:" << endl;
            //ind_to_fsap[((RegressionStep*)(current_steps[i]))->op->nondet_index]->dump();
            ind_to_fsap[((RegressionStep*)(current_steps[i].get()))->op->nondet_index]->relevant = true;
        }
    }

    if (return_if_possible && (-1 == best_index)) {
        for (int i = 0; i < current_steps.size(); i++) {

            if ((-1 == best_index) || (*((RegressionStep*)(current_steps[i].get())) < *((RegressionStep*)(current_steps[best_index].get()))))
                best_index = i;

            if (g_optimized_scd && ((RegressionStep*)(current_steps[i].get()))->is_sc &&
                ((-1 == best_sc_index) || (*((RegressionStep*)(current_steps[i].get())) < *((RegressionStep*)(current_steps[best_sc_index].get())))))
                best_sc_index = i;
        }
    }
    g_timer_policy_use.stop();

    if (-1 == best_index) {
        return 0;
    } else if (g_optimized_scd && (-1 != best_sc_index)) {
        if (g_record_relevant_pairs) {
            //cout << "Marking relevant." << endl;
            //((RegressionStep*)(current_steps[best_sc_index]))->relevant = true;
            current_steps[best_sc_index]->relevant = true;
        }
        return std::static_pointer_cast<RegressionStep>(current_steps[best_sc_index]);
    } else {
        if (g_record_relevant_pairs) {
            //cout << "Marking relevant." << endl;
            //((RegressionStep*)(current_steps[best_index]))->relevant = true;
            current_steps[best_index]->relevant = true;
        }
        return std::static_pointer_cast<RegressionStep>(current_steps[best_index]);
    }
}

Policy::Policy() {
    root = 0;
    score = 0.0;
    complete = false;
    return_if_possible = false;

    seen = NULL;
    open_list = NULL;
    created_states = NULL;

    opt_scd_count = 0;
    opt_scd_countdown = 0;
    opt_scd_countdown_step = 1;
    opt_scd_last_size = 0;
    opt_scd_skipped = false;

    not_sorted = true;
}

Policy::~Policy() {
    if (root)
        delete root;

//    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = all_items.begin();
//         op_iter != all_items.end(); ++op_iter)
//         delete *op_iter;

    delete seen;
    delete open_list;
    delete created_states;
}

void Policy::dump() const {
    cout << "Policy:" << endl;
    //root->dump("  ");
    for(const auto & item:all_items){
        item->dump();
    }
//    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = all_items.begin();
//         op_iter != all_items.end(); ++op_iter)
//         (*op_iter)->dump();
}
void Policy::generate_cpp_input(ofstream &outfile) const {
    root->generate_cpp_input(outfile);
}


bool Policy::better_than(Policy * other) {
    if (get_score() > other->get_score())
        return true;

    if (get_score() < other->get_score())
        return false;

    if (is_strong_cyclic() != other->is_strong_cyclic())
        return is_strong_cyclic();

    return get_size() > other->get_size();
}


double Policy::get_score() {
    if (0.0 == score)
        evaluate();
    return min(score, 1.0);
}

void Policy::evaluate() {
    if (1.0 <= score)
        return;

    g_timer_policy_eval.resume();

    //evaluate_analytical();
    evaluate_random();

    g_timer_policy_eval.stop();
}

void Policy::evaluate_analytical() {

}

void Policy::evaluate_random() {
    Simulator *sim = new Simulator(false);
    int succeeded = 0;
    int NUMTRIALS = 1000;
    for (int i = 0; i < NUMTRIALS; i++) {
        sim->run_once(true, this);
        if (sim->succeeded)
            succeeded++;
    }
    score = double(succeeded) / double(NUMTRIALS);
    delete sim;
}






/***************************
 * Strong cyclic detection *
 ***************************/




bool Policy::goal_sc_reachable(const PartialState &_curr) {

    std::shared_ptr<PartialState> curr = make_shared<PartialState> (_curr);
    std::shared_ptr<PartialState> old;
    std::shared_ptr<RegressionStep> rstep;

    // Instead of keeping a closed list, we just try to get to the
    //  goal within an upper bound.
    for (int i=0; i < all_items.size(); i++) {

        rstep = get_best_step(*curr);

        if (!rstep || !(rstep->is_sc)) {
            return false;
        }

        if (rstep->is_goal) {
            return true;
        }

        old = curr;
        curr = make_shared<PartialState> (*old, *(rstep->op));

    }

    return false;
}


void Policy::init_scd(bool force_count_reset) {

    if (g_safetybelt_optimized_scd) {
        if (force_count_reset) {
            opt_scd_last_size = 0;
            opt_scd_countdown = 0;
            opt_scd_countdown_step = 1;
        }

        if (opt_scd_countdown > 0)
            return;
    }

    opt_scd_count = all_items.size();

    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = all_items.begin();
         op_iter != all_items.end(); ++op_iter)
         ((RegressionStep *)(*op_iter).get())->is_sc = true;
}

bool Policy::step_scd(vector< DeadendTuple * > &failed_states, bool skip_deadends) {

    bool made_change = false;
//    bool debug_scd = false;
    bool debug_scd = !g_silent_planning;

    // Skip the SCD phase if we are in a safety belt zone
    if (g_safetybelt_optimized_scd && (opt_scd_countdown > 0)) {
        opt_scd_countdown--;
        opt_scd_skipped = true;
        return false;
    } else
        opt_scd_skipped = false;

    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = all_items.begin();
         op_iter != all_items.end(); ++op_iter)
    {
        RegressionStep *rs = (RegressionStep *)(*op_iter).get();

        if (rs->is_sc && !(rs->is_goal)) {

            if (debug_scd) {
                cout << "\n\n (#" << g_debug_count++ << ") Testing RegStep:" << endl;
                rs->dump();
            }

            for (int i = 0; i < g_nondet_mapping[rs->op->nondet_index]->size(); i++) {
                // We use the sc_state for computing the guaranteed items, rather than
                //  the original state for the regression step. The sc_state will be a
                //  superset of the original state that includes the regression of newly
                //  closed leafs from the jic compilation phase. This may increase the
                //  number of regression steps we are guaranteed to see in a state matching
                //  succ_state, but as long as we remain in parts of the policy that return
                //  regression steps based on their sc_state then this will be valid.
                std::shared_ptr<PartialState> succ_state = make_shared<PartialState> (*(rs->state), *((*(g_nondet_mapping[rs->op->nondet_index]))[i]));
                vector<std::shared_ptr<PolicyItem>> guaranteed_steps;
                root->generate_applicable_items(*succ_state, guaranteed_steps, false, false);

                if (debug_scd) {
                    cout << "\nTesting successor (" << (i+1) << "/" << g_nondet_mapping[rs->op->nondet_index]->size() << "):" << endl;
                    succ_state->dump_pddl();
                }


                // We need at least one guaranteed step to be sure that we can continue
                //  executing. The one exception is if the state is already known to be
                //  a deadend, in which case we can assume that it will be handled by
                //  the jic compiler.
                if (0 == guaranteed_steps.size()) {
                    bool is_failed_state = false;
                    for (int j = 0; j < failed_states.size(); j++) {
                        // Unfortunately, we can't just check the states equivalence,
                        //  since the succ_state is a partial state. So we instead check
                        //  that succ_state entails the failed state.
                        is_failed_state = true;
                        for (int k = 0; k < g_variable_name.size(); k++) {
                            if (((*(failed_states[j]->de_state))[k] != -1) &&
                                ((*(failed_states[j]->de_state))[k] != (*succ_state)[k])) {
                                is_failed_state = false;
                                break;
                            }
                        }

                        if (debug_scd) {
                            cout << "-+- Left marked due to existing failed state." << endl;
                        }
                    }



                    if (!is_failed_state) {
                        if (g_deadend_states->check_match(*succ_state, true)) {
                            is_failed_state = true;
                            if (debug_scd) {
                                cout << "-+- Left marked due to existing deadend state." << endl;
                            }
                        }
                    }

                    // If succ_state is a failed state, then we will avoid using
                    //  this pair in the future as it will be a FSAP. Thus, we
                    //  can leave it marked so that the search doesn't continue
                    //  down this path -- leaving it marked here doesn't mean
                    //  that the policy is strong cyclic, but rather that no
                    //  strong cyclic plan exists and further search should be
                    //  avoided.
                    if (is_failed_state && skip_deadends) {
                        break;
                    }

                    // If succ_state isn't a failed state (as far as we're aware
                    //  of), then it may be reached during rollout. The possibility
                    //  of not handling the reached state means that this can't be
                    //  strongly cyclic. The break take us to the next regression
                    //  step to check (i.e., no need to check the rest of the action
                    //  outcomes for the current regression step).
                    else {
                        if (debug_scd) {
                            cout << "--- Umarking: No guaranteed steps." << endl;
                        }
                        opt_scd_count--;
                        rs->is_sc = false;
                        made_change = true;
                        break;
                    }
                }

                // We only consider the possible steps that have a cheaper cost than
                //  the cheapest guaranteed cost. We will be returning the cheapest
                //  cost in a get_best_step, so only those regsteps with a chance of
                //  overiding all guaranteed regsteps must be looked at.
                int min_cost = 999999; // This will only be too low if we found a plan of length 10^6
                int min_sc_cost = 999999;
                for (int j = 0; j < guaranteed_steps.size(); j++) {

                    if (((RegressionStep *)guaranteed_steps[j].get())->distance < min_cost)
                        min_cost = ((RegressionStep *)guaranteed_steps[j].get())->distance;

                    if ((((RegressionStep *)guaranteed_steps[j].get())->distance < min_sc_cost) &&
                        ((RegressionStep *)guaranteed_steps[j].get())->is_sc)
                        min_sc_cost = ((RegressionStep *)guaranteed_steps[j].get())->distance;

                }

                // The guaranteed step can't be further from the goal than
                //  the current candidate strong cyclic pair. Otherwise,
                //  we could get an unsound loop of presumed strong cyclicity.
                if (debug_scd && (999999 == min_sc_cost)) {
                    cout << "--- Umarking: No strong cyclic guaranteed step." << endl;
                }

                if ((999999 == min_sc_cost) ||
                    ((min_sc_cost >= rs->distance) &&
                    !(goal_sc_reachable(*succ_state)))) {

                    if (debug_scd && (999999 != min_sc_cost)) {
                        cout << "--- Unmarking: Strong cyclic guaranteed step failed to reach the goal." << endl;
                    }
                    //cout << "Min sc cost = " << min_sc_cost << endl;
                    opt_scd_count--;
                    rs->is_sc = false;
                    made_change = true;
                    i = g_nondet_mapping[rs->op->nondet_index]->size();

                } else {

                    if (debug_scd) {
                        cout << "+++ Left marked." << endl;
                    }
                }
            }
        }
    }

    if (!made_change && g_safetybelt_optimized_scd) {
        if (opt_scd_count > 1.5*opt_scd_last_size) {
            opt_scd_last_size = opt_scd_count;
            opt_scd_countdown = 0;
            opt_scd_countdown_step = 1;
        } else {
            opt_scd_countdown = opt_scd_countdown_step;
            opt_scd_countdown_step *= 2;
        }
    }

    return made_change;
}

bool regstep_compare(std::shared_ptr<PolicyItem> first, std::shared_ptr<PolicyItem> second) {
    return *((RegressionStep*)first.get()) < *((RegressionStep*)second.get());
}
void Policy::dump_human_policy(bool fsap,bool fact_name_display) {

    if (!fsap)
        all_items.sort(regstep_compare);

    string file_prefix = g_main_path + "/policy";
    fstream outfile;
    if(fact_name_display)
        file_prefix += "_display_fact";
    if (!fsap)
        outfile.open((file_prefix + ".out").c_str(), ios::out);
    else
        outfile.open((file_prefix + ".fsap").c_str(), ios::out);

    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = all_items.begin();
         op_iter != all_items.end(); ++op_iter) {

        outfile << "\nIf holds:";
        std::shared_ptr<PartialState> s;
        if (fsap)
            s = ((NondetDeadend*)(*op_iter).get())->state;
        else
            s = ((RegressionStep*)(*op_iter).get())->state;
        for (int i = 0; i < g_variable_domain.size(); i++) {
            if (-1 != (*s)[i]) {
                outfile << " ";
                outfile << g_variable_name[i] << ":" << (*s)[i];
                if (fact_name_display){
                    outfile << "【 "<< g_fact_names[i][(*s)[i]] << " 】";
                }
            }
        }
        outfile << endl;
        if (fsap)
            outfile << "Forbid: " << ((NondetDeadend*)(*op_iter).get())->get_name() << endl;
        else
            outfile << "Execute: " << ((RegressionStep*)(*op_iter).get())->get_name() << endl;


    }

    outfile.close();
}

std::string renderState(std::shared_ptr<PartialState>  s){
    std::string res;
    for (int i = 0; i < g_variable_domain.size(); i++) {
        if (-1 != (*s)[i]) {
            res += g_fact_names[i][(*s)[i]] + "|";
        }
    }
    res = res.substr(0,res.size()-1);
    return res;
}

void Policy::dump_man_policy(bool fsap, bool full)
{

//    list<std::shared_ptr<PolicyItem>> reg_steps;
    string path = g_main_path;

    if (!fsap)
        all_items.sort(regstep_compare);

    fstream outfile;
    fstream outgraph;
    fstream outstateind;
    if (!fsap && full){
        outfile.open(path+"/man.out", ios::out);
        outgraph.open(path+"/graph.out",ios::out);
        outstateind.open(path+"/state.ind",ios::out);
    }
    else if (!fsap && !full){
        outfile.open(path+"/pman.out", ios::out);
        outgraph.open(path+"/pgraph.out",ios::out);
        outstateind.open(path+"/pstate.ind",ios::out);
    }
    else
        outfile.open(path+"/man.fsap", ios::out);

    if(!fsap && full){

        int node_ind = 0;
        std::shared_ptr<RegressionStep>  current_step;
        queue<std::shared_ptr<PartialState> > open_list;
        map<PartialState,int> seen;

        std::shared_ptr<PartialState>  current_full_state = make_shared<PartialState> (g_initial_state());
        open_list.push(current_full_state);
        seen[*current_full_state] = node_ind++;

        outstateind << renderState(current_full_state);

        while (!open_list.empty()){
            current_full_state = open_list.front();
            open_list.pop();

            outgraph << seen[*current_full_state];

            current_step = get_best_step(*current_full_state);

            if(current_step == 0){
                cout << "No Road!" << endl;
                outfile.close();
                outgraph.close();
                outstateind.close();
                exit(0);
//                break;
            }
            //        current_full_state->dump_pddl();
    //        current_step->dump();
    //        cout<<"is goal: "<< current_step->is_goal;
            if(current_step->is_goal){
                outgraph << "\n";
                continue;
            }
//            current_full_state->dump_pddl();
//            cout << current_step->op->nondet_index << endl;
//            cout << current_step->op->get_name() << endl;
//            cout << current_step->op->get_nondet_name() << endl;
            string action_name = ((RegressionStep*)(current_step.get()))->get_name();
            int distance = ((RegressionStep*)(current_step.get()))->distance;
            string real_name = action_name.substr(0, action_name.find(" / SC")).substr(0, action_name.find(" / NSC"));
            if(g_findsc_info) cout << real_name << endl;
//            reg_steps.push_back(new RegressionStep(make_shared<PartialState> (*current_full_state),current_step->op->nondet_index));
            outgraph << " " << current_step->op->nondet_index << " " << distance;

            outfile << "\nIf holds:";
            std::shared_ptr<PartialState> s = current_full_state;
            for (int i = 0; i < g_variable_domain.size(); i++) {
                if (-1 != (*s)[i]) {
                    outfile << " var";
                    outfile << i << ":" << (*s)[i];
                }
            }
            outfile << endl;
            outfile << "Forbid: " << real_name << endl;

            // Extend
            for(int i = 0;i < g_nondet_mapping[current_step->op->nondet_index]->size();++i){
                std::shared_ptr<PartialState>  new_state;
                Operator *op = (*(g_nondet_mapping[current_step->op->nondet_index]))[i];
                new_state = make_shared<PartialState> (*current_full_state,*op);
                if (seen.count(*new_state)==0){
                    open_list.push(new_state);
                    seen[*new_state] = node_ind++;
                    outstateind << "\n" << renderState(new_state);
                }
                outgraph << " " << seen[*new_state];
            }
            outgraph << "\n";
        }


    }
    else if(!fsap && !full){

        int node_ind = 0;
        std::shared_ptr<RegressionStep>  current_step;
        queue<std::shared_ptr<PartialState> > open_list;
        map<PartialState,int> pseen;
        set<PartialState> seen;

        std::shared_ptr<PartialState>  current_full_state = make_shared<PartialState> (g_initial_state());
        open_list.push(current_full_state);

        // get partial first
        current_step = get_best_step(*current_full_state);

        pseen[*current_step->state] = node_ind++;
        seen.insert(*current_full_state);

        outstateind << renderState(current_step->state);

        while (!open_list.empty()){
            current_full_state = open_list.front();
            open_list.pop();

            current_step = get_best_step(*current_full_state);

            if(current_step == 0){
                cout << "No Road!" << endl;
//                break;
                outfile.close();
                outgraph.close();
                outstateind.close();
                exit(0);
//                break;
            }


            outgraph << pseen[*current_step->state];

    //        current_full_state->dump_pddl();
    //        current_step->dump();
    //        cout<<"is goal: "<< current_step->is_goal;
            if(current_step->is_goal){
                outgraph << "\n";
                continue;
            }
//            current_full_state->dump_pddl();
//            cout << current_step->op->nondet_index << endl;
//            cout << current_step->op->get_name() << endl;
//            cout << current_step->op->get_nondet_name() << endl;
            string action_name = ((RegressionStep*)(current_step.get()))->get_name();
            string real_name = action_name.substr(0, action_name.find(" / SC")).substr(0, action_name.find(" / NSC"));
//            cout << real_name << endl;
//            reg_steps.push_back(new RegressionStep(make_shared<PartialState> (*current_full_state),current_step->op->nondet_index));
            outgraph << " " << current_step->op->nondet_index;

//            outfile << "\nIf holds:";
//            std::shared_ptr<PartialState> s = current_full_state;
//            for (int i = 0; i < g_variable_domain.size(); i++) {
//                if (-1 != (*s)[i]) {
//                    outfile << " var";
//                    outfile << i << ":" << (*s)[i];
//                }
//            }
//            outfile << endl;
//            outfile << "Forbid: " << real_name << endl;

            // Extend
            for(int i = 0;i < g_nondet_mapping[current_step->op->nondet_index]->size();++i){
                std::shared_ptr<PartialState>  new_state;
                Operator *op = (*(g_nondet_mapping[current_step->op->nondet_index]))[i];
                new_state = make_shared<PartialState> (*current_full_state,*op);
                std::shared_ptr<RegressionStep> new_step = get_best_step(*new_state);
                if(new_step == 0){
                                    cout << "No Road!" << endl;
//                                    break;
                                    outfile.close();
                                    outgraph.close();
                                    outstateind.close();
                                    exit(0);
                    //                break;
                                }

                if (seen.count(*new_state)==0){
                    open_list.push(new_state);
                    seen.insert(*new_state);
                }
                if(pseen.count(*new_step->state)==0){
                    pseen[*new_step->state] = node_ind++;
                    outstateind << "\n" << renderState(new_step->state);
                }
                outgraph << " " << pseen[*new_step->state];
            }
            outgraph << "\n";
        }


    }
    else{
        for (list<std::shared_ptr<PolicyItem>>::iterator op_iter = all_items.begin();
             op_iter != all_items.end(); ++op_iter) {

            outfile << "\nIf holds:";
            std::shared_ptr<PartialState> s;
            if (fsap)
                s = ((NondetDeadend*)(*op_iter).get())->state;
    //        else if(full)
    //            s = (*op_iter)->state;
            else
                s = ((RegressionStep*)(*op_iter).get())->state;
            for (int i = 0; i < g_variable_domain.size(); i++) {
                if (-1 != (*s)[i]) {
                    outfile << " var";
                    outfile << i << ":" << (*s)[i];
                }
            }
            outfile << endl;
            if (fsap)
                outfile << "Forbid: " << ((NondetDeadend*)(*op_iter).get())->get_name() << endl;
            else
                outfile << "Execute: " << ((RegressionStep*)(*op_iter).get())->get_name() << endl;

        }
    }

    outfile.close();
    outgraph.close();
    outstateind.close();

}



void Policy::cout_human_policy(bool fsap,bool op_flag) {  // Anno

    if (!fsap)
        all_items.sort(regstep_compare);

//    fstream outfile;
    if (!fsap)
        cout << endl << "policy.out";
    else
        cout << endl << "policy.fsap";

    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = all_items.begin();
         op_iter != all_items.end(); ++op_iter) {

        cout << "\nIf holds:";
        std::shared_ptr<PartialState> s;
        if (fsap)
            s = ((NondetDeadend*)(*op_iter).get())->state;
        else
            s = ((RegressionStep*)(*op_iter).get())->state;
        for (int i = 0; i < g_variable_domain.size(); i++) {
            if (-1 != (*s)[i]) {
                cout << " " << "[ " << i << " ] ";
                cout << g_variable_name[i] << ":" << (*s)[i] << "【 "<< g_fact_names[i][(*s)[i]] << " 】";
            }
        }
        cout << endl;
        if(op_flag){
            if (fsap)
                cout << "Forbid: " << ((NondetDeadend*)(*op_iter).get())->op_index << " " << ((NondetDeadend*)(*op_iter).get())->get_name() << endl;
            else
                cout << "Execute: " << ((RegressionStep*)(*op_iter).get())->get_name() << endl;
            cout << "A: " << g_nondet_index_mapping[((RegressionStep*)(*op_iter).get())->get_name()] << endl;
        }

    }

    //    outfile.close();
}

void Policy::cout_item(list<std::shared_ptr<PolicyItem>>::iterator op_iter,bool fsap)
{

    cout << "\nIf holds:";
    std::shared_ptr<PartialState> s;
    if (fsap)
        s = ((NondetDeadend*)(*op_iter).get())->state;
    else
        s = ((RegressionStep*)(*op_iter).get())->state;
    for (int i = 0; i < g_variable_domain.size(); i++) {
        if (-1 != (*s)[i]) {
            cout << " ";
            cout << g_variable_name[i] << ":" << (*s)[i] << "【 "<< g_fact_names[i][(*s)[i]] << " 】";
        }
    }
    cout << endl;
    if (fsap)
        cout << "Forbid: " << ((NondetDeadend*)(*op_iter).get())->get_name() << endl;
    else
        cout << "Execute: " << ((RegressionStep*)(*op_iter).get())->get_name() << endl;
}

void Policy::dumpPolicyStatesGraph(ostream &outfile, ostream &outstateind, ostream &outgraph, bool fsap)
{
    if(fsap){
        cerr << "DumpPolicyStatesGraph is Not applicable for forbidden-policy!" << endl;
        exit(1);
        return;
    }
//    list<std::shared_ptr<PolicyItem>> reg_steps;
    string path = g_main_path;

//        if (!fsap)
    if(not_sorted){
        all_items.sort(regstep_compare);
        not_sorted = false;
    }

    int node_ind = 0;
    std::shared_ptr<RegressionStep>  current_step;
    queue<std::shared_ptr<PartialState> > open_list;
    map<PartialState,int> seen;

    std::shared_ptr<PartialState>  current_full_state = make_shared<PartialState> (g_initial_state());
    open_list.push(current_full_state);
    seen[*current_full_state] = node_ind++;

    outstateind << renderState(current_full_state);

    while (!open_list.empty()){
        current_full_state = open_list.front();
        open_list.pop();

        current_step = get_best_step(*current_full_state);

        if(current_step == 0){
            cout << "Node " << seen[*current_full_state] << " No Road!" << endl;
            current_full_state->dump_pddl();
            cout << "=========" << endl;
            outgraph << seen[*current_full_state] << "\n";
            cerr << "Please check the current policy is a complete policy!!!";
            exit(1);
        }

        if(current_step->is_goal){
            outgraph << seen[*current_full_state] << "\n";
            continue;
        }
        string action_name = ((RegressionStep*)(current_step.get()))->get_name();
        int distance = ((RegressionStep*)(current_step.get()))->distance;
        string real_name = action_name.substr(0, action_name.find(" / SC")).substr(0, action_name.find(" / NSC"));
        if(g_findsc_info) cout << real_name << endl;

        std::shared_ptr<PartialState> s = current_full_state;
        for (int i = 0; i < g_variable_domain.size(); i++) {
            if (-1 != (*s)[i]) {
                outfile << i << ":" << (*s)[i];
                outfile << " ";
            }
        }
        outfile << current_step->op->nondet_index << "\n";

        // Expand
        set<PartialState> seen_local;
        for(int i = 0;i < g_nondet_mapping[current_step->op->nondet_index]->size();++i){
            std::shared_ptr<PartialState>  new_state;
            Operator *op = (*(g_nondet_mapping[current_step->op->nondet_index]))[i];
            new_state = make_shared<PartialState> (*current_full_state,*op);
            if (seen.count(*new_state)==0){
                open_list.push(new_state);
                seen[*new_state] = node_ind++;
                outstateind << "\n" << renderState(new_state);
            }
            // We only record the first transition.
            if(seen_local.count(*new_state) == 0){
                outgraph << seen[*current_full_state] << " " << op->det_index << " " << distance;
                outgraph << " " << seen[*new_state] << "\n";
                seen_local.insert(*new_state);
            }
        }
//        outgraph << "\n";
    }

}

void Policy::dumpCompactPolicyStatesGraph(ostream &outfile, ostream &outstateind, ostream &outgraph, bool fsap)
{

    if(fsap){
        cerr << "DumpPolicyStatesGraph is Not applicable for forbidden-policy!" << endl;
        exit(1);
        return;
    }

    int node_ind = 0;
    std::shared_ptr<RegressionStep>  current_step;
    queue<std::shared_ptr<PartialState> > open_list;
    map<PartialState,int> pseen;
    set<PartialState> seen;

    std::shared_ptr<PartialState>  current_full_state = make_shared<PartialState> (g_initial_state());
    open_list.push(current_full_state);

    // get partial first
    current_step = get_best_step(*current_full_state);

    pseen[*current_step->state] = node_ind++;
    seen.insert(*current_full_state);

    outstateind << renderState(current_step->state);

    while (!open_list.empty()){
        current_full_state = open_list.front();
        open_list.pop();

        current_step = get_best_step(*current_full_state);

        if(current_step == 0){
            cout << "No Road!" << endl;
            current_full_state->dump_pddl();
            cout << "=========" << endl;
            cerr << "Please check the current policy is a complete policy!!!";
            exit(1);
        }

        if(current_step->is_goal){
            outgraph << pseen[*current_step->state] << "\n";
            continue;
        }
//            current_full_state->dump_pddl();
//            cout << current_step->op->nondet_index << endl;
//            cout << current_step->op->get_name() << endl;
//            cout << current_step->op->get_nondet_name() << endl;
        string action_name = ((RegressionStep*)(current_step.get()))->get_name();
        int distance = ((RegressionStep*)(current_step.get()))->distance;
        string real_name = action_name.substr(0, action_name.find(" / SC")).substr(0, action_name.find(" / NSC"));
        if(g_findsc_info) cout << real_name << endl;

        std::shared_ptr<PartialState> s = current_step->state;
        for (int i = 0; i < g_variable_domain.size(); i++) {
            if (-1 != (*s)[i]) {
                outfile << i << ":" << (*s)[i];
                outfile << " ";
            }
        }
        outfile << current_step->op->nondet_index << "\n";

        // Expand
        set<PartialState> seen_local;
        for(int i = 0;i < g_nondet_mapping[current_step->op->nondet_index]->size();++i){
            std::shared_ptr<PartialState>  new_state;
            Operator *op = (*(g_nondet_mapping[current_step->op->nondet_index]))[i];
            new_state = make_shared<PartialState> (*current_full_state,*op);

            std::shared_ptr<RegressionStep> new_step = get_best_step(*new_state);
            if(new_step == 0){
                cout << "No Road!" << endl;
                current_full_state->dump_pddl();
                cout << "=========" << endl;
                cerr << "Please check the current policy is a complete policy!!!";
                exit(1);
            }

            if (seen.count(*new_state)==0){
                open_list.push(new_state);
                seen.insert(*new_state);
            }
            if(pseen.count(*new_step->state)==0){
                pseen[*new_step->state] = node_ind++;
                outstateind << "\n" << renderState(new_step->state);
            }
            // We only record the first transition.
            if(seen_local.count(*new_step->state) == 0){
                outgraph << pseen[*current_step->state] << " " << op->det_index << " " << distance;
                outgraph << " " << pseen[*new_step->state] << "\n";
                seen_local.insert(*new_step->state);
            }
        }
    }


}

void Policy::dumpHumanPolicy(ostream &outfile, bool fsap, bool var_line){

    if (!fsap){
        if(not_sorted){
            all_items.sort(regstep_compare);
            not_sorted = false;
        }
    }

    stringstream var_line_ss;
    for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = all_items.begin();
         op_iter != all_items.end(); ++op_iter) {
        ;

        outfile << "\nIf:";
        var_line_ss.clear();
        var_line_ss << "\nIf holds:";
        std::shared_ptr<PartialState> s;
        if (fsap)
            s = ((NondetDeadend*)(*op_iter).get())->state;
        else
            s = ((RegressionStep*)(*op_iter).get())->state;
        string facts_str;
        for (int i = 0; i < g_variable_domain.size(); i++) {
            if (-1 != (*s)[i]) {
//                outfile << " ";
                var_line_ss << g_variable_name[i] << ":" << (*s)[i];
                facts_str += " " + g_fact_names[i][(*s)[i]] + " |";
            }
        }
        facts_str = facts_str.substr(0,facts_str.size()-2);
        outfile << facts_str << endl;
        if(var_line)
            outfile << var_line_ss.str() << endl;

        if (fsap)
            outfile << "Forbid: " << ((NondetDeadend*)(*op_iter).get())->get_name() << endl;
        else
            outfile << "Execute: " << ((RegressionStep*)(*op_iter).get())->get_name() << endl;


    }
}

