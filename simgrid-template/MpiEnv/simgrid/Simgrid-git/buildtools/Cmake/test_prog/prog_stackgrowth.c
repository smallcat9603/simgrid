/* Copyright (c) 2010, 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <stdio.h>
#include <stdlib.h>
static int iterate = 10;
static int growsdown(int *x)
{
  auto int y;
  y = (x > &y);
  if (--iterate > 0)
    y = growsdown(&y);
  if (y != (x > &y))
    exit(1);
  return y;
}

int main(int argc, char *argv[])
{
  FILE *f;
  auto int x;
  if ((f = fopen("conftestval", "w")) == NULL)
    exit(1);
  fprintf(f, "%s\n", growsdown(&x) ? "down" : "up");;
  fclose(f);
  exit(0);
  return 1;
}
