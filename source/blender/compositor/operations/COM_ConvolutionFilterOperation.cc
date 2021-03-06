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
 * Copyright 2011, Blender Foundation.
 */

#include "COM_ConvolutionFilterOperation.h"

#include "BLI_utildefines.h"

#include "MEM_guardedalloc.h"

namespace blender::compositor {

ConvolutionFilterOperation::ConvolutionFilterOperation()
{
  this->addInputSocket(DataType::Color);
  this->addInputSocket(DataType::Value);
  this->addOutputSocket(DataType::Color);
  this->setResolutionInputSocketIndex(0);
  this->m_inputOperation = nullptr;
  this->flags.complex = true;
}
void ConvolutionFilterOperation::initExecution()
{
  this->m_inputOperation = this->getInputSocketReader(0);
  this->m_inputValueOperation = this->getInputSocketReader(1);
}

void ConvolutionFilterOperation::set3x3Filter(
    float f1, float f2, float f3, float f4, float f5, float f6, float f7, float f8, float f9)
{
  this->m_filter[0] = f1;
  this->m_filter[1] = f2;
  this->m_filter[2] = f3;
  this->m_filter[3] = f4;
  this->m_filter[4] = f5;
  this->m_filter[5] = f6;
  this->m_filter[6] = f7;
  this->m_filter[7] = f8;
  this->m_filter[8] = f9;
  this->m_filterHeight = 3;
  this->m_filterWidth = 3;
}

void ConvolutionFilterOperation::deinitExecution()
{
  this->m_inputOperation = nullptr;
  this->m_inputValueOperation = nullptr;
}

void ConvolutionFilterOperation::executePixel(float output[4], int x, int y, void * /*data*/)
{
  float in1[4];
  float in2[4];
  int x1 = x - 1;
  int x2 = x;
  int x3 = x + 1;
  int y1 = y - 1;
  int y2 = y;
  int y3 = y + 1;
  CLAMP(x1, 0, getWidth() - 1);
  CLAMP(x2, 0, getWidth() - 1);
  CLAMP(x3, 0, getWidth() - 1);
  CLAMP(y1, 0, getHeight() - 1);
  CLAMP(y2, 0, getHeight() - 1);
  CLAMP(y3, 0, getHeight() - 1);
  float value[4];
  this->m_inputValueOperation->read(value, x2, y2, nullptr);
  const float mval = 1.0f - value[0];

  zero_v4(output);
  this->m_inputOperation->read(in1, x1, y1, nullptr);
  madd_v4_v4fl(output, in1, this->m_filter[0]);
  this->m_inputOperation->read(in1, x2, y1, nullptr);
  madd_v4_v4fl(output, in1, this->m_filter[1]);
  this->m_inputOperation->read(in1, x3, y1, nullptr);
  madd_v4_v4fl(output, in1, this->m_filter[2]);
  this->m_inputOperation->read(in1, x1, y2, nullptr);
  madd_v4_v4fl(output, in1, this->m_filter[3]);
  this->m_inputOperation->read(in2, x2, y2, nullptr);
  madd_v4_v4fl(output, in2, this->m_filter[4]);
  this->m_inputOperation->read(in1, x3, y2, nullptr);
  madd_v4_v4fl(output, in1, this->m_filter[5]);
  this->m_inputOperation->read(in1, x1, y3, nullptr);
  madd_v4_v4fl(output, in1, this->m_filter[6]);
  this->m_inputOperation->read(in1, x2, y3, nullptr);
  madd_v4_v4fl(output, in1, this->m_filter[7]);
  this->m_inputOperation->read(in1, x3, y3, nullptr);
  madd_v4_v4fl(output, in1, this->m_filter[8]);

  output[0] = output[0] * value[0] + in2[0] * mval;
  output[1] = output[1] * value[0] + in2[1] * mval;
  output[2] = output[2] * value[0] + in2[2] * mval;
  output[3] = output[3] * value[0] + in2[3] * mval;

  /* Make sure we don't return negative color. */
  output[0] = MAX2(output[0], 0.0f);
  output[1] = MAX2(output[1], 0.0f);
  output[2] = MAX2(output[2], 0.0f);
  output[3] = MAX2(output[3], 0.0f);
}

bool ConvolutionFilterOperation::determineDependingAreaOfInterest(
    rcti *input, ReadBufferOperation *readOperation, rcti *output)
{
  rcti newInput;
  int addx = (this->m_filterWidth - 1) / 2 + 1;
  int addy = (this->m_filterHeight - 1) / 2 + 1;
  newInput.xmax = input->xmax + addx;
  newInput.xmin = input->xmin - addx;
  newInput.ymax = input->ymax + addy;
  newInput.ymin = input->ymin - addy;

  return NodeOperation::determineDependingAreaOfInterest(&newInput, readOperation, output);
}

void ConvolutionFilterOperation::get_area_of_interest(const int input_idx,
                                                      const rcti &output_area,
                                                      rcti &r_input_area)
{
  switch (input_idx) {
    case IMAGE_INPUT_INDEX: {
      const int add_x = (m_filterWidth - 1) / 2 + 1;
      const int add_y = (m_filterHeight - 1) / 2 + 1;
      r_input_area.xmin = output_area.xmin - add_x;
      r_input_area.xmax = output_area.xmax + add_x;
      r_input_area.ymin = output_area.ymin - add_y;
      r_input_area.ymax = output_area.ymax + add_y;
      break;
    }
    case FACTOR_INPUT_INDEX: {
      r_input_area = output_area;
      break;
    }
  }
}

void ConvolutionFilterOperation::update_memory_buffer_partial(MemoryBuffer *output,
                                                              const rcti &area,
                                                              Span<MemoryBuffer *> inputs)
{
  const MemoryBuffer *image = inputs[IMAGE_INPUT_INDEX];
  const int last_x = getWidth() - 1;
  const int last_y = getHeight() - 1;
  for (BuffersIterator<float> it = output->iterate_with(inputs, area); !it.is_end(); ++it) {
    const int left_offset = (it.x == 0) ? 0 : -image->elem_stride;
    const int right_offset = (it.x == last_x) ? 0 : image->elem_stride;
    const int down_offset = (it.y == 0) ? 0 : -image->row_stride;
    const int up_offset = (it.y == last_y) ? 0 : image->row_stride;

    const float *center_color = it.in(IMAGE_INPUT_INDEX);
    zero_v4(it.out);
    madd_v4_v4fl(it.out, center_color + down_offset + left_offset, m_filter[0]);
    madd_v4_v4fl(it.out, center_color + down_offset, m_filter[1]);
    madd_v4_v4fl(it.out, center_color + down_offset + right_offset, m_filter[2]);
    madd_v4_v4fl(it.out, center_color + left_offset, m_filter[3]);
    madd_v4_v4fl(it.out, center_color, m_filter[4]);
    madd_v4_v4fl(it.out, center_color + right_offset, m_filter[5]);
    madd_v4_v4fl(it.out, center_color + up_offset + left_offset, m_filter[6]);
    madd_v4_v4fl(it.out, center_color + up_offset, m_filter[7]);
    madd_v4_v4fl(it.out, center_color + up_offset + right_offset, m_filter[8]);

    const float factor = *it.in(FACTOR_INPUT_INDEX);
    const float m_factor = 1.0f - factor;
    it.out[0] = it.out[0] * factor + center_color[0] * m_factor;
    it.out[1] = it.out[1] * factor + center_color[1] * m_factor;
    it.out[2] = it.out[2] * factor + center_color[2] * m_factor;
    it.out[3] = it.out[3] * factor + center_color[3] * m_factor;

    /* Make sure we don't return negative color. */
    CLAMP4_MIN(it.out, 0.0f);
  }
}

}  // namespace blender::compositor
