def extractFairness(entry):
    fairness_pair = [[],[]]
    assert len(entry) > 1 and entry[1] == ":a"
    AB_index = 0 # A
    for part in entry[2:]:
        if part == ":b":
            AB_index = 1
            continue
        fairness_pair[AB_index].append(" ".join(part))
    return fairness_pair
