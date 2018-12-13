# Graph Analysis
# 
# Usage:
# R --vanilla --slave -f path.R --args input_file [switch_delay]
# 
# Input file format:
# ------------------------------------------------------------
# <src-node-id> <dst-node-id> [<weight>]
# ------------------------------------------------------------

# To calculate latency
thost <- 300 # node delay [ns]
tswitch <- 100 # switch delay [ns]
tcable <- 5 # cable delay [ns/m]

#===============================================================================
library(igraph0)

nonzero <- function(v) {
	v[v != 0]
}

# argv[1]     filename    topology    
# ----------  ----------  ----------  
# hoge        hoge.edges  hoge        
# hoge.edges  hoge.edges  hoge        
# hoge.txt    hoge.txt    hoge.txt    
argv <- commandArgs(trailingOnly=TRUE) 
topology <- sub("\\.edges$", "", basename(argv[1]))

# Input from file
edgefn <- argv[1]
if (!file.exists(argv[1])) {
	edgefn <- paste0(argv[1], ".edges")
}
g <- read.graph(edgefn, format="ncol", weights=TRUE, directed=FALSE)

# Set switch delay
if (!is.na(argv[2])) {
	tswitch <- argv[2]
}

# Basic parameters
n_node <- length(V(g))
n_edge <- length(E(g))
max_deg <- max(degree(g))
sum_len <- sum(E(g)$weight)
avg_len <- mean(E(g)$weight)

# Nearest path
hop <- shortest.paths(g, weights=NA)
hop_nz <- nonzero(hop)
max_hop <- max(hop_nz)
avg_hop <- mean(hop_nz)
std_hop <- sd(hop_nz)

#====== Convert length to latency ======
E(g)$weight <- E(g)$weight * 0.01 # cm -> m
E(g)$weight <- E(g)$weight * tcable + tswitch # m -> ns

# Fastest path
lat <- shortest.paths(g)
lat_nz <- nonzero(lat) + (thost + tswitch)
max_lat <- max(lat_nz)
avg_lat <- mean(lat_nz)
std_lat <- sd(lat_nz)

#====== Set edges' weight to 1 ======
# E(g)$weight <- 1

# Minimum cut
# min_cut <- graph.mincut(g)

#====== Output to stdout ======
writeLines(paste(
	topology,
	n_node,
	n_edge,
	max_deg,
	max_hop,
	avg_hop,
	sum_len,
	avg_len,
	max_lat,
	avg_lat,
# 	min_cut,
	sep="\t"))
