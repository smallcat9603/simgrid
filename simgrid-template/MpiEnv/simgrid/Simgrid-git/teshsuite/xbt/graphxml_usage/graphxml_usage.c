/* A few basic tests for the graphxml library                               */

/* Copyright (c) 2006-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdio.h>

#ifdef _MSC_VER
#define snprintf  _snprintf
#endif

#include "xbt/module.h"
#include "xbt/sysdep.h"
#include "xbt/graph.h"
#include "xbt/graphxml.h"
#include "xbt/log.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(test, "Logging specific to graphxml test");


static void *node_label_and_data(xbt_node_t node, const char *label,
                                 const char *data)
{
  char *lbl = xbt_strdup(label);
  return lbl;
}

#define free_label free

static const char *node_name(xbt_node_t n)
{
  return xbt_graph_node_get_data(n);
}

void test(char *graph_file);
void test(char *graph_file)
{
  int test_node_deletion = 0;
  int test_edge_deletion = 0;
  int test_export_xml = 1;
  int test_export_dot = 1;
  int test_export_length = 1;
  int test_shortest_paths = 1;
  int test_topo_sort = 1;

  unsigned long i, j;
  unsigned long n;

  xbt_dynar_t edges = NULL;
  xbt_dynar_t nodes = NULL;

  xbt_graph_t graph =
      xbt_graph_read(graph_file, &node_label_and_data, NULL);

  n = xbt_dynar_length(xbt_graph_get_nodes(graph));

  if (test_export_xml) {
    XBT_INFO("---- Testing XML export. Exporting to testgraph.xml ----");
    xbt_graph_export_graphxml(graph, "testgraph.xml", NULL, NULL, NULL,
                              NULL);
  }
  if (test_export_dot) {
    XBT_INFO("---- Testing GraphViz export. Exporting to testgraph.dot ----");
    xbt_graph_export_graphviz(graph, "testgraph.dot", node_name, NULL);
  }

  if (test_export_length) {
    char *buf = NULL;
    double *adj = NULL;

    XBT_INFO("---- Dumping Edge lengths ----");
    adj = xbt_graph_get_length_matrix(graph);
    buf = xbt_new0(char, n * 20);
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
        sprintf(buf + strlen(buf), "%6.3f\t", adj[i * n + j]);
      }
      XBT_INFO("%s", buf);
      buf[0] = '\000';
    }
    free(buf);
    free(adj);
  }

  if (test_shortest_paths) {
    char *buf = NULL;
    xbt_node_t *route = NULL;

    XBT_INFO("---- Testing Shortest Paths ----");
    route = xbt_graph_shortest_paths(graph);
    buf = xbt_new0(char, n * 40);
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
        if (route[i * n + j])
          snprintf(buf + strlen(buf), 40, "%s\t",
                   node_name(route[i * n + j]));
      }
      XBT_INFO("%s", buf);
      buf[0] = '\000';
    }
    free(buf);
    free(route);
  }

  if (test_topo_sort) {
    xbt_node_t *sorted = NULL;

    XBT_INFO("---- Testing Topological Sort ----");
    sorted = xbt_graph_topo_sort(graph);
    for (i = 0; i < n; i++) {
      if (sorted[i]) {
        XBT_INFO("sorted[%lu] = %s", i, node_name(sorted[i]));
      }
    }
    free(sorted);
  }


  if (test_node_deletion) {
    XBT_INFO("---- Testing Node Deletion ----");
    nodes = xbt_graph_get_nodes(graph);
    edges = xbt_graph_get_edges(graph);
    XBT_INFO("Before Node deletion: %lu nodes, %lu edges",
          xbt_dynar_length(nodes), xbt_dynar_length(edges));

    while (!xbt_dynar_is_empty(nodes))
      xbt_graph_free_node(graph,
                          *((xbt_node_t *) xbt_dynar_get_ptr(nodes, 0)),
                          free_label, NULL);
    XBT_INFO("After Node deletion:  %lu nodes, %lu edges",
          xbt_dynar_length(nodes), xbt_dynar_length(edges));
  }

  if (test_edge_deletion) {
    XBT_INFO("---- Testing Edge Deletion ----");
    nodes = xbt_graph_get_nodes(graph);
    edges = xbt_graph_get_edges(graph);
    XBT_INFO("Before Edge deletion: %lu nodes, %lu edges",
          xbt_dynar_length(nodes), xbt_dynar_length(edges));

    while (!xbt_dynar_is_empty(edges))
      xbt_graph_free_edge(graph,
                          *((xbt_edge_t *) xbt_dynar_get_ptr(edges, 0)),
                          NULL);

    XBT_INFO("After Edge deletion:  %lu nodes, %lu edges",
          xbt_dynar_length(nodes), xbt_dynar_length(edges));
  }

  xbt_graph_free_graph(graph, free_label, NULL, NULL);

}

#ifdef __BORLANDC__
#pragma argsused
#endif

int main(int argc, char **argv)
{
  xbt_init(&argc, argv);
  if (argc == 1) {
    fprintf(stderr, "Usage : %s graph.xml\n", argv[0]);

    return 1;
  }
  test(argv[1]);
  return 0;
}
