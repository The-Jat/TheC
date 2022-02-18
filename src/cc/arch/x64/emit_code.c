#include "emit_code.h"

#include <assert.h>
#include <inttypes.h>  // PRIdPTR
#include <limits.h>  // CHAR_BIT
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "codegen.h"
#include "ir.h"
#include "lexer.h"
#include "regalloc.h"
#include "table.h"
#include "type.h"
#include "util.h"
#include "var.h"
#include "x64.h"

static void eval_initial_value_member(Expr *expr, Expr **pvar, Fixnum *poffset) {
  switch (expr->kind) {
  case EX_VAR:
    assert(*pvar == NULL);
    *pvar = expr;
    break;
  case EX_MEMBER:
    {
      eval_initial_value_member(expr->member.target, pvar, poffset);

      const Type *type = expr->member.target->type;
      if (ptr_or_array(type))
        type = type->pa.ptrof;
      assert(type->kind == TY_STRUCT);
      const Vector *members = type->struct_.info->members;
      const MemberInfo *member = members->data[expr->member.index];
      *poffset += member->offset;
    }
    break;
  default: assert(!"illegal"); break;
  }
}

static void eval_initial_value(Expr *expr, Expr **pvar, Fixnum *poffset) {
  switch (expr->kind) {
  case EX_CAST:
    eval_initial_value(expr->unary.sub, pvar, poffset);
    return;
  case EX_REF:
    {
      Expr *sub = expr->unary.sub;
      switch (sub->kind) {
      case EX_DEREF:
        eval_initial_value(sub->unary.sub, pvar, poffset);
        break;
      case EX_VAR:
        eval_initial_value(sub, pvar, poffset);
        break;
      case EX_MEMBER:
        eval_initial_value_member(sub, pvar, poffset);
        break;
      default:
        assert(false);
        break;
      }
    }
    return;
  case EX_VAR:
    assert(*pvar == NULL);
    *pvar = expr;
    break;
  case EX_FIXNUM:
    if (!is_const(expr))
      assert(!"initializer type error");
    *poffset = expr->fixnum;
    return;
  case EX_STR:
    assert(!"should be handled in parser");
    break;
  case EX_ADD:
  case EX_SUB:
    {
      Expr *var1 = NULL, *var2 = NULL;
      Fixnum offset1 = 0, offset2 = 0;
      eval_initial_value(expr->bop.lhs, &var1, &offset1);
      eval_initial_value(expr->bop.rhs, &var2, &offset2);
      if (var1 != NULL) {
        assert(var2 == NULL);
        *pvar = var1;
      } else if (var2 != NULL) {
        assert(expr->kind == EX_ADD);
        *pvar = var2;
      }
      if (expr->kind == EX_SUB)
        offset2 = -offset2;
      *poffset = offset1 + offset2;
    }
    break;
  default: assert(!"illegal"); break;
  }
}

static void construct_initial_value(const Type *type, const Initializer *init) {
  assert(init == NULL || init->kind != IK_DOT);

  switch (type->kind) {
#ifndef __NO_FLONUM
  case TY_FLONUM:
    switch (type->flonum.kind) {
    case FL_DOUBLE:
      {
        union {double f; uint64_t h;} v;
        v.f = 0;
        if (init != NULL) {
          assert(init->kind == IK_SINGLE);
          Expr *value = init->single;
          if (!(is_const(value) && is_flonum(value->type)))
            error("Illegal initializer: constant number expected");
          v.f = value->flonum;
        }
#if 0
        _DOUBLE(FLONUM(v.d));
#else
        _QUAD(HEXNUM(v.h));
#endif
      }
      break;
    case FL_FLOAT:
      {
        union {float f; uint32_t h;} v;
        v.f = 0;
        if (init != NULL) {
          assert(init->kind == IK_SINGLE);
          Expr *value = init->single;
          if (!(is_const(value) && is_flonum(value->type)))
            error("Illegal initializer: constant number expected");
          v.f = value->flonum;
        }
#if 0
        _FLOAT(FLONUM(v.f));
#else
        _LONG(HEXNUM(v.h));
#endif
      }
      break;
    }
    break;
#endif
  case TY_FIXNUM:
  case TY_PTR:
    {
      Expr *var = NULL;
      Fixnum offset = 0;
      if (init != NULL) {
        assert(init->kind == IK_SINGLE);
        eval_initial_value(init->single, &var, &offset);
      }
      const char *output;
      if (var == NULL) {
        output = NUM(offset);
      } else {
        const Name *name = var->var.name;
        Scope *scope;
        VarInfo *varinfo = scope_find(var->var.scope, name, &scope);
        assert(varinfo != NULL);
        if (!is_global_scope(scope) && varinfo->storage & VS_STATIC) {
          varinfo = varinfo->static_.gvar;
          assert(varinfo != NULL);
          name = varinfo->name;
        }

        const char *label = fmt_name(name);
        if ((varinfo->storage & VS_STATIC) == 0)
          label = MANGLE(label);

        if (offset == 0) {
          output = label;
        } else {
          output = fmt("%s + %" PRIdPTR, label, offset);
        }
      }
      if (type->kind == TY_PTR) {
        _QUAD(output);
      } else {
        switch (type->fixnum.kind) {
        case FX_CHAR:  _BYTE(output); break;
        case FX_SHORT: _WORD(output); break;
        case FX_LONG:  _QUAD(output); break;
        case FX_LLONG: _QUAD(output); break;
        default:
          assert(false);
          // Fallthrough
        case FX_INT: case FX_ENUM:
          _LONG(output);
          break;
        }
      }
    }
    break;
  case TY_ARRAY:
    if (init == NULL || init->kind == IK_MULTI) {
      const Type *elem_type = type->pa.ptrof;
      ssize_t index = 0;
      if (init != NULL) {
        Vector *init_array = init->multi;
        for (ssize_t i = 0; i < init_array->len; ++i, ++index) {
          const Initializer *init_elem = init_array->data[i];
          if (init_elem->kind == IK_ARR) {
            ssize_t next = init_elem->arr.index->fixnum;
            for (ssize_t j = index; j < next; ++j)
              construct_initial_value(elem_type, NULL);
            index = next;
            init_elem = init_elem->arr.value;
          }
          construct_initial_value(elem_type, init_elem);
        }
      }
      // Padding
      for (ssize_t i = index, n = type->pa.length; i < n; ++i)
        construct_initial_value(elem_type, NULL);
      break;
    }
    if (init->kind == IK_SINGLE && is_char_type(type->pa.ptrof)) {
      Expr *e = strip_cast(init->single);
      if (e->kind == EX_STR) {
        size_t src_size = e->str.size;
        size_t size = type_size(type);
        if (src_size > size)
          src_size = size;

        UNUSED(size);
        StringBuffer sb;
        sb_init(&sb);
        sb_append(&sb, "\"", NULL);
        escape_string(e->str.buf, src_size, &sb);
        if (size > src_size) {
          const char NULCHR[] = "\\0";
          for (size_t i = 0, n = size - src_size; i < n; ++i)
            sb_append(&sb, NULCHR, NULL);
        }
        sb_append(&sb, "\"", NULL);
        _ASCII(sb_to_string(&sb));
        break;
      }
    }
    error("Illegal initializer");
    break;
  case TY_STRUCT:
    {
      assert(init == NULL || init->kind == IK_MULTI);

      const StructInfo *sinfo = type->struct_.info;
      int count = 0;
      int offset = 0;
      for (int i = 0, n = sinfo->members->len; i < n; ++i) {
        const MemberInfo *member = sinfo->members->data[i];
        const Initializer *mem_init;
        if (init == NULL) {
          if (sinfo->is_union)
            continue;
          mem_init = NULL;
        } else {
          mem_init = init->multi->data[i];
        }
        if (mem_init != NULL || !sinfo->is_union) {
          int align = align_size(member->type);
          if (offset % align != 0) {
            EMIT_ALIGN(align);
            offset = ALIGN(offset, align);
          }
          construct_initial_value(member->type, mem_init);
          ++count;
          offset = ALIGN(offset, align);
          offset += type_size(member->type);
        }
      }
      if (sinfo->is_union && count <= 0) {
        const MemberInfo *member = sinfo->members->data[0];
        construct_initial_value(member->type, NULL);
        offset += type_size(member->type);
      }

      size_t size = type_size(type);
      if (size != (size_t)offset) {
        // Put padding.
        int d = size - offset;
        switch (d) {
        case 1:  _BYTE(NUM(0)); break;
        case 2:  _WORD(NUM(0)); break;
        case 4:  _LONG(NUM(0)); break;
        case 8:  _QUAD(NUM(0)); break;
        default:
          for (int i = 0; i < d; ++i)
            _BYTE(NUM(0));
          break;
        }
      }
    }
    break;
  default:
    fprintf(stderr, "Global initial value for type %d not implemented (yet)\n", type->kind);
    assert(false);
    break;
  }
}

static void emit_varinfo(const VarInfo *varinfo, const Initializer *init) {
  const Name *name = varinfo->name;
  if (init != NULL) {
    if (varinfo->type->qualifier & TQ_CONST)
      _RODATA();
    else
      _DATA();
  }

  const char *label = fmt_name(name);
  if ((varinfo->storage & VS_STATIC) == 0) {  // global
    label = MANGLE(label);
    _GLOBL(label);
  }

  if (init != NULL) {
    EMIT_ALIGN(align_size(varinfo->type));
    EMIT_LABEL(label);
    //size_t size = type_size(varinfo->type);
    construct_initial_value(varinfo->type, init);
  } else {
    size_t size = type_size(varinfo->type);
    if (size < 1)
      size = 1;

    size_t align = align_size(varinfo->type);
    if (align <= 1)
      _COMM(label, NUM(size));
    else
      _COMM(label, fmt("%" PRIdPTR ",%" PRIdPTR, size, align));
  }
}

////////////////////////////////////////////////

static bool is_asm(Stmt *stmt) {
  return stmt->kind == ST_ASM;
}

static VarInfo *find_ret_var(Scope *scope) {
  const Name *retval_name = alloc_name(RET_VAR_NAME, NULL, false);
  return scope_find(scope, retval_name, NULL);
}

static void put_args_to_stack(Function *func) {
  static const char *kReg8s[] = {DIL, SIL, DL, CL, R8B, R9B};
  static const char *kReg16s[] = {DI, SI, DX, CX, R8W, R9W};
  static const char *kReg32s[] = {EDI, ESI, EDX, ECX, R8D, R9D};
  static const char *kReg64s[] = {RDI, RSI, RDX, RCX, R8, R9};
  static const char **kRegTable[] = {NULL, kReg8s, kReg16s, NULL, kReg32s, NULL, NULL, NULL, kReg64s};
#ifndef __NO_FLONUM
  static const char *kFReg64s[] = {XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7};
#endif

  int arg_index = 0;
  if (is_stack_param(func->type->func.ret)) {
    Scope *top_scope = func->scopes->data[0];
    VarInfo *varinfo = find_ret_var(top_scope);
    assert(varinfo != NULL);
    const Type *type = varinfo->type;
    int size = type_size(type);
    int offset = varinfo->local.reg->offset;
    assert(size < (int)(sizeof(kRegTable) / sizeof(*kRegTable)) &&
           kRegTable[size] != NULL);
    MOV(kRegTable[size][0], OFFSET_INDIRECT(offset, RBP, NULL, 1));
    ++arg_index;
  }

  // Store arguments into local frame.
  const Vector *params = func->type->func.params;
  if (params == NULL)
    return;

  int len = params->len;
  if (!func->type->func.vaargs) {
#ifndef __NO_FLONUM
    int farg_index = 0;
#endif
    for (int i = 0; i < len; ++i) {
      const VarInfo *varinfo = params->data[i];
      const Type *type = varinfo->type;
      int offset = varinfo->local.reg->offset;

      if (is_stack_param(type))
        continue;

#ifndef __NO_FLONUM
      if (is_flonum(type)) {
        if (farg_index < MAX_FREG_ARGS) {
          switch (type->flonum.kind) {
          case FL_FLOAT:   MOVSS(kFReg64s[farg_index], OFFSET_INDIRECT(offset, RBP, NULL, 1)); break;
          case FL_DOUBLE:  MOVSD(kFReg64s[farg_index], OFFSET_INDIRECT(offset, RBP, NULL, 1)); break;
          default: assert(false); break;
          }
          ++farg_index;
        }
        continue;
      }
#endif

      switch (type->kind) {
      case TY_FIXNUM:
      case TY_PTR:
        break;
      default: assert(false); break;
      }

      if (arg_index < MAX_REG_ARGS) {
        int size = type_size(type);
        assert(size < (int)(sizeof(kRegTable) / sizeof(*kRegTable)) &&
               kRegTable[size] != NULL);
        MOV(kRegTable[size][arg_index], OFFSET_INDIRECT(offset, RBP, NULL, 1));
        ++arg_index;
      }
    }
  } else {  // vaargs
    int ip = 0;
    for (int i = arg_index; i < MAX_REG_ARGS; ++i) {
      const VarInfo *varinfo = NULL;
      while (ip < len) {
        const VarInfo *p = params->data[ip++];
        const Type *type = p->type;
        if (!is_stack_param(type)
#ifndef __NO_FLONUM
            && !is_flonum(type)
#endif
        ) {
          varinfo = p;
          break;
        }
      }
      if (varinfo != NULL) {
        const Type *type = varinfo->type;
        assert(type->kind == TY_FIXNUM || type->kind == TY_PTR);
        int size = type_size(type);
        assert(size < (int)(sizeof(kRegTable) / sizeof(*kRegTable)) &&
               kRegTable[size] != NULL);
        int offset = varinfo->local.reg->offset;
        MOV(kRegTable[size][i], OFFSET_INDIRECT(offset, RBP, NULL, 1));
      } else {
        int size = type_size(&tyVoidPtr);
        assert(size < (int)(sizeof(kRegTable) / sizeof(*kRegTable)) &&
               kRegTable[size] != NULL);
        int offset = (i - MAX_REG_ARGS - MAX_FREG_ARGS) * WORD_SIZE;
        MOV(kRegTable[size][i], OFFSET_INDIRECT(offset, RBP, NULL, 1));
      }
    }

#ifndef __NO_FLONUM
    ip = 0;
    for (int i = 0; i < MAX_FREG_ARGS; ++i) {
      const VarInfo *varinfo = NULL;
      while (ip < len) {
        const VarInfo *p = params->data[ip++];
        const Type *type = p->type;
        if (!is_stack_param(type)
            && is_flonum(type)
        ) {
          varinfo = p;
          break;
        }
      }
      if (varinfo != NULL) {
        const Type *type = varinfo->type;
        assert(type->kind == TY_FLONUM);
        int offset = varinfo->local.reg->offset;
        switch (type->flonum.kind) {
        case FL_FLOAT:   MOVSS(kFReg64s[i], OFFSET_INDIRECT(offset, RBP, NULL, 1)); break;
        case FL_DOUBLE:  MOVSD(kFReg64s[i], OFFSET_INDIRECT(offset, RBP, NULL, 1)); break;
        default: assert(false); break;
        }
      } else {
        int offset = (i - MAX_FREG_ARGS) * WORD_SIZE;
        MOVSD(kFReg64s[i], OFFSET_INDIRECT(offset, RBP, NULL, 1));
      }
    }
#endif
  }
}

static void emit_defun(Function *func) {
  if (func->scopes == NULL)  // Prototype definition
    return;

  assert(stackpos == 8);

  emit_comment(NULL);
  _TEXT();

  bool global = true;
  const VarInfo *varinfo = scope_find(global_scope, func->name, NULL);
  if (varinfo != NULL) {
    global = (varinfo->storage & VS_STATIC) == 0;
  }

  const char *label = fmt_name(func->name);
  if (global) {
    const char *gl = MANGLE(label);
    _GLOBL(gl);
    EMIT_LABEL(gl);
  } else {
    emit_comment("%.*s: static func", func->name->bytes, func->name->chars);
    EMIT_LABEL(label);
  }

  bool no_stmt = true;
  if (func->stmts != NULL) {
    for (int i = 0; i < func->stmts->len; ++i) {
      Stmt *stmt = func->stmts->data[i];
      if (stmt == NULL)
        continue;
      if (!is_asm(stmt)) {
        no_stmt = false;
        break;
      }
    }
  }

  // Prologue
  // Allocate variable bufer.
  if (!no_stmt) {
    PUSH(RBP); PUSH_STACK_POS();
    MOV(RSP, RBP);
    if (func->ra->frame_size > 0) {
      SUB(IM(func->ra->frame_size), RSP);
      stackpos += func->ra->frame_size;
    }

    put_args_to_stack(func);

    // Callee save.
    push_callee_save_regs(func->ra->used_reg_bits);
  }

  emit_bb_irs(func->bbcon);

  // Epilogue
  if (!no_stmt) {
    pop_callee_save_regs(func->ra->used_reg_bits);

    MOV(RBP, RSP);
    stackpos -= func->ra->frame_size;
    POP(RBP); POP_STACK_POS();
  }

  RET();

  // Output static local variables.
  for (int i = 0; i < func->scopes->len; ++i) {
    Scope *scope = func->scopes->data[i];
    if (scope->vars == NULL)
      continue;
    for (int j = 0; j < scope->vars->len; ++j) {
      VarInfo *varinfo = scope->vars->data[j];
      if (!(varinfo->storage & VS_STATIC))
        continue;
      VarInfo *gvarinfo = varinfo->static_.gvar;
      assert(gvarinfo != NULL);
      emit_varinfo(gvarinfo, gvarinfo->global.init);
    }
  }

  assert(stackpos == 8);
}

void emit_code(Vector *decls) {
  for (int i = 0, len = decls->len; i < len; ++i) {
    Declaration *decl = decls->data[i];
    if (decl == NULL)
      continue;

    switch (decl->kind) {
    case DCL_DEFUN:
      emit_defun(decl->defun.func);
      break;
    case DCL_VARDECL:
      {
        emit_comment(NULL);
        Vector *decls = decl->vardecl.decls;
        for (int i = 0; i < decls->len; ++i) {
          VarDecl *vd = decls->data[i];
          if ((vd->storage & VS_EXTERN) != 0)
            continue;
          const Name *name = vd->ident->ident;
          const VarInfo *varinfo = scope_find(global_scope, name, NULL);
          assert(varinfo != NULL);

          emit_varinfo(varinfo, varinfo->global.init);
        }
      }
      break;

    default:
      error("Unhandled decl in emit_code: %d", decl->kind);
      break;
    }
  }
}