// Copyright (c) 2022，Horizon Robotics.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _DETECTION_PTQ_YOLO5_OUTPUT_PARSER_H_
#define _DETECTION_PTQ_YOLO5_OUTPUT_PARSER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dnn/hb_dnn_ext.h"
#include "dnn_node/dnn_node_data.h"
#include "dnn_node/util/output_parser/perception_common.h"

namespace hobot {
namespace dnn_node {

/**
 * Config definition for Yolo5
 */
struct PTQYolo5Config {
  std::vector<int> strides;
  std::vector<std::vector<std::pair<double, double>>> anchors_table;
  int class_num;
  std::vector<std::string> class_names;
  std::vector<std::vector<float>> dequantize_scale;

  std::string Str() {
    std::stringstream ss;
    ss << "strides: ";
    for (const auto &stride : strides) {
      ss << stride << " ";
    }

    ss << "; anchors_table: ";
    for (const auto &anchors : anchors_table) {
      for (auto data : anchors) {
        ss << "[" << data.first << "," << data.second << "] ";
      }
    }
    ss << "; class_num: " << class_num;
    return ss.str();
  }
};

extern PTQYolo5Config default_ptq_yolo5_config;

class Yolo5AssistParser : public SingleBranchOutputParser<Dnn_Parser_Result> {
 public:
  int32_t Parse(
      std::shared_ptr<Dnn_Parser_Result> &output,
      std::vector<std::shared_ptr<InputDescription>> &input_descriptions,
      std::shared_ptr<OutputDescription> &output_description,
      std::shared_ptr<DNNTensor> &output_tensor) override {
    return 0;
  }
};

class Yolo5OutputParser : public MultiBranchOutputParser<Dnn_Parser_Result> {
 public:
  int32_t Parse(
      std::shared_ptr<Dnn_Parser_Result> &output,
      std::vector<std::shared_ptr<InputDescription>> &input_descriptions,
      std::shared_ptr<OutputDescription> &output_descriptions,
      std::shared_ptr<DNNTensor> &output_tensor,
      std::vector<std::shared_ptr<OutputDescription>> &depend_output_descs,
      std::vector<std::shared_ptr<DNNTensor>> &depend_output_tensors,
      std::vector<std::shared_ptr<DNNResult>> &depend_outputs) override;

 private:
  int PostProcess(std::vector<std::shared_ptr<DNNTensor>> &output_tensors,
                  Perception &perception);

  void PostProcess(std::shared_ptr<DNNTensor> tensor,
                   int layer,
                   std::vector<Detection> &dets);

  double Dequanti(int32_t data,
                  int layer,
                  bool big_endian,
                  int offset,
                  hbDNNTensorProperties &properties);

 private:
  PTQYolo5Config yolo5_config_ = default_ptq_yolo5_config;
  float score_threshold_ = 0.4;
  float nms_threshold_ = 0.5;
  int nms_top_k_ = 5000;
  bool has_dequanti_node_ = true;
  std::string dequanti_file_ = "";
};

}  // namespace dnn_node
}  // namespace hobot
#endif  // _DETECTION_PTQ_YOLO5_OUTPUT_PARSER_H_