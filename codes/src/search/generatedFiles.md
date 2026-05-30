

##### qnp_action.info

- 在 [qnp_reducer.py](../numeric/qnp_reducer.py) 的数值启发式路径中生成，用于将动作关于数值变量的效果信息传递给PRP

- 第一行为一个数字，表明每个动作占的行数（暂时未用到），对于每个动作来说，第一行为名字，第二行为增加的数值变量的名称（xxx到PRP中变为“Atom xxx()”）

```
3
load
objInT-gr0
tc-gr0
unload
tc-gr0
objs-gr0 objInT-gr0
mvd
dtS-gr0
fuel-gr0 dtD-gr0
mvs
dtD-gr0
fuel-gr0 dtS-gr0
getfuel
fuel-gr0
```

##### action.ind

- 在 [planner.cc](planner.cc) 中生成，用于生成解图（ [renderPolicy.py](../renderPolicy.py) ）的时候更清晰展示动作效果
- 空格隔开分别为: 动作名称  前件 效果，在生成解图时会先将其中的NegatedAtom 和Atom 替换掉...

```
getfuel  (=0)NegatedAtom fuel-gr0(), (↑)Atom fuel-gr0(),
load  (=0)NegatedAtom dts-gr0(),(>0)Atom tc-gr0(), (↑)Atom objint-gr0(),(↓)Atom tc-gr0(),
mvd  (>0)Atom dtd-gr0(),(>0)Atom fuel-gr0(), (↑)Atom dts-gr0(),(↓)Atom dtd-gr0(),(↓)Atom fuel-gr0(),
mvs  (>0)Atom dts-gr0(),(>0)Atom fuel-gr0(), (↑)Atom dtd-gr0(),(↓)Atom dts-gr0(),(↓)Atom fuel-gr0(),
unload  (=0)NegatedAtom dtd-gr0(),(>0)Atom objint-gr0(),(>0)Atom objs-gr0(), (↑)Atom tc-gr0(),(↓)Atom objint-gr0(),(↓)Atom objs-gr0(),
```

##### graph.out

- 在 [planner.cc](planner.cc) 中生成，用于生成解图 [renderPolicy.py](../renderPolicy.py)
- 每一行的第一个数字表示start结点的编号，第二个数字表示动作编号（与`action.ind`一致），后面的数字表示转移到哪些后继结点（只有第一个数字的行表示该节点满足目标状态）

```
0 1 1 2
1 2 3 4 5 6
2 2 7 8 9 10
3 2 3 4 5 6
4 4 4 11 12 13
5 0 3
6 4 6 14 15 16
7 2 7 8 9 10
8 4 4 11 12 13
9 0 7
10 4 6 14 15 16
11
12 3 17 0 18 19
13
14
15 0 12
16
17 3 17 0 18 19
18 0 17
19 0 0
```

##### state.ind

- 每一行对应一个状态（这里是全状态）

```
Atom fuel-gr0(),Atom dtd-gr0(),NegatedAtom dts-gr0(),NegatedAtom objint-gr0(),Atom tc-gr0(),Atom objs-gr0()
Atom fuel-gr0(),Atom dtd-gr0(),NegatedAtom dts-gr0(),Atom objint-gr0(),Atom tc-gr0(),Atom objs-gr0()
Atom fuel-gr0(),Atom dtd-gr0(),NegatedAtom dts-gr0(),Atom objint-gr0(),NegatedAtom tc-gr0(),Atom objs-gr0()
Atom fuel-gr0(),Atom dtd-gr0(),Atom dts-gr0(),Atom objint-gr0(),Atom tc-gr0(),Atom objs-gr0()
Atom fuel-gr0(),NegatedAtom dtd-gr0(),Atom dts-gr0(),Atom objint-gr0(),Atom tc-gr0(),Atom objs-gr0()
NegatedAtom fuel-gr0(),Atom dtd-gr0(),Atom dts-gr0(),Atom objint-gr0(),Atom tc-gr0(),Atom objs-gr0()
NegatedAtom fuel-gr0(),NegatedAtom dtd-gr0(),Atom dts-gr0(),Atom objint-gr0(),Atom tc-gr0(),Atom objs-gr0()
Atom fuel-gr0(),Atom dtd-gr0(),Atom dts-gr0(),Atom objint-gr0(),NegatedAtom tc-gr0(),Atom objs-gr0()
...
```

> 前缀为p时表示部分状态形式表示，为nt时表示断点于strong cyclic solution但不(B)QNP termination的解

##### man.fsap和man.out

- 其中的vari并不对应于output文件中的var，（而应该是和PRP程序中的g_fact_names中的对应？——或者反过来）
