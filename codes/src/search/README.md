#### Arguments

###### --message-debug 0

- set to 1 for more detailed output

###### --all-qnp 0

- set to 1 for finding all qnp solution (w.r.t. the specified termination rule)

###### --from-scratch 1

- set to 1 for seraching from scratch, 0 for using the current files such as man.fsap...

###### --bqnp 0 

- 0 for QNP Termination, 1 for BQNP Termination and 2 for smallBqnpTermination (seperated into serveral simple cycles and test the termination individually)

###### --policy_steps_type 0

- 1 for only forbidding cycle_items build from **cycle_state_idx** 
- 0 for forbidding all items in a sc policy (may changed order due to **cycle_state_idx**)

###### --planner-path "./GTF-debug"

- the path to the underlying PRP solver (modified)

###### --inits_values "-1,-2,-3"

- use numerical planner to reweight actions, "-1,-2,-3" means calling numerical solver serveral times with all initial_values to "1", "2", "3" respectively, then sum up the times an action doesn't appear as the weight.
- when given "", not perform reweighting

###### --detect_nont_p 0 

- 1 for return false early if non-T cycles detected in the **p**icked steps
- defaults to 0

###### --sort_ban_items 1 

- 1 for additionally sorting the ban items in the order of action-redundancy weight
- defaults to 1

###### --sort_scc 1

- 1 for sorting the SCC in ascending order of the minimum index of the nodes
- defaults to 0

###### --early_return_nont 1

- 1 for return false in SIEVE when detect a SCC of the solution graph
- defaults to 1

###### --compact_nont_scc 1

- 1 for only record cycle_state_idx for the compact nonT SCC, i.e., the scc that cannot find any satisfiable edge to delete.

- defaults to 0

###### --online_keyact_weight -1

- set value to weight the actions in the nonT cycles
  - every time an action appear in a nonT cycle, the weight of this action add up by "online_keyact_weight"

###### --pick_mode 1

- 0 for not adding the rest actions to fsap, 1 for adding the rest, 2 for adding the applicable only.
- defaults to 1

###### --cycle_items_first 1

- 1 for banning items in the NonT cycles first
- 0 for banning in the normal order (BFS order)
- defaults to 1

###### --save_all_qnp_files 1

> Need all_qnp to be true

- whether to save qnp_result files when finding a qnp solution
- defaults to 0

###### --abort_when_sc_duplicates 1

- whether to abort program when generating a duplicated SC solution.
- defaults to 0

###### --monitoring_sc 1

- Whether to monitor the unique sc solution
- defaults to 0 (from version 3.5)

#### About the cycle_state_idx

- This version, we directly add node index in the non-t cycles to the cycle_state_idx, for adjusting the forbid order in the planner.cc (or only forbid these fsap in cycle_state_idx)

#### About the g_pick_mode

In the implementation, the code about pick_mode is as below:

```c++
// After banning a s-a pair, finding it unSC, and restoring that s-a pair
        if(g_pick_mode == 1){
            for(int i = 0;i < g_nondet_mapping.size();++i){
                if(i == cur_action){
                    continue;
                }
                fsap_items.push_back(new NondetDeadend(new PartialState(*cur_state),i));
            }
        }
        else if (g_pick_mode == 2){
            for(int i = 0;i < g_nondet_mapping.size();++i){
                if(i == cur_action){
                    continue;
                }
                if((*g_nondet_mapping[i])[0]->is_applicable(*cur_state)){
                    NondetDeadend* non_dead = new NondetDeadend(new PartialState(*cur_state),i);
                    fsap_items.push_back(non_dead);
                }
            }
        }
        else{ // 0
            ;
        }

```

i.e. 

- 0 for doing nothing
- 1 for Fix the exact action

##### Notice!

- For Normal QNP solving mode (all_qnp = 0), all these choices work. **But** for the purpose of Finding all SC solutions (e.g. all_qnp = 1), set the mode to "0" will dump lots of Duplicated Solution (and slow down the process).

- In other words, only the "unSC" is real can the "0" pick mode work for finding distinct solutions.

  - For example:

    ```c++
    [ x11,x12,x13 | ] => Q1 [x21,x22,...,x2m]
    [ x11,x12,x13,x21 | ] => Q2 [x31,x32,...x3n]
    [ x11,x12,x13,x21,x31 | ] => UNSAT
    [ x11,x12,x13,x21,x32 | ] => UNSAT
    ...
    [ x11,x12,x13,x21,x3n | ] => UNSAT
    // Fake unSC for x21
    [ x11,x12,x13,x22 | ] => Q2 [x31,x32,...x3n] // i.e. the same as [ x11,x12,x13,x21 | ]
    ```

  - For other mode, the same example will be like:

    ```c++
    [ x11,x12,x13 | ] => Q1 [x21,x22,...,x2m]
    [ x11,x12,x13,x21 | ] => Q2 [x31,x32,...x3n]
    [ x11,x12,x13,x21,x31 | ] => UNSAT
    [ x11,x12,x13,x21,x32 | ] => UNSAT
    ...
    [ x11,x12,x13,x21,x3n | ] => UNSAT
    // Fake unSC for x21
    [ x11,x12,x13,x22 | x21 ] => Q2 [x31,x32,...x3n] // i.e. the same as [ x11,x12,x13,x21 | ]
    ```

##### Cancel

TBD
