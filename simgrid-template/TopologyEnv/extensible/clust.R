# Usage:
# R --vanilla --slave -f clust.R --args rring-256-4 hc-ward 10


#Include the igraph library (install it if necessary)
if ( !file.exists("./packages/")){
	dir.create("./packages")
	install.packages("igraph",lib="./packages/",repos="http://cran.us.r-project.org");
}
library(igraph,lib.loc="./packages/")

#==== For Debug ====#
topology <- "rring-256-8-1"
clmethod <- "seq"
maxsize <- 16
figure_type <- "png"
#==== For Batch ====#
argv <- commandArgs(trailingOnly=TRUE) 
topology <- gsub("\\.\\w+$", "", basename(argv[1]))
clmethod <- argv[2]
maxsize <- as.numeric(argv[3])
if (!is.na(argv[4])) {
	figure_type <- argv[4]
} else {
	figure_type <- ""
}
#===================#

# for hclust()
my.dendrogram.to.membership.upto <- function(merges, maxsize) {
  n <- nrow(merges) + 1
  membership <- rep(NA, n)
  merged <- as.list(seq(1, n))
  for (i in 1:(n-1)) {
    k1 <- merges[i, 1]
    k2 <- merges[i, 2]
    if (k1 < 0) k1 <- -k1 else k1 <- n + k1
    if (k2 < 0) k2 <- -k2 else k2 <- n + k2
    v1 <- merged[[k1]]
    v2 <- merged[[k2]]
    if (length(v1) + length(v2) <= maxsize) {
      merged[[n+i]] <- c(v1, v2)
      merged[[k1]] <- NA
      merged[[k2]] <- NA
    } else if (length(v1) < length(v2)) {
      merged[[n+i]] <- v1
      merged[[k1]] <- NA
    } else {
      merged[[n+i]] <- v2
      merged[[k2]] <- NA   
    }
  }
  k <- 0
  for (i in 1:length(merged)) {
    v <- merged[[i]]
    if (length(v) > 1 || !is.na(v)) {
      membership[v] <- k
      k <- k + 1
    } 
  }
  na.fail(membership)
  membership
}

# for leading.eigenvector.community() etc.
my.community.to.membership.upto <- function(merges, maxsize) {
  n <- nrow(merges) + 1
  membership <- rep(NA, n)
  merged <- as.list(seq(0, n-1))
  for (i in 1:(n-1)) {
    k1 <- merges[i, 1] + 1
    k2 <- merges[i, 2] + 1
    v1 <- merged[[k1]]
    v2 <- merged[[k2]]
    if (length(v1) + length(v2) <= maxsize) {
      merged[[n+i]] <- c(v1, v2)
      merged[[k1]] <- NA
      merged[[k2]] <- NA
    } else if (length(v1) < length(v2)) {
      merged[[n+i]] <- v1
      merged[[k1]] <- NA
    } else {
      merged[[n+i]] <- v2
      merged[[k2]] <- NA   
    }
  }
  k <- 0
  for (i in 1:length(merged)) {
    v <- merged[[i]]
    if (length(v) > 1 || !is.na(v)) {
      membership[v+1] <- k
      k <- k + 1
    } 
  }
  na.fail(membership)
  membership
}

my.graph.contract <- function(graph, membership_vector) {
  a <- get.adjacency(graph,sparse=FALSE)
  m <- table(membership_vector, 0:(length(membership_vector)-1))
  b <- m %*% a %*% t(m)
  graph.adjacency(b, weighted=TRUE, mode="undirected")
}

my.graph.plot <- function(filename, ext, g, colors="Cyan", widths=1, layout=layout.graphopt) {
  sink("/dev/null")
  if (ext == "eps") {
    postscript(file=paste0(filename, ".eps"), horizontal=FALSE, onefile=FALSE, paper="special", height=8, width=12)
  } else {
    png(paste0(filename, ".png"), width=480, height=480)
    par(mai=c(0,0,0,0)) # no margin
  }
  V(g)$size <- 5
  V(g)$label <- NA
  V(g)$color <- colors
  E(g)$width <- widths
  E(g)$color <- "black"
  plot(g, layout=layout)
  dev.off()
  sink()
}

my.dendrogram.plot <- function(filename, ext, g) {
  sink("/dev/null")
  if (ext == "eps") {
    postscript(file=paste0(filename, ".eps"), horizontal=FALSE, onefile=FALSE, paper="special", height=8, width=12)
  } else {
    png(paste0(filename, ".png"), width=960, height=480)
    par(mai=c(0,0,0,0)) # no margin
  }
  plot(g)
  dev.off()
  sink()
}

basename <- paste0(topology, ".", clmethod, "-", maxsize)
start_time <- Sys.time()

logf <- file(paste0(basename, ".log"), "w")
writeLines(paste0("Started ", toString(start_time)), logf)
writeLines(topology, logf)
writeLines(clmethod, logf)
writeLines(toString(maxsize), logf)
writeLines("", logf)

# g0 <- read.graph(paste0(topology, ".edges"), format="edgelist", directed=FALSE)
# g0 <- read.graph(paste0(topology, ".edges"), format="ncol", directed=FALSE, weights=FALSE)
nameg0 <- paste0(topology, ".edges")
writeLines(paste0("Reading edges from ",nameg0), logf)
g0 <- read.graph(paste0(topology, ".edges"), format="ncol", directed=FALSE, weights='no')
n0 <- length(V(g0))
if (clmethod == "naive" || clmethod == "seq" || clmethod == "sequential") {
  nc <- ceiling(n0 / maxsize)
  mem <- floor(seq(0, nc, length.out=n0+1))[1:n0]
} else if (clmethod == "ward" || clmethod == "hc-ward") {
  clu <- hclust(as.dist(shortest.paths(g0)), "ward")
  mem <- my.dendrogram.to.membership.upto(clu$merge, maxsize)
} else if (clmethod == "single" || clmethod == "hc-single") {
  clu <- hclust(as.dist(shortest.paths(g0)), "single")
  mem <- my.dendrogram.to.membership.upto(clu$merge, maxsize)
} else if (clmethod == "complete" || clmethod == "hc-complete") {
  clu <- hclust(as.dist(shortest.paths(g0)), "complete")
  mem <- my.dendrogram.to.membership.upto(clu$merge, maxsize)
} else if (clmethod == "average" || clmethod == "hc-average") {
  clu <- hclust(as.dist(shortest.paths(g0)), "average")
  mem <- my.dendrogram.to.membership.upto(clu$merge, maxsize)  
} else if (clmethod == "mcquitty" || clmethod == "hc-mcquitty") {
  clu <- hclust(as.dist(shortest.paths(g0)), "mcquitty")
  mem <- my.dendrogram.to.membership.upto(clu$merge, maxsize)
} else if (clmethod == "median" || clmethod == "hc-median") {
  clu <- hclust(as.dist(shortest.paths(g0)), "median")
  mem <- my.dendrogram.to.membership.upto(clu$merge, maxsize)
} else if (clmethod == "centroid" || clmethod == "hc-centroid") {
  clu <- hclust(as.dist(shortest.paths(g0)), "centroid")
  mem <- my.dendrogram.to.membership.upto(clu$merge, maxsize)
} else if (clmethod == "girvan" || clmethod == "betweenness") {
  cmn <- edge.betweenness.community(g0)
  clu <- as.dendrogram(cmn)
  mem <- my.community.to.membership.upto(cmn$merges, maxsize)
} else if (clmethod == "newman" || clmethod == "fastgreedy") {
  cmn <- fastgreedy.community(g0)
  mem <- my.community.to.membership.upto(cmn$merges, maxsize)
} else if (clmethod == "eigenvector") {
  cmn <- leading.eigenvector.community(g0, options=list(maxiter=1000000))
  mem <- cmn$membership
} else if (clmethod == "spinglass") {
  cmn <- spinglass.community(g0)
  mem <- cmn$membership
} else if (clmethod == "walk" || clmethod == "walktrap") {
  cmn <- walktrap.community(g0)
  mem <- my.community.to.membership.upto(cmn$merges, maxsize)
} else {
  writeLines(paste0("Error: no such clustering method"), logf)
}
g1 <- simplify(my.graph.contract(g0, mem))
# save(g0, g1, mem, file=paste0(basename, ".Rdata"))


if (figure_type != "") {
	my.graph.plot(paste0(basename, ".node"), figure_type, g0, rainbow(max(mem)+1)[mem+1], layout=layout.circle)
	my.graph.plot(paste0(basename, ".rack"), figure_type, g1, rainbow(length(V(g1))), E(g1)$weight, layout=layout.circle)
	if (exists("clu")) {
	  my.dendrogram.plot(paste0(basename, ".dend"), figure_type, clu)
	}
}
write.graph(g1, paste0(basename, ".edges"), format="ncol")


n_node <- length(V(g0))
n_edge <- length(E(g0))
write("HER21","")
degree <- max(degree(g0))
diameter <- diameter(g0)
avg_dist <- average.path.length(g0)
n_rack <- length(V(g1))
n_tube <- length(E(g1))
n_link <- sum(E(g1)$weight)
write("HER24","")
#modularity <- modularity(g0, mem)
write("HER25","")
finish_time <- Sys.time()
elapsed <- difftime(finish_time, start_time)

writeLines(paste(
  topology,
  clmethod,
  toString(maxsize),
  n_node,
  n_edge,
  degree,
  diameter,
  avg_dist,
  n_rack,
  n_tube,
  n_link,
 # modularity,
  "", # paste(mem, collapse=","),
  format(elapsed),
  toString(finish_time),
  sep="\t"))

writeLines(paste0("#node      = ", n_node), logf)
writeLines(paste0("#edge      = ", n_edge), logf)
writeLines(paste0("degree     = ", degree), logf)
writeLines(paste0("diameter   = ", diameter), logf)
writeLines(paste0("avg_dist   = ", avg_dist), logf)
writeLines(paste0("#rack      = ", n_rack), logf)
writeLines(paste0("#tube      = ", n_tube), logf)
writeLines(paste0("#link      = ", n_link), logf)
#writeLines(paste0("modularity = ", modularity), logf)
writeLines(paste0("membership = ", paste(mem, collapse=",")), logf)
writeLines(paste0("Finished ", toString(finish_time)), logf)
close(logf)
