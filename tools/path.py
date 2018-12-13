#!/usr/bin/python2.7
# 
# 	Graph analysis
# 	Ikki Fujiwara <ikki@nii.ac.jp>
# 

from __future__ import print_function
import sys
import os
import re
import math
import argparse
import networkx as nx

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument(dest="input_file", help="\"*.arcs\": unidirectional, other: bidirectional")
	group = parser.add_mutually_exclusive_group()
	group.add_argument("-u", dest="unidirectional", action="store_true", help="force unidirectional mode")
	group.add_argument("-b", dest="bidirectional", action="store_true", help="force bidirectional mode")
	parser.add_argument("-s", dest="switch_delay", type=int, help="switch delay [ns] (default=0)")
	parser.add_argument("--hist", dest="histogram", action="store_true", help="add a histogram of hop counts")
	args = parser.parse_args()
	
	basename, extname = os.path.splitext(os.path.basename(args.input_file))
	if args.switch_delay:
		switch_delay = args.switch_delay
		parameter = "{0}.t={1}".format(basename, switch_delay)
	else:
		switch_delay = 0 # switch delay [ns]
		parameter = "{0}".format(basename)
	
	# Build a graph
	if (extname == ".arcs" and not args.bidirectional) or args.unidirectional:
# 		print("Unidirectional mode", file=sys.stderr)
		G = nx.read_weighted_edgelist(args.input_file, nodetype=int, create_using=nx.DiGraph())
	else:
# 		print("Bidirectional mode", file=sys.stderr)
		G = nx.read_weighted_edgelist(args.input_file, nodetype=int, create_using=nx.Graph())
	
	# Get basic values
	n_node = G.number_of_nodes()
	n_edge = G.number_of_edges()
	max_deg = max(G.degree().itervalues())
	
	# Calculate cable lengths
	cable_lengths = nx.get_edge_attributes(G, "weight")
	sum_len, max_len, avg_len = sum_max_avg_for_list(cable_lengths.itervalues())
# 	print(sum_len, max_len, avg_len)
	
	# Calculate latency for each link
	for (i, j), len in cable_lengths.iteritems():
		G.edge[i][j]["latency"] = switch_delay + len * 0.05 # cable delay = 0.05 [ns/cm]
# 		print(i, j, G.edge[i][j]["weight"], G.edge[i][j]["latency"])
	
	# Search for shortest-hop paths
	nearest_hops = nx.shortest_path_length(G, weight=None)
# 	max_hop = nx.diameter(G)
# 	avg_hop = nx.average_shortest_path_length(G)
	sum_hop, max_hop, avg_hop = sum_max_avg_for_matrix(nearest_hops)
	
	# Search for shortest-latency paths and calculate latency for each path
	fastest_lats = nx.shortest_path_length(G, weight="latency")
	for i in fastest_lats:
		for j in fastest_lats[i]:
			if i != j:
				fastest_lats[i][j] += switch_delay + 300 # packet injection delay = 300 [ns]
# 				print(i, j, fastest_lats[i][j])
	sum_lat, max_lat, avg_lat = sum_max_avg_for_matrix(fastest_lats)
	
	# Report
	if args.histogram:
		hist_hop = histogram_for_matrix(nearest_hops)
	else:
		hist_hop = ""
	print(parameter, n_node, n_edge, max_deg, max_hop, avg_hop, sum_len, avg_len, max_lat, avg_lat, hist_hop, sep="\t")
	return

def sum_max_avg_for_list(data):
	cnt = 0
	sum = 0.0
	max = 0.0
	for e in data:
		cnt += 1
		sum += e
		if max < e:
			max = e
	return sum, max, sum / cnt

def sum_max_avg_for_matrix(data):
	cnt = 0
	sum = 0.0
	max = 0.0
	for i in data:
		for j in data[i]:
			if i != j: # exclude diagonal elements
				cnt += 1
				sum += data[i][j]
				if max < data[i][j]:
					max = data[i][j]
	return sum, max, sum / cnt

def histogram_for_matrix(data):
	hist = {0: 0}
	for i in data:
		for j in data[i]:
			if i != j: # exclude diagonal elements
				if hist.has_key(data[i][j]):
					hist[data[i][j]] += 1
				else:
					hist[data[i][j]] = 1
	return hist

if __name__ == '__main__':
	main()
