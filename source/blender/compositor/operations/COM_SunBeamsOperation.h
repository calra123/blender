/* This program is free software; you can redistribute it and/or
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
 * Copyright 2014, Blender Foundation.
 */

#pragma once

#include "COM_MultiThreadedOperation.h"

namespace blender::compositor {

class SunBeamsOperation : public MultiThreadedOperation {
 public:
  SunBeamsOperation();

  void executePixel(float output[4], int x, int y, void *data) override;

  void initExecution() override;

  void *initializeTileData(rcti *rect) override;

  bool determineDependingAreaOfInterest(rcti *input,
                                        ReadBufferOperation *readOperation,
                                        rcti *output) override;

  void setData(const NodeSunBeams &data)
  {
    m_data = data;
  }

  void update_memory_buffer_partial(MemoryBuffer *output,
                                    const rcti &area,
                                    Span<MemoryBuffer *> inputs) override;
  void get_area_of_interest(int input_idx, const rcti &output_area, rcti &r_input_area) override;

 private:
  void calc_rays_common_data();

 private:
  NodeSunBeams m_data;

  float m_source_px[2];
  float m_ray_length_px;
};

}  // namespace blender::compositor
