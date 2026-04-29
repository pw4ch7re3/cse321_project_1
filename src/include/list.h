#ifndef LIST_H
#define LIST_H

#include "loader.h"
#include <cstddef>

struct List
{
  bool leaf;    // Flag indicates if this is a leaf
  size_t n;     // Item counter
  List **p;     // Points to children
  int *k;       // Keys
  Record **v;   // Values
  List *next;   // Next node

  List ()
  : leaf (true), n (0),
    p (nullptr), k (nullptr), v (nullptr),
    next (nullptr) {}
  ~List ()
  {
    if (p) delete[] p;
    if (k) delete[] k;
    if (v) delete[] v;
  }
};

List *allocate_list (List *next, int d, bool leaf=true);

#endif // LIST_H