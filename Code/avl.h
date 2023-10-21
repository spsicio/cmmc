#ifndef CMMC_AVL_H
#define CMMC_AVL_H

#include "lexer.h"

typedef struct AVLNode {
  struct AVLNode *fa;
  struct AVLNode *lc;
  struct AVLNode *rc;
  char name[MAX_TOKEN_LEN];
  void *ent;
  int bf;
} AVLNode;

AVLNode* avl_insert(AVLNode*, const char*, void*);
AVLNode* avl_find(AVLNode*, const char*);
void free_avl(AVLNode*);
void print_avl(AVLNode*, int);

#endif  // CMMC_AVL_H

