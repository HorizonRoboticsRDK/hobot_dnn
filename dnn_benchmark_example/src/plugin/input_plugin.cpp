// Copyright (c) 2020 Horizon Robotics.All Rights Reserved.
//
// The material in this file is confidential and contains trade secrets
// of Horizon Robotics Inc. This is proprietary information owned by
// Horizon Robotics Inc. No part of this work may be disclosed,
// reproduced, copied, transmitted, or used in any way for any purpose,
// without the express written permission of Horizon Robotics Inc.

#include "plugin/input_plugin.h"
#include "rapidjson/document.h"

int InputProducerPlugin::Init(std::string config_file,
                              std::string config_string)
{
  return BasePlugin::Init(config_file, config_string);
}

void InputProducerPlugin::registerWork(Workflow *work_)
{
  this->work = work_;
}

void InputProducerPlugin::set_model_input_width_height(int model_input_w,
                                                      int model_input_h)
{
  data_iterator_->set_model_input_width_height(model_input_w, model_input_h);
}

void InputProducerPlugin::Run()
{
  while (data_iterator_->HasNext() && !stop_)
  {
    NV12PyramidInputPtr pyramid = nullptr;
    if (!data_iterator_->Next(pyramid))
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    std::unique_lock<std::mutex> lk(m_);
    cv_.wait(lk, [&] { return produced_count_ - released_count_ < limit_; });
    RCLCPP_DEBUG(rclcpp::get_logger("example"),
      "Run input msg, id: %d", produced_count_);
    produced_count_++;
    work->FeedWorkflow(pyramid);
  }
  RCLCPP_DEBUG(rclcpp::get_logger("example"),
      "InputProducerPlugin Run finish");
}

int InputProducerPlugin::Release()
{
  std::unique_lock<std::mutex> lk(m_);
  released_count_++;
  lk.unlock();
  cv_.notify_one();
  return 0;
}

int InputProducerPlugin::Start()
{
  stop_ = false;
  produce_thread_ =
      std::make_shared<std::thread>(&InputProducerPlugin::Run, this);
  if (!produce_thread_)
  {
    RCLCPP_ERROR(rclcpp::get_logger("example"), "Start Input thread failed");
    return -1;
  }
  RCLCPP_INFO(rclcpp::get_logger("example"), "InputProducerPlugin start");
  return 0;
}

bool InputProducerPlugin::IsRunning()
{
  return !stop_ && data_iterator_->HasNext();
}

int InputProducerPlugin::Stop()
{
  stop_ = true;
  while (released_count_ < produced_count_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  if (produce_thread_) {
    if (produce_thread_->joinable()) {
      produce_thread_->join();
    }
  }
  RCLCPP_INFO(rclcpp::get_logger("example"), "InputProducerPlugin stop");
  return 0;
}

int InputProducerPlugin::LoadConfig(std::string &config_string)
{
  rapidjson::Document document;
  document.Parse(config_string.data());

  if (document.HasParseError())
  {
    RCLCPP_ERROR(rclcpp::get_logger("example"), "Parsing config file failed");
    return -1;
  }

  if (document.HasMember("limit"))
  {
    limit_ = document["limit"].GetInt();
  }

  if (document.HasMember("input_type"))
  {
    std::string input_type = document["input_type"].GetString();
    data_iterator_ = DataIterator::GetImpl(input_type);
  } else {
    RCLCPP_ERROR(rclcpp::get_logger("example"),
      "input config don not have parameter input_type! please check!");
    return -1;
  }

  return data_iterator_->Init("", config_string);
}

InputProducerPlugin::~InputProducerPlugin()
{
  if (data_iterator_)
  {
    delete data_iterator_;
    data_iterator_ = nullptr;
  }
}
