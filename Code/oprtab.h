#ifndef CMMC_OPRTAB_H
#define CMMC_OPRTAB_H

typedef struct Oprent {
  int off;
  int siz;
} Oprent;

Oprent* oprtab_access(const char *name);
void fill_off(Irnode *p);

#endif  // CMMC_OPRTAB_H

