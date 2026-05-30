#ifndef FP_TER_H
#define FP_TER_H

#include <algorithm>
#include <climits>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "../globals.h"

class FpAction;

struct ABPair {
    std::set<FpAction *> *A;
    std::set<FpAction *> *B;

    ABPair(std::set<FpAction *> *A, std::set<FpAction *> *B)
        : A(A),
          B(B) {
    }
};

class Graph {
public:
    class Node;

    Graph() {
    }

    Graph(const std::vector<std::shared_ptr<Node> > &nodes) {
        for (size_t i = 0; i < nodes.size(); ++i) {
            addNode(nodes[i]);
        }
    }

    void addNode(std::shared_ptr<Node> node) {
        nodes.insert(node);
    }

    void deleteEdgeFrom(std::shared_ptr<Node> from) {
        from->successors.clear();
        from->action = -1;
    }

    std::set<std::shared_ptr<Node> > nodes;

    class Node {
    public:
        Node(int node_index, int action, int distance = -1)
            : index(node_index),
              distance(distance),
              action(action) {
        }

        explicit Node(const std::shared_ptr<Node> node)
            : index(node->index),
              distance(node->distance),
              action(node->action) {
        }

        void addSuccessor(std::shared_ptr<Node> successor) {
            successors.insert(successor);
        }

        void update(int new_action_index, int new_distance) {
            action = new_action_index;
            distance = new_distance;
        }

        int index;
        int distance;
        int action;
        std::set<std::shared_ptr<Node> > successors;

    private:
        Node(const Node &);
        Node &operator=(const Node &);
    };
};

inline bool oneTerminate(const Graph &scc) {
    return scc.nodes.size() == 1 && (*scc.nodes.begin())->successors.empty();
}

Graph ConstructGraph(const std::set<std::shared_ptr<Graph::Node> > &nodes);

class FpAction {
public:
    FpAction(std::string op_name, int op_idx = -1)
        : op_name(op_name),
          op_idx(op_idx) {
    }

    int getIndex() {
        return op_idx;
    }

    std::string getName() {
        return op_name;
    }

    void belongsToA(ABPair *AB) {
        inA.insert(AB);
    }

    void belongsToB(ABPair *AB) {
        inB.insert(AB);
    }

    const std::set<ABPair *> &getInA() {
        return inA;
    }

    const std::set<ABPair *> &getInB() {
        return inB;
    }

private:
    std::string op_name;
    int op_idx;
    std::set<ABPair *> inA;
    std::set<ABPair *> inB;
};

class Tarjan {
public:
    std::vector<Graph> SCCs;
    int timeStamp;
    std::map<std::shared_ptr<Graph::Node>, int> DFN;
    std::map<std::shared_ptr<Graph::Node>, int> LOW;
    std::stack<std::shared_ptr<Graph::Node> > Stack;
    std::set<std::shared_ptr<Graph::Node> > InStack;
    std::vector<std::pair<int, int> > SCC_order;
    int num_graph_nodes;
    std::set<std::shared_ptr<Graph::Node> > graph_nodes;

    Tarjan(Graph &graph)
        : graph_nodes(graph.nodes) {
        timeStamp = 0;
        num_graph_nodes = graph.nodes.size();

        for (std::set<std::shared_ptr<Graph::Node> >::iterator it = graph.nodes.begin();
             it != graph.nodes.end(); ++it) {
            std::shared_ptr<Graph::Node> node = *it;
            DFN[node] = timeStamp;
            LOW[node] = timeStamp;
        }

        for (std::set<std::shared_ptr<Graph::Node> >::iterator it = graph.nodes.begin();
             it != graph.nodes.end(); ++it) {
            std::shared_ptr<Graph::Node> node = *it;
            if (DFN[node] == 0)
                RunTarjan(node);
        }

        if (g_sort_scc)
            std::sort(SCC_order.begin(), SCC_order.end());
    }

    void RunTarjan(std::shared_ptr<Graph::Node> node) {
        timeStamp++;
        DFN[node] = timeStamp;
        LOW[node] = timeStamp;
        Stack.push(node);
        InStack.insert(node);

        for (std::set<std::shared_ptr<Graph::Node> >::iterator it = node->successors.begin();
             it != node->successors.end(); ++it) {
            std::shared_ptr<Graph::Node> succ = *it;
            if (graph_nodes.count(succ) == 0)
                continue;

            if (DFN[succ] == 0) {
                RunTarjan(succ);
                LOW[node] = std::min(LOW[node], LOW[succ]);
            } else if (InStack.count(succ) > 0) {
                LOW[node] = std::min(LOW[node], DFN[succ]);
            }
        }

        if (DFN[node] == LOW[node]) {
            std::set<std::shared_ptr<Graph::Node> > node_set;
            std::shared_ptr<Graph::Node> tmp = 0;
            int min_idx = INT_MAX;
            while (tmp != node) {
                tmp = Stack.top();
                if (tmp->index < min_idx)
                    min_idx = tmp->index;
                Stack.pop();
                InStack.erase(tmp);
                node_set.insert(tmp);
            }
            SCC_order.push_back(std::make_pair(min_idx, SCCs.size()));
            SCCs.push_back(ConstructGraph(node_set));
        }
    }
};

#endif
