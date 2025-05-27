import re

SUFFIX_MULTI_CONDITIONS = "_V"
SUFFIX_DISJUNCTIONS = "_OR_"
SUFFIX_DETERMINIZATION = "_DETDUP_"


def get_nondet_name(name):
    return re.sub(f"{SUFFIX_DETERMINIZATION}\d+","",name)

def get_raw_name(name):
    return re.sub(f"{SUFFIX_DISJUNCTIONS}\d+","",re.sub(f"{SUFFIX_MULTI_CONDITIONS}\d+","",get_nondet_name(name)))