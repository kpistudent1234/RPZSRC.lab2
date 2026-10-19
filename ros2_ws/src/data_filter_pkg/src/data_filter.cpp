#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

#include "rcl/service_introspection.h"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp/qos_overriding_options.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/empty.hpp"

class DataFilterNode : public rclcpp::Node {
public:
    DataFilterNode() : Node("data_filter") {
        // QoS та OverridingOptions для видавця
        rclcpp::QoS qos_profile(1);
        qos_profile.transient_local();
        qos_profile.reliable();

        rclcpp::PublisherOptions pub_options;
        pub_options.qos_overriding_options = rclcpp::QosOverridingOptions{
            rclcpp::QosPolicyKind::Depth,
            rclcpp::QosPolicyKind::Reliability,
            rclcpp::QosPolicyKind::Durability
        };

        // Matched callback для Lazy Publisher
        pub_options.event_callbacks.matched_callback = 
            [this](rclcpp::MatchedInfo & info) {
                RCLCPP_INFO(
                    this->get_logger(),
                    "Подія matched_callback: кількість слухачів на uppercase_topic = %zu",
                    info.current_count
                );

                if (info.current_count > 0) {
                    if (!sub_) {
                        RCLCPP_INFO(
                            this->get_logger(),
                            "З'явився слухач на виході. Створюємо підписку на input_topic (Lazy Publisher активовано)."
                        );
                        sub_ = this->create_subscription<std_msgs::msg::String>(
                            "input_topic", 10,
                            std::bind(&DataFilterNode::input_callback, this, std::placeholders::_1)
                        );
                    }
                } else {
                    if (sub_) {
                        RCLCPP_INFO(
                            this->get_logger(),
                            "Кількість слухачів = 0. Видаляємо підписку на input_topic для економії ресурсів (Lazy Publisher)."
                        );
                        sub_.reset();
                    }
                }
            };

        pub_ = this->create_publisher<std_msgs::msg::String>(
            "uppercase_topic", qos_profile, pub_options
        );

        // 1. Створення сервісного сервера ResetFilter
        reset_service_ = this->create_service<std_srvs::srv::Empty>(
            "reset_filter",
            [this](const std::shared_ptr<std_srvs::srv::Empty::Request> request,
                   std::shared_ptr<std_srvs::srv::Empty::Response> response) {
                (void)request;
                (void)response;
                processed_count_ = 0;
                RCLCPP_INFO(
                    this->get_logger(),
                    "Сервіс ResetFilter викликано! Лічильник оброблених повідомлень скинуто в 0."
                );
            }
        );

        // 2. Увімкнення інтроспекції сервісу зі станом RCL_SERVICE_INTROSPECTION_CONTENTS
        reset_service_->configure_introspection(
            this->get_clock(),
            rclcpp::SystemDefaultsQoS(),
            RCL_SERVICE_INTROSPECTION_CONTENTS
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Вузол data_filter запущено. Сервіс 'reset_filter' активний з інтроспекцією CONTENTS."
        );
    }

private:
    void input_callback(const std_msgs::msg::String::SharedPtr msg) {
        processed_count_++;
        std::string processed_data = msg->data;
        std::transform(
            processed_data.begin(), processed_data.end(), processed_data.begin(),
            [](unsigned char c) { return std::toupper(c); }
        );

        auto out_msg = std_msgs::msg::String();
        out_msg.data = processed_data;

        RCLCPP_INFO(
            this->get_logger(),
            "[#%zu] Отримано: '%s' -> Оброблено та опубліковано: '%s'",
            processed_count_, msg->data.c_str(), out_msg.data.c_str()
        );

        pub_->publish(out_msg);
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_{nullptr};
    rclcpp::Service<std_srvs::srv::Empty>::SharedPtr reset_service_;
    size_t processed_count_{0};
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<DataFilterNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
