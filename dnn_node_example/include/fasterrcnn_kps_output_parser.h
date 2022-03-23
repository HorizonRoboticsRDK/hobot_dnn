// Copyright (c) 2021 Horizon Robotics.All Rights Reserved.
//
// The material in this file is confidential and contains trade secrets
// of Horizon Robotics Inc. This is proprietary information owned by
// Horizon Robotics Inc. No part of this work may be disclosed,
// reproduced, copied, transmitted, or used in any way for any purpose,
// without the express written permission of Horizon Robotics Inc.

#ifndef FASTERRCNN_KPS_OUTPUT_PARSER_H
#define FASTERRCNN_KPS_OUTPUT_PARSER_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "easy_dnn/data_structure.h"
#include "easy_dnn/model.h"
#include "easy_dnn/output_parser.h"
#include "easy_dnn/output_parser/detection/filter2d_output_parser.h"

using hobot::easy_dnn::MultiBranchOutputParser;
using hobot::easy_dnn::DNNResult;
using hobot::easy_dnn::DNNTensor;
using hobot::easy_dnn::InputDescription;
using hobot::easy_dnn::OutputDescription;
using hobot::easy_dnn::Model;
using hobot::easy_dnn::Filter2DResult;

/**
 * \~Chinese @brief 2D坐标点
 */
template <typename Dtype>
struct Point_ {
  inline Point_() {}
  inline Point_(Dtype x_, Dtype y_, float score_ = 0.0)
      : x(x_), y(y_), score(score_) {}

  Dtype x = 0;
  Dtype y = 0;
  float score = 0.0;
};
typedef Point_<float> Point;
typedef std::vector<Point> Landmarks;

class LandmarksResult : public DNNResult {
 public:
  std::vector<Landmarks> values;

  void Reset() override { values.clear(); }
};

class DetectOutDesc : public OutputDescription {
 public:
  DetectOutDesc(Model *mode, int index, std::string type = "detection")
          : OutputDescription(mode, index, type) { }

  uint32_t output_size;
};

struct FasterRcnnKpsParserPara {
  int kps_points_number_ = 19;
  float kps_pos_distance_ = 0.1;
  float kps_anchor_param_ = -0.46875;
  int kps_feat_width_ = 16;
  int kps_feat_height_ = 16;
  std::vector<int> aligned_kps_dim;
  std::vector<uint32_t> kps_shifts_;
};

class FasterRcnnKpsOutputParser : public MultiBranchOutputParser {
 public:
  FasterRcnnKpsOutputParser(
    std::shared_ptr<FasterRcnnKpsParserPara> parser_para) {
    if (parser_para) {
      parser_para_ = parser_para;
    }
  }
  ~FasterRcnnKpsOutputParser() {}

  int32_t Parse(
      std::shared_ptr<DNNResult>& output,
      std::vector<std::shared_ptr<InputDescription>>& input_descriptions,
      std::shared_ptr<OutputDescription>& output_descriptions,
      std::shared_ptr<DNNTensor>& output_tensor,
      std::vector<std::shared_ptr<OutputDescription>>& depend_output_descs,
      std::vector<std::shared_ptr<DNNTensor>>& depend_output_tensors,
      std::vector<std::shared_ptr<DNNResult>>& depend_outputs) override;

 private:
  std::shared_ptr<FasterRcnnKpsParserPara> parser_para_ = nullptr;
};

#endif  // FASTERRCNN_KPS_OUTPUT_PARSER_H