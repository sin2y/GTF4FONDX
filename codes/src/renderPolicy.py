from graphviz import Digraph
import math
import os
import argparse


args_parser = argparse.ArgumentParser(description='generate policy graph in the main path')
args_parser.add_argument('--main_path',
    help='Path to main dir')
args_parser.add_argument('--p',action="store_true")
args_parser.add_argument('--nt',action="store_true")
args_parser.add_argument('--ntc',action="store_true")
# args_parser.add_argument('--to_dir',
    # help='Path to store file')
params = vars(args_parser.parse_args())
#file = params["path_qnp"]
path = os.path.abspath(params['main_path'])
parital_pre = "p" if params['p'] else ""


action_path = path + "/action.ind"
state_path = path + f"/{parital_pre}state.ind"
graph_path = path + f"/{parital_pre}graph.out"
output_path = path + f"/{parital_pre}output.dot"

nont_nodes = set()
nont_pairs = set()

if params['nt']:
    output_path = path + f"/nt_{parital_pre}output.dot"
    with open(path + "/ntgraph.out","r") as f:
        for line in f.readlines():
            nodes = line.split()
            start_node = nodes[0]
            nont_nodes.add(start_node)
            for n in nodes[1:]:
                nont_pairs.add((start_node,n))

if params['ntc']:
    output_path = path + f"/ntc_{parital_pre}output.dot"
    with open(path + "/ntcgraph.out","r") as f:
        for line in f.readlines():
            nodes = line.split()
            start_node = nodes[0]
            nont_nodes.add(start_node)
            for n in nodes[1:]:
                nont_pairs.add((start_node,n))



actions = []
states = []

G = Digraph()

with open(state_path,"r") as f:
    for ind,line in enumerate(f.readlines()):
        # atoms = []
        # for s in line.split(","):
        #     if "NegatedAtom" in s:
        #         continue
        #     atoms.append(s.replace("Atom ","").strip())
        atoms = [s.replace("NegatedAtom ","¬").replace("Atom ","").strip() for s in line.split("|")]
        # atoms = [s.replace("NegatedAtom ","-").replace("Atom ","").strip() for s in line.split(",")]
        rjust_len = max(map(len,atoms)) + 3
        node_desc = f"[{ind}]\n"
        col_num = int(math.sqrt(len(atoms)))
        for i in range(math.ceil(len(atoms)/col_num)):
            node_desc += " ".join([a.rjust(rjust_len) for a in atoms[i*col_num:i*col_num+col_num]])
            node_desc += "\n"

        node_desc = node_desc[:-1]
        # "\n".join([] + atoms)
        states.append(node_desc)
        if str(ind) in nont_nodes:
            G.node(str(ind),node_desc,color="red")
        else:
            G.node(str(ind),node_desc)

# for s in states:
#     print(s)

with open(action_path,"r") as f:
    for i,line in enumerate(f.readlines()):
        line = line.replace("NegatedAtom ","¬").replace("Atom ","")
        # replace "_DETDUP_" to "."
        line = line.replace("_DETDUP_",".")         
        items = line.split(";")
        if len(items) == 2:
            action_name,effs = items
            pres = []
        else:
            action_name,pres,effs = items
        pres_info = pres#[:-1]
        effs_info = effs#[:-1]
        actions.append(f"{pres_info}\n【 {action_name} 】\n{effs_info}")

# for a in actions:
#     print(a)

edges = set()
with open(graph_path,"r") as f:
    for line in f.readlines():
        splits = line.split()
        if len(splits) == 1:
            continue

        start = splits[0]
        action = actions[int(splits[1])]
        distance = int(splits[2])
        for dest in splits[3:]:
            edges.add((start,dest,action+f"/{distance}"))

for e in edges:
    if e[:-1] in nont_pairs:
        G.edge(*e,color="red")
    else:
        G.edge(*e)

G.render(output_path)
