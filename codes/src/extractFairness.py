#! /usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import argparse
import re


argparser = argparse.ArgumentParser()
argparser.add_argument("--domain",type=str,)
argparser.add_argument("--problem",type=str)
argparser.add_argument("--save_path",type=str)
args = argparser.parse_args()

input_text = ""
output_text = ""

with open(args.problem,"r") as f:
    for line in f.readlines():
        if ";" in line:
            line = line.replace(re.findall("(;.*)",line)[0],"")
        input_text += line + "\n"

#    input_text = f.read()
#print(input_text)

f_pattern = re.compile(r'\(:fairness.*?\)\)', re.DOTALL)
fairness_blocks = f_pattern.findall(input_text)

a_pattern = r':a\s*((\([^()]*\)\s*)+)\s*:*'
b_pattern = r':b\s*((\([^()]*\)\s*)+)\s*\)'

output_text += "2\n"
for block in fairness_blocks:
    a_matches = re.search(a_pattern,block,re.DOTALL)
    if a_matches:
        output_text += (" ".join(a_matches.group(1).replace("\n","").strip().split()))
    output_text += ("\n")
    b_matches = re.search(b_pattern,block,re.DOTALL)
    if b_matches:
        output_text += (" ".join(b_matches.group(1).replace("\n","").strip().split()))
    output_text += "\n"

#print(output_text)

with open(os.path.join(args.save_path,"fp_ABs.info"),"w") as f:
    f.write(output_text)

