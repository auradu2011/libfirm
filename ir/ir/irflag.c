/* Copyright (C) 1998 - 2000 by Universitaet Karlsruhe
** All rights reserved.
**
** Authors: Christian Schaefer
**
** irflag --- optimization flags
*/

# include "irflag.h"

/* 0 - don't do this optimization
   1 - lets see, if there is a better graph */
int opt_cse = 0;
int opt_constant_folding = 1;
int opt_dead_node_elimination = 0;
int optimized = 1;

/* set the flags with set_flagname, get the flag with get_flagname */

void
set_opt_cse (int value)
{
  opt_cse = value;
}

int
get_opt_cse (void)
{
  return opt_cse;
}

void
set_opt_constant_folding (int value)
{
  opt_constant_folding=value;
}

int
get_opt_constant_folding (void)
{
  return opt_constant_folding;
}

void set_opt_dead_node_elimination (int value)
{
  opt_dead_node_elimination=value;
}

int  get_opt_dead_node_elimination (void)
{
  return opt_dead_node_elimination;
}

void
set_optimize (int value)
{
  optimized = value;
}

int
get_optimize (void)
{
  return optimized;
}
