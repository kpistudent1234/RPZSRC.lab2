#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "data_filter_interfaces/action/process_data.hpp"

class ProcessDataActionServer : public rclcpp::Node {
public:
    using ProcessData = data_filter_interfaces::action::ProcessData;
    using GoalHandleProcessData = rclcpp_action::ServerGoalHandle<ProcessData>;

    explicit ProcessDataActionServer(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
    : Node("process_data_action_server", options)
    {
        using namespace std::placeholders;

        this->action_server_ = rclcpp_action::create_server<ProcessData>(
            this,
            "process_data",
            std::bind(&ProcessDataActionServer::handle_goal, this, _1, _2),
            std::bind(&ProcessDataActionServer::handle_cancel, this, _1),
            std::bind(&ProcessDataActionServer::handle_accepted, this, _1)
        );

        RCLCPP_INFO(this->get_logger(), "Action-сервер 'process_data' запущено і готовий приймати завдання.");
    }

private:
    rclcpp_action::Server<ProcessData>::SharedPtr action_server_;

    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const ProcessData::Goal> goal)
    {
        (void)uuid;
        RCLCPP_INFO(this->get_logger(), "Отримано запит на обробку масиву з %zu елементів.", goal->data.size());
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<GoalHandleProcessData> goal_handle)
    {
        (void)goal_handle;
        RCLCPP_INFO(this->get_logger(), "Отримано запит на скасування обробки.");
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accepted(const std::shared_ptr<GoalHandleProcessData> goal_handle)
    {
        // Виконання завдання в окремому потоці для неблокуючої роботи вузла
        std::thread{std::bind(&ProcessDataActionServer::execute, this, goal_handle)}.detach();
    }

    void execute(const std::shared_ptr<GoalHandleProcessData> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Початок асинхронної обробки масиву даних в окремому потоці...");
        const auto goal = goal_handle->get_goal();
        auto feedback = std::make_shared<ProcessData::Feedback>();
        auto result = std::make_shared<ProcessData::Result>();

        const auto & input_data = goal->data;
        std::vector<int32_t> processed_data;
        size_t total = input_data.size();

        for (size_t i = 0; i < total; ++i) {
            if (goal_handle->is_canceling()) {
                result->result_data = processed_data;
                goal_handle->canceled(result);
                RCLCPP_INFO(this->get_logger(), "Виконання дії було скасовано клієнтом.");
                return;
            }

            // Обробка елемента (наприклад, множення на 2)
            processed_data.push_back(input_data[i] * 2);

            // Розрахунок та публікація періодичного фідбеку
            int progress = static_cast<int>((static_cast<double>(i + 1) / total) * 100.0);
            feedback->progress = progress;
            goal_handle->publish_feedback(feedback);

            RCLCPP_INFO(
                this->get_logger(),
                "Публікація фідбеку: прогрес = %d%% (оброблено %zu/%zu)",
                progress, i + 1, total
            );

            // Імітація тривалої обчислювальної операції (1 секунда на елемент)
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Успішне завершення дії
        if (rclcpp::ok()) {
            result->result_data = processed_data;
            goal_handle->succeed(result);
            RCLCPP_INFO(this->get_logger(), "Обробку масиву завершено! Досягнуто стану Succeeded.");
        }
    }
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ProcessDataActionServer>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
