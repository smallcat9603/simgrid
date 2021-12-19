/* Copyright (c) 2011-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SURFXML_PARSE_VALUES_H_
#define SURFXML_PARSE_VALUES_H_


typedef struct s_surf_parsing_link_up_down *surf_parsing_link_up_down_t;
typedef struct s_surf_parsing_link_up_down {
  void* link_up;
  void* link_down;
} s_surf_parsing_link_up_down_t;

#endif /* SURFXML_PARSE_VALUES_H_ */
