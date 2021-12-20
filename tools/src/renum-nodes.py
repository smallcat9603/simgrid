# by smallcat 211003 modified 211114
# renumber nodes so that neighboring ranks are mapped to neighboring nodes
# Algorithm of re-numbering nodes (in TAD topology)
# STEP 1: Add weight = 1 to all TAD links
# STEP 2: Obtain the Complement Graph (TAD_C) of TAD
# STEP 3: Set weight = LN (Large Number) to all TAD_C edges, and merge them to TAD
# STEP 4: Solve TSP problem using a greedy method in TAD
# STEP 5: Re-number nodes according to their order in result list
 

import pandas as pd
import networkx as nx
from networkx.algorithms import approximation as approx
import sys, getopt

def main(argv):
    try:
        opts, args = getopt.getopt(argv, "hf:s:")
    except getopt.GetoptError:
        print('Usage: python3 renum-nodes.py -f <inputfile>')
        sys.exit(2)
    edgefile = ""
    stride = 1
    for opt, arg in opts:
        if opt == '-h':
            print('Usage: python3 renum-nodes.py -f <inputfile>')
            sys.exit()
        elif opt == "-f":
            edgefile = arg
        elif opt == "-s":
            stride = int(arg)
        else:
            print('Usage: python3 renum-nodes.py -f <inputfile>')
            sys.exit(2)

    # edgefile = "n256d8g.edges" #n256d4g.edges, n256d8g.edges
    LN = 10000 #large number, should be larger than node number

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

    cycle_stride = []
    for s in range(stride):
        for i in range(s, len(cycle)-1, stride): #duplicate head=tail
            cycle_stride.append(cycle[i])

    # print(cycle)
    # print(cycle_stride)

    # renumber nodes
    for number, node in enumerate(cycle_stride):
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
    edgefile_re = edgefile.split('.edges')[0] + ".re" + str(stride) + ".edges"
    df = pd.DataFrame(edges)
    df.to_csv(edgefile_re, sep=" ", index=False, header=False)

if __name__ == "__main__":
   main(sys.argv[1:])  
