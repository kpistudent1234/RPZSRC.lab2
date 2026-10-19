#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "point_cloud_adapter.hpp"

class PointCloudProducer : public rclcpp::Node {
public:
    explicit PointCloudProducer(const rclcpp::NodeOptions & options)
    : Node("point_cloud_producer", options)
    {
        pub_ = this->create_publisher<AdaptedPointCloud>("point_cloud_topic", 10);
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(500),
            std::bind(&PointCloudProducer::on_timer, this)
        );
    }

private:
    void on_timer() {
        auto cloud = std::make_unique<pcl::PointCloud<pcl::PointXYZ>>();
        cloud->header.frame_id = "lidar_frame";
        cloud->points.push_back(pcl::PointXYZ(1.0f, 2.0f, 3.0f));

        RCLCPP_INFO(
            this->get_logger(),
            "[Producer] Публікація хмари точок. Адреса в пам'яті: %p",
            static_cast<void*>(cloud.get())
        );

        pub_->publish(std::move(cloud));
    }

    rclcpp::Publisher<AdaptedPointCloud>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

class PointCloudConsumer : public rclcpp::Node {
public:
    explicit PointCloudConsumer(const rclcpp::NodeOptions & options)
    : Node("point_cloud_consumer", options)
    {
        sub_ = this->create_subscription<AdaptedPointCloud>(
            "point_cloud_topic", 10,
            [this](std::unique_ptr<pcl::PointCloud<pcl::PointXYZ>> cloud) {
                RCLCPP_INFO(
                    this->get_logger(),
                    "[Consumer] Отримано хмару! Кількість точок: %zu. Адреса в пам'яті: %p",
                    cloud->points.size(),
                    static_cast<void*>(cloud.get())
                );
            }
        );
    }

private:
    rclcpp::Subscription<AdaptedPointCloud>::SharedPtr sub_;
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);

    bool use_intra = false;
    if (argc > 1 && std::string(argv[1]) == "--intra") {
        use_intra = true;
    }

    rclcpp::NodeOptions options;
    options.use_intra_process_comms(use_intra);

    std::cout << "\n=======================================================\n";
    std::cout << "РЕЖИМ INTRA-PROCESS COMMUNICATION: " << (use_intra ? "УВІМКНЕНО (TRUE)" : "ВИМКНЕНО (FALSE)") << "\n";
    std::cout << "=======================================================\n\n";

    auto producer = std::make_shared<PointCloudProducer>(options);
    auto consumer = std::make_shared<PointCloudConsumer>(options);

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(producer);
    executor.add_node(consumer);

    // Працюємо 2 секунди для наочного зняття логу
    auto start = std::chrono::steady_clock::now();
    while (rclcpp::ok() && (std::chrono::steady_clock::now() - start < std::chrono::seconds(2))) {
        executor.spin_some();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    rclcpp::shutdown();
    return 0;
}
