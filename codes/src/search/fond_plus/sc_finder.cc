#include "sc_finder.h"


ScFinder::ScFinder()
    : engine(0)
    , sim(0)
    , is_sc(false)
    , full_steps(0)
    , state_idx_map(0)
    , solution_graph(0)
{
}

void ScFinder::clear()
{
    if(sim) {delete sim; sim = 0;}
    if(g_policy) {delete g_policy; g_policy = 0;}
    if(g_deadend_states) {delete g_deadend_states; g_deadend_states = 0;}
    if(g_deadend_policy) {delete g_deadend_policy; g_deadend_policy = 0;}
    if(g_temporary_deadends) {delete g_temporary_deadends; g_temporary_deadends = 0;}
    if(full_steps) {/*delete full_steps; */full_steps = 0;}
    if(state_idx_map) {/*delete state_idx_map; */state_idx_map = 0;}
    if(solution_graph) {/*solution_graph->Recycling();*/ /*delete solution_graph; */solution_graph = 0;}
    if(engine) { delete engine; engine = 0; }
}

void ScFinder::reset()
{
    clear();

    g_timer_regression.stop();
    g_timer_simulator.stop();
    g_timer_engine_init.stop();
    g_timer_search.stop();
    g_timer_policy_build.stop();
    g_timer_policy_eval.stop();
    g_timer_policy_use.stop();

    g_timer_regression.reset();
    g_timer_simulator.reset();
    g_timer_engine_init.reset();
    g_timer_search.reset();
    g_timer_policy_build.reset();
    g_timer_policy_eval.reset();
    g_timer_policy_use.reset();

    g_timer_engine_init.resume();
    engine = OptionParser(*g_search_option, false).start_parsing<SearchEngine *>();
    g_timer_engine_init.stop();

    sim = new Simulator(engine, !g_silent_planning);
    g_policy = new Policy();
    g_deadend_states = new Policy();

    g_deadend_policy = new Policy();

    g_temporary_deadends = new Policy();
}

void clearFiles(){
    fstream outfile;
    outfile.open(g_main_path+"/man.out", ios::out);
    outfile.close();
    outfile.open(g_main_path+"/policy.out", ios::out);
    outfile.close();
    outfile.open(g_main_path+"/graph.out", ios::out);
    outfile.close();
    outfile.open(g_main_path+"/ntgraph.out", ios::out);
    outfile.close();
    outfile.open(g_main_path+"/ntcgraph.out", ios::out);
    outfile.close();
}

bool ScFinder::solve(list<std::shared_ptr<PolicyItem>> fsaps) // TODO: reference?
{
    // reset
    reset();
    // build ban constraint
//    list<std::shared_ptr<PolicyItem>> ban_items;
//    for(list<std::shared_ptr<PolicyItem>>::const_iterator it_item = g_man_fsap.begin(); it_item != g_man_fsap.end();++it_item)
//        delete *it_item;

    // Only when we run in debug mode do we need to clear files.
    if(g_running_debug || g_debug_nont)
        clearFiles();

    g_man_fsap.clear();

    g_man_fsap.splice(g_man_fsap.end(),fsaps);
//    for(list<PolicyStep>::const_iterator it_ban = fsap.begin(); it_ban != fsap.end(); ++it_ban){
//        NondetDeadend * nondet_deadend = (*it_ban).toNondetDeadend();
//        cout << "address of nondet_deadend: ";
//        cout<<nondet_deadend << endl;
//        g_man_fsap.push_back(nondet_deadend);
//    }

    // For strong-cyclic planning, g_check_with_forbidden is initially set to false; otherwise should be true.
    if(!g_sc_planning)
        g_check_with_forbidden = true;

    g_deadend_policy->update_policy(g_man_fsap);
//    g_deadend_policy->cout_human_policy(true);





    /*********************
     * Handle JIC Limits *
     *********************/

    // Adjust the g_jic_limit so the epochs are handled properly
//    int epochs_remaining = g_num_epochs;
    double single_jic_limit = g_jic_limit / (double)g_num_epochs;
    g_jic_limit = single_jic_limit;

//    cout << "Max time for each of the " << epochs_remaining << " epochs: " << g_jic_limit << endl << endl;

    // We start the jit timer here since we should include the initial search / policy construction
//    g_timer_jit.resume();
    g_timer_search.resume();
    engine->search();
    g_timer_search.stop();

    engine->save_plan_if_necessary();
    engine->statistics();
    engine->heuristic_statistics();

//    cout << "Initial search time: " << g_timer_search << endl;
//    cout << "Initial total time: " << g_timer << endl;

    if (!engine->found_solution()) {
//        cout << "No solution -- aborting repairs." << endl;
        is_sc = false;
        return false;
//        clearFiles();
//        exit(1);
    }


//    cout << "\n\nCreating the simulator..." << endl;
//    Simulator *sim = new Simulator(engine, !g_silent_planning);

//    cout << "\n\nRegressing the plan..." << endl;
    list<std::shared_ptr<PolicyItem>> regression_steps = perform_regression(engine->get_plan(), g_matched_policy, 0, true);

//    cout << "\n\nGenerating an initial policy..." << endl;
//    g_policy = new Policy();
    g_policy->update_policy(regression_steps);
//    g_best_policy = g_policy;
//    g_best_policy_score = g_policy->get_score();

    if (g_sample_for_depth1_deadends)
        sample_for_depth1_deadends(engine->get_plan(), make_shared<PartialState> (g_initial_state()));

//    cout << "\n\nComputing just-in-time repairs..." << endl;
    bool changes_made = true;
//    g_deadend_policy->update_policy(g_man_fsap);
    while (changes_made) {
        changes_made = perform_jit_repairs(sim);
        if (!g_silent_planning)
            cout << "Finished repair round." << endl;

//        if (g_timer_jit() >= g_jic_limit) {
//            epochs_remaining--;
//            if (epochs_remaining > 0)
//                g_jic_limit += single_jic_limit;
//        }

        // Check if we should re-run the repairs with forbidden ops used
        //  in the heurstic computation.
        if (!changes_made && !g_check_with_forbidden &&
            g_detect_deadends && !(g_policy->is_strong_cyclic()) /*&&
            (g_timer_jit() < g_jic_limit)*/) {

            g_check_with_forbidden = true;
            changes_made = true;
//            if (g_best_policy != g_policy)
//                delete g_policy;
            g_policy = new Policy();

            // We need to reset the deadends since they may have been
            //  generated based on faulty heuristic computations that
            //  ignored the forbidden state-action pairs.
            if (g_deadend_policy)
                delete g_deadend_policy;
            if (g_deadend_states)
                delete g_deadend_states;
            g_deadend_policy = new Policy();
            g_deadend_policy->update_policy(g_man_fsap);
            g_deadend_states = new Policy();
        }
    }
    if (!g_silent_planning)
        cout << "Done repairing..." << endl;
//    g_timer_jit.stop();
    g_timer.stop();

    if(!(g_policy->is_strong_cyclic())){
        is_sc = false;
        return false;
    }

    is_sc = true;

    if(g_running_debug){
        g_policy->dump_human_policy();
        g_policy->dump_human_policy(/*be default*/false,/*fact_name_display=*/true);
        g_deadend_policy->dump_human_policy(true);
        g_deadend_policy->dump_human_policy(true,/*fact_name_display=*/true);
        g_policy->dump_man_policy(false,true);
        g_policy->dump_man_policy(false,false);
        g_deadend_policy->dump_man_policy(true);
    }
//    g_policy->cout_human_policy();
//    g_deadend_policy->cout_human_policy(true);
    buildPlan();
    return true;

}

void ScFinder::buildPlan()
{
    full_steps = std::make_shared<list<std::shared_ptr<PolicyItem>>>();
    state_idx_map = std::make_shared<map<PartialState,int>>();
    solution_graph = std::make_shared<Graph>();

        vector<std::shared_ptr<Graph::Node>> all_nodes;

    int node_ind = 0;
    std::shared_ptr<RegressionStep>  current_step;
    queue<std::shared_ptr<PartialState> > open_list;
//        map<PartialState,int> seen;

    std::shared_ptr<PartialState>  current_full_state = make_shared<PartialState> (g_initial_state());
    open_list.push(current_full_state);
    std::shared_ptr<Graph::Node> cur_node = make_shared<Graph::Node> (node_ind,-1);
    all_nodes.push_back(cur_node);
    solution_graph->addNode(cur_node);
    (*state_idx_map)[*current_full_state] = node_ind++;

    while (!open_list.empty()){

        current_full_state = open_list.front();
        open_list.pop();

        cur_node = all_nodes[(*state_idx_map)[*current_full_state]];
        current_step = g_policy->get_best_step(*current_full_state);

        // Every reachable state should be prescribed an action.
        assert(current_step != 0);

        if(current_step->is_goal){
            continue;
        }

        int act_idx = current_step->op->nondet_index;
        int distance = ((RegressionStep*)(current_step.get()))->distance;
        cur_node->update(act_idx,distance);

        full_steps->push_back(
                    make_shared<NondetDeadend>(
                        current_full_state, act_idx
                        )
                    );

        std::shared_ptr<PartialState>  new_state;
        std::shared_ptr<Graph::Node> new_node;
        Operator *op;
        for(int i = 0;i < g_nondet_mapping[current_step->op->nondet_index]->size();++i){
            op = (*(g_nondet_mapping[current_step->op->nondet_index]))[i];
            new_state = make_shared<PartialState> (*current_full_state,*op);
            if ((*state_idx_map).count(*new_state)==0){
                open_list.push(new_state);
                new_node = make_shared<Graph::Node> (node_ind,-1);

                // record New node
                all_nodes.push_back(new_node);
                solution_graph->addNode(new_node);

                cur_node->addSuccessor(new_node);
                (*state_idx_map)[*new_state] = node_ind++;
            }
            else{
                new_node = all_nodes[(*state_idx_map)[*new_state]];
                cur_node->addSuccessor(new_node);
            }
        }
    }
}
