#!/usr/bin/python3
# coding: utf-8
#=======================================================================
#
#	Create a graph with (possible) largest bisection bandwidth
#	by smallcat
#	2021-10-14
#
#=======================================================================

import networkx as nx
import argparse
import os

argumentparser = argparse.ArgumentParser()
argumentparser.add_argument('nnodes', type=int)
argumentparser.add_argument('degree', type=int)
argumentparser.add_argument('-i', type=int, default=1000) #iteration

def main(args):
	nnodes = args.nnodes
	degree = args.degree
	assert degree < nnodes
	LOOP = args.i
	
	low_diam, low_aspl = lower_bound_of_diam_aspl(nnodes, degree)
	g = nx.random_regular_graph(degree, nnodes, 0)
	
	basename = "n{}d{}r.bisec".format(nnodes, degree)
	edgefile = basename + ".edges"
	dir = "output/"
	if not os.path.exists(dir):
		os.makedirs(dir)
		
	# get initial bisection bandwidth
	bisec = calc_bisec(g, dir+edgefile)		
	print("nnodes\tdegree\tdiam\taspl\tdiam_diff\taspl_diff\taspl_diff_rel\tbisec\titeration")
	output(basename, g, nnodes, degree, low_diam, low_aspl, bisec, LOOP)

	while(LOOP > 0):
		g_temp = g.copy()		
		# 2-opt (A-B C-D --> A-D B-C)
		nx.connected_double_edge_swap(g)
		bisec_update = calc_bisec(g, dir+edgefile)
		if bisec_update < bisec: # restore
			g = g_temp.copy()
			LOOP = LOOP - 1
		elif bisec_update == bisec: # accept but no update
			LOOP = LOOP - 1
		else:
			bisec = bisec_update # update
			LOOP = args.i
		print(LOOP, end=" ", flush=True)
		# print(".", end="", flush=True) # prevent freezing
	print()
	output(basename, g, nnodes, degree, low_diam, low_aspl, bisec, LOOP)

	return

def calc_bisec(g, edgefile):
	save_edges(g, edgefile)
	# ikki tool (ruby)
	cmd = "./bisec.rb " + edgefile 
	rr = os.popen(cmd).read()
	return int(rr.split("\t")[1])

def output(basename, g, nnodes, degree, low_diam, low_aspl, bisec, iteration):
	if nx.is_connected(g):
		hops = nx.shortest_path_length(g, weight=None)
		diam, aspl = max_avg_for_matrix(hops)
	else:
		diam, aspl = float("inf"), float("inf")
	print("{}\t{}\t{}\t{}\t{}\t{}\t{}%\t{}\t{}".format(nnodes, degree, diam, aspl, diam - low_diam, aspl - low_aspl, 100 * (aspl - low_aspl) / low_aspl, bisec, iteration))
	file = open("output/"+basename+".txt", mode='a')
	file.writelines("{}\t{}\t{}\t{}\t{}\t{}\t{}%\t{}\t{}\n".format(nnodes, degree, diam, aspl, diam - low_diam, aspl - low_aspl, 100 * (aspl - low_aspl) / low_aspl, bisec, iteration))
	file.close()

def save_edges(g, filepath):
	nx.write_edgelist(g, filepath, data=False)
	return

def lower_bound_of_diam_aspl(nnodes, degree):
	diam = -1
	aspl = 0.0
	n = 1
	r = 1
	while True:
		tmp = n + degree * pow(degree - 1, r - 1)
		if tmp >= nnodes:
			break
		n = tmp
		aspl += r * degree * pow(degree - 1, r - 1)
		diam = r
		r += 1
	diam += 1
	aspl += diam * (nnodes - n)
	aspl /= (nnodes - 1)
	return diam, aspl

def max_avg_for_matrix(data):
	cnt = 0
	sum = 0.0
	max = 0.0
	if nx.__version__ >= "2.0":
		for i, row in data:
			for j, val in row.items():
				if i != j:
					cnt += 1
					sum += val
					if max < val:
						max = val
	else:
		for i in data:
			for j in data[i]:
				if i != j:
					cnt += 1
					sum += data[i][j]
					if max < data[i][j]:
						max = data[i][j]
	return max, sum / cnt

if __name__ == '__main__':
	main(argumentparser.parse_args())

