#include "gtf.h"


Controller::Controller(const list<std::shared_ptr<PolicyItem>> &policy_steps, const set<pair<PartialState, int> > &last_KeySteps,
    const list<std::shared_ptr<PolicyItem>> & last_Fsaps)
    : steps(policy_steps)
    , cur_step(steps.begin())
    , KeySteps(last_KeySteps)
    , Fsaps(last_Fsaps)
{

}

std::shared_ptr<PolicyItem>Controller::readStep()
{
    if(cur_step != steps.end())
        return *cur_step++;
    else
        return 0;
}

void Controller::addKey(std::shared_ptr<PolicyItem> item)
{
    KeySteps.insert(make_pair(*((NondetDeadend*)item.get())->state,((NondetDeadend*)item.get())->op_index));
}

bool Controller::isInKeys(std::shared_ptr<PolicyItem> item)
{
    return KeySteps.find(make_pair(*((NondetDeadend*)item.get())->state,((NondetDeadend*)item.get())->op_index)) != KeySteps.end();
}


GTF::GTF(ScFinder * sc_finder, bool strengthen_key, bool test_key)
    : pruning(false)
    , cur_ctl(0)
    , strengthen_key(strengthen_key)
    , test_key(test_key)
    , weights_compare()
    , sc_finder(sc_finder)
{
    Controller* new_ctl = new Controller({}, {}, {}); // TODO: C++11 {}
    ctls.push(new_ctl);
    cur_ctl = new_ctl;
    fsaps = cur_ctl->getFsaps();

    // For strong-cyclic planning, there is no need to compute AdversarialFsap(s).
    if(!g_sc_planning && g_adv_fsaps)
        AddAdversarialFsap(g_adv_fsaps);
}


void GTF::AddAdversarialFsap(int types)
{

    // For actions that never appear in any fairness A-set, forbid self-loops.
    map<FpAction*, vector<ABPair*> > A_ABs;
    for(int i = 0; i < g_fp_ABs.size(); ++i){
        for(set<FpAction*>::const_iterator it = g_fp_ABs[i]->A->begin(); it != g_fp_ABs[i]->A->end(); ++it){
            A_ABs[*it].push_back(g_fp_ABs[i]);
        }
    }

    list<std::shared_ptr<PolicyItem>> fsap_items;

    /*
     * Self Loop
     */
    if(types & 1){
        int count_self_loop_fsap = 0;
        //    fsap_items.push_back(new NondetDeadend(make_shared<PartialState> (*cur_state),i));
        for(int oi = 0;oi < g_nondet_mapping.size();++oi){
            // Pass if det action or action in some "A" (of A/B)
            if((*g_nondet_mapping[oi]).size() == 1 || A_ABs.count(g_fp_actions[oi])!=0){
                continue;
            }

            for(int di = 0; di < (*g_nondet_mapping[oi]).size(); ++ di){
                bool safe = false;
                std::shared_ptr<PartialState>  new_state = make_shared<PartialState> ();
                vector<PrePost> pps = (*g_nondet_mapping[oi])[di]->get_pre_post();
                vector<Prevail> pvs = (*g_nondet_mapping[oi])[di]->get_prevail();

                for(int i = 0;i < pvs.size();++i){
                    (*new_state)[pvs[i].var] = pvs[i].prev;
                }


                for(int i = 0;i < pps.size();++i){
                    if(pps[i].pre != -1){
                        (*new_state)[pps[i].var] = pps[i].pre;
                        // but if pre(!=-1) != post i.e. some atoms will be deleted, we pass it too.
                        if(pps[i].pre != pps[i].post){
                            safe = true;
                            break;
                        }
                    }
                    else{
                        (*new_state)[pps[i].var] = pps[i].post;
                    }
                }

                if(safe){
                    continue;
                }
                else{
                    fsap_items.push_back(make_shared<NondetDeadend>(new_state,oi));
                    count_self_loop_fsap += 1;
                }
            }
        }
        cout << "Number of Self Loop FSAP: " << count_self_loop_fsap << endl;
    }

    fsaps->splice(fsaps->end(),fsap_items);
}

void GTF::saveKey(std::shared_ptr<PolicyItem>item)
{
    cur_ctl->addKey(item);
    if(test_key){
        if(!testKey(cur_ctl->getKeySteps())){
            ++g_stat_pruning_key;
            pruning = true;
            return;
        }
    }
    if(strengthen_key){
        strengthenKey(item);
    }
}

void GTF::strengthenKey(std::shared_ptr<PolicyItem>item)
{
    std::shared_ptr<PartialState> cur_state = ((NondetDeadend*)item.get())->state;
    int cur_action = ((NondetDeadend*)item.get())->op_index;
//    int count = 0;
    for(int i = 0;i < g_nondet_mapping.size();++i){
        if(i == cur_action){
            continue;
        }
        if((*g_nondet_mapping[i])[0]->is_applicable(*cur_state)){
            std::shared_ptr<NondetDeadend> non_dead = make_shared<NondetDeadend>(make_shared<PartialState> (*cur_state),i);
            fsaps->push_back(non_dead);
//            count++;
        }
    }
//    cur_ctl->addUpFsapCount(count);
}


bool GTF::testKey(const set<pair<PartialState,int> > & key_steps){
    vector<std::shared_ptr<Graph::Node>> gen_nodes;
    set<PartialState> visited;
    std::map<PartialState, int> state_idx_map;
    std::map<int, std::shared_ptr<Graph::Node>> idx_node_map;
    int cur_node_idx = 0;
    for(set<pair<PartialState,int> >::const_iterator it_fix = key_steps.begin();
        it_fix != key_steps.end();
        it_fix ++ ){

        if(state_idx_map.count((*it_fix).first)==0){ // TODO: count -> find
            idx_node_map[cur_node_idx] = make_shared<Graph::Node>(cur_node_idx,0);
            state_idx_map[(*it_fix).first] = cur_node_idx++;
        }

        std::shared_ptr<Graph::Node> node = idx_node_map[state_idx_map[(*it_fix).first]];
        node->action = (*it_fix).second;

        // Extend
        for(int i = 0;i < g_nondet_mapping[(*it_fix).second]->size();++i){
            PartialState new_state((*it_fix).first,*((*(g_nondet_mapping[(*it_fix).second]))[i]));

            if(state_idx_map.count(new_state)==0){ // TODO: count -> find
                idx_node_map[cur_node_idx] = make_shared<Graph::Node>(cur_node_idx,0);
                state_idx_map[new_state] = cur_node_idx++;
            }

            std::shared_ptr<Graph::Node> new_node = idx_node_map[state_idx_map[new_state]];

            node->addSuccessor(new_node);
        }

//        cout << "Action id: " << (*it_fix).second << endl
//             << "Action name: " << (*(g_nondet_mapping[(*it_fix).second]))[0]->get_nondet_name() << endl << "------------" << endl;

    }

    for(std::map<int, std::shared_ptr<Graph::Node>>::const_iterator it_node_fix = idx_node_map.begin();
        it_node_fix != idx_node_map.end();
        it_node_fix ++ ){
        gen_nodes.push_back((*it_node_fix).second);
    }
    Graph fix_graph(gen_nodes);
    test_algo->reset();
    bool res = (*test_algo)(fix_graph);
//    fix_graph.Recycling();
    return res;
}

void GTF::BackTrack()
{
    pruning = false;
    delete cur_ctl;
    ctls.pop();
    if(ctls.size() == 1){
        ctls.pop();
        return;
    }
    cur_ctl = ctls.top();
    fsaps = cur_ctl->getFsaps();
    std::shared_ptr<PolicyItem> fsap = fsaps->back();
    fsaps->pop_back();
    saveKey(fsap);
    return;
}

ScSolution GTF::FindSC()
{
    list<std::shared_ptr<PolicyItem>> fsap_items;

    bool succ = sc_finder->solve(*fsaps);
    if(!succ){
        return ScSolution({},{},*fsaps); // TODO: check
    }

    fsap_items = *fsaps; // TODO: should avoid Copy
    if(g_sc_fsaps)
        fsap_items = g_man_fsap;

    list<std::shared_ptr<PolicyItem>> out_items = *sc_finder->full_steps; // OPT
    state_ind_map.clear();
    state_ind_map = *sc_finder->state_idx_map;

    return ScSolution(out_items,*sc_finder->solution_graph,fsap_items); // TODO: C++ 11 will make this moved/RVO
}

void GTF::Rearrange(list<std::shared_ptr<PolicyItem>> &pi)
{
    list<std::shared_ptr<PolicyItem>> cycle_items;
    if(g_cycle_items_first){
        const set<int>& cycle_state_idx = test_algo->getCycleStateIdx();
        for(list<std::shared_ptr<PolicyItem>>::const_iterator it = pi.begin();it != pi.end(); ){
            if(cycle_state_idx.count( // TODO: count -> find
                                      state_ind_map[*(*it)->state]
                        ) != 0){
                if(g_message_debug){
                    cout << "=======Cycle items=======" << endl;
                    (*it)->dump();
                    cout << "Action: " << ((((NondetDeadend*)(*it).get())->get_name())) << endl;
                }
                if(g_online_keyact_weight){
                    g_action_redundant_weight[((NondetDeadend*)(*it).get())->get_index()] += g_online_keyact_weight;
                }
                cycle_items.insert(cycle_items.begin(),*it);
                pi.erase(it++);

            }
            else
                ++it;
        }
    }

    if(g_sort_ban_items){
        pi.sort(weights_compare);
        cycle_items.sort(weights_compare);
        if(g_message_debug){
            cout << "Cycle_items After sorting ban items: -------------" << endl;
            for (list<std::shared_ptr<PolicyItem>>::const_iterator op_iter = cycle_items.begin();
                     op_iter != cycle_items.end(); ++op_iter){
                cout << state_ind_map[*(*op_iter)->state] << " ";
            }
        }
    }
    pi.splice(pi.begin(),cycle_items);
}


bool GTF::run(SieveStar * test_algo)
{

    cout << "==================== Generate-Test-Forbid ====================" << endl;
    g_timer_solving.reset();
    g_timer_solving.resume();
    this->test_algo = test_algo;
//    int count_sc = 0;
//    int count_solutions = 0;

    map<string,int> map_solution_idx;

    auto pass_or_stack = [&](ScSolution & scr){
        ++g_num_SCs;

        test_algo->reset();
        bool is_solution = (*test_algo)(scr.graph);
        bool is_new = false;

        if(is_solution){
            ++g_num_solutions;
            if(g_solving_mode == SolvingMode::GTF_BFF){
                is_new = true;
                ++g_num_unique_solutions;
                if(g_save_all_solutions)
                    savePolicyFiles(g_policy, g_main_path + "/" + "S_" + std::to_string(g_num_unique_solutions));
            }
            else{
                ostringstream ss;
                ofstream null_stream("/dev/null");
                g_policy->dumpPolicyStatesGraph(ss,null_stream,null_stream,false);
                if(map_solution_idx.find(ss.str()) == map_solution_idx.end()){
                    is_new = true;
                    map_solution_idx[ss.str()] = ++g_num_unique_solutions;
                    if(g_save_all_solutions)
                        savePolicyFiles(g_policy, g_main_path + "/" + "S_" + std::to_string(g_num_unique_solutions));
                }
                else{
                    if(!g_silent_planning){
                        cout << "Solution alreadly exists, index: " << map_solution_idx.at(ss.str());
                    }
                }
            }
            if(g_silent_planning <= 2 && is_new){
                std::cout << "\r#SC: " << g_num_SCs
                          << " #Solutions: " << g_num_solutions
                          << " #Unique_Solutions: " << g_num_unique_solutions
                          << " Elapsed_time: " << g_timer_solving << endl;
            }

        }

        if(!g_silent_planning){
            cout << "==========Found SC==========" << "        Count: " << g_num_SCs;
            if("DEBUG"){
                cout << "\tSize:" << scr.pi.size();
            }
            cout << endl;
            cout << "is " << (is_solution?"":"not ") << "a solution" << endl;
            cout << " #Solutions: " << g_num_solutions << endl;
            cout << " #Unique_Solutions: " << g_num_unique_solutions
                 << " Elapsed_time: " << g_timer_solving << endl;
        }
        else if(g_silent_planning <= 1){
            std::cout << "\r#SC: " << g_num_SCs
                      << " #Solutions: " << g_num_solutions
                      << " #Unique_Solutions: " << g_num_unique_solutions
                      << std::flush;
        }

        /* If solution found, decrease the num of solutions to find.
         * If num decreases to 0, finish the process.
         */
        if(is_solution && !(--g_num_solution2find)){
            cout << "\n==================== FOND+ solution(s) found! ====================\n";
            return true;
        }

        if(!g_silent_planning && g_num_solution2find == 1)
            cout<< "==========But fails to pass Testing!=========="<<endl;

        if(g_debug_nont && !is_solution){
            string nont_path = g_main_path + "/" + (g_debug_nont == 2 ? "NT_" + std::to_string(g_num_SCs) : "NT");
            savePolicyFiles(g_policy, nont_path);
            if(system(("cp " + g_main_path + "/ntgraph.out" + " " + nont_path).c_str())){
                cerr << "Fail to execute: cp " + g_main_path + "/ntgraph.out" + " " + nont_path << endl;
            }
        }

        // --------------------------Numerical Heuristic--------------------------------
        if(g_inits_values.size()!=0){
            NumSearch n_search(g_working_src_path + "/numeric/qnp_reducer.py");

            for(int i = 0; i < g_inits_values.size(); ++i){
                NumResult n_result = n_search.solve(g_ff_solver,g_main_path,g_inits_values[i]);
                for(int i = 0; i < n_result.get_redundant_actions().size(); ++i){

                    g_action_redundant_weight[g_nondet_index_mapping[n_result.get_redundant_actions()[i]]] += 1;
                }
            }
            // Make sure we only compute once.
            g_inits_values.clear();
        }

        Rearrange(scr.pi); // TODO: more args
        Controller* new_ctl = new Controller(scr.pi,cur_ctl->getKeySteps(),*fsaps);
        ctls.push(new_ctl);
        cur_ctl = new_ctl;
        fsaps = cur_ctl->getFsaps();
        return false;
    };


    ScSolution sc = FindSC(); // move TODO?
    if(sc.pi.empty()){
        // no SC
        cout << "==================== No SC at all! ====================" << endl;
        return false;
    }
    else{
        // For strong-cyclic planning, directly return true.
        if(g_sc_planning || pass_or_stack(sc)) return true;
    }


    while(!ctls.empty()){
        std::shared_ptr<PolicyItem> fsap = cur_ctl->readStep();
        if(pruning || !fsap){
            if(!g_silent_planning)
                cout<<"==========Pruning || There is no more Pairs need to be forbiddened.==========" << endl;
            BackTrack();
        }
        else{
            if(g_skip_key && cur_ctl->isInKeys(fsap)) continue;
            if(!g_silent_planning)
                cout << "    Current Forbid: " << ((NondetDeadend*)(fsap.get()))->get_name() << endl;
            fsaps->push_back(fsap);
            // we don't have to add up the f_count
            ScSolution sc = FindSC(); // move TODO?
            if(sc.pi.empty()){
                // no SC
                if(!g_silent_planning)
                    cout<< "==========Found No SC=========="<<endl;
                fsaps->pop_back();
                saveKey(fsap);
            }
            else{
                if(pass_or_stack(sc)) return true;
            }
        }
    }
    cout << "==================== Traversal of all SCs Completed. ====================" << endl;
    return false;

}
