#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ROS_INFO_STREAM("Driving the robot to the specified direction");
    
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
   
    if (!client.call(srv))
        ROS_ERROR("Failed to call service DriveToTarget");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel_R = 255;
    int white_pixel_G = 255;
    int white_pixel_B = 255;

    // Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

   int height = img.height;
   int step = img.step;
   int ct_left, ct_forward, ct_right = 0;
   int index, left_index, forward_index, right_index = 0;
   float x, z = 0.0;
 
   ct_left = 0;
   ct_forward = 0;
   ct_right = 0;
   
   for (int i = 0; i < height; i++) {
          for (int j = 0; j < step; j=j+1) {
	       index = j + i*step;
	       left_index = step/3.0 + i*step;
               forward_index = 2.0*step/3.0 + i*step;
               right_index = step + i*step;
  	       if (index < left_index) {
		  //if (img.data[index] == white_pixel)
		   if (img.data[index] == 255 && img.data[index+1] == 255 && img.data[index+2] == 255)
		   {
			ct_left = ct_left + 1; 
	           }
	       }
  	       else if (index >= left_index && index < forward_index ) {
  	       // if (img.data[index] == white_pixel)
  	       if (img.data[index] == 255 && img.data[index+1] == 255 && img.data[index+2] == 255) {
			ct_forward = ct_forward + 1; 
	           }
	       }
  	       else if (index >= forward_index && index < right_index ) {
  	       // if (img.data[index] == white_pixel)
  	       if (img.data[index] == 255 && img.data[index+1] == 255 && img.data[index+2] == 255) {
			ct_right = ct_right + 1; 
	           }
	       }
	       else {
		   ROS_INFO_STREAM("Error!!!");
	       }

	   }
	
   }

   if (ct_right + ct_forward + ct_left == 0.0){
	x = 0.0;
	z = 0.0;
   }
   else { 
	   if (ct_left > ct_forward && ct_left > ct_right){
		x = 0.1;
		z = 0.3;
		}
	
	   else if (ct_forward > ct_left && ct_forward > ct_right){
		x = 0.2;
		z = 0.0;
		}

	   else if (ct_right > ct_forward && ct_right > ct_left){
		x = 0.1;
		z = -0.3;
		 }
	   else
	       { 
		x = 0.0;
		z = 0.0;
	       }
	}

   drive_robot(x, z);


}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
