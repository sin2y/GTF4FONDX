#include "globals.h"
#include "global_man.h"
#include "operator.h"
#include "option_parser.h"
#include "ext/tree_util.hh"
#include "timer.h"
#include "utilities.h"
#include "search_engine.h"
#include "policy-repair/regression.h"
#include "policy-repair/simulator.h"
#include "policy-repair/policy.h"
#include "policy-repair/jit.h"
#include "policy-repair/partial_state.h"
#include "fond_plus/gtf.h"

#include <iostream>
#include <list>
#include <new>
using namespace std;

int main(int argc, const char **argv) {
    g_argc = argc;
    g_argv = argv;
    clock_t start_parse = clock();
    time_t start_parse_time = time(NULL);

    register_event_handlers();
    cout << argv[0] << " "<< endl;

//    OptionParser::parse_cmd_line(argc, argv, true);

//    SearchEngine *engine = 0;

    g_timer_solving.stop();
    g_timer_engine_init.stop();

    g_timer_engine_init.reset();
    g_timer_engine_init.resume();

    fstream res_info_file;
    string result_info_path; 
    try {
        OptionParser::parse_cmd_line(argc, argv, true);

        result_info_path = g_main_path + "/result.info";
        res_info_file.open(result_info_path.c_str(),std::ios::out);
        
        // PreProcess
        if(g_preprocess_online){
            if(system((g_working_src_path + "/preOutput_plus " + g_main_path + " " + g_main_path).c_str())){
                std::cerr << "Failed to execute PreProcessing" << std::endl;
                res_info_file << "Failed to execute PreProcessing" << std::endl;
                std::cout << "Success to write into "<< result_info_path <<std::endl;
                res_info_file.close();
                exit(1);
            }
        }

        stringstream ss_input;
        if (1) {
            // Load output.sas
            stringstream ss_input;
            ifstream input_sas_file((g_main_path+"/output").c_str());

            if (!input_sas_file.is_open()) {
                std::cerr << "Unable to open file!" << std::endl;
                exit(1);
            }

            ss_input << input_sas_file.rdbuf();
            input_sas_file.close();
            std::istringstream custom_input(ss_input.str());
            read_everything(custom_input);
        }
        else if (string(argv[1]).compare("--help") != 0)
            read_everything(cin);

        OptionParser::parse_cmd_line(argc, argv, false);
    } catch (ParseError &pe) {
        cerr << pe << endl;
        exit_with(EXIT_INPUT_ERROR);
    }
    g_timer_engine_init.stop();

    if (g_detect_deadends) {
        generate_regressable_ops();
    }

    /***************************************
     * Assert the settings are consistent. *
     ***************************************/
    if (((g_record_online_deadends || g_generalize_deadends) && !g_detect_deadends) ||
        ((g_partial_planlocal || g_plan_locally_limited) && !g_plan_locally) ||
        (g_optimized_scd && (g_jic_limit == 0)) ||
        (g_forgetpolicy && (g_jic_limit > 0))) {

        cout << "\nParameter Error: Make sure that the set of parameters is consistent.\n" << endl;
        exit(1);
    }

    if(g_sc_planning){
        if(g_solving_mode != SolvingMode::Others ||
           g_strong_planning){
            cerr << "\nStrong-Cyclic Planning is not consistent with other solving modes." << endl;
            exit(1);
        }
    }

    // If we are going to do a final FSAP-free round, then we modify the
    //  time limits to give a 50/50 split between the JIC phase and final
    //  round phase
    double jic_ratio = 0.5;
    if (g_final_fsap_free_round)
        g_jic_limit *= jic_ratio;

//    cout << "Max time for core JIC (remaining used in final-round repairs): " << g_jic_limit << endl;

//    // Adjust the g_jic_limit so the epochs are handled properly
//    int epochs_remaining = g_num_epochs;
//    double single_jic_limit = g_jic_limit / (double)g_num_epochs;
//    g_jic_limit = single_jic_limit;

//    cout << "Max time for each of the " << epochs_remaining << " epochs: " << g_jic_limit << endl << endl;

    // Chech solving mode
    if(
        (   g_solving_mode == SolvingMode::GTF_BFF
            &&
            !( !g_inits_values.size()
            && !g_sort_ban_items
            && !g_sort_scc
            && !g_cycle_items_first
            && g_strengthen_key)
        )
        ||
        (   g_solving_mode == SolvingMode::GTF_3F
            &&
            !( !g_inits_values.size()
            && g_sort_ban_items
            && g_sort_scc
            && g_cycle_items_first)
        )
        ||
        (   g_solving_mode == SolvingMode::GTF_3FN
            &&
            !( g_inits_values.size()
            && g_sort_ban_items
            && g_sort_scc
            && g_cycle_items_first
            && g_op_redundant_coefficient > 0)
        )
       ){
        g_solving_mode = SolvingMode::Others;
    }


    assert( ! (g_online_keyact_weight != 0 && g_inits_values.size() > 0)
         && ! (g_online_keyact_weight != 0 && !g_sort_ban_items));

//    assert( ! (g_all_qnp && g_pick_mode == 0));

//    assert( ! (!g_all_solutions && g_save_all_solutions) );

    for (int i = 0; i < argc; ++i) {
        res_info_file << string(argv[i]) << " ";
    }
    res_info_file  << std::endl;

//    if(g_debug_nont){
//        g_ntgraph_path = g_main_path + "/ntgraph.out";
//        fstream file;
//        file.open(g_ntgraph_path.c_str(),ios::out);
//        file.close();
//        dumpActionInd(g_main_path + "/action.ind");
//    }

//    set<pair<PartialState,int> > mark_reg_steps;
//    set<pair<PartialState,int> > fix_reg_steps;

//    vector<int> actions_weights(g_nondet_mapping.size(),0);
    g_action_redundant_weight = vector<int> (g_nondet_mapping.size(),0);

    ScFinder *sc_finder = new ScFinder();

    GTF gtf(sc_finder,g_strengthen_key,g_detect_nont_p);
    SieveStar * sstar = new SieveStar();

    clock_t start = clock();
    time_t start_time = time(NULL);

    bool fully_explored = !gtf.run(sstar);
    std::cout << "\r#SC: " << g_num_SCs
              << " #Solutions: " << g_num_solutions
              << " #Unique_Solutions: " << g_num_unique_solutions
              << " Final_Elapsed_time: " << g_timer_solving << endl;

    clock_t end = clock();
    time_t end_time = time(NULL);

    if(fully_explored){
//    if(genTestForbid(fix_reg_steps,argc, argv,0,0,ss_input,count_solution,0)!=2){
        if (g_num_solutions == 0){
            cout<< "-----------There is no solution for this ";
//            if(g_bqnp==1)
//                cout << "B";
            cout << "FOND+ Problem.-----------"<<endl;
        }
        else{
            cout << "============= The number of FOND+ solutions is: ";
            cout << g_num_solutions << "." << endl;
            cout << "============= The number of Unique FOND+ solutions is: " << g_num_unique_solutions << "." << endl;
        }
    }
    else{
//        g_policy->dump_human_policy();
//        g_policy->dump_human_policy(/*be default*/false,/*fact_name_display=*/true);
//        g_deadend_policy->dump_human_policy(true);
//        g_deadend_policy->dump_human_policy(true,/*fact_name_display=*/true);
//        g_policy->dump_man_policy(false,true);
        dumpActionInd(g_main_path + "/action.ind");
//        g_policy->dump_man_policy(false,false);
//        g_deadend_policy->dump_man_policy(true);
//        outputPolicy(g_main_path + "/policy.out");
//        ofstream file_hum_policy(g_main_path + "/policy.out");

        ostringstream s_policy_out;
        g_policy->dumpHumanPolicy(s_policy_out,false,/*var_line*/false);

        if(g_dump_policy){
            ofstream file_policy_out(g_main_path + "/policy.out");
            ofstream file_policy_fsap(g_main_path + "/policy.fasp");
            file_policy_out << s_policy_out.str();
            g_deadend_policy->dumpHumanPolicy(file_policy_fsap,true,/*var_line*/false);
        }

        if(g_show_policy){
            std::cout << s_policy_out.str();
            cout << endl << "----------------------------";
        }

        if(g_dump_graph){
            ofstream file_policy(g_main_path + "/policy");
            ofstream file_graph(g_main_path + "/graph.out");
            ofstream file_state(g_main_path + "/state.ind");
            g_policy->dumpPolicyStatesGraph(file_policy,file_state,file_graph,false);
            ofstream file_ppolicy(g_main_path + "/ppolicy");
            ofstream file_pgraph(g_main_path + "/pgraph.out");
            ofstream file_pstate(g_main_path + "/pstate.ind");
            g_policy->dumpCompactPolicyStatesGraph(file_ppolicy,file_pstate,file_pgraph,false);
        }
        cout << endl;
    }
    std::cout << "Number of successfully pruning: " << g_stat_pruning_key << std::endl;
    std::cout << "Parse time(s): " <<
        (double)(start - start_parse) / CLOCKS_PER_SEC << " s" << std::endl;
    std::cout << "Total Parse time(s): " <<
        (double)(start_time - start_parse_time) << " s" << std::endl;
    std::cout << "Solve time(s): " <<
        (double)(end - start) / CLOCKS_PER_SEC << " s" << std::endl;
    std::cout << "Total Solve time(s): " <<
        (double)(end_time - start_time) << " s" << std::endl;

    res_info_file << "Parse time(s): " <<
                     (double)(start - start_parse) / CLOCKS_PER_SEC << " s" << std::endl
                  << "Total Parse time(s): " <<
                     (double)(start_time - start_parse_time) << " s" << std::endl
                  << "Solve time(s): " <<
                          (double)(end - start) / CLOCKS_PER_SEC << " s" << std::endl
                  << "Total Solve time(s): " <<
                          (double)(end_time - start_time) << " s" << std::endl;
    res_info_file.close();
    std::cout << "Success to write into "<< result_info_path <<std::endl;
    //    prp(argc, argv,0,0);



}
