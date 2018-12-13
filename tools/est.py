#!/usr/bin/python2.7
# coding: utf-8
# 
# 	ÉOÉâÉtâêÕ
# 	Graph analysis to estimate cost, zero-load latency, hop counts and bisection
# 	Ikki Fujiwara <ikki@nii.ac.jp>
# 

# Running time measured on calc3
#        |   4096 nodes    16384 nodes
# option |     12 degree      16 degree
# -------+-------------------------------
# --nl   |     72 sec       7810 sec
# --fl   |     22 sec        408 sec
# --nh   |      8 sec        128 sec
# --fh   |     44 sec       4120 sec

from __future__ import print_function
import sys
import os
import re
import math
import argparse
import tempfile
import subprocess
import igraph as ig

link_delay = 0.05 # propagation delay in a fiber [ns/cm]
host_delay =  300 # total injection/ejection delay at both end host [ns]
bandwidth  =   40 # bandwidth [Gbps], which the cable cost is in proportion to

parser = argparse.ArgumentParser(conflict_handler="resolve")
parser.add_argument(dest="input_file", help="*.arcs: unidirectional, *.edges: bidirectional")
parser.add_argument(
	"-e", "--nl", dest="nearest_lats", action="store_true",
	help="calculate zero-load latency for nearest (shortest-hop) paths"
)
parser.add_argument(
	"-f", "--fl", dest="fastest_lats", action="store_true",
	help="calculate zero-load latency for fastest (lowest-latency) paths"
)
parser.add_argument(
	"-g", "--nh", dest="nearest_hops", action="store_true",
	help="calculate hop counts for nearest (shortest-hop) paths, i.e. diameter and average shortest path length"
)
parser.add_argument(
	"-h", "--fh", dest="fastest_hops", action="store_true",
	help="calculate hop counts for fastest (lowest-latency) paths"
)
parser.add_argument(
	"--bisec", dest="num_partition", type=int, default=1,
	help="calculate bisection (given --bisec=2) or minimum cuts of 2- to <K>-way uniform partitions (given --bisec=<K>)"
)
parser.add_argument(
	"--sw", dest="switch_delay", type=int, default=0,
	help="switch delay [ns] (default=0)"
)
parser.add_argument(
	"--plot", dest="plot_format", metavar="FORMAT",
	help="plot the graph to a file with given extension (e.g. --plot=png, --plot=eps, etc.)"
)
parser.add_argument(
	"--layout", dest="plot_layout", metavar="ALGORITHM", default="fr",
	help="layout algorithm for the plot (e.g. --layout=circle, --layout=grid, etc. See http://j.mp/1SJg7wm for complete list) (default=fr)"
)
group = parser.add_mutually_exclusive_group()
group.add_argument(
	"--bidir", dest="bidir", action="store_true",
	help="force bidirectional mode"
)
group.add_argument(
	"--unidir", dest="unidir", action="store_true",
	help="force unidirectional mode"
)
if len(sys.argv) < 2:
	parser.print_help()
	exit(0)
args = parser.parse_args()

# Cost model taken from Besta's "Slim Fly" paper (2014)
class CostModel:
	@classmethod
	def cost_of_cable(cls, length):
		meter = length * 0.01
		dollar = min([meter * 0.4079 + 0.5771, meter * 0.0919 + 7.2745]) * bandwidth
		return round(dollar, 2)
	
	@classmethod
	def cost_of_switch(cls, nports):
		return nports * 350.4 - 892.3 # [dollar]

	@classmethod
	def power_of_switch(cls, nports):
		return nports * 4 * 0.7 # [W]

def main():
	basename, extname = os.path.splitext(os.path.basename(args.input_file))
	parameter = "{}.t={}".format(basename, args.switch_delay)
	
	# Read a graph
	if (extname == ".arcs" and not args.bidir) or args.unidir:
		g = ig.Graph.Read_Ncol(args.input_file, names=False, directed=True)
	else:
		g = ig.Graph.Read_Ncol(args.input_file, names=False, directed=False)
	
	# Assign latency for each link
	if args.nearest_lats or args.fastest_hops or args.fastest_lats:
		g.es["latency"] = [length * link_delay + args.switch_delay for length in g.es["weight"]]
	
	# Analyze
	sum_deg, max_deg, avg_deg = sum_max_avg_for_list(g.degree())
	sum_len, max_len, avg_len = sum_max_avg_for_list(g.es["weight"])
	max_hop_n, avg_hop_n, max_lat_n, avg_lat_n = analyze_nearest_paths(g)
	max_hop_f, avg_hop_f, max_lat_f, avg_lat_f = analyze_fastest_paths(g)
	total_cost, cable_cost, switch_cost, switch_power = estimate_costs(g)
	
	# Partitioning
	edgecuts = []
	if args.num_partition > 1:
		edgecuts = calculate_bisection(g, range(2, args.num_partition + 1))
	
	# Draw the graph
	if args.plot_format:
		save_image_ig(g, "{}.{}".format(parameter, args.plot_format))
	
	# Report
	print("{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}\t{}".format(parameter, sum_len, avg_len, max_lat_n, max_lat_f, avg_lat_n, avg_lat_f, max_hop_n, max_hop_f, avg_hop_n, avg_hop_f, max_deg, avg_deg, total_cost, cable_cost, switch_cost, switch_power, edgecuts))
	return

def analyze_nearest_paths(g):
	max_hop, avg_hop, max_lat, avg_lat = 0.0, 0.0, 0.0, 0.0
	add_delay = args.switch_delay + host_delay
	if not g.is_connected():
		return "Inf", "Inf", "Inf", "Inf"
	if args.nearest_hops:
		# Calculate hop count for each nearest path
		nearest_hops = g.shortest_paths(weights=None)
		max_hop, avg_hop = max_avg_for_ig_matrix(nearest_hops)
	if args.nearest_lats:
		# Calculate hop count and latency for each nearest path
		nearest_lats = [[0.0 for v1 in g.vs] for v2 in g.vs]
		for i, v1 in enumerate(g.vs):
			# The first-found path among the same-distance paths is used
			paths = g.get_shortest_paths(v1, weights=None, output="epath")
			for j, path in enumerate(paths):
				if i != j:
					nearest_lats[i][j] = sum([g.es[k]["latency"] for k in path]) + add_delay
		max_lat, avg_lat = max_avg_for_ig_matrix(nearest_lats)
	return max_hop, avg_hop, max_lat, avg_lat

def analyze_fastest_paths(g):
	max_hop, avg_hop, max_lat, avg_lat = 0.0, 0.0, 0.0, 0.0
	add_delay = args.switch_delay + host_delay
	if not g.is_connected():
		return "Inf", "Inf", "Inf", "Inf"
	if args.fastest_lats:
		# Calculate latency for each fastest path
		fastest_lats = g.shortest_paths(weights="latency")
		for i, v1 in enumerate(g.vs):
			for j, v2 in enumerate(g.vs):
				if i != j:
					fastest_lats[i][j] += add_delay
		max_lat, avg_lat = max_avg_for_ig_matrix(fastest_lats)
	if args.fastest_hops:
		# Calculate hop count and latency for each fastest path
		fastest_hops = [[0 for v1 in g.vs] for v2 in g.vs]
		for i, v1 in enumerate(g.vs):
			paths = g.get_shortest_paths(v1, weights="latency", output="epath")
			for j, path in enumerate(paths):
				if i != j:
					fastest_hops[i][j] = len(path)
		max_hop, avg_hop = max_avg_for_ig_matrix(fastest_hops)
	return max_hop, avg_hop, max_lat, avg_lat

def estimate_costs(g):
	cable_cost = sum([CostModel.cost_of_cable(length) for length in g.es["weight"]])
	switch_cost = sum([CostModel.cost_of_switch(degree) for degree in g.degree()])
	switch_power = sum([CostModel.power_of_switch(degree) for degree in g.degree()])
	return cable_cost + switch_cost, cable_cost, switch_cost, switch_power

def calculate_bisection(g, parts):
	mtsfile = tempfile.mkstemp(suffix=".metis")[1]
	with open(mtsfile, 'w') as f:
		f.write("{} {}\n".format(g.vcount(), g.ecount()))
		for adjs in g.get_adjlist():
			f.write(" ".join([str(i + 1) for i in adjs]) + "\n")
	edgecuts = {}
	for n in parts:
		res = subprocess.check_output(["gpmetis", "-nooutput", mtsfile, str(n)])
		cut = int(re.search(r"Edgecut: *(\d+)", res).group(1))
		edgecuts[n] = cut
	os.remove(mtsfile)
	return edgecuts

def sum_max_avg_for_list(data):
	cnt = 0
	sum = 0.0
	max = 0.0
	avg = 0.0
	for x in data:
		cnt += 1
		sum += x
		if max < x:
			max = x
	if cnt != 0:
		avg = sum / cnt
	return sum, max, avg

def max_avg_for_ig_matrix(data):
	cnt = 0
	sum = 0.0
	max = 0.0 
	avg = 0.0
	# data is an array of arrays (as in igraph)
	for i, row in enumerate(data):
		for j, val in enumerate(row):
			if i != j:
				cnt += 1
				sum += val
				if max < val:
					max = val
	if cnt != 0:
		avg = sum / cnt
	return max, avg

# Draw a graph (for igraph)
def save_image_ig(g, filepath):
	ig.drawing.plot(g, filepath, vertex_size=6, layout=args.plot_layout)
	return

if __name__ == '__main__':
	main()
