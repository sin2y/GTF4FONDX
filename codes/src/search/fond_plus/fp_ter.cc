#include "fp_ter.h"

Graph ConstructGraph(const std::set<std::shared_ptr<Graph::Node> > &nodes) {
    std::map<std::shared_ptr<Graph::Node>, std::shared_ptr<Graph::Node> > node_map;
    std::set<std::shared_ptr<Graph::Node> > new_nodes;

    for (std::set<std::shared_ptr<Graph::Node> >::const_iterator it = nodes.begin();
         it != nodes.end(); ++it) {
        std::shared_ptr<Graph::Node> node = *it;
        std::shared_ptr<Graph::Node> new_node = std::make_shared<Graph::Node>(node);
        node_map[node] = new_node;
        new_nodes.insert(new_node);
    }

    for (std::set<std::shared_ptr<Graph::Node> >::const_iterator it = nodes.begin();
         it != nodes.end(); ++it) {
        std::shared_ptr<Graph::Node> node = *it;
        for (std::set<std::shared_ptr<Graph::Node> >::const_iterator succ_it = node->successors.begin();
             succ_it != node->successors.end(); ++succ_it) {
            std::shared_ptr<Graph::Node> successor = *succ_it;
            if (nodes.count(successor) > 0)
                node_map[node]->successors.insert(node_map[successor]);
        }
    }

    Graph graph;
    for (std::set<std::shared_ptr<Graph::Node> >::const_iterator it = new_nodes.begin();
         it != new_nodes.end(); ++it) {
        graph.addNode(*it);
    }
    return graph;
}
