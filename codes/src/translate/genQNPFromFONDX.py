import argparse
from itertools import chain
from fondx_parser import FondPlusParser

class FVs:
    def __init__(self,pairs):
        self.pairs = pairs
        pass

    def dump_line(self,out_stream):
        formated_pairs = [" ".join(list(map(str,fv_pair))) for fv_pair in self.pairs]
        print(" ".join([str(len(formated_pairs))] + formated_pairs),file=out_stream)

def convert2QFName(atom):
    return "_".join([atom.predicate] + list(atom.args))

def convert2QFVs(atoms):
    '''Convert to Qnp Features-values'''
    new_atoms = [
        (
            convert2QFName(atom),
            1*(not atom.negated)
        ) for atom in atoms
        ]
    new_atoms.sort()
    return FVs(new_atoms)

def generateQFVs(atoms,func4atom):
    new_atoms = [(
            convert2QFName(atom),
            1*(func4atom(atom))
        ) for atom in atoms
        ]
    new_atoms.sort()
    return FVs(new_atoms)

def is_numeric(literal):
    return "-gr0" in literal.predicate

def is_positive(literal):
    return not literal.negated

def convert2NormlizedNames(atoms):
    new_atoms = ["_".join([atom.predicate] + list(atom.args)) for atom in atoms]
    new_atoms.sort()
    return new_atoms

def generateQAction(name,one_actions):
    '''name, pre, eff'''
    res = [ name, [], [] ]
    # print(list(map(lambda x:x.name,one_actions)))
    if "_DETDUP_" in repr(next(iter(one_actions))) and len(one_actions) < 2:
        print("Warning! Only one det_action with '_DETDUP_' was input.")
    # all preconditons should be the same
    preconditionss = set(tuple(action.precondition) for action in one_actions)
    assert(len(preconditionss) == 1)

    preconditions = set(*(preconditionss))
    # print(" ".join([" ".join(pair) for pair in convert2QFVs(preconditions)]))
    res[1] = generateQFVs(preconditions, is_positive)
    
    add_ceffs = set(chain.from_iterable([[(tuple(c),e) for c,e in action.add_effects] for action in one_actions]))
    add_cs = set([c for c,_ in add_ceffs])
    # all cs should be empty
    assert(add_cs == {()} or not add_cs)
    add_effs = set([e for _,e in add_ceffs])

    del_ceffs = set(chain.from_iterable([[(tuple(c),e) for c,e in action.del_effects] for action in one_actions]))
    del_cs = set([c for c,_ in del_ceffs])
    # all cs should be empty
    assert(del_cs == {()} or not del_cs)
    del_effs = set([e for _,e in del_ceffs])
    
    actual_effs = set()

    # for del_eff in del_effs:
    #     if "gr0" in convert2QF(del_eff):
    #     # num
    #         # >0 in del: DEC
    #         if(not del_eff.negated):
    #             actual_effs.add(del_eff.negate())
    #         ...
    #     else:
    #         actual_effs.add(del_eff.negate())
    [actual_effs.add(del_eff.negate()) for del_eff in del_effs]

    for add_eff in add_effs:
        if is_numeric(add_eff):
        # num
            # >0 in add and not in dels!: INC
            if add_eff.negate() not in actual_effs:
                actual_effs.add(add_eff)
            # =0 in add: DEC [This is already included when processing DEL_EFF]
            ...
        else:
            actual_effs.add(add_eff)

    res[2] = generateQFVs(actual_effs, is_positive)
    return res

class QProblem:
    def __init__(self, name=None):
        self.problem_name = "UnknowQNP" if not name else name
        self.featureFVs : FVs
        self.initFVs : FVs
        self.goalFVs : FVs
        self.qactions = []
        pass

    def convertFrom(self, fp:FondPlusParser):
        self.featureFVs = generateQFVs(fp.atoms,is_numeric)
        self.initFVs = generateQFVs(fp.atoms,lambda x:convert2QFName(x) in set(map(convert2QFName,fp.task.init)))
        self.goalFVs = generateQFVs(fp.task.goal.parts, is_positive)
        for key,item in fp._name_actions_map.items():
            action_name = (key.replace(" ","_"))
            self.qactions.append(generateQAction(action_name, item))
        ...

    def dump(self,output_stream):
        '''Dump generated QNP in .qnp format to output_stream.'''
        print(self.problem_name, file=output_stream)
        self.featureFVs.dump_line(output_stream)
        self.initFVs.dump_line(output_stream)
        self.goalFVs.dump_line(output_stream)
        # Dump Actions
        print(len(self.qactions), file=output_stream)
        for action in self.qactions:
            # name, pre, eff
            print(action[0], file=output_stream)
            (action[1]).dump_line(output_stream)
            (action[2]).dump_line(output_stream)
        ...

def parse_args():
    argparser = argparse.ArgumentParser()
    argparser.add_argument("domain", help="domain path in pddl format")
    argparser.add_argument("problem", help="problem path in pddl format")
    argparser.add_argument("qname", default=None, help="name for QNP problem")
    argparser.add_argument("output", help="output file path for QNP problem")
    return argparser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    fp = FondPlusParser(args.domain,args.problem)
    print("======== FONDX Info ========")
    fp.dumpInfo()
    qp = QProblem(args.qname)
    qp.convertFrom(fp)
    with open(args.output,"w") as f:
        qp.dump(f)
