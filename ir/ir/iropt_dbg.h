/*
 * Project:     libFIRM
 * File name:   ir/ir/iropt_dbg.h
 * Purpose:     Debug makros used in iropt.
 * Author:      Goetz Lindenmaier
 * Modified by:
 * Created:
 * CVS-ID:      $Id$
 * Copyright:   (c) 2001-2003 Universitšt Karlsruhe
 * Licence:     This file protected by GPL -  GNU GENERAL PUBLIC LICENSE.
 */


/* This file contains makros that generate the calls to
   update the debug information after a transformation. */

#define SIZ(x)    sizeof(x)/sizeof((x)[0])


/**
 * Merge the debug info due to dead code elimination
 */
#define DBG_OPT_DEAD                                                \
  do {                                                       \
	  ir_node *ons[2];                                         \
	  ons[0] = oldn;                                           \
	  ons[1] = get_Block_cfgpred(oldn, 0);                     \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_STG);	   \
	  __dbg_info_merge_sets(&n, 1, ons, SIZ(ons), dbg_dead_code); \
	} while(0)


/**
 * Merge the debug info due to a straightening optimization
 */
#define DBG_OPT_STG                                                \
  do {                                                       \
	  ir_node *ons[2];                                         \
	  ons[0] = oldn;                                           \
	  ons[1] = get_Block_cfgpred(oldn, 0);                     \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_STG);	   \
	  __dbg_info_merge_sets(&n, 1, ons, SIZ(ons), dbg_straightening); \
	} while(0)

/**
 * Merge the debug info due to an if simplification
 */
#define DBG_OPT_IFSIM                                                 \
  do {                                                          \
	  ir_node *ons[4];                                            \
	  ons[0] = oldn;                                                 \
	  ons[1] = a;                                                 \
	  ons[2] = b;                                                 \
	  ons[3] = get_Proj_pred(a);                                  \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_IFSIM);	   \
	  __dbg_info_merge_sets(&n, 1, ons, SIZ(ons), dbg_if_simplification); \
	} while(0)

/**
 * Merge the debug info due to an algebraic_simplification
 */
#define DBG_OPT_ALGSIM0                                               	\
  do {                                                          	\
	  stat_merge_nodes(&n, 1, &oldn, 1, STAT_OPT_CONST_EVAL);    	\
          __dbg_info_merge_pair(n, oldn, dbg_const_eval);		\
	} while(0)

#define DBG_OPT_ALGSIM1                                               \
  do {                                                          \
	  ir_node *ons[3];                                            \
	  ons[0] = oldn;                                              \
	  ons[1] = a;                                                 \
	  ons[2] = b;                                                 \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_ALGSIM);    \
	  __dbg_info_merge_sets(&n, 1, ons, SIZ(ons), dbg_algebraic_simplification); \
	} while(0)

#define DBG_OPT_ALGSIM2                                               \
  do {                                                          \
	  ir_node *ons[3];                                            \
	  ons[0] = oldn;                                              \
	  ons[1] = get_unop_op(n);                                    \
	  ons[2] = n;                                                 \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_ALGSIM);    \
	  __dbg_info_merge_sets(&n, 1, ons, SIZ(ons), dbg_algebraic_simplification); \
	} while(0)

#define DBG_OPT_ALGSIM3                                               \
  do {                                                          \
	  ir_node *ons[2];                                            \
	  ons[0] = oldn;                                              \
	  ons[1] = a;                                                 \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_ALGSIM);    \
	  __dbg_info_merge_sets(&n, 1, ons, SIZ(ons), dbg_algebraic_simplification); \
	} while(0)

#define DBG_OPT_PHI                                                   \
  do {                                                          \
	  ir_node *ons[2];                                            \
	  ons[0] = oldn;                                              \
	  ons[1] = first_val;                                         \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_PHI);	      \
	  __dbg_info_merge_sets(&n, 1, ons, SIZ(ons), dbg_opt_ssa);   \
	} while(0)


#define DBG_OPT_WAW                                                   \
  do {                                                          \
	  ir_node *ons[2];                                            \
	  ons[0] = oldn;                                              \
	  ons[1] = n;                                                 \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_WAW);	      \
	  __dbg_info_merge_sets(&n, 1, ons, SIZ(ons), dbg_write_after_write);\
	} while(0)

#define DBG_OPT_WAR                                                   \
  do {                                                          \
	  ir_node *ons[2];                                            \
	  ons[0] = oldn;                                              \
	  ons[1] = c;                                                 \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_WAR);	      \
	  __dbg_info_merge_sets(&c, 1, ons, SIZ(ons), dbg_write_after_read); \
	} while(0)

#define DBG_OPT_TUPLE                                                 \
  do {                                                          \
	  ir_node *ons[3];                                            \
	  ons[0] = oldn;                                              \
	  ons[1] = a;                                                 \
	  ons[2] = n;                                                 \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_TUPLE);     \
	  __dbg_info_merge_sets(&n, 1, ons, SIZ(ons), dbg_opt_auxnode);      \
	} while(0)

#define DBG_OPT_ID                                                \
  do {                                                          \
	  ir_node *ons[2];                                            \
	  ons[0] = oldn;                                              \
	  ons[1] = n;                                                 \
	  stat_merge_nodes(&n, 1, ons, SIZ(ons), STAT_OPT_ID);	      \
	  __dbg_info_merge_sets(&n, 1, ons, SIZ(ons), dbg_opt_auxnode);      \
	} while(0)
