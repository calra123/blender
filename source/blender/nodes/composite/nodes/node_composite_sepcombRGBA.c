/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2006 Blender Foundation.
 * All rights reserved.
 */

/** \file
 * \ingroup cmpnodes
 */

#include "node_composite_util.h"

/* **************** SEPARATE RGBA ******************** */
static bNodeSocketTemplate cmp_node_seprgba_in[] = {
    {SOCK_RGBA, N_("Image"), 1.0f, 1.0f, 1.0f, 1.0f},
    {-1, ""},
};
static bNodeSocketTemplate cmp_node_seprgba_out[] = {
    {SOCK_FLOAT, N_("R")},
    {SOCK_FLOAT, N_("G")},
    {SOCK_FLOAT, N_("B")},
    {SOCK_FLOAT, N_("A")},
    {-1, ""},
};

void register_node_type_cmp_seprgba(void)
{
  static bNodeType ntype;

  cmp_node_type_base(&ntype, CMP_NODE_SEPRGBA, "Separate RGBA", NODE_CLASS_CONVERTER, 0);
  node_type_socket_templates(&ntype, cmp_node_seprgba_in, cmp_node_seprgba_out);

  nodeRegisterType(&ntype);
}

/* **************** COMBINE RGBA ******************** */
static bNodeSocketTemplate cmp_node_combrgba_in[] = {
    {SOCK_FLOAT, N_("R"), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, PROP_NONE},
    {SOCK_FLOAT, N_("G"), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, PROP_NONE},
    {SOCK_FLOAT, N_("B"), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, PROP_NONE},
    {SOCK_FLOAT, N_("A"), 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, PROP_NONE},
    {-1, ""},
};
static bNodeSocketTemplate cmp_node_combrgba_out[] = {
    {SOCK_RGBA, N_("Image")},
    {-1, ""},
};

void register_node_type_cmp_combrgba(void)
{
  static bNodeType ntype;

  cmp_node_type_base(&ntype, CMP_NODE_COMBRGBA, "Combine RGBA", NODE_CLASS_CONVERTER, 0);
  node_type_socket_templates(&ntype, cmp_node_combrgba_in, cmp_node_combrgba_out);

  nodeRegisterType(&ntype);
}
