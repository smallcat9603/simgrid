#!/usr/bin/env Rscript
args = commandArgs(trailingOnly=TRUE)
library(ggplot2)

# Read the data
df_state = read.csv("lu.S.4.state.csv", header=F, strip.white=T)
names(df_state) = c("Type", "Rank", "Container", "Start", "End", "Duration", "Level", "State");
df_state = df_state[!(names(df_state) %in% c("Type","Container","Level"))]
df_state$Rank = as.numeric(gsub("rank-","",df_state$Rank))

# Draw the Gantt Chart
gc = ggplot(data=df_state) + geom_rect(aes(xmin=Start, xmax=End, ymin=Rank, ymax=Rank+1,fill=State))

# Produce the output
plot(gc)
dev.off()
