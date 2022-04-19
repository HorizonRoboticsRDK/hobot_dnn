// Copyright (c) 2020 Horizon Robotics.All Rights Reserved.
//
// The material in this file is confidential and contains trade secrets
// of Horizon Robotics Inc. This is proprietary information owned by
// Horizon Robotics Inc. No part of this work may be disclosed,
// reproduced, copied, transmitted, or used in any way for any purpose,
// without the express written permission of Horizon Robotics Inc.

#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>

#include "input/image_list_data_iterator.h"
#include "rapidjson/document.h"
#include "include/utils/image_utils.h"

DEFINE_AND_REGISTER_DATA_ITERATOR(image, ImageListDataIterator)

/**
 * parse image list from config file
 * @param[in] list_file image list config file
 * @param[out] files list of images
 * @return
 */
static bool ParseImageList(const std::string &list_file,
                           std::vector<std::string> &files)
{
  std::ifstream lst_ifstream(list_file);
  if (!lst_ifstream)
  {
    RCLCPP_ERROR(rclcpp::get_logger("example"),
        "open image list file : %s failed!", list_file.c_str());
    return false;
  }
  std::string line;
  while (std::getline(lst_ifstream, line))
  {
    std::istringstream ss(line);
    std::string temp;
    std::getline(ss, temp, ' ');
    if (!temp.empty())
    {
      RCLCPP_DEBUG(rclcpp::get_logger("example"),
        "get Image file : %s", temp.c_str());
      files.push_back(std::move(temp));
    }
  }
  lst_ifstream.close();
  return true;
}

NV12PyramidInputPtr ImageListDataIterator::PraseLocalImageToMemory(
  int image_type,
  std::string imagefilepath)
{
  if (access(imagefilepath.c_str(), R_OK) == -1)
  {
    RCLCPP_ERROR(rclcpp::get_logger("example"),
          "Imagefile:%s is not exist", imagefilepath.c_str());
    return nullptr;
  }

  NV12PyramidInputPtr pyramid = nullptr;
  if (static_cast<int>(ImageType::BGR) == image_type)
  {
    // bgr img，支持将图片resize到模型输入size
    pyramid = ImageUtils::GetNV12Pyramid(imagefilepath, ImageType::BGR,
      model_input_height_, model_input_width_);
    if (!pyramid)
    {
      RCLCPP_ERROR(rclcpp::get_logger("example"),
        "Get Nv12 pym fail with image: %s", imagefilepath.c_str());
      return nullptr;
    }
  } else if (static_cast<int>(ImageType::NV12) == image_type) {
    // nv12 img，不支持resize，图片size必须和模型输入size一致
    pyramid = ImageUtils::GetNV12Pyramid(imagefilepath, ImageType::NV12,
      model_input_height_, model_input_width_);
    if (!pyramid)
    {
      RCLCPP_ERROR(rclcpp::get_logger("example"),
        "Get Nv12 pym fail with image: %s", imagefilepath.c_str());
      return nullptr;
    }
  } else {
    RCLCPP_ERROR(rclcpp::get_logger("example"),
      "Invalid image type: %d", image_type);
    return nullptr;
  }
  if (pyramid)
  {
    RCLCPP_DEBUG(rclcpp::get_logger("example"),
      "Image: %s load success!", imagefilepath.c_str());
  }
  return pyramid;
}

int ImageListDataIterator::Init(std::string config_file,
                                std::string config_string)
{
  int ret_code = DataIterator::Init(config_file, config_string);
  if (ret_code != 0)
  {
    return -1;
  }

  if (!ParseImageList(image_list_file_, image_files_))
  {
    RCLCPP_ERROR(rclcpp::get_logger("example"), "Parse image list error!");
  }

  if (cache_able_)
  {
    for (size_t i = 0; i < max_cache_size_; i++)
    {
      if (i >= image_files_.size()) break;
      NV12PyramidInputPtr pyramid =
            PraseLocalImageToMemory(data_type_, image_files_[i]);
      if (pyramid)
      {
        cache_.push_back(std::move(pyramid));
      }
    }
  }
  return 0;
}

bool ImageListDataIterator::Next(NV12PyramidInputPtr &pyramid)
{
  if (last_frame_id + 1 >= max_frame_count_)
  {
    is_finish_ = true;
    return false;
  }

  if (cache_able_)
  {
    if (send_index_ >= cache_.size())
    {
      is_finish_ = true;
      return false;
    }
    pyramid = cache_[send_index_++];
    if (loop_able_)
    {
      send_index_ = send_index_ % cache_.size();
    }
  } else {
    if (send_index_ >= image_files_.size())
    {
      is_finish_ = true;
      return false;
    }
    std::string image_file = image_files_[send_index_++];
    pyramid = PraseLocalImageToMemory(data_type_, image_file);

    if (loop_able_)
    {
      send_index_ = send_index_ % image_files_.size();
    }
  }
  return true;
}

int ImageListDataIterator::LoadConfig(std::string &config_string)
{
  rapidjson::Document document;
  document.Parse(config_string.data());

  if (document.HasParseError())
  {
    RCLCPP_ERROR(rclcpp::get_logger("example"), "Parsing config file failed!");
    return -1;
  }

  if (document.HasMember("image_list_file"))
  {
    image_list_file_ = document["image_list_file"].GetString();
    RCLCPP_DEBUG(rclcpp::get_logger("example"),
        "image_list_file: %s", image_list_file_.c_str());
  }

  if (document.HasMember("width"))
  {
    width_ = document["width"].GetInt();
  }

  if (document.HasMember("height"))
  {
    height_ = document["height"].GetInt();
  }

  if (document.HasMember("need_pre_load"))
  {
    cache_able_ = document["need_pre_load"].GetBool();
  }

  if (document.HasMember("need_loop"))
  {
    loop_able_ = document["need_loop"].GetBool();
  }

  if (document.HasMember("max_frame_count"))
  {
    max_frame_count_ = document["max_frame_count"].GetInt();
  }

  if (document.HasMember("max_cache")) {
    max_cache_size_ = document["max_cache"].GetInt();
  }

  if (document.HasMember("data_type"))
  {
    data_type_ = document["data_type"].GetInt();
  }

  return 0;
}

ImageListDataIterator::~ImageListDataIterator() {}

bool ImageListDataIterator::HasNext() { return !is_finish_; }