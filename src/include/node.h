#ifndef NODE_H
#define NODE_H

#include "loader.h"
#include <cstddef>

struct Node
{
  bool leaf;  // Flag indicates if this is a leaf
  size_t n;   // Item counter
  Node **p;   // Points to children
  int *k;     // Keys
  Record **v;  // Values

  Node ()
  : leaf (true), n (0),
    p (nullptr), k (nullptr), v (nullptr) {}
  ~Node ()
  {
    if (p) delete[] p;
    if (k) delete[] k;
    if (v) delete[] v;
  }
};

Node *allocate_node (int d);

#endif // NODE_H