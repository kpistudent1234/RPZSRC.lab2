#ifndef POINT_CLOUD_ADAPTER_HPP_
#define POINT_CLOUD_ADAPTER_HPP_

#include <iostream>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/type_adapter.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>

template <>
struct rclcpp::TypeAdapter<pcl::PointCloud<pcl::PointXYZ>, sensor_msgs::msg::PointCloud2> {
    using is_specialized = std::true_type;
    using custom_type = pcl::PointCloud<pcl::PointXYZ>;
    using ros_message_type = sensor_msgs::msg::PointCloud2;

    static void convert_to_ros_message(
        const custom_type & source,
        ros_message_type & destination)
    {
        std::cout << ">>> [TypeAdapter]: convert_to_ros_message ВИКЛИКАНО! (Виконується конвертація) <<<\n";
        pcl::toROSMsg(source, destination);
    }

    static void convert_to_custom(
        const ros_message_type & source,
        custom_type & destination)
    {
        std::cout << ">>> [TypeAdapter]: convert_to_custom ВИКЛИКАНО! (Виконується десеріалізація) <<<\n";
        pcl::fromROSMsg(source, destination);
    }
};

using AdaptedPointCloud = rclcpp::TypeAdapter<pcl::PointCloud<pcl::PointXYZ>, sensor_msgs::msg::PointCloud2>;

#endif
