#include "num_search.h"



NumResult NumSearch::solve(std::string ff_solver,std::string qnp_dir,std::string init_vals)
{
    std::string arg_init_vals = "";
//    for(int i = 0; i < init_vals.size(); ++i)
//        arg_init_vals += std::to_string(init_vals[i]) + " ";
    std::fstream file_redundant_actions;
    file_redundant_actions.open((qnp_dir+"/qnp_num/"+"redundant_actions").c_str(),std::ios::out);
    file_redundant_actions.close();
    arg_init_vals += init_vals;
    if(system(("python3 " + exec_path
             + " --ff_solver " + ff_solver
             + " --init_vals " + arg_init_vals
             + " --output_dir " + qnp_dir
             + " --path_qnp " + qnp_dir
             ).c_str()))
        exit(1);
//    {}
    // Parser result
    return NumResult(qnp_dir + "/qnp_num/");
}

NumResult::NumResult(std::string output_dir)
{
    std::string redundant_actions_path = output_dir+'/'+"redundant_actions";
    std::ifstream ra_file(redundant_actions_path.c_str());

    if (!ra_file.is_open()) {
        std::cerr << "Could not open the file" << redundant_actions_path << std::endl;
        throw std::runtime_error("File not found.");
    }
    std::string line;
    while(std::getline(ra_file, line)){
        redundant_actions.push_back(line);
    }

    ra_file.close();
}
