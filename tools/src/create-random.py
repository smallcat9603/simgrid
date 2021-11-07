#!/usr/bin/python3
# coding: utf-8
#=======================================================================
#
#	Create a random graph
#	by Ikki Fujiwara, National Institute of Informatics
#	2018-11-12
#
#=======================================================================
# "create-random.py" is licensed under a Creative Commons Attribution 4.0 International License.
# http://creativecommons.org/licenses/by/4.0/

author = "(random)"
email = "graphgolf@nii.ac.jp"
text1 = "A random graph provided as a baseline."

import networkx as nx
import argparse
argumentparser = argparse.ArgumentParser()
argumentparser.add_argument('nnodes', type=int)
argumentparser.add_argument('degree', type=int)

def main(args):
	nnodes = args.nnodes
	degree = args.degree
	assert degree < nnodes
	
	low_diam, low_aspl = lower_bound_of_diam_aspl(nnodes, degree)
	g = nx.random_regular_graph(degree, nnodes, 0)
	if nx.is_connected(g):
		hops = nx.shortest_path_length(g, weight=None)
		diam, aspl = max_avg_for_matrix(hops)
	else:
		diam, aspl = float("inf"), float("inf")
	print("{}\t{}\t{}\t{}\t{}\t{}\t{}%".format(nnodes, degree, diam, aspl, diam - low_diam, aspl - low_aspl, 100 * (aspl - low_aspl) / low_aspl))
	
	basename = "n{}d{}.random".format(nnodes, degree)
	save_edges(g, basename + ".edges")
# 	save_image(g, basename + ".png")
# 	save_json(author, email, text1, basename + ".edges", basename + ".json")
	return

def save_edges(g, filepath):
	nx.write_edgelist(g, filepath, data=False)
	return

def save_image(g, filepath):
	import matplotlib as mpl
	mpl.use('Agg')
	import matplotlib.pyplot as plt
	
# 	layout = nx.spring_layout(g)
	layout = nx.circular_layout(g)
	nx.draw(g, with_labels=False, node_size=50, linewidths=0, alpha=0.5, node_color='#3399ff', edge_color='#666666', pos=layout)
	plt.draw()
	plt.savefig(filepath)
	return

def save_json(author, email, text1, graph_file, filepath):
	import os
	import json
	from datetime import datetime
	
	metadata = {}
	metadata['author'] = author
	metadata['email'] = email
	metadata['text1'] = text1
	metadata['disclose'] = True
	metadata['graph_file'] = os.path.basename(graph_file)
	metadata['stamp'] = datetime.isoformat(datetime.utcnow())
	with open(filepath, 'w') as f:
		json.dump(metadata, f, indent=1)
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
