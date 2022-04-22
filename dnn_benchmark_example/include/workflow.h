// Copyright (c) 2020 Horizon Robotics.All Rights Reserved.
//
// The material in this file is confidential and contains trade secrets
// of Horizon Robotics Inc. This is proprietary information owned by
// Horizon Robotics Inc. No part of this work may be disclosed,
// reproduced, copied, transmitted, or used in any way for any purpose,
// without the express written permission of Horizon Robotics Inc.

#ifndef _WORKFLOW_H_
#define _WORKFLOW_H_

#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include "rclcpp/rclcpp.hpp"
#include "include/input/input_data.h"
#include "include/plugin/base_plugin.h"
#include "include/utils/pc_queue.h"
#include "include/utils/utils.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/writer.h"
#include "include/utils/image_utils.h"
#define EMPTY ""

using rclcpp::NodeOptions;

using hobot::dnn_node::DNNInput;
using hobot::dnn_node::DnnNode;
using hobot::dnn_node::DnnNodeOutput;
using hobot::dnn_node::DnnNodePara;
using hobot::dnn_node::DNNResult;
using hobot::dnn_node::ModelTaskType;
using hobot::dnn_node::TaskId;

using hobot::dnn_node::Model;
using hobot::dnn_node::ModelInferTask;
using hobot::dnn_node::ModelManager;
using hobot::dnn_node::ModelRoiInferTask;

struct FasterRcnnOutput : public DnnNodeOutput
{
  uint64_t Predict_Start_time;
  uint64_t Predict_duration;
};

class Workflow : public DnnNode
{
 public:
  Workflow(const std::string node_name = "dnn_benchamark_node",
                 const NodeOptions &options = NodeOptions());
  ~Workflow() override;

  int WorkflowInit();

  int WorkflowStart();

  int FeedWorkflow(NV12PyramidInputPtr pyramid);

  void WorkflowRun();

  int Stop();

 protected:
  int SetNodePara() override;
  int SetOutputParser() override;

  int PostProcess(const std::shared_ptr<DnnNodeOutput> &outputs)
      override;

 private:
  bool stop_ = false;
  std::shared_ptr<std::thread> thread_;

  int is_sync_mode_ = 1;

  PCQueue<NV12PyramidInputPtr> pc_queue_;
  std::string model_file_name_ = "config/multitask_body_kps_960x544.hbm";
  std::string model_name_ = "multitask_body_kps_960x544";
  ModelTaskType model_task_type_ = ModelTaskType::ModelInferType;

  int model_input_width_ = -1;
  int model_input_height_ = -1;
  int32_t model_output_count_ = 1;
  // box output index is 1
  // const int32_t box_output_index_ = 1;
  // kps output index is 2
  const int32_t output_index_ = 1;

  int show_fps_log = 1;
  int show_latency_log = 1;
  std::string config_file = "config/hobot_benchmark_config.json";
  int statistic_cycle = 500;
  uint64_t frame_cnt = 0;
  std::mutex frame_cnt_mtx;

  int Predict(std::vector<std::shared_ptr<DNNInput>> &inputs,
    const std::shared_ptr<std::vector<hbDNNRoi>> rois,
    std::shared_ptr<DnnNodeOutput> dnn_output);
};

#endif  // _WORKFLOW_H_
