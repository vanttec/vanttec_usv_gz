#include "rclcpp/rclcpp.hpp"
#include "usv_interfaces/msg/usv_loc.hpp"
#include "geometry_msgs/msg/pose2_d.hpp" // x, y, theta pose
#include <geometry_msgs/msg/detail/pose2_d__struct.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/subscription.hpp>
#include <rclcpp/timer.hpp>
#include <std_msgs/msg/detail/float64__struct.hpp>
#include <std_msgs/msg/float64.hpp>
#include <usv_interfaces/msg/detail/usv_loc__struct.hpp>
#include <vector>
#include <math.h>

using namespace std::chrono_literals;

class SimplisticControl : public rclcpp::Node{
public:
  SimplisticControl() : Node("simplistic_control"){

    loc_sub = this->create_subscription<usv_interfaces::msg::UsvLoc>("/usv_loc", 
      10, std::bind(&SimplisticControl::loc_callback, this, std::placeholders::_1));
    
    target_pos_sub = this->create_subscription<geometry_msgs::msg::Pose2D>("/target_pose", 
      10, std::bind(&SimplisticControl::target_pos_callback, this, std::placeholders::_1));

    left_thruster_pub = this->create_publisher<std_msgs::msg::Float64>("/usv/left_thruster", 10);
    right_thruster_pub = this->create_publisher<std_msgs::msg::Float64>("/usv/right_thruster", 10);

    control_timer = this->create_wall_timer(50ms, std::bind(&SimplisticControl::control_timer_callback, this));

    poseReady = false;

    RCLCPP_INFO(this->get_logger(), "Control node up");
  }

private:
  rclcpp::Subscription<usv_interfaces::msg::UsvLoc>::SharedPtr loc_sub;
  rclcpp::Subscription<geometry_msgs::msg::Pose2D>::SharedPtr target_pos_sub;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr left_thruster_pub;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr right_thruster_pub;

  rclcpp::TimerBase::SharedPtr control_timer;
  
  

  std::vector<double> targetPose = {0.0,0.0,0.0};
  std::vector<double> currentPose = {0.0,0.0,0.0};
  bool poseReady;
  
  double Kp_heading = 8.0;
  double Kp_position = 1.0;
  
  void target_pos_callback(const geometry_msgs::msg::Pose2D::SharedPtr msg) {
    targetPose = {msg->x, msg->y, msg->theta};
    poseReady = true;

    RCLCPP_INFO(this->get_logger(), "Read target pose");
  
  }


  void loc_callback(const usv_interfaces::msg::UsvLoc::SharedPtr msg) {
    currentPose = {msg->x, msg->y, msg->yaw};
  
  }

  void control_timer_callback(){
    // Compute position error
    std::vector<double> error = {targetPose[0] - currentPose[0], targetPose[1] - currentPose[1], 0};
    
    // Fix heading error for circular measurements using atan2
    // We are ignoring target heading, and using the angle to the target x and y coordinates instead
    double desired_heading = atan2(error[1], error[0]);

    // Calculate heading error (desired heading minus current heading)
    double rawYawError = desired_heading - currentPose[2];

    // Normalize the heading error to [-pi, pi] for shortest rotation
    error[2] = atan2(sin(rawYawError), cos(rawYawError));
    
    // RCLCPP_INFO(this->get_logger(), "Position Error: (%f, %f, %f) target heading: %f", error[0], error[1], error[2], desired_heading);
    
    // Because of the use differential thrust for steering, two control signals are needed for the 3 degrees of freedom we are targeting

    // Heading controller (proportional)
    double headingSignal = error[2] * Kp_heading;
    
    // Position controller (proportional)
    double positionSignal = sqrt(error[0]*error[0]+error[1]*error[1]) * Kp_position;
    // Cap position signal to not prevent turning
    if(positionSignal > 15){
      positionSignal=15;
    }
    
    // Combine controllers
    std::vector<double> controlSignal = {positionSignal + headingSignal, positionSignal - headingSignal};

    // Cap signals to real values
    if(controlSignal[0] > 36.0){
      controlSignal[0]=36.0;
    } else if(controlSignal[0] < -30.0){
      controlSignal[0] = -30;
    }
    if(controlSignal[1] > 36.0){
      controlSignal[1]=36.0;
    } else if(controlSignal[1] < -30.0){
      controlSignal[1] = -30;
    }

    if(poseReady){
      // Publish control signals
      std_msgs::msg::Float64 left_signal;
      left_signal.data = controlSignal[0];
      left_thruster_pub->publish(left_signal);
      std_msgs::msg::Float64 right_signal;
      right_signal.data = controlSignal[1];
      right_thruster_pub->publish(right_signal);
    }
    

  }

};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SimplisticControl>());
  rclcpp::shutdown();
  return 0;
}
