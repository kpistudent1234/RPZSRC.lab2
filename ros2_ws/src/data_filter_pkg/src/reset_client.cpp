#include <chrono>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/empty.hpp"

using namespace std::chrono_literals;

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared("reset_client");

    auto client = node->create_client<std_srvs::srv::Empty>("reset_filter");

    RCLCPP_INFO(node->get_logger(), "Очікування доступності сервісу 'reset_filter'...");
    if (!client->wait_for_service(3s)) {
        RCLCPP_ERROR(node->get_logger(), "Сервіс 'reset_filter' недоступний! Завершення роботи.");
        rclcpp::shutdown();
        return 1;
    }

    auto request = std::make_shared<std_srvs::srv::Empty::Request>();
    RCLCPP_INFO(node->get_logger(), "Надсилаємо асинхронний запит async_send_request до сервісу...");

    // Запуск обробки подій клієнта в окремому фоновому потоці
    std::thread spin_thread([node]() {
        rclcpp::spin(node);
    });

    // Асинхронний виклик сервісу
    auto future_result = client->async_send_request(request);

    // Контроль таймауту через future.wait_for(...)
    std::chrono::seconds timeout(3);
    if (future_result.wait_for(timeout) == std::future_status::ready) {
        RCLCPP_INFO(node->get_logger(), "Успіх! Отримано підтвердження від сервісу ResetFilter.");
    } else {
        RCLCPP_ERROR(node->get_logger(), "Помилка: минув таймаут очікування відповіді від сервісу!");
    }

    rclcpp::shutdown();
    if (spin_thread.joinable()) {
        spin_thread.join();
    }

    return 0;
}
