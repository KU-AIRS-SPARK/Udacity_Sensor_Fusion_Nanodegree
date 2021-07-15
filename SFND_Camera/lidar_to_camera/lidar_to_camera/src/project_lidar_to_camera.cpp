#include <iostream>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "structIO.hpp"

using namespace std;

void loadCalibrationData(cv::Mat& P_rect_00, cv::Mat& R_rect_00, cv::Mat& RT) {
  
  // RT.at<double>(0, 0) = 7.533745e-03;
  // RT.at<double>(0, 1) = -9.999714e-01;
  // RT.at<double>(0, 2) = -6.166020e-04;
  // RT.at<double>(0, 3) = -4.069766e-03;
  // RT.at<double>(1, 0) = 1.480249e-02;
  // RT.at<double>(1, 1) = 7.280733e-04;
  // RT.at<double>(1, 2) = -9.998902e-01;
  // RT.at<double>(1, 3) = -7.631618e-02;
  // RT.at<double>(2, 0) = 9.998621e-01;
  // RT.at<double>(2, 1) = 7.523790e-03;
  // RT.at<double>(2, 2) = 1.480755e-02;
  // RT.at<double>(2, 3) = -2.717806e-01;
  
  
  RT.at<double>(0, 0) = 2.220446049250e-16;
  RT.at<double>(0, 1) = 0.000000000000e+00;
  RT.at<double>(0, 2) = 1.000000000000e+00;
  RT.at<double>(0, 3) = -1.679200977087e-01;
  RT.at<double>(1, 0) = -1.000000000000e+00;
  RT.at<double>(1, 1) = 2.220446049250e-16;
  RT.at<double>(1, 2) = 2.220446049250e-16;
  RT.at<double>(1, 3) = -0.000000000000e+00;
  RT.at<double>(2, 0) = -2.220446049250e-16;
  RT.at<double>(2, 1) = -1.000000000000e+00;
  RT.at<double>(2, 2) = 1.232595164408e-32;
  RT.at<double>(2, 3) = 6.119999885559e-01;
  
  RT.at<double>(3, 0) = 0.0;
  RT.at<double>(3, 1) = 0.0;
  RT.at<double>(3, 2) = 0.0;
  RT.at<double>(3, 3) = 1.0;

  // we are not using R_rect

  // R_rect_00.at<double>(0, 0) = 9.999239e-01;
  // R_rect_00.at<double>(0, 1) = 9.837760e-03;
  // R_rect_00.at<double>(0, 2) = -7.445048e-03;
  // R_rect_00.at<double>(0, 3) = 0.0;
  // R_rect_00.at<double>(1, 0) = -9.869795e-03;
  // R_rect_00.at<double>(1, 1) = 9.999421e-01;
  // R_rect_00.at<double>(1, 2) = -4.278459e-03;
  // R_rect_00.at<double>(1, 3) = 0.0;
  // R_rect_00.at<double>(2, 0) = 7.402527e-03;
  // R_rect_00.at<double>(2, 1) = 4.351614e-03;
  // R_rect_00.at<double>(2, 2) = 9.999631e-01;

  // R_rect_00.at<double>(2, 3) = 0.0;
  // R_rect_00.at<double>(3, 0) = 0;
  // R_rect_00.at<double>(3, 1) = 0;
  // R_rect_00.at<double>(3, 2) = 0;
  // R_rect_00.at<double>(3, 3) = 1;

  P_rect_00.at<double>(0, 0) = 1.158033737075e+03;
  P_rect_00.at<double>(0, 1) = 0.000000e+00;
  P_rect_00.at<double>(0, 2) = 9.600000000000e+02;
  P_rect_00.at<double>(0, 3) = 0.000000e+00;
  P_rect_00.at<double>(1, 0) = 0.000000e+00;
  P_rect_00.at<double>(1, 1) = 1.158033737075e+03;
  P_rect_00.at<double>(1, 2) = 5.400000000000e+02;
  P_rect_00.at<double>(1, 3) = 0.000000e+00;
  P_rect_00.at<double>(2, 0) = 0.000000e+00;
  P_rect_00.at<double>(2, 1) = 0.000000e+00;
  P_rect_00.at<double>(2, 2) = 1.000000e+00;
  P_rect_00.at<double>(2, 3) = 0.000000e+00;
}

void projectLidarToCamera2() {
  // load image from file
  //cv::Mat img = cv::imread("../images/0000000000.png");
  cv::Mat img = cv::imread("../test_data/camera.png");

  // load Lidar points from file
  std::vector<LidarPoint> lidarPoints;
  readLidarPts("../test_data/lidar.pcd", lidarPoints);
  cout << lidarPoints.size() << std::endl;

  // store calibration data in OpenCV matrices
  cv::Mat P_rect_00(
      3, 4,
      cv::DataType<double>::type);  // 3x4 projection matrix after rectification
  cv::Mat R_rect_00(
      4, 4,
      cv::DataType<double>::
          type);  // 3x3 rectifying rotation to make image planes co-planar
  cv::Mat RT(
      4, 4,
      cv::DataType<double>::type);  // rotation matrix and translation vector
  loadCalibrationData(P_rect_00, R_rect_00, RT);

  // TODO: project lidar points
  cv::Mat visImg = img.clone();
  cv::Mat overlay = visImg.clone();

  cv::Mat X(4, 1, cv::DataType<double>::type);
  cv::Mat Y(3, 1, cv::DataType<double>::type);

  // Added for ensuring the LiDAR points that get visualised are those
  // we can actually see in the camera plane
  const float maxX = 25.0f, maxY = 30.0f, minZ = -20.0f, minR = 0.01f;//x->25, y->6, z->-1.4,
  
  for (auto it = lidarPoints.begin(); it != lidarPoints.end(); ++it) {
    // Check to be sure this is a LiDAR point we actually want to look at
    // If any of the checks below are true, skip
    // Check #1 - If LiDAR point is behind the vehicle
    // Check #2 - If LiDAR point is too far
    // Check #3 - If the difference horizontally is too far away from the vehicle
    // Check #4 - If the point is on the road
    // Check #5 - If the reflectivity is low (low reliability)
    cout << "Doing conversion" << std::endl;
    // if (it->x < 0.0f) {
    //   continue;
    // }
    // 1. Convert current Lidar point into homogeneous coordinates and store it in the 4D variable X.
    X.at<double>(0, 0) = it->x;
    X.at<double>(1, 0) = it->y;
    X.at<double>(2, 0) = it->z;
    X.at<double>(3, 0) = 1.0;

    // 2. Then, apply the projection equation as detailed in lesson 5.1 to map X onto the image plane of the camera.
    // Store the result in Y.
    Y = P_rect_00 * RT * X;

    // 3. Once this is done, transform Y back into Euclidean coordinates and store the result in the variable pt.
    cv::Point pt(Y.at<double>(0, 0) / Y.at<double>(2, 0),
                 Y.at<double>(1, 0) / Y.at<double>(2, 0));
    
    cout << "x: " << pt.x << "y: " << pt.y << std::endl;

    float val = it->x;
    float maxVal = 20.0;
    int red = min(255, (int)(255 * abs((val - maxVal) / maxVal)));
    int green = min(255, (int)(255 * (1 - abs((val - maxVal) / maxVal))));
    cv::circle(overlay, pt, 5, cv::Scalar(0, green, red), -1);
  }

  float opacity = 0.6;
  cv::addWeighted(overlay, opacity, visImg, 1 - opacity, 0, visImg);

  string windowName = "LiDAR data on image overlay";
  cv::namedWindow(windowName, 3);
  cv::imshow(windowName, visImg);
  cv::waitKey(0);  // wait for key to be pressed
}

int main() {
  projectLidarToCamera2();
}