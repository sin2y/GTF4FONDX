#! /usr/bin/env python3
# -*- coding: utf-8 -*-
ACTION_LINES = 3

import argparse
import os
import qnp2pddl
import shutil
import re

def parsePddls(pddls):
    reorder_paths = {"domain":None,"problem":None}
    assert len(pddls) >= 2
    for pddl in pddls:
        with open(pddl,"r")as f:
            content = f.read()
            if len(re.findall("\(define.*?\(problem",content,re.S)) > 0:
                reorder_paths["problem"] = pddl
            elif len(re.findall("\(define.*?\(domain",content,re.S)) > 0:
                reorder_paths["domain"] = pddl
    return reorder_paths

def parseQnpPath(path_qnp)->str:
    if (os.path.isdir(path_qnp)):
        qnps = [file for file in os.listdir(path_qnp) if ".qnp"  in file]
        pddls = [os.path.join(path_qnp,file) for file in os.listdir(path_qnp) if ".pddl"  in file]
        if len(pddls) >= 2:
            if os.path.join(path_qnp,"domain.pddl") in pddls and os.path.join(path_qnp,"problem.pddl") in pddls:
                ...
            else:
        #   pddls mode 
                domain_problem = parsePddls(pddls)
                if domain_problem["domain"] != os.path.join(path_qnp,"domain.pddl"):
                    shutil.copy(domain_problem["domain"],os.path.join(path_qnp,"domain.pddl"))
                if domain_problem["problem"] != os.path.join(path_qnp,"problem.pddl"):
                    shutil.copy(domain_problem["problem"],os.path.join(path_qnp,"problem.pddl"))
            return path_qnp,"PDDLS"
        assert len(qnps) == 1, "There are more than 1 qnp file under the dir path, attempt to specify the exact file path?"
        return os.path.join(path_qnp,qnps[0]),"dir"
    else:
        return path_qnp,"file"

args_parser = argparse.ArgumentParser(description='generate fond-asp input for qnp')
args_parser.add_argument('--path_qnp',
    help='Path to qnp file')
args_parser.add_argument('--to_dir',
    help='Path to store file')
params = vars(args_parser.parse_args())
#file = params["path_qnp"]
path_qnp,mode = parseQnpPath(os.path.abspath(params['path_qnp']))
qnp_name = path_qnp.split("/")[-1].split('.')[0]
if mode == "file":
    to_dir = os.path.join(os.path.abspath(params['to_dir']), qnp_name)
    try:
        os.makedirs(to_dir)
    except:
        pass
    shutil.copy(path_qnp,to_dir)
elif mode == "dir":
    to_dir = os.path.abspath(params['path_qnp'])
elif mode == "PDDLS":
    to_dir = path_qnp
    print(to_dir)
    exit(0)

#
#def genActionInfo(qp):
#    info = ""
#    info += str(ACTION_LINES)
#    for a_name,action in qp.qnp.actions.items():
#        info += "\n"
#        info += (f"{a_name}\n{' '.join([nv+'-gr0' for nv in action.increment_names])}\n{' '.join([nv+'-gr0' for nv in action.decrement_names])}")
#    
#    return info

with open(path_qnp,'r') as f:
    q = qnp2pddl.QnpParser()
    q.loadQnp(f)
    qp = qnp2pddl.QnpPddl(q)
#    with open(os.path.join(to_dir,"qnp_action.info"),'w') as f:
#        f.write(genActionInfo(qp))
    with open(os.path.join(to_dir,"domain.pddl"),'w') as f:
        f.write(qp.outDomainPddl())
    with open(os.path.join(to_dir,"problem.pddl"),'w') as f:
        f.write(qp.outFairProblemPddl())
print(to_dir)
