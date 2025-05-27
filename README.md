## Solving QNP and FOND$`^+`$ with Generating, Testing and Forbidding

> This is a rough initial version of the codebase, intended for early access and preview. A more complete and refined version will be released shortly.

In ./code

### GTF-BFF

```shell
./src/search/GTF "--heuristic" "h=ff(cost_type=1)" --search "lazy_greedy([h],preferred=[h])" --random-seed 814102613 --planlocal 0 --partial-planlocal 0 --limit-planlocal 0 --optimized-scd 0 --message-debug 0 --findsc_info 0 --running-debug 0 --silent-planning 2 --num-solutions 1 --save_all_solutions 0 --working_src_path $PWD/src --from-scratch 1 --bqnp 0  --policy_steps_type 0 --solving-mode 0 --detect_nont_p 1 --online_keyact_weight 0 --abort_when_sc_duplicates 0 --monitoring_sc 0 --var_action 0 --adv_fsaps 1 --sc_fsaps 1 --skip_key 1 --strengthen_key 1 --preprocess_online 1 --show-policy 1 --dump-graph 0 --debug_nont 0 --main-path {PROBLEM_PATH}
```



### GTF-3FF

```shell
./src/search/GTF "--heuristic" "h=ff(cost_type=1)" --search "lazy_greedy([h],preferred=[h])" --random-seed 814102613 --planlocal 0 --partial-planlocal 0 --limit-planlocal 0 --optimized-scd 0 --message-debug 0 --findsc_info 0 --running-debug 0 --silent-planning 2 --num-solutions 1 --save_all_solutions 0 --working_src_path $PWD/src --from-scratch 1 --bqnp 0  --policy_steps_type 0 --solving-mode 1 --detect_nont_p 1 --online_keyact_weight 0 --abort_when_sc_duplicates 0 --monitoring_sc 0 --var_action 0 --adv_fsaps 1 --sc_fsaps 1 --skip_key 1 --strengthen_key 1 --preprocess_online 1 --show-policy 1 --dump-graph 0 --debug_nont 0 --main-path {PROBLEM_PATH}
```



### GTF-3FN

```shell
./src/search/GTF "--heuristic" "h=ff(cost_type=1)" --search "lazy_greedy([h],preferred=[h])" --random-seed 814102613 --planlocal 0 --partial-planlocal 0 --limit-planlocal 0 --optimized-scd 0 --message-debug 0 --findsc_info 0 --running-debug 0 --silent-planning 2 --num-solutions 1 --save_all_solutions 0 --working_src_path $PWD/src --from-scratch 1 --bqnp 0  --policy_steps_type 0 --solving-mode 1 --detect_nont_p 1 --online_keyact_weight 0 --abort_when_sc_duplicates 0 --monitoring_sc 0 --var_action 0 --adv_fsaps 1 --sc_fsaps 1 --skip_key 1 --strengthen_key 1 --preprocess_online 1 --show-policy 1 --dump-graph 0 --debug_nont 0 --main-path {PROBLEM_PATH}
```

