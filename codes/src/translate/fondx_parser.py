from translate import *
from collections import defaultdict
import os,re

class FondPlusParser:
    
    def __init__(self,domain_path,problem_path = None):
        if not problem_path:
            problem_path = os.path.join(domain_path,"problem.pddl")
            domain_path = os.path.join(domain_path,"domain.pddl")
        self.main_paths = [domain_path,problem_path]
        self.actions_names = set()
        self._name_actions_map = defaultdict(set)
        self.task = pddl.open(
            domain_filename=domain_path, 
            task_filename=problem_path)
        
        self.task.INVARIANT_TIME_LIMIT = int(1800)
        
        normalize.normalize(self.task)
        (relaxed_reachable, atoms, actions, axioms,
         reachable_action_params) = instantiate.explore(self.task)
        self.atoms = atoms
        self.actions = actions
        for pa in actions:
            nondet_name = ""
            nont_name_parts = re.findall("_DETDUP_\d",pa.name)
            if len(nont_name_parts) > 0:
                nondet_name = (pa.name.replace(nont_name_parts[0],""))
            else:
                nondet_name = (pa.name)
            self.actions_names.add(nondet_name)
            self._name_actions_map[nondet_name].add(pa)
        self.objects = self.task.objects

        ################ SAS PLAN #####################
        self.sas_task = pddl_to_sas(self.task)

    def dumpInfo(self):
        print(f"path: {self.main_paths}")
        print(f"#objects: {len(self.objects)}")
        print(f"#atoms: {len(self.atoms)}")
        print(f"#actions: {len(self.actions_names)}")
        print("--------------------------------------")
        dump_statistics(self.sas_task)

    def getNumObjects(self):
        return len(self.objects)

    def getNumActions(self):
        return len(self.actions_names)

    def getNumAtoms(self):
        return len(self.atoms)
        
