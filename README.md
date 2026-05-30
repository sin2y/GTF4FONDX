# Solving QNP and FOND$^+$ with Generating, Testing and Forbidding

This project contains a GTF solver for qualitative numeric planning (QNP) and
FOND$^+$ planning. The runnable code is under `codes`; the benchmark inputs are
under `domains`.

## Repository Layout

- `codes/src/preOutput_plus`: preprocessing wrapper. It converts QNP input to
  FOND$^+$ PDDL when needed, then runs the translator and preprocessor.
- `codes/src/qnp2pddl.py` and `codes/src/genFONDInput_plus.py`: QNP parser and
  QNP-to-PDDL conversion.
- `codes/src/translate`: modified Fast Downward translator with FOND$^+$ fairness
  support.
- `codes/src/preprocess`: Fast Downward-style preprocessor. It reads
  `output.sas` and writes `output`.
- `codes/src/search`: modified PRP/Fast Downward search code. `search/downward`
  is the built solver binary. `search/fond_plus` contains the generating,
  testing and forbidding loop.
- `codes/src/numeric`: numeric FF-based reducer used by the `3FN` mode.
- `codes/src/renderPolicy.py`: optional Graphviz rendering for dumped policy
  graphs.
- `domains/QNP`: QNP instances and their PDDL translations.
- `domains/FONDX`: FOND$^+$ PDDL instances.

## Requirements

- Linux or another Unix-like environment with `bash`.
- `python3`.
- `make` and a C++ compiler with C++11 support.
- Optional for `3FN`: `flex`, `bison`, and a working build of
  `codes/src/numeric/ff`.
- Optional for policy rendering: Graphviz command line tools and the Python
  `graphviz` package.

## Build

From the project root:

```shell
cd codes/src
./build_all
```

`build_all` builds `src/preprocess/preprocess` and `src/search/downward`.

For `3FN`, also build the numeric FF:

```shell
cd codes/src/numeric
make -f makefile
```

## Input Format

Set `PROBLEM_PATH` to a directory containing one of the following:

- FOND$^+$ PDDL: `domain.pddl` and `problem.pddl`.
- QNP: exactly one `.qnp` file.

The QNP text format used by `qnp2pddl.py` is:

1. problem name,
2. feature declaration line: `N name type ...`, where type `0` is Boolean and
   type `1` is numeric,
3. initial feature values,
4. goal feature values,
5. number of actions, followed by each action name, its precondition line, and
   its effect line.

## Run

Use the wrapper script `codes/GTF`. It selects the solver preset, resolves
`--working_src_path` to `codes/src`, checks that the planner binary exists, and
then forwards the request to `src/search/downward`.

```shell
cd codes

./GTF 3FF ../domains/FONDX/BlocksColumns-Adv/p3-1-1
```

Examples:

```shell
./GTF BFF ../domains/FONDX/BlocksColumns-Adv/p3-1-1
./GTF 3FF ../domains/FONDX/BlocksColumns-Adv/p3-1-1 --show-policy 0
./GTF 3FN ../domains/QNP/BlocksColumns/InQNP/p4-1-3 --num-solutions 2
```

Extra options are appended after the wrapper defaults. For simple scalar
options, pass the option again to override the default value.

## Generated Files

The preprocessing and search pipeline writes intermediate and result files into
`PROBLEM_PATH`, including:

- `domain.pddl` and `problem.pddl` when a `.qnp` instance is translated.
- `output.sas`, `output`, and `elapsed.time` from translation/preprocessing.
- `result.info`, `policy.out`, and `policy.fasp` from search.
- `action.ind`, `state.ind`, `graph.out`, `pstate.ind`, and `pgraph.out` when
  graph dumping is enabled.
- `qnp_num/` and `redundant_actions` when `3FN` runs the numeric reducer.
- `S_*`, `NT`, or `NT_*` when saving all solutions or non-terminating debug
  policies is enabled.

These products are ignored by `.gitignore`; the repository should keep source
code and benchmark inputs, not local run outputs or binaries.

## Policy Graph Rendering

Enable graph dumping in the solver command:

```shell
--dump-graph 1
```

Then render a policy graph:

```shell
python3 src/renderPolicy.py --main_path "$PROBLEM_PATH"
```

Useful flags are `--p` for compact partial-state graphs, `--nt` for
non-terminating edges, and `--ntc` for compact non-terminating edges.
