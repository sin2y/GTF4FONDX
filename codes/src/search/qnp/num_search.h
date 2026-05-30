#ifndef NUM_SEARCH_H
#define NUM_SEARCH_H

//#include <string>
#include "../globals.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdlib>
//using namespace std;

class NumResult
{
public:
    NumResult(std::string output_dir);
    std::vector<std::string> get_plan(){return numeric_plan;};
    std::vector<std::string> get_redundant_actions(){return redundant_actions;};
private:
    std::vector<std::string> redundant_actions;
    std::vector<std::string> numeric_plan;
};

class NumSearch
{
public:
    NumSearch(std::string exec_path)
        :exec_path(exec_path)
    {};

    NumResult solve(std::string ff_solver, std::string qnp_dir, std::string init_vals);

private:
    std::string exec_path;
};

#endif // NUM_SEARCH_H
