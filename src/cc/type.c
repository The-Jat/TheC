#include "type.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "lexer.h"

const Type tyChar =  {.type=TY_NUM, .u={.num={.type=NUM_CHAR}}};
const Type tyShort = {.type=TY_NUM, .u={.num={.type=NUM_SHORT}}};
const Type tyInt =   {.type=TY_NUM, .u={.num={.type=NUM_INT}}};
const Type tyLong =  {.type=TY_NUM, .u={.num={.type=NUM_LONG}}};
const Type tyEnum =  {.type=TY_NUM, .u={.num={.type=NUM_ENUM}}};
const Type tyVoid =  {.type=TY_VOID};

bool is_number(enum eType type) {
  return type == TY_NUM;
}

bool is_char_type(const Type *type) {
  return type->type == TY_NUM && type->u.num.type == NUM_CHAR;
}

bool is_void_ptr(const Type *type) {
  return type->type == TY_PTR && type->u.pa.ptrof->type == TY_VOID;
}

bool same_type(const Type *type1, const Type *type2) {
  for (;;) {
    if (type1->type != type2->type)
      return false;

    switch (type1->type) {
    case TY_VOID:
      return true;
    case TY_NUM:
      return type1->u.num.type == type2->u.num.type;
    case TY_ARRAY:
    case TY_PTR:
      type1 = type1->u.pa.ptrof;
      type2 = type2->u.pa.ptrof;
      continue;
    case TY_FUNC:
      if (!same_type(type1->u.func.ret, type2->u.func.ret) ||
          type1->u.func.param_types->len != type2->u.func.param_types->len)
        return false;
      for (int i = 0, len = type1->u.func.param_types->len; i < len; ++i) {
        const Type *t1 = (const Type*)type1->u.func.param_types->data[i];
        const Type *t2 = (const Type*)type2->u.func.param_types->data[i];
        if (!same_type(t1, t2))
          return false;
      }
      return true;
    case TY_STRUCT:
      {
        if (type1->u.struct_.info != NULL) {
          if (type2->u.struct_.info != NULL)
            return type1->u.struct_.info == type2->u.struct_.info;
          const Type *tmp = type1;
          type1 = type2;
          type2 = tmp;
        } else if (type2->u.struct_.info == NULL) {
          return strcmp(type1->u.struct_.name, type2->u.struct_.name) == 0;
        }
        // Find type1 from name.
        StructInfo *sinfo = find_struct(type1->u.struct_.name);
        if (sinfo == NULL)
          return false;
        return sinfo == type2->u.struct_.info;
      }
    }
  }
}

Type* ptrof(const Type *type) {
  Type *ptr = malloc(sizeof(*ptr));
  ptr->type = TY_PTR;
  ptr->u.pa.ptrof = type;
  return ptr;
}

const Type *array_to_ptr(const Type *type) {
  if (type->type != TY_ARRAY)
    return type;
  return ptrof(type->u.pa.ptrof);
}

Type* arrayof(const Type *type, size_t length) {
  Type *arr = malloc(sizeof(*arr));
  arr->type = TY_ARRAY;
  arr->u.pa.ptrof = type;
  arr->u.pa.length = length;
  return arr;
}

Type* new_func_type(const Type *ret, Vector *param_types, bool vaargs) {
  Type *f = malloc(sizeof(*f));
  f->type = TY_FUNC;
  f->u.func.ret = ret;
  f->u.func.vaargs = vaargs;
  f->u.func.param_types = param_types;
  return f;
}

// Struct

Map *struct_map;

StructInfo *find_struct(const char *name) {
  return (StructInfo*)map_get(struct_map, name);
}

void define_struct(const char *name, StructInfo *sinfo) {
  map_put(struct_map, name, sinfo);
}

// Enum

Map *enum_map;
Map *enum_value_map;

Type *find_enum(const char *name) {
  return map_get(enum_map, name);
}

Type *define_enum(const Token *ident) {
  Type *type = malloc(sizeof(*type));
  type->type = TY_NUM;
  type->u.num.type = NUM_ENUM;
  type->u.num.enum_.ident = ident;
  type->u.num.enum_.members = new_vector();

  if (ident != NULL) {
    map_put(enum_map, ident->u.ident, type);
  }

  return type;
}

void add_enum_member(Type *type, const Token *ident, int value) {
  assert(type->type == TY_NUM && type->u.num.type == NUM_ENUM);
  EnumMember *member = malloc(sizeof(*member));
  member->ident = ident;
  member->value = value;
  vec_push(type->u.num.enum_.members, member);

  map_put(enum_value_map, ident->u.ident, (void*)(intptr_t)value);
}

bool find_enum_value(const char *name, intptr_t *output) {
  return map_try_get(enum_value_map, name, (void**)output);
}

#if 0
void dump_type(FILE *fp, const Type *type) {
  switch (type->type) {
  case TY_VOID: fprintf(fp, "void"); break;
  case TY_NUM:
    switch (type->u.num.type) {
    case NUM_CHAR:  fprintf(fp, "char"); break;
    case NUM_SHORT: fprintf(fp, "short"); break;
    case NUM_INT:   fprintf(fp, "int"); break;
    case NUM_LONG:  fprintf(fp, "long"); break;
    case NUM_ENUM:  fprintf(fp, "enum"); break;
    default: assert(false); break;
    }
    break;
  case TY_PTR: dump_type(fp, type->u.pa.ptrof); fprintf(fp, "*"); break;
  case TY_ARRAY: dump_type(fp, type->u.pa.ptrof); fprintf(fp, "[%d]", (int)type->u.pa.length); break;
  default: assert(false); break;
  }
}
#endif