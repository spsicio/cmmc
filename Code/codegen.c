#include <stdio.h>
#include "ir.h"
#include "oprtab.h"
#include "reg.h"

static FILE *fout;

#define myprint(log_fmt, ...) \
  do { \
    fprintf(fout, log_fmt, ##__VA_ARGS__); \
  } while (0)

static int arg_siz = 0;

static void sht_sp(int off) {
  while (off != 0) {
    int d = off;
    if (off >  32767) d =  32767;
    if (off < -32768) d = -32768;
    myprint("  addi $sp, $sp, %d\n", d);
    off -= d;
  }
}

static void load_opr(Opr opr, REG reg) {
  if (opr.kind == OPR_LIT) {
    myprint("  li $%d, %d\n", reg, opr.litval);
  } else {
    symstr name;
    if (opr.kind == OPR_VAR) sprintf(name, "v%d", opr.varno);
    else sprintf(name, "t%d", opr.tmpno);
    Oprent *ent = oprtab_access(name);

    if (opr.ref && opr.deref) opr.ref = opr.deref = false;
    if (opr.ref) {
      myprint("  la $%d, %d($fp)\n", reg, ent->off);
    } else if (opr.deref) {
      myprint("  lw $t9, %d($fp)\n", ent->off);
      myprint("  lw $%d, 0($t9)\n", reg);
    } else {
      myprint("  lw $%d, %d($fp)\n", reg, ent->off);
    }
  }
}

static void save_opr(Opr opr, REG reg) {
  symstr name;
  if (opr.kind == OPR_VAR) sprintf(name, "v%d", opr.varno);
  else sprintf(name, "t%d", opr.tmpno);
  Oprent *ent = oprtab_access(name);

  if (opr.ref && opr.deref) opr.ref = opr.deref = false;
  if (opr.deref) {
    myprint("  lw $t9, %d($fp)\n", ent->off);
    myprint("  sw $%d, 0($t9)\n", reg);
  } else {
    myprint("  sw $%d, %d($fp)\n", reg, ent->off);
  }
}

IR(IR_VISIT_FUNDEC)
VISITOR_DEF(IR, visitor)

void gen_code(Irnode *p, FILE *file) {
  fout = file;
  myprint(".data\n"
          "_prompt: .asciiz \"Enter an integer:\"\n"
          "_ret: .asciiz \"\\n\"\n"
          ".globl main\n"
          ".text\n"
          "read:\n"
          "  li $v0, 4\n"
          "  la $a0, _prompt\n"
          "  syscall\n"
          "  li $v0, 5\n"
          "  syscall\n"
          "  jr $ra\n\n"
          "write:\n"
          "  li $v0, 1\n"
          "  syscall\n"
          "  li $v0, 4\n"
          "  la $a0, _ret\n"
          "  syscall\n"
          "  move $v0, $0\n"
          "  jr $ra\n");
  while (p != NULL) {
    IrVisitorDispatch(&visitor, p, NULL);
    p = p->nxt;
  }
}

IR_MAKE_VISIT(IR_ASN) {
  load_opr(p->src1, REG_T0);
  save_opr(p->dst, REG_T0);
}

IR_MAKE_VISIT(IR_ARI) {
  load_opr(p->src1, REG_T0);
  load_opr(p->src2, REG_T1);
  switch (p->op) {
    case kPLUS:  myprint("  add $t0, $t0, $t1\n"); break;
    case kMINUS: myprint("  sub $t0, $t0, $t1\n"); break;
    case kSTAR:  myprint("  mul $t0, $t0, $t1\n"); break;
    case kDIV:   myprint("  div $t0, $t1\n"
                         "  mflo $t0\n");          break; 
  }
  save_opr(p->dst, REG_T0);
}

IR_MAKE_VISIT(IR_DEC) {}

IR_MAKE_VISIT(IR_LBL) {
  myprint("label%d:\n", p->dst.lblno);
}

IR_MAKE_VISIT(IR_JMP) {
  myprint("  j label%d\n", p->dst.lblno);
}

IR_MAKE_VISIT(IR_JCN) {
  load_opr(p->src1, REG_T0);
  load_opr(p->src2, REG_T1);
  switch (p->op) {
    case kEQ : myprint("  beq $t0, $t1, label%d\n", p->dst.lblno); break;
    case kNEQ: myprint("  bne $t0, $t1, label%d\n", p->dst.lblno); break;
    case kLT:  myprint("  blt $t0, $t1, label%d\n", p->dst.lblno); break;
    case kLE:  myprint("  ble $t0, $t1, label%d\n", p->dst.lblno); break;
    case kGT:  myprint("  bgt $t0, $t1, label%d\n", p->dst.lblno); break;
    case kGE:  myprint("  bge $t0, $t1, label%d\n", p->dst.lblno); break;
  }
}

IR_MAKE_VISIT(IR_FUN) {
  Oprent *ent = oprtab_access(p->fun_name);
  myprint("\n%s:\n", p->fun_name);
  myprint("  addi $sp, $sp, -4\n"
          "  sw $fp, 0($sp)\n"
          "  move $fp, $sp\n");
  sht_sp(ent->off + 4);
}

IR_MAKE_VISIT(IR_PAR) {}

IR_MAKE_VISIT(IR_RET) {
  load_opr(p->dst, REG_V0);
  myprint("  move $sp, $fp\n"
          "  lw $fp, 0($sp)\n"
          "  addi $sp, $sp, 4\n"
          "  jr $ra\n");
}

IR_MAKE_VISIT(IR_CAL) {
  myprint("  addi $sp, $sp, -4\n"
          "  sw $ra, 0($sp)\n");
  myprint("  jal %s\n", p->fun_name);
  myprint("  lw $ra, 0($sp)\n"
          "  addi $sp, $sp, 4\n");
  save_opr(p->dst, REG_V0);
  sht_sp(arg_siz);
  arg_siz = 0;
}

IR_MAKE_VISIT(IR_ARG) {
  arg_siz += 4;
  load_opr(p->dst, REG_T0);
  myprint("  addi $sp, $sp, -4\n"
          "  sw $t0, 0($sp)\n");
}

IR_MAKE_VISIT(IR_RED) {
  myprint("  addi $sp, $sp, -4\n"
          "  sw $ra, 0($sp)\n"
          "  jal read\n"
          "  lw $ra, 0($sp)\n"
          "  addi $sp, $sp, 4\n");
  save_opr(p->dst, REG_V0);
}

IR_MAKE_VISIT(IR_RIT) {
  load_opr(p->dst, REG_T0);
  myprint("  move $a0, $t0\n"
          "  addi $sp, $sp, -4\n"
          "  sw $ra, 0($sp)\n"
          "  jal write\n"
          "  lw $ra, 0($sp)\n"
          "  addi $sp, $sp, 4\n");
}

