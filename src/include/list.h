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

  /* Leaf node */
  
  int *rid;     // Rids
  List *next;   // Next node

  List ()
  : leaf (true), n (0),
    p (nullptr), k (nullptr), rid (nullptr),
    next (nullptr) {}
  ~List ()
  {
    if (p) delete[] p;
    if (k) delete[] k;
    if (rid) delete[] rid;
  }
};

List *allocate_list (int d, bool leaf=true, List *next=nullptr);

#endif // LIST_H