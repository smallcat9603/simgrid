import pandas as pd
import networkx as nx
from networkx.algorithms import approximation as approx

edgefile = "n256d8g.edges" #n256d4g.edges, n256d8g.edges
LN = 10000 #large number

# read edge info from edge file
columns = ["s", "d"]
data = pd.read_csv(edgefile, comment="#", sep="\s+", names=columns)
edges = []
for row in range(len(data)):
    edges.append([data["s"][row], data["d"][row]])

#print(edges)

# G = nx.Graph()
# G.add_nodes_from([1, 2, 3, 4, 5])
# G.add_edges_from([(1,2), (2,3)])

# create graph based on edge file
G = nx.read_edgelist(edgefile, comments='#', nodetype=int)
# print(G.nodes())
# print(G.edges(data=True))
G.add_edges_from(G.edges(), weight=1)
# print(G.edges(data=True))
# print(nx.__version__)

# create complete graph
C = nx.complement(G)
G.add_edges_from(C.edges(), weight=LN)
# print(G.edges(data=True))

# solve TSP problem   
cycle = approx.greedy_tsp(G, source=0)
# print(cycle)

# renumber nodes
for number, node in enumerate(cycle):
    if number == len(cycle)-1: #duplicate head=tail
        break
    for num in range(len(edges)):
        if edges[num][0] == node:
            edges[num][0] = number + LN
        elif edges[num][1] == node:
            edges[num][1] = number + LN

# restore numbers
for edge in edges:
    edge[0] = edge[0] - LN
    edge[1] = edge[1] - LN
# print(edges)

# write to file
edgefile_re = edgefile + ".re.edges"
df = pd.DataFrame(edges)
df.to_csv(edgefile_re, sep=" ", index=False, header=False)
