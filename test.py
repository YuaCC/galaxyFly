import numpy as np
from  finitefield import GF
q=1024
field= GF(q)
elts_map = {}
for (i, v) in enumerate(field):
    elts_map[i] =v
print(elts_map)
rev_elts_map = {v:k for k,v in elts_map.items()}
print(rev_elts_map)
add_table = np.zeros((q,q))
for i in range(q):
    for j in range(q):
        add_table[i][j] = rev_elts_map[elts_map[i]+elts_map[j]]
mul_table = np.zeros((q,q))
for i in range(q):
    for j in range(q):
        mul_table[i][j] = rev_elts_map[elts_map[i]*elts_map[j]]
print(add_table)
print(mul_table)


