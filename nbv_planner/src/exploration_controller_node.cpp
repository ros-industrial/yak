#include <exploration_controller_node.hpp>

Explorer::Explorer(ros::NodeHandle &nh) {
//  moveit::planning_interface::MoveGroupInterface move_group("manipulator");
//  move_group_ = move_group;
  nbv_client_ = nh.serviceClient<nbv_planner::GetNBV>("/get_nbv");
//  exploration_server_ = nh.advertiseService<std_srvs::Empty>("do_exploration", &Explorer::MoveToNBVs, this);

}

bool Explorer::MoveToNBVs(moveit::planning_interface::MoveGroupInterface &move_group) {
  /*
   * While "finished" criteria is False:
   * - Get next best view given current information
   * - Move to view
   */
  while (true)
  {
    nbv_planner::GetNBV srv;
    nbv_client_.call(srv);
    if (srv.response.exploration_done) {
      ROS_INFO("Exploration reasonably completed");
      break;
    }
    geometry_msgs::PoseArray move_targets = srv.response.bestViewPose;

    // Go to the best reachable pose
    int currentPoseIndex = 0;
    geometry_msgs::Pose targetPose = move_targets.poses[currentPoseIndex];
    move_group.setPoseTarget(targetPose);
    ROS_INFO_STREAM("Moving to pose: " << targetPose);
    while (!move_group.move())
    {
      currentPoseIndex++;
      if (move_targets.poses.size() == 0) {
        ROS_ERROR("Couldn't reach any of the provided poses!");
        break;
      }
      targetPose = move_targets.poses[currentPoseIndex];
      ROS_INFO_STREAM("Pose not reachable, trying next best pose: " << targetPose);
      move_group.setPoseTarget(targetPose);
    }
  }
}



int main(int argc, char* argv[])
{
    ros::init(argc, argv, "exploration_controller_node");
    ros::NodeHandle nh;

    moveit::planning_interface::MoveGroupInterface move_group("manipulator_ensenso");

    Explorer explorer(nh);

    ros::AsyncSpinner async_spinner(1);
    async_spinner.start();

    explorer.MoveToNBVs(move_group);

//    ros::waitForShutdown();

    return 0;
}
