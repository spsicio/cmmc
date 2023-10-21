#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "avl.h"

#define linklc(f, s) do { \
    (f)->lc = s; \
    if ((s) != NULL) (s)->fa = f; \
  } while (0)

#define linkrc(f, s) do { \
    (f)->rc = s; \
    if ((s) != NULL) (s)->fa = f; \
  } while (0)

static AVLNode* avl_init(const char *name, void *ent) {
  AVLNode *p = (AVLNode*) malloc(sizeof(AVLNode));
  p->fa = NULL;
  p->lc = NULL;
  p->rc = NULL;
  strcpy(p->name, name);
  p->ent = ent;
  p->bf = 0;
  return p;
}

static AVLNode* rotateR(AVLNode *p, AVLNode *q) {
  AVLNode *r = q->rc;
  linklc(p, r);
  linkrc(q, p);
  p->bf = 0;
  q->bf = 0;
  return q;
}

static AVLNode* rotateL(AVLNode *p, AVLNode *q) {
  AVLNode *r = q->lc;
  linkrc(p, r);
  linklc(q, p);
  p->bf = 0;
  q->bf = 0;
  return q;
}

static AVLNode* rotateLR(AVLNode *p, AVLNode *q) {
  AVLNode *r = q->rc;
  AVLNode *t2 = r->lc;
  AVLNode *t3 = r->rc;
  linklc(p, t3);
  linkrc(q, t2);
  linklc(r, q);
  linkrc(r, p);
  if (r->bf < 0) {
    q->bf = 0;
    p->bf = 1;
  } else {
    q->bf = -1;
    p->bf = 0;
  }
  r->bf = 0;
  return r;
}

static AVLNode* rotateRL(AVLNode *p, AVLNode *q) {
  AVLNode *r = q->lc;
  AVLNode *t2 = r->lc;
  AVLNode *t3 = r->rc;
  linkrc(p, t2);
  linklc(q, t3);
  linklc(r, p);
  linkrc(r, q);
  if (r->bf < 0) {
    p->bf = 0;
    q->bf = 1;
  } else {
    p->bf = -1;
    q->bf = 0;
  }
  r->bf = 0;
  return r;
}

AVLNode* avl_insert(AVLNode *root, const char *name, void *ent) {
  AVLNode *p = root;
  AVLNode *q = avl_init(name, ent);
  if (p == NULL) return root = q;
  for (;;) {
    if (strcmp(name, p->name) < 0) {
      if (p->lc == NULL) { linklc(p, q); break; }
      p = p->lc;
    } else {
      if (p->rc == NULL) { linkrc(p, q); break; }
      p = p->rc;
    }
  }
  for (; p != NULL; p = p->fa, q = q->fa) {
    AVLNode *prev_fa = NULL;
    AVLNode *subroot = NULL;
    if (p->lc == q) {
      if (p->bf < 0) {
        prev_fa = p->fa;
        subroot = q->bf <= 0 ? rotateR(p, q) : rotateLR(p, q);
      } else if (p->bf == 0) {
        p->bf = -1; continue;
      } else {
        p->bf = 0; break;
      }
    } else {
      if (p->bf < 0) {
        p->bf = 0; break;
      } else if (p->bf == 0) {
        p->bf = 1; continue;
      } else {
        prev_fa = p->fa;
        subroot = q->bf >= 0 ? rotateL(p, q) : rotateRL(p, q);
      }
    }
    if (prev_fa == NULL) {
      root = subroot;
      root->fa = NULL;
    } else {
      if (prev_fa->lc == p) linklc(prev_fa, subroot);
      else linkrc(prev_fa, subroot);
    }
    break;
  }
  return root;
}

AVLNode* avl_find(AVLNode *p, const char *name) {
  while (p != NULL) {
    int res = strcmp(name, p->name);
    if (res < 0) p = p->lc;
    else if (res == 0) return p;
    else p = p->rc;
  }
  return NULL;
}

void free_avl(AVLNode *p) {
  if (p == NULL) return;
  free_avl(p->lc);
  free_avl(p->rc);
  free(p->ent);
  free(p);
}

void print_avl(AVLNode *p, int indent) {
  for (int i=0; i<indent; ++i) putchar(' ');
  if (p == NULL) {
    puts("()");
  } else {
    printf("%s: %d\n", p->name, p->bf);
    print_avl(p->lc, indent+2);
    print_avl(p->rc, indent+2);
  }
}

