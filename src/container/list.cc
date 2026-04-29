#include "../include/list.h"

List *
allocate_list (int d, List *next)
{
  List *x = new List;
  x->p = new List *[2 * d + 1];
  x->k = new int[2 * d];
  x->v = new Record *[2 * d];
  x->next = next;
  return x;
}