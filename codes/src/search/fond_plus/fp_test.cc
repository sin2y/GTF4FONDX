#include "fp_test.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <stack>

using namespace std;

bool SieveStar::operator()(Graph &graph)
{
    assert(graph.nodes.size() >= 1);

    if (oneTerminate(graph)){
        return true;
    }

    Tarjan tarjan(graph);

    bool global_really_nont = false;

    for(int io = 0;io<tarjan.SCC_order.size();++io){
        bool scc_really_nont = false;
        int i = tarjan.SCC_order[io].second;
        Graph& scc = tarjan.SCCs[i];
        if(g_message_debug){
            cout << "Current SCC nodes: " << endl;
            for(std::set<std::shared_ptr<Graph::Node>>::const_iterator it = scc.nodes.begin();it != scc.nodes.end();++it){
                cout << (*it)->index << " ";
            }
            cout << endl;
        }

        if(oneTerminate(scc))
            continue;
        map<ABPair*, stack<std::shared_ptr<Graph::Node>> > AB_fairnodes;
        map<ABPair*, int > AB_Bcount;

        for(std::set<std::shared_ptr<Graph::Node>>::const_iterator it = scc.nodes.begin();it != scc.nodes.end();++it){
            std::shared_ptr<Graph::Node> node = *it;
            for(set<ABPair* >::const_iterator it_ABp = g_fp_actions[node->action]->getInA().begin();
                it_ABp != g_fp_actions[node->action]->getInA().end();
                ++it_ABp)
                AB_fairnodes[*it_ABp].push(node);

            for(set<ABPair* >::const_iterator it_ABp = g_fp_actions[node->action]->getInB().begin();
                it_ABp != g_fp_actions[node->action]->getInB().end();
                ++it_ABp)
                AB_Bcount[*it_ABp]++;
        }

        stack<ABPair*> open;
        for(map<ABPair*, stack<std::shared_ptr<Graph::Node>> >::const_iterator it_ABnodes = AB_fairnodes.begin();
            it_ABnodes != AB_fairnodes.end(); ++it_ABnodes){
            if(AB_Bcount[(*it_ABnodes).first] == 0)
                open.push((*it_ABnodes).first);
        }

        bool scc_changing = false;
        while(!open.empty()){
            stack<std::shared_ptr<Graph::Node>> & fnodes = AB_fairnodes[open.top()];
            open.pop();
            std::shared_ptr<Graph::Node> fnode = 0;
            while(!fnodes.empty()){
                fnode = fnodes.top();
                fnodes.pop();
                if(fnode->action == -1)
                    continue;
                if(fnode->successors.size() < (*g_nondet_mapping[fnode->action]).size()){
                    if(g_message_debug){
                        cout << "Terminating Detected! Index: " << fnode->index << endl;
                    }
                    scc_changing = true;
                    for(set<ABPair* >::const_iterator it_ABp = g_fp_actions[fnode->action]->getInB().begin();
                        it_ABp != g_fp_actions[fnode->action]->getInB().end();
                        ++it_ABp){
                        if(--AB_Bcount[*it_ABp] == 0){
                            open.push(*it_ABp);
                        }
                    }
                    scc.deleteEdgeFrom(fnode);
                }
            }
        }
        if(!scc_changing){
            scc_really_nont = true;
            global_really_nont = true;
            assert(g_compact_nont_scc);
            if(!g_silent_planning)
                cout << "----------NOT QNPT, graph:----------";

            fstream file;
            if(g_debug_nont){
                file.open((g_main_path+"/ntgraph.out").c_str(),ios::out|ios::app);
            }

            for(std::set<std::shared_ptr<Graph::Node>>::iterator it = scc.nodes.begin();it != scc.nodes.end();++it){
                if((*it)->action != -1){
                    if(!g_silent_planning)
                        cout << (*it)->index << " ";
                    cycle_state_idx.insert((*it)->index);

                    if(g_debug_nont){
                        file << (*it)->index << " ";
                        for(std::set<std::shared_ptr<Graph::Node>>::iterator isucc = (*it)->successors.begin();isucc != (*it)->successors.end();++isucc){
                            file << (*isucc)->index << " ";
                        }
                        file << "\n";
                    }
                }
            }
            if(!g_silent_planning)
                cout << endl;

            if(g_debug_nont)
                file.close();
            if(g_early_return_nont)
                return false;
        }

        if(scc_really_nont){
            assert(!g_early_return_nont);
            continue;
        }

        if((*this)(scc)){
            continue;
        }
        else{
            return false;
        }
    }

    if(global_really_nont){
        assert(!g_early_return_nont);
        return false;
    }
    return true;
}
