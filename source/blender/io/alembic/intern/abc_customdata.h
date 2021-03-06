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
 * The Original Code is Copyright (C) 2016 Kévin Dietrich.
 * All rights reserved.
 */
#pragma once

/** \file
 * \ingroup balembic
 */

#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>

#include <map>

struct CustomData;
struct MLoop;
struct MLoopUV;
struct MPoly;
struct MVert;
struct Mesh;

using Alembic::Abc::ICompoundProperty;
using Alembic::Abc::OCompoundProperty;
namespace blender::io::alembic {

struct UVSample {
  std::vector<Imath::V2f> uvs;
  std::vector<uint32_t> indices;
};

struct CDStreamConfig {
  MLoop *mloop;
  int totloop;

  MPoly *mpoly;
  int totpoly;

  MVert *mvert;
  int totvert;

  MLoopUV *mloopuv;

  CustomData *loopdata;

  bool pack_uvs;

  /* TODO(kevin): might need a better way to handle adding and/or updating
   * custom data such that it updates the custom data holder and its pointers properly. */
  Mesh *mesh;
  void *(*add_customdata_cb)(Mesh *mesh, const char *name, int data_type);

  float weight;
  float time;
  bool use_vertex_interpolation;
  Alembic::AbcGeom::index_t index;
  Alembic::AbcGeom::index_t ceil_index;

  const char **modifier_error_message;

  /* Alembic needs Blender to keep references to C++ objects (the destructors finalize the writing
   * to ABC). The following fields are all used to keep these references. */

  /* Mapping from UV map name to its ABC property, for the 2nd and subsequent UV maps; the primary
   * UV map is kept alive by the Alembic mesh sample itself. */
  std::map<std::string, Alembic::AbcGeom::OV2fGeomParam> abc_uv_maps;

  /* OCRO coordinates, aka Generated Coordinates. */
  Alembic::AbcGeom::OV3fGeomParam abc_ocro;

  CDStreamConfig()
      : mloop(NULL),
        totloop(0),
        mpoly(NULL),
        totpoly(0),
        totvert(0),
        pack_uvs(false),
        mesh(NULL),
        add_customdata_cb(NULL),
        weight(0.0f),
        time(0.0f),
        index(0),
        ceil_index(0),
        modifier_error_message(NULL)
  {
  }
};

/* Get the UVs for the main UV property on a OSchema.
 * Returns the name of the UV layer.
 *
 * For now the active layer is used, maybe needs a better way to choose this. */
const char *get_uv_sample(UVSample &sample, const CDStreamConfig &config, CustomData *data);

void write_generated_coordinates(const OCompoundProperty &prop, CDStreamConfig &config);

void read_generated_coordinates(const ICompoundProperty &prop,
                                const CDStreamConfig &config,
                                const Alembic::Abc::ISampleSelector &iss);

void write_custom_data(const OCompoundProperty &prop,
                       CDStreamConfig &config,
                       CustomData *data,
                       int data_type);

void read_custom_data(const std::string &iobject_full_name,
                      const ICompoundProperty &prop,
                      const CDStreamConfig &config,
                      const Alembic::Abc::ISampleSelector &iss);

void read_velocity(const Alembic::Abc::ICompoundProperty &prop,
                   const Alembic::Abc::PropertyHeader *prop_header,
                   const Alembic::Abc::ISampleSelector &selector,
                   const CDStreamConfig &config,
                   const char *velocity_name,
                   const float velocity_scale);
typedef enum {
  ABC_UV_SCOPE_NONE,
  ABC_UV_SCOPE_LOOP,
  ABC_UV_SCOPE_VERTEX,
} AbcUvScope;

AbcUvScope get_uv_scope(const Alembic::AbcGeom::GeometryScope scope,
                        const CDStreamConfig &config,
                        const Alembic::AbcGeom::UInt32ArraySamplePtr &indices);

}  // namespace blender::io::alembic
