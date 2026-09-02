#include <memory>
#include <random>
#include <sbg_driver/msg/detail/sbg_ekf_quat__struct.hpp>
#include <std_msgs/msg/detail/float64__struct.hpp>
#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "sensor_msgs/msg/nav_sat_fix.hpp"
#include "sensor_msgs/msg/nav_sat_status.hpp"
#include "std_msgs/msg/float64.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Matrix3x3.h"
#include "sbg_driver/msg/sbg_ekf_quat.hpp"

using namespace std::chrono_literals;

class QuatConversionNode : public rclcpp::Node
{
public:
  QuatConversionNode()
  : Node("quat_conversion_node")
  {
    // Create Publisher for the extracted Z angle (Yaw)
    publisher_ = this->create_publisher<sbg_driver::msg::SbgEkfQuat>(
      "/sbg/ekf_quat", 10);

    publisher_2 = this->create_publisher<std_msgs::msg::Float64>(
      "/mausv/distance", 10);

    // Create Subscription to the Odometry topic
    subscription_ = this->create_subscription<nav_msgs::msg::Odometry>(
      "/gz/odometry", 10, 
      std::bind(&QuatConversionNode::odom_callback, this, std::placeholders::_1));

    // GPS conversion

    // Initialize Parameters for the reference origin (using values sarasota)
    this->declare_parameter("ref_lat", 27.37502785979457);
    this->declare_parameter("ref_lon", -82.45251868595196);
    this->declare_parameter("ref_alt", -20.740567168107255);
    
    // Extracted variances from gps measurements (based on SBG performance)
    this->declare_parameter("variance_x", 0.28);
    this->declare_parameter("variance_y", 0.30);
    this->declare_parameter("variance_z", 3.50);

    timer_gps = this->create_wall_timer(
      200ms, std::bind(&QuatConversionNode::gps_callback, this)
    );

    // Initialize publisher
    gps_pub = this->create_publisher<sensor_msgs::msg::NavSatFix>("/imu/nav_sat_fix",10);

    // Initialize random number generator for Gaussian noise
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    generator_.seed(seed);

  }

private:
  void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {

    RCLCPP_INFO(this->get_logger(), "Calling back");
    // 1. Extract the quaternion from the Odometry message
    tf2::Quaternion q(
      msg->pose.pose.orientation.x,
      msg->pose.pose.orientation.y,
      msg->pose.pose.orientation.z,
      msg->pose.pose.orientation.w);

    sbg_driver::msg::SbgEkfQuat out_msg = sbg_driver::msg::SbgEkfQuat();

    

    out_msg.quaternion.x = q.getX();
    out_msg.quaternion.y = q.getY();
    out_msg.quaternion.z = q.getZ();
    out_msg.quaternion.w = q.getW();


    // 4. Publish the angle
    publisher_->publish(out_msg);

    double current_x = msg->pose.pose.position.x;
    double distance = 18.3-current_x;
    std_msgs::msg::Float64 msg_distance = std_msgs::msg::Float64();
    msg_distance.data = distance;
    publisher_2->publish(msg_distance);

    latest_odom_ = msg;

  }

  void gps_callback(){
    // Wait for first odometry measurement
    if(!latest_odom_){
      return;
    }

    // Retrieve parameters
    double ref_lat = this->get_parameter("ref_lat").as_double();
    double ref_lon = this->get_parameter("ref_lon").as_double();
    double ref_alt = this->get_parameter("ref_alt").as_double();
    double var_x = this->get_parameter("variance_x").as_double();
    double var_y = this->get_parameter("variance_y").as_double();
    double var_z = this->get_parameter("variance_z").as_double();

    // Create Normal Distributions for noise (std_dev = sqrt(variance))
    std::normal_distribution<double> noise_x(0.0, std::sqrt(var_x));
    std::normal_distribution<double> noise_y(0.0, std::sqrt(var_y));
    std::normal_distribution<double> noise_z(0.0, std::sqrt(var_z));

    // Extract Odometry positions (Assuming standard ENU: x=East, y=North, z=Up)
    double odom_x = latest_odom_->pose.pose.position.x;
    double odom_y = latest_odom_->pose.pose.position.y;
    double odom_z = latest_odom_->pose.pose.position.z;

    // Apply noise
    double noisy_x = odom_x + noise_x(generator_);
    double noisy_y = odom_y + noise_y(generator_);
    double noisy_z = odom_z + noise_z(generator_);

    // Earth radius in meters
    const double R = 6378137.0; 
    const double PI = 3.14159265358979323846;

    // Convert Cartesian (m) to Geodetic (Lat/Lon/Alt)
    double delta_lat = (noisy_y / R) * (180.0 / PI);
    double delta_lon = (noisy_x / (R * std::cos(ref_lat * PI / 180.0))) * (180.0 / PI);

    auto gps_msg = sensor_msgs::msg::NavSatFix();

    // Set Header
    gps_msg.header.stamp = this->now();
    gps_msg.header.frame_id = "imu_link_ned";

    // Set Status matching your sample
    gps_msg.status.status = sensor_msgs::msg::NavSatStatus::STATUS_FIX; // 1
    gps_msg.status.service = sensor_msgs::msg::NavSatStatus::SERVICE_GLONASS; // 2 (Based on your output)

    // Set Coordinates
    gps_msg.latitude = ref_lat + delta_lat;
    gps_msg.longitude = ref_lon + delta_lon;
    gps_msg.altitude = ref_alt + noisy_z;

    // Set Covariance (Row-major: [xx, xy, xz, yx, yy, yz, zx, zy, zz])
    gps_msg.position_covariance.fill(0.0);
    gps_msg.position_covariance[0] = var_x; // East/East
    gps_msg.position_covariance[4] = var_y; // North/North
    gps_msg.position_covariance[8] = var_z; // Up/Up
    gps_msg.position_covariance_type = sensor_msgs::msg::NavSatFix::COVARIANCE_TYPE_DIAGONAL_KNOWN; // 2

    gps_pub->publish(gps_msg);
  }

  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr subscription_;
  rclcpp::Publisher<sbg_driver::msg::SbgEkfQuat>::SharedPtr publisher_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr publisher_2;
  rclcpp::Publisher<sensor_msgs::msg::NavSatFix>::SharedPtr gps_pub;
  rclcpp::TimerBase::SharedPtr timer_gps;
  nav_msgs::msg::Odometry::SharedPtr latest_odom_;

  std::default_random_engine generator_;
  
  
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<QuatConversionNode>());
  rclcpp::shutdown();
  return 0;
}