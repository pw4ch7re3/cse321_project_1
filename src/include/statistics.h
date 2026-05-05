#ifndef STATISTICS_H
#define STATISTICS_H

#include <cstddef>
#include "node.h"
#include "list.h"

struct Statistics
{
  size_t n_nodes;     // Number of existing nodes
  size_t n_keys;      // Number of stored keys
  size_t height;      // Tree height
  size_t n_bytes;     // Memory space in bytes
  double utilization; // # of keys / # of nodes
};

void collect_node_stats (Node *, size_t,
                         size_t &, size_t &,
                         size_t &);
void collect_list_stats (List *, size_t,
                         size_t &, size_t &,
                         size_t &, size_t &);

#endif // STATISTICS_H