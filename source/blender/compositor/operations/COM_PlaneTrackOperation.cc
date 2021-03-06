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
 * Copyright 2013, Blender Foundation.
 */

#include "COM_PlaneTrackOperation.h"
#include "COM_ReadBufferOperation.h"

#include "MEM_guardedalloc.h"

#include "BLI_listbase.h"
#include "BLI_math.h"
#include "BLI_math_color.h"

#include "BKE_movieclip.h"
#include "BKE_node.h"
#include "BKE_tracking.h"

namespace blender::compositor {

/* ******** PlaneTrackCommon ******** */

PlaneTrackCommon::PlaneTrackCommon()
{
  this->m_movieClip = nullptr;
  this->m_framenumber = 0;
  this->m_trackingObjectName[0] = '\0';
  this->m_planeTrackName[0] = '\0';
}

void PlaneTrackCommon::read_and_calculate_corners(PlaneDistortBaseOperation *distort_op)
{
  float corners[4][2];
  if (distort_op->m_motion_blur_samples == 1) {
    readCornersFromTrack(corners, this->m_framenumber);
    distort_op->calculateCorners(corners, true, 0);
  }
  else {
    const float frame = (float)this->m_framenumber - distort_op->m_motion_blur_shutter;
    const float frame_step = (distort_op->m_motion_blur_shutter * 2.0f) /
                             distort_op->m_motion_blur_samples;
    float frame_iter = frame;
    for (int sample = 0; sample < distort_op->m_motion_blur_samples; sample++) {
      readCornersFromTrack(corners, frame_iter);
      distort_op->calculateCorners(corners, true, sample);
      frame_iter += frame_step;
    }
  }
}

void PlaneTrackCommon::readCornersFromTrack(float corners[4][2], float frame)
{
  MovieTracking *tracking;
  MovieTrackingObject *object;

  if (!this->m_movieClip) {
    return;
  }

  tracking = &this->m_movieClip->tracking;

  object = BKE_tracking_object_get_named(tracking, this->m_trackingObjectName);
  if (object) {
    MovieTrackingPlaneTrack *plane_track;
    plane_track = BKE_tracking_plane_track_get_named(tracking, object, this->m_planeTrackName);
    if (plane_track) {
      float clip_framenr = BKE_movieclip_remap_scene_to_clip_frame(this->m_movieClip, frame);
      BKE_tracking_plane_marker_get_subframe_corners(plane_track, clip_framenr, corners);
    }
  }
}

void PlaneTrackCommon::determineResolution(unsigned int resolution[2],
                                           unsigned int /*preferredResolution*/[2])
{
  resolution[0] = 0;
  resolution[1] = 0;

  if (this->m_movieClip) {
    int width, height;
    MovieClipUser user = {0};
    BKE_movieclip_user_set_frame(&user, this->m_framenumber);
    BKE_movieclip_get_size(this->m_movieClip, &user, &width, &height);
    resolution[0] = width;
    resolution[1] = height;
  }
}

/* ******** PlaneTrackMaskOperation ******** */

void PlaneTrackMaskOperation::init_data()
{
  PlaneDistortMaskOperation::init_data();
  if (execution_model_ == eExecutionModel::FullFrame) {
    PlaneTrackCommon::read_and_calculate_corners(this);
  }
}

/* TODO(manzanilla): to be removed with tiled implementation. */
void PlaneTrackMaskOperation::initExecution()
{
  PlaneDistortMaskOperation::initExecution();
  if (execution_model_ == eExecutionModel::Tiled) {
    PlaneTrackCommon::read_and_calculate_corners(this);
  }
}

/* ******** PlaneTrackWarpImageOperation ******** */

void PlaneTrackWarpImageOperation::init_data()
{
  PlaneDistortWarpImageOperation::init_data();
  if (execution_model_ == eExecutionModel::FullFrame) {
    PlaneTrackCommon::read_and_calculate_corners(this);
  }
}

/* TODO(manzanilla): to be removed with tiled implementation. */
void PlaneTrackWarpImageOperation::initExecution()
{
  PlaneDistortWarpImageOperation::initExecution();
  if (execution_model_ == eExecutionModel::Tiled) {
    PlaneTrackCommon::read_and_calculate_corners(this);
  }
}

}  // namespace blender::compositor
