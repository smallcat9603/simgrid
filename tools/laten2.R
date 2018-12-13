# To compare Nearest route and Fastest route

# Usage:
# R --vanilla --slave -f laten2.R --args 128.8+rr-9 0 100 5 eps
library(igraph0)

#==== For Debug ====#
topology <- "128.8+rr-9"
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

my.histogram.plot <- function(filename, v, xlabel="Value", ylabel="Frequency") {
	sink("/dev/null")
	png(filename, width=480, height=480)
	plot(hist(v), main="Histogram", xlab=xlabel, ylab=ylabel, col="Cyan")
	dev.off()
	sink()
}

my.box.plot <- function(filename, title1, vx1, vy1, title2, vx2, vy2, xlabel, xmax, ylabel, ymax) {
	sink("/dev/null")
	xl <- c(min(1, min(vx1), min(vx2)), max(xmax, max(vx1), max(vx2)))
	yl <- c(min(0, min(vy1), min(vy2)), max(ymax, max(vy1), max(vy2)))
	postscript(file=paste0(filename, ".eps"),
		horizontal=FALSE, onefile=FALSE,
		paper="special", height=4, width=6)
	par(mfrow=c(1,2))
	boxplot(vy1 ~ vx1,
		 xlim=xl, ylim=yl, 
		 main=title1, xlab=xlabel, ylab=ylabel)
	boxplot(vy2 ~ vx2,
		 xlim=xl, ylim=yl, 
		 main=title2, xlab=xlabel, ylab=ylabel)
	dev.off()
	sink()
}

# Get weighted path length along unweighetd shortest path
my.nearest.weights <- function(g) {
	weights <- matrix(nrow=length(V(g)), ncol=length(V(g)))
	for (v1 in V(g)) {
		p1 <- get.shortest.paths(g, v1, weights=NA)
		for (p12 in p1) {
			v2 <- p12[length(p12)]
			weights[v1+1, v2+1] <- sum(E(g, path=p12)$weight)
		}
	}
	weights
}

# Get unweighted path length along weighetd shortest path
my.fastest.distances <- function(g) {
	distances <- matrix(nrow=length(V(g)), ncol=length(V(g)))
	for (v1 in V(g)) {
		p1 <- get.shortest.paths(g, v1)
		for (p12 in p1) {
			v2 <- p12[length(p12)]
			distances[v1+1, v2+1] <- length(p12) - 1
		}
	}
	distances
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
# my.graph.plot(g0)

n_node <- length(V(g0))
n_edge <- length(E(g0))
sum_len <- sum(E(g0)$weight)
avg_len <- mean(E(g0)$weight)
max_deg <- max(degree(g0))
# tab_deg <- capture.output(table(degree(g0)))
# tab_len <- capture.output(table(E(g0)$weight))

# [weight = latency]
E(g0)$weight <- E(g0)$weight * 0.01 # cm -> m
E(g0)$weight <- E(g0)$weight * tcable + tswitch # m -> ns

# *1: Nearest path
hop1 <- my.nearest.distances(g0)
hop1_nz <- nonzero(hop1)
max_hop1 <- max(hop1_nz)
avg_hop1 <- mean(hop1_nz)
std_hop1 <- sd(hop1_nz)
lat1 <- my.nearest.weights(g0)
lat1_nz <- nonzero(lat1) + (thost + tswitch)
max_lat1 <- max(lat1_nz)
avg_lat1 <- mean(lat1_nz)
std_lat1 <- sd(lat1_nz)
# *2: Fastest path
hop2 <- my.fastest.distances(g0)
hop2_nz <- nonzero(hop2)
max_hop2 <- max(hop2_nz)
avg_hop2 <- mean(hop2_nz)
std_hop2 <- sd(hop2_nz)
lat2 <- my.fastest.weights(g0)
lat2_nz <- nonzero(lat2) + (thost + tswitch)
max_lat2 <- max(lat2_nz)
avg_lat2 <- mean(lat2_nz)
std_lat2 <- sd(lat2_nz)
# *3: Difference
hop3_nz <- hop2_nz - hop1_nz
max_hop3 <- max(hop3_nz)
avg_hop3 <- mean(hop3_nz)
lat3_nz <- lat2_nz - lat1_nz
max_lat3 <- min(lat3_nz)
avg_lat3 <- mean(lat3_nz)

finish_time <- Sys.time()
elapsed <- difftime(finish_time, start_time)


if (figure_type == "eps") {
	my.box.plot(basename, 
		"Nearest route", hop1_nz, lat1_nz, 
		"Fastest route", hop2_nz, lat2_nz, 
		"Hop count", 6, 
		"Latency [ns]", 100)
}
# •Ê‰ð
# 	library(gplots)
# 	hist2d(hop1_nz, lat1_nz, nbins=c(2,50), show=TRUE, col=c("white", "purple"))

writeLines(paste(
	basename,
	sum_len,
	avg_len,
	max_lat1,
	max_lat2,
	avg_lat1,
	avg_lat2,
	max_hop1,
	max_hop2,
	avg_hop1,
	avg_hop2,
	max_deg,
	sep="\t"))

writeLines(basename, logf)
writeLines("", logf)
writeLines(paste0("n_node          = ", n_node), logf)
writeLines(paste0("n_edge          = ", n_edge), logf)
writeLines(paste0("degree          = ", max_deg), logf)
writeLines(paste0("sum_len         = ", sum_len), logf)
writeLines(paste0("avg_len         = ", avg_len), logf)
writeLines(paste0("max_lat_nearest = ", max_lat1), logf)
writeLines(paste0("max_lat_fastest = ", max_lat2), logf)
writeLines(paste0("max_lat_diff    = ", max_lat3), logf)
writeLines(paste0("avg_lat_nearest = ", avg_lat1), logf)
writeLines(paste0("avg_lat_fastest = ", avg_lat2), logf)
writeLines(paste0("avg_lat_diff    = ", avg_lat3), logf)
writeLines(paste0("std_lat_nearest = ", std_lat1), logf)
writeLines(paste0("std_lat_fastest = ", std_lat2), logf)
writeLines(paste0("max_hop_nearest = ", max_hop1), logf)
writeLines(paste0("max_hop_fastest = ", max_hop2), logf)
writeLines(paste0("max_hop_diff    = ", max_hop3), logf)
writeLines(paste0("avg_hop_nearest = ", avg_hop1), logf)
writeLines(paste0("avg_hop_fastest = ", avg_hop2), logf)
writeLines(paste0("avg_hop_diff    = ", avg_hop3), logf)
writeLines(paste0("std_hop_nearest = ", std_hop1), logf)
writeLines(paste0("std_hop_fastest = ", std_hop2), logf)
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
