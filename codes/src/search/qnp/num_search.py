import os
import argparse


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, os.pardir, os.pardir))
DEFAULT_REDUCER_PATH = os.path.join(SRC_DIR, "numeric", "qnp_reducer.py")


def parseQnpPath(path_qnp)->str:
    if (os.path.isdir(path_qnp)):
        qnps = [file for file in os.listdir(path_qnp) if ".qnp"  in file]
        assert len(qnps) == 1
        return os.path.join(path_qnp, qnps[0])
    return path_qnp



if __name__ == "__main__":
    args_parser = argparse.ArgumentParser(description="wrapper for qnp_reducer.py")

    args_parser.add_argument('--qnp_reducer_path',
        help='path to qnp_reducer.py',default=DEFAULT_REDUCER_PATH)
    args_parser.add_argument('--path_qnp',
        help='Path to qnp file, if given the parent dir, will search the qnp under the dir')
    args_parser.add_argument('--init_vals',
        help='init-vals list for numerical variables, e.g. "1 2 10" or "-1" to set every numerical variables\'s initial values',default="")
    args_parser.add_argument('--output_dir',
        help='Path to store files, if not given, delete mid_dir after done',default="")

    params = vars(args_parser.parse_args())
    path_qnp = parseQnpPath(params["path_qnp"])
    print(path_qnp)


    

