/* Copyright (C) 1998 - 2000 by Universitaet Karlsruhe
* All rights reserved.
*
* Authors: Martin Trapp, Christian Schaefer
*
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

# include <stdlib.h>
# include <stddef.h>
# include <string.h>

# include "entity_t.h"
# include "mangle.h"
# include "typegmod_t.h"
# include "array.h"
/* All this is needed to build the constant node for methods: */
# include "irprog_t.h"
# include "ircons.h"
# include "tv_t.h"

/*******************************************************************/
/** general                                                       **/
/*******************************************************************/

void
init_entity (void)
{
}

/*******************************************************************/
/** ENTITY                                                        **/
/*******************************************************************/

/* redeclared to declare INLINE. */
INLINE entity *get_entity_overwrites   (entity *ent, int pos);
INLINE entity *get_entity_overwrittenby   (entity *ent, int pos);
INLINE type   *get_entity_owner (entity *ent);

INLINE void insert_entity_in_owner (entity *ent) {
  type *owner = ent->owner;
  switch (get_type_tpop_code(owner)) {
  case tpo_class: {
    add_class_member (owner, ent);
  } break;
  case tpo_struct: {
    add_struct_member (owner, ent);
  } break;
  case tpo_union: {
    add_union_member (owner, ent);
  } break;
  case tpo_array: {
    set_array_element_entity(owner, ent);
  } break;
  default: assert(0);
  }
}

entity *
new_entity (type *owner, ident *name, type *type)
{
  entity *res;
  ir_graph *rem;

  assert(!id_contains_char(name, ' ') && "entity name should not contain spaces");

  res = (entity *) xmalloc (sizeof (entity));
  res->kind = k_entity;
  assert_legal_owner_of_ent(owner);
  res->owner = owner;
  res->name = name;
  res->type = type;
  if (get_type_tpop(type) == type_method)
    res->allocation = static_allocated;
  else
    res->allocation = automatic_allocated;
  res->visibility = local;
  res->offset = -1;
  if (is_method_type(type)) {
    res->variability = constant;
    rem = current_ir_graph;
    current_ir_graph = get_const_code_irg();
    res->value = new_Const(mode_P_mach, new_tarval_from_entity(res, mode_P_mach));
    current_ir_graph = rem;
  } else {
    res->variability = uninitialized;
    res->value = NULL;
    res->values = NULL;
  }
  res->peculiarity = existent;
  res->volatility = non_volatile;
  res->ld_name = NULL;
  res->overwrites = NEW_ARR_F(entity *, 1);
  res->overwrittenby = NEW_ARR_F(entity *, 1);

  res->irg = NULL;

#ifdef DEBUG_libfirm
  res->nr = get_irp_new_node_nr();
#endif

  res->visit = 0;

  /* Remember entity in it's owner. */
  insert_entity_in_owner (res);
  return res;
}
entity *
new_d_entity (type *owner, ident *name, type *type, dbg_info *db) {
  entity *res = new_entity(owner, name, type);
  set_entity_dbg_info(res, db);
  return res;
}
INLINE void free_entity_attrs(entity *ent) {
  assert(ent);
  if (get_type_tpop(get_entity_owner(ent)) == type_class) {
    DEL_ARR_F(ent->overwrites);
    DEL_ARR_F(ent->overwrittenby);
  }
}

entity *
copy_entity_own (entity *old, type *new_owner) {
  entity *new;

  assert_legal_owner_of_ent(new_owner);
  if (old->owner == new_owner) return old;
  new = (entity *) xmalloc (sizeof (entity));
  memcpy (new, old, sizeof (entity));
  new->owner = new_owner;
  /*
  if ((get_type_tpop(get_entity_owner(old)) == type_class) &&
      (get_type_tpop(new_owner) == type_class)) {
    new->overwrites = DUP_ARR_F(entity *, old->overwrites);
    new->overwrittenby = DUP_ARR_F(entity *, old->overwrittenby);
  } else if ((get_type_tpop(get_entity_owner(old)) != type_class) &&
	     (get_type_tpop(new_owner) == type_class)) {
    new->overwrites = NEW_ARR_F(entity *, 1);
    new->overwrittenby = NEW_ARR_F(entity *, 1);
  }
  */
  if (is_class_type(new_owner)) {
    new->overwrites = NEW_ARR_F(entity *, 1);
    new->overwrittenby = NEW_ARR_F(entity *, 1);
  }
#ifdef DEBUG_libfirm
  new->nr = get_irp_new_node_nr();
#endif

  insert_entity_in_owner (new);

  return new;
}

entity *
copy_entity_name (entity *old, ident *new_name) {
  entity *new;

  if (old->name == new_name) return old;
  new = (entity *) xmalloc (sizeof (entity));
  memcpy (new, old, sizeof (entity));
  new->name = new_name;
  new->ld_name = NULL;
  if (is_class_type(new->owner)) {
    new->overwrites = DUP_ARR_F(entity *, old->overwrites);
    new->overwrittenby = DUP_ARR_F(entity *, old->overwrittenby);
  }
#ifdef DEBUG_libfirm
  new->nr = get_irp_new_node_nr();
#endif

  insert_entity_in_owner (new);

  return new;
}


void
free_entity (entity *ent) {
  free_tarval_entity(ent);
  free_entity_attrs(ent);
  free(ent);
}

/* Outputs a unique number for this node */
INLINE long
get_entity_nr(entity *ent) {
  assert(ent);
#ifdef DEBUG_libfirm
  return ent->nr;
#else
  return 0;
#endif
}

INLINE const char *
get_entity_name (entity *ent) {
  assert (ent);
  return get_id_str(get_entity_ident(ent));
}

ident *
get_entity_ident    (entity *ent) {
  assert(ent);
  return ent->name;
}

/*
void   set_entitye_ld_name  (entity *, char *ld_name);
void   set_entity_ld_ident (entity *, ident *ld_ident);
*/

INLINE type *
get_entity_owner (entity *ent) {
  return ent->owner = skip_tid(ent->owner);
}

INLINE void
set_entity_owner (entity *ent, type *owner) {
  assert_legal_owner_of_ent(owner);
  ent->owner = owner;
}

INLINE void   /* should this go into type.c? */
assert_legal_owner_of_ent(type *owner) {
  assert (get_type_tpop_code(owner) == tpo_class ||
          get_type_tpop_code(owner) == tpo_union ||
          get_type_tpop_code(owner) == tpo_struct ||
	  get_type_tpop_code(owner) == tpo_array);   /* Yes, array has an entity
							-- to select fields! */
}

INLINE ident *
get_entity_ld_ident (entity *ent)
{
  if (ent->ld_name == NULL)
    ent->ld_name = mangle_entity (ent);
  return ent->ld_name;
}

INLINE void
set_entity_ld_ident (entity *ent, ident *ld_ident) {
  ent->ld_name = ld_ident;
}

INLINE const char *
get_entity_ld_name (entity *ent) {
  return get_id_str(get_entity_ld_ident(ent));
}

/*
char  *get_entity_ld_name  (entity *);
void   set_entity_ld_name  (entity *, char *ld_name);
*/

INLINE type *
get_entity_type (entity *ent) {
  return ent->type = skip_tid(ent->type);
}

INLINE void
set_entity_type (entity *ent, type *type) {
  ent->type = type;
}


INLINE ent_allocation
get_entity_allocation (entity *ent) {
  return ent->allocation;
}

INLINE void
set_entity_allocation (entity *ent, ent_allocation al) {
  ent->allocation = al;
}

/* return the name of the visibility */
const char *get_allocation_name(ent_allocation all)
{
#define X(a)	case a: return #a
  switch (all) {
    X(automatic_allocated);
    X(parameter_allocated);
    X(dynamic_allocated);
    X(static_allocated);
    default: return "BAD VALUE";
  }
#undef X
}


INLINE ent_visibility
get_entity_visibility (entity *ent) {
  return ent->visibility;
}

INLINE void
set_entity_visibility (entity *ent, ent_visibility vis) {
  if (vis != local)
    assert((ent->allocation == static_allocated) ||
	   (ent->allocation == automatic_allocated));
  /* @@@ Test that the owner type is not local, but how??
         && get_class_visibility(get_entity_owner(ent)) != local));*/
  ent->visibility = vis;
}

/* return the name of the visibility */
const char *get_visibility_name(ent_visibility vis)
{
#define X(a)	case a: return #a
  switch (vis) {
    X(local);
    X(external_visible);
    X(external_allocated);
    default: return "BAD VALUE";
  }
#undef X
}

INLINE ent_variability
get_entity_variability (entity *ent) {
  return ent->variability;
}

INLINE void
set_entity_variability (entity *ent, ent_variability var){
  if (var == part_constant)
    assert(is_class_type(ent->type) || is_struct_type(ent->type));
  if ((is_compound_type(ent->type)) &&
      (ent->variability == uninitialized) && (var != uninitialized)) {
    /* Allocate datastructures for constant values */
    ent->values = NEW_ARR_F(ir_node *, 1);
    ent->val_paths = NEW_ARR_F(compound_graph_path *, 1);
  }
  if ((is_compound_type(ent->type)) &&
      (var == uninitialized) && (ent->variability != uninitialized)) {
    /* Free datastructures for constant values */
    DEL_ARR_F(ent->values);
    DEL_ARR_F(ent->val_paths);
  }
  ent->variability = var;
}

/* return the name of the variablity */
const char *get_variability_name(ent_variability var)
{
#define X(a)	case a: return #a
  switch (var) {
    X(uninitialized);
    X(initialized);
    X(part_constant);
    X(constant);
    default: return "BAD VALUE";
  }
#undef X
}

INLINE ent_volatility
get_entity_volatility (entity *ent) {
  assert (ent);
  return ent->volatility;
}

INLINE void
set_entity_volatility (entity *ent, ent_volatility vol) {
  assert (ent);
  ent->volatility = vol;
}

/* return the name of the volatility */
const char *get_volatility_name(ent_volatility var)
{
#define X(a)	case a: return #a
  switch (var) {
    X(non_volatile);
    X(is_volatile);
    default: return "BAD VALUE";
  }
#undef X
}

INLINE peculiarity
get_entity_peculiarity (entity *ent) {
  assert (ent);
  return ent->peculiarity;
}

INLINE void
set_entity_peculiarity (entity *ent, peculiarity pec) {
  assert (ent);
  /* @@@ why peculiarity only for methods? */
  assert (is_method_type(ent->type));
  ent->peculiarity = pec;
}

/* return the name of the peculiarity */
const char *get_peculiarity_name(peculiarity var)
{
#define X(a)	case a: return #a
  switch (var) {
    X(description);
    X(inherited);
    X(existent);
    default: return "BAD VALUE";
  }
#undef X
}

/* Set has no effect for existent entities of type method. */
INLINE ir_node *
get_atomic_ent_value(entity *ent) {
  assert(ent); assert(is_atomic_entity(ent));
  assert((ent->variability != uninitialized));
  return ent->value;
}

INLINE void
set_atomic_ent_value(entity *ent, ir_node *val) {
  assert(ent && is_atomic_entity(ent) && (ent->variability != uninitialized));
  if ((is_method_type(ent->type)) && (ent->peculiarity==existent)) return;
  ent->value = val;
}


ir_node *copy_const_value(ir_node *n) {
  ir_node *nn;
  ir_mode *m;

  m = get_irn_mode(n);
  switch(get_irn_opcode(n)) {
  case iro_Const:
    nn = new_Const(m, get_Const_tarval(n)); break;
  case iro_SymConst:
    nn = new_SymConst(get_SymConst_type_or_id(n), get_SymConst_kind(n)); break;
  case iro_Add:
    nn = new_Add(copy_const_value(get_Add_left(n)), copy_const_value(get_Add_right(n)), m); break;
  case iro_Unknown:
    nn = new_Unknown(); break;
  default:
    DDMN(n);
    assert(0 && "opdope invalid or not implemented");
    nn = NULL;
    break;
  }
  return nn;
}

compound_graph_path *
new_compound_graph_path(type *tp, int length) {
  compound_graph_path *res;
  assert(is_type(tp) && is_compound_type(tp));
  assert(length > 0);

  res = (compound_graph_path *) malloc (sizeof(compound_graph_path) + (length-1) * sizeof(entity *));
  res->kind = k_ir_compound_graph_path;
  res->tp = tp;
  res->len = length;
  memset(res->nodes, 0, sizeof(entity *) * length);
  return res;
}

INLINE void
free_compound_graph_path (compound_graph_path *gr) {
  assert(gr && is_compound_graph_path(gr));
  free(gr);
}

INLINE int
is_compound_graph_path(void *thing) {
  return (get_kind(thing) == k_ir_compound_graph_path);
}

/* checks whether nodes 0..pos are correct (all lie on a path.) */
/* @@@ not implemented */
INLINE int is_proper_compound_graph_path(compound_graph_path *gr, int pos) {
  int i;
  entity *node;
  type *owner = gr->tp;
  for (i = 0; i <= pos; i++) {
    node = get_compound_graph_path_node(gr, i);
    if (get_entity_owner(node) != owner) return false;
    owner = get_entity_type(node);
  }
  if (pos == get_compound_graph_path_length(gr) -1)
    if (!is_atomic_type(owner)) return false;
  return true;
}

INLINE int
get_compound_graph_path_length(compound_graph_path *gr) {
  assert(gr && is_compound_graph_path(gr));
  return gr->len;
}

INLINE entity *
get_compound_graph_path_node(compound_graph_path *gr, int pos) {
  assert(gr && is_compound_graph_path(gr));
  assert(pos >= 0 && pos < gr->len);
  return gr->nodes[pos];
}

INLINE void
set_compound_graph_path_node(compound_graph_path *gr, int pos, entity *node) {
  assert(gr && is_compound_graph_path(gr));
  assert(pos >= 0 && pos < gr->len);
  assert(is_entity(node));
  gr->nodes[pos] = node;
  assert(is_proper_compound_graph_path(gr, pos));
}

/* A value of a compound entity is a pair of value and the corresponding path to a member of
   the compound. */
INLINE void
add_compound_ent_value_w_path(entity *ent, ir_node *val, compound_graph_path *path) {
  assert(ent && is_compound_entity(ent) && (ent->variability != uninitialized));
  ARR_APP1 (ir_node *, ent->values, val);
  ARR_APP1 (compound_graph_path *, ent->val_paths, path);
}

INLINE void
set_compound_ent_value_w_path(entity *ent, ir_node *val, compound_graph_path *path, int pos) {
  assert(ent && is_compound_entity(ent) && (ent->variability != uninitialized));
  ent->values[pos+1] = val;
  ent->val_paths[pos+1] = path;
}

INLINE int
get_compound_ent_n_values(entity *ent) {
  assert(ent && is_compound_entity(ent) && (ent->variability != uninitialized));
  return (ARR_LEN (ent->values))-1;
}

INLINE ir_node  *
get_compound_ent_value(entity *ent, int pos) {
  assert(ent && is_compound_entity(ent) && (ent->variability != uninitialized));
  return ent->values[pos+1];
}

INLINE compound_graph_path *
get_compound_ent_value_path(entity *ent, int pos) {
  assert(ent && is_compound_entity(ent) && (ent->variability != uninitialized));
  return ent->val_paths[pos+1];
}

void
remove_compound_ent_value(entity *ent, entity *value_ent) {
  int i;
  assert(ent && is_compound_entity(ent) && (ent->variability != uninitialized));
  for (i = 1; i < (ARR_LEN (ent->val_paths)); i++) {
    compound_graph_path *path = ent->val_paths[i];
    if (path->nodes[path->len-1] == value_ent) {
      for(; i < (ARR_LEN (ent->val_paths))-1; i++) {
	ent->val_paths[i] = ent->val_paths[i+1];
	ent->values[i]   = ent->values[i+1];
      }
      ARR_SETLEN(entity*,  ent->val_paths, ARR_LEN(ent->val_paths) - 1);
      ARR_SETLEN(ir_node*, ent->values,    ARR_LEN(ent->values)    - 1);
      break;
    }
  }
}

INLINE void
add_compound_ent_value(entity *ent, ir_node *val, entity *member) {
  assert(ent && is_compound_entity(ent) && (ent->variability != uninitialized));
  compound_graph_path *path = new_compound_graph_path(get_entity_owner(ent), 1);
  path->nodes[0] = member;
  add_compound_ent_value_w_path(ent, val, path);
}

/* Copies the firm subgraph referenced by val to const_code_irg and adds
   the node as constant initialization to ent.
   The subgraph may not contain control flow operations.
INLINE void
copy_and_add_compound_ent_value(entity *ent, ir_node *val, entity *member) {
  ir_graph *rem = current_ir_graph;

  assert(get_entity_variability(ent) != uninitialized);
  current_ir_graph = get_const_code_irg();

  val = copy_const_value(val);
  add_compound_ent_value(ent, val, member);
  current_ir_graph = rem;
  }*/

/* Copies the value i of the entity to current_block in current_ir_graph.
ir_node *
copy_compound_ent_value(entity *ent, int pos) {
  assert(ent && is_compound_entity(ent) && (ent->variability != uninitialized));
  return copy_const_value(ent->values[pos+1]);
  }*/

INLINE entity   *
get_compound_ent_value_member(entity *ent, int pos) {
  compound_graph_path *path;
  assert(ent && is_compound_entity(ent) && (ent->variability != uninitialized));
  path = get_compound_ent_value_path(ent, pos);
  assert(path->len == 1);

  return get_compound_graph_path_node(path, get_compound_graph_path_length(path)-1);
}

INLINE void
set_compound_ent_value(entity *ent, ir_node *val, entity *member, int pos) {
  compound_graph_path *path;
  assert(ent && is_compound_entity(ent) && (ent->variability != uninitialized));
  path = get_compound_ent_value_path(ent, pos);
  set_compound_graph_path_node(path, 0, member);
  set_compound_ent_value_w_path(ent, val, path, pos);
}

void
set_array_entity_values(entity *ent, tarval **values, int num_vals) {
  int i;
  ir_graph *rem = current_ir_graph;
  type *arrtp = get_entity_type(ent);
  ir_node *val;

  assert(is_array_type(arrtp));
  assert(get_array_n_dimensions(arrtp) == 1);
  /* One bound is sufficient, the nunmber of constant fields makes the
     size. */
  assert(get_array_lower_bound (arrtp, 0) || get_array_upper_bound (arrtp, 0));
  assert(get_entity_variability(ent) != uninitialized);
  current_ir_graph = get_const_code_irg();

  for (i = 0; i < num_vals; i++) {
    val = new_Const(get_tarval_mode (values[i]), values[i]);
    add_compound_ent_value(ent, val, get_array_element_entity(arrtp));
  }
  current_ir_graph = rem;
}

INLINE int
get_entity_offset (entity *ent) {
  return ent->offset;
}

INLINE void
set_entity_offset (entity *ent, int offset) {
  ent->offset = offset;
}

INLINE void
add_entity_overwrites   (entity *ent, entity *overwritten) {
  assert(ent);
  assert(is_class_type(get_entity_owner(ent)));
  ARR_APP1 (entity *, ent->overwrites, overwritten);
  ARR_APP1 (entity *, overwritten->overwrittenby, ent);
}

INLINE int
get_entity_n_overwrites (entity *ent) {
  assert(ent);
  assert(is_class_type(get_entity_owner(ent)));
  return (ARR_LEN (ent->overwrites))-1;
}

int
get_entity_overwrites_index(entity *ent, entity *overwritten) {
  int i;
  assert(ent && is_class_type(get_entity_owner(ent)));
  for (i = 0; i < get_entity_n_overwrites(ent); i++)
    if (get_entity_overwrites(ent, i) == overwritten)
      return i;
  return -1;
}

INLINE entity *
get_entity_overwrites   (entity *ent, int pos) {
  assert(ent);
  assert(is_class_type(get_entity_owner(ent)));
  assert(pos < get_entity_n_overwrites(ent));
  return ent->overwrites[pos+1];
}

INLINE void
set_entity_overwrites   (entity *ent, int pos, entity *overwritten) {
  assert(ent);
  assert(is_class_type(get_entity_owner(ent)));
  assert(pos < get_entity_n_overwrites(ent));
  ent->overwrites[pos+1] = overwritten;
}

void
remove_entity_overwrites(entity *ent, entity *overwritten) {
  int i;
  assert(ent && is_class_type(get_entity_owner(ent)));
  for (i = 1; i < (ARR_LEN (ent->overwrites)); i++)
    if (ent->overwrites[i] == overwritten) {
      for(; i < (ARR_LEN (ent->overwrites))-1; i++)
	ent->overwrites[i] = ent->overwrites[i+1];
      ARR_SETLEN(entity*, ent->overwrites, ARR_LEN(ent->overwrites) - 1);
      break;
    }
}

INLINE void
add_entity_overwrittenby   (entity *ent, entity *overwrites) {
  assert(ent);
  assert(is_class_type(get_entity_owner(ent)));
  add_entity_overwrites(overwrites, ent);
}

INLINE int
get_entity_n_overwrittenby (entity *ent) {
  assert(ent);
  assert(is_class_type(get_entity_owner(ent)));
  return (ARR_LEN (ent->overwrittenby))-1;
}

int
get_entity_overwrittenby_index(entity *ent, entity *overwrites) {
  int i;
  assert(ent && is_class_type(get_entity_owner(ent)));
  for (i = 0; i < get_entity_n_overwrittenby(ent); i++)
    if (get_entity_overwrittenby(ent, i) == overwrites)
      return i;
  return -1;
}

INLINE entity *
get_entity_overwrittenby   (entity *ent, int pos) {
  assert(ent);
  assert(is_class_type(get_entity_owner(ent)));
  assert(pos < get_entity_n_overwrittenby(ent));
  return ent->overwrittenby[pos+1];
}

INLINE void
set_entity_overwrittenby   (entity *ent, int pos, entity *overwrites) {
  assert(ent);
  assert(is_class_type(get_entity_owner(ent)));
  assert(pos < get_entity_n_overwrittenby(ent));
  ent->overwrittenby[pos+1] = overwrites;
}

void    remove_entity_overwrittenby(entity *ent, entity *overwrites) {
  int i;
  assert(ent && is_class_type(get_entity_owner(ent)));
  for (i = 1; i < (ARR_LEN (ent->overwrittenby)); i++)
    if (ent->overwrittenby[i] == overwrites) {
      for(; i < (ARR_LEN (ent->overwrittenby))-1; i++)
	ent->overwrittenby[i] = ent->overwrittenby[i+1];
      ARR_SETLEN(entity*, ent->overwrittenby, ARR_LEN(ent->overwrittenby) - 1);
      break;
    }
}

/* A link to store intermediate information */
void *
get_entity_link(entity *ent) {
  assert(ent);
  return ent->link;
}

void
set_entity_link(entity *ent, void *l) {
  assert(ent);
  ent->link = l;
}

INLINE ir_graph *
get_entity_irg(entity *ent) {
  assert (ent);
  assert (is_method_type(ent->type));
  return ent->irg;
}

INLINE void
set_entity_irg(entity *ent, ir_graph *irg) {
  assert (ent && ent->type);
  /* Wie kann man die Referenz auf einen IRG l�schen, z.B. wenn die
   * Methode selbst nicht mehr aufgerufen werden kann, die Entit�t
   * aber erhalten bleiben soll. */
  /* assert (irg); */
  assert (is_method_type(ent->type));
  assert (ent->peculiarity == existent);
  ent->irg = irg;
}

int is_entity (void *thing) {
  assert(thing);
  if (get_kind(thing) == k_entity)
    return 1;
  else
    return 0;
}

int is_atomic_entity(entity *ent) {
  type* t = get_entity_type(ent);
  return (is_primitive_type(t) || is_pointer_type(t) ||
	  is_enumeration_type(t) || is_method_type(t));
}

int is_compound_entity(entity *ent) {
  type* t = get_entity_type(ent);
  return (is_class_type(t) || is_struct_type(t) ||
	  is_array_type(t) || is_union_type(t));
}

/* @@@ not implemnted!!! */
bool equal_entity(entity *ent1, entity *ent2) {
  printf(" calling unimplemented equal entity!!! \n");
  return true;
}


unsigned long get_entity_visited(entity *ent) {
  assert (ent);
  return ent->visit;
}
void        set_entity_visited(entity *ent, unsigned long num) {
  assert (ent);
  ent->visit = num;
}
/* Sets visited field in entity to entity_visited. */
void        mark_entity_visited(entity *ent) {
  assert (ent);
  ent->visit = type_visited;
}


INLINE bool entity_visited(entity *ent) {
  return get_entity_visited(ent) >= type_visited;
}
INLINE bool entity_not_visited(entity *ent) {
  return get_entity_visited(ent) < type_visited;
}
