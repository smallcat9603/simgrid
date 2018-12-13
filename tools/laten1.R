# To see scalability: no nearest-path latency or fastest-path hopcount.

# Usage:
# R --vanilla --slave -f laten2.R --args 256.4.sky+ro-2 0 100 5
library(igraph0)

#==== For Debug ====#
topology <- "256.4.sky+ro-2"
thost <- 300 # latency in node [ns]
tswitch <- 60 # latency in switch [ns]
tcable <- 5 # latency in cable [ns/m]
figure_type <- "eps"
#==== For Batch ====#
argv <- commandArgs(trailingOnly=TRUE) 
topology <- basename(argv[1])
thost <- as.numeric(argv[2])
tswitch <- as.numeric(argv[3])
tcable <- as.numeric(argv[4])
if (!is.na(argv[5])) {
	figure_type <- argv[5]
} else {
	figure_type <- ""
}
#===================#

my.graph.plot <- function(g, colors="Cyan") {
	#	sink("/dev/null")
	#	png(filename, width=480, height=480)
	#	par(mai=c(0,0,0,0)) # no margin
	V(g)$size <- 5
	V(g)$label <- NA
	V(g)$color <- colors
	E(g)$width <- E(g)$weight
	E(g)$color <- "black"
	plot(g, layout=layout.graphopt)
	#   dev.off()
	#   sink()
}

my.nearest.distances <- function(g) {
	shortest.paths(g, weights=NA)
}

my.fastest.weights <- function(g) {
	shortest.paths(g)
}

nonzero <- function(v) {
	v[v != 0]
}

basename <- paste0(topology, ".t-", thost, "-", tswitch, "-", tcable)
logf <- file(paste0(basename, ".log"), "w")
start_time <- Sys.time()

# [weight = length]
g0 <- read.graph(paste0(topology, ".edges"), format="ncol", weights=TRUE, directed=FALSE)

n_node <- length(V(g0))
n_edge <- length(E(g0))
sum_len <- sum(E(g0)$weight)
avg_len <- mean(E(g0)$weight)
max_deg <- max(degree(g0))
# tab_deg <- capture.output(table(degree(g0)))
# tab_len <- capture.output(table(E(g0)$weight))
dia <- diameter(g0, weights=NA)

# [weight = latency]
E(g0)$weight <- E(g0)$weight * 0.01 # cm -> m
E(g0)$weight <- E(g0)$weight * tcable + tswitch # m -> ns

# *2: Fastest path
lat2 <- my.fastest.weights(g0)
lat2_nz <- nonzero(lat2) + (thost + tswitch)
max_lat2 <- max(lat2_nz)
avg_lat2 <- mean(lat2_nz)
std_lat2 <- sd(lat2_nz)

finish_time <- Sys.time()
elapsed <- difftime(finish_time, start_time)

writeLines(paste(
	basename,
	sum_len,
	avg_len,
	0,
	max_lat2,
	0,
	avg_lat2,
	0,
	0,
	0,
	0,
	max_deg,
	sep="\t"))

writeLines(basename, logf)
writeLines("", logf)
writeLines(paste0("n_node          = ", n_node), logf)
writeLines(paste0("n_edge          = ", n_edge), logf)
writeLines(paste0("degree          = ", max_deg), logf)
writeLines(paste0("sum_len         = ", sum_len), logf)
writeLines(paste0("avg_len         = ", avg_len), logf)
writeLines(paste0("max_lat_fastest = ", max_lat2), logf)
writeLines(paste0("avg_lat_fastest = ", avg_lat2), logf)
writeLines(paste0("std_lat_fastest = ", std_lat2), logf)
# writeLines(paste0("max_hop_nearest = ", max_hop1), logf)
# writeLines(paste0("avg_hop_nearest = ", avg_hop1), logf)
# writeLines(paste0("std_hop_nearest = ", std_hop1), logf)
# writeLines("", logf)
# writeLines(paste0("tab_deg ="), logf)
# writeLines(paste0("\t", tab_deg), logf)
# writeLines("", logf)
# writeLines(paste0("tab_len ="), logf)
# writeLines(paste0("\t", tab_len), logf)
writeLines("", logf)
writeLines(paste0("Started  ", toString(start_time)), logf)
writeLines(paste0("Finished ", toString(finish_time)), logf)
close(logf)
