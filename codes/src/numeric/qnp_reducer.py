import argparse
from qnp2pddl import QnpParser, QnpPddl
from qnp2numpddl import QnpNumPddl
import os
import shutil
import subprocess
import re
import traceback

CUR_FILE_DIR = os.path.dirname(os.path.abspath(__file__))
ACTION_LINES = 3


def genActionInfo(qnp):
    info = ""
    info += str(ACTION_LINES)
    for a_name, action in qnp.actions.items():
        info += "\n"
        info += (f"{a_name}\n{' '.join([nv+'-gr0' for nv in action.increment_names])}\n{' '.join([nv+'-gr0' for nv in action.decrement_names])}")
    return info


def ff_plan(ff_solver, domain_path, problem_path):
    command = f"{CUR_FILE_DIR}/{ff_solver} -o {domain_path} -f {problem_path}"
    print("==========Begin to run FF===========")
    output = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    pattern = "ff: found legal plan as follows.*?step(.*?)time spent:"
    pattern_unsat = "problem proven unsolvable."
    raw_seq = ""
    try:
        raw_seq = re.findall(pattern, output.stdout, re.S)[0]
    except Exception as e:
        print(e)
        try:
            assert pattern_unsat in output.stdout
            print(pattern_unsat)
        except Exception as e:
            print(command)
            print(output.stdout)
            print(e)
            traceback.print_exc()
            exit()
    actions_seq = re.findall(": (.*?)\n", raw_seq)
    actions_seq = list(map(lambda x: x.lower(), actions_seq))
    print("plan len: ", len(actions_seq))
    return actions_seq


class QnpReducer:
    def __init__(self, ff_solver, qnp_path, output_dir, clean=False) -> None:
        self.qnp = QnpParser()
        self.qnp_path = qnp_path
        self.ff_solver = ff_solver
        with open(qnp_path, 'r') as f:
            self.qnp.loadQnp(f)
        self.clean_up = False
        if output_dir == "":
            self.output_dir = os.path.join(os.path.abspath(output_dir), ".temp")
            self.clean_up = True
        else:
            self.output_dir = os.path.join(os.path.abspath(output_dir), "qnp_num")
        os.makedirs(self.output_dir, exist_ok=True)
        shutil.copy(qnp_path, self.output_dir)

    def __del__(self):
        if self.clean_up:
            shutil.rmtree(self.output_dir, ignore_errors=True)

    def init_pddler(self, num_init_values):
        self.pddler = QnpNumPddl(self.qnp, debug=False, num_init_values=num_init_values)
        return

    def detect_redundant(self) -> list:
        domain_path = os.path.join(self.output_dir, "num_domain.pddl")
        problem_path = os.path.join(self.output_dir, "num_problem.pddl")
        with open(domain_path, 'w') as f:
            f.write(self.pddler.outDomainPddl())
        with open(problem_path, 'w') as f:
            f.write(self.pddler.outProblemPddl())

        actions_seq = set(ff_plan(self.ff_solver, domain_path, problem_path))
        print(f"Needed: {len(actions_seq)} actions")
        redundant_actions = []
        for action in self.qnp.actions:
            if action not in actions_seq:
                redundant_actions.append(action)
                if len(redundant_actions) <= 10:
                    print(action)
                if len(redundant_actions) == 11:
                    print("...")
        print(f"Redundant: {len(redundant_actions)} actions")
        return redundant_actions

    def reduce_and_dump(self, std_out=False):
        redundant_actions = self.detect_redundant()
        reduced_qnp = QnpParser()
        with open(self.qnp_path, 'r') as f:
            reduced_qnp.loadQnp(f)
        for redundant_action in redundant_actions:
            reduced_qnp.actions.pop(redundant_action)
        reduced_pddler = QnpPddl(reduced_qnp)
        with open(os.path.join(self.output_dir, "redundant_actions"), 'w') as f:
            f.write("\n".join(redundant_actions))
        with open(os.path.join(self.output_dir, "qnp_action.info"), 'w') as f:
            f.write(genActionInfo(reduced_qnp))
        with open(os.path.join(self.output_dir, "domain.pddl"), 'w') as f:
            f.write(reduced_pddler.outDomainPddl())
        with open(os.path.join(self.output_dir, "problem.pddl"), 'w') as f:
            f.write(reduced_pddler.outFairProblemPddl())
        if std_out:
            print("============= Redundant Actions =============")
            print(redundant_actions)
            print("============= qnp_action =============")
            print(reduced_qnp)
            print("============= Domain =============")
            print(reduced_pddler.outDomainPddl())
            print("============= Problem =============")
            print(reduced_pddler.outFairProblemPddl())


def parseQnpPath(path_qnp)->str:
    if (os.path.isdir(path_qnp)):
        qnps = [file for file in os.listdir(path_qnp) if ".qnp"  in file]
        assert len(qnps) == 1, "There are more than 1 qnp file under the dir path, attempt to specify the exact file path?"
        return os.path.join(path_qnp, qnps[0])
    return path_qnp


if __name__ == "__main__":
    args_parser = argparse.ArgumentParser(description='generate numerical_pddls from qnp')

    args_parser.add_argument('--init_vals',
        help='init-vals list for numerical variables, e.g. "1 2 10" or "-1" to set every numerical variables\'s initial values',default="")
    args_parser.add_argument('--ff_solver',
        help='version of ff solver, default is ff',default="ff")
    args_parser.add_argument('--path_qnp',
        help='Path to qnp file')
    args_parser.add_argument('--output_dir',
        help='Path to store files, if not given, delete mid_dir after done',default="")
    args_parser.add_argument('--std_out',
        help='Whether to dump to std_out',default=False)

    params = vars(args_parser.parse_args())

    init_values = params["init_vals"].split()

    std_out = (params['std_out'])

    reducer = QnpReducer(params['ff_solver'], parseQnpPath(params['path_qnp']), params['output_dir'], clean=(params['output_dir']==""))
    reducer.init_pddler(init_values)
    reducer.reduce_and_dump(std_out)
