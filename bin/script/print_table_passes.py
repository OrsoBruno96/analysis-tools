#!/usr/bin/env python
# -*- coding:utf-8 -*-

from ROOT import TFile, TTree, TParameter
from sys import argv



if len(argv) != 2:
    raise

input_file = TFile(argv[1], "read")
tree = input_file.Get("output_tree")
lista = tree.GetUserInfo()
valori = dict()
valori["lep"] = list()
valori["chr"] = list()

for l in lista:
    print(str(l.GetName()) + "\t\t" + str(l.GetVal()))
    if "lep" in l.GetName():
        valori["lep"].append((l.GetName()[:-4], l.GetVal()))
    else:
        valori["chr"].append((l.GetName()[:-4], l.GetVal()))

print(valori)
