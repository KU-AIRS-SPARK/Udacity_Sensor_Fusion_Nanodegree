#include <iostream>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>

#include "structIO.hpp"

using namespace std;

void loadCalibrationData(cv::Mat& L, cv::Mat& C, cv::Mat& Tc, double& CONVERSION_CONSTANT_X, double& CONVERSION_CONSTANT_Y) {

  /*
  int W = 1920;
  int H = 1080;
  double HORIZONTAL_FOV = 79.0;
  double VERTICAL_FOV = 50.0;
  double CONVERSION_CONSTANT_X = (W/2)*(1+(1/(tan(HORIZONTAL_FOV/2))));
  double CONVERSION_CONSTANT_Y = (H/2)*(1+(1/(tan(VERTICAL_FOV/2))));*/

  double CONVERSION_CONSTANT_X = 2124.57312393;
  double CONVERSION_CONSTANT_Y = 1698.03373708;

  L.at<double>(0, 0) = 0.0;
  L.at<double>(1, 0) = 2.312;
  L.at<double>(2, 0) = -0.3679201;

  C.at<double>(0, 0) = 0.0;
  C.at<double>(1, 0) = 1.7;
  C.at<double>(2, 0) = -0.2;

  Tc.at<double>(0, 0) = 1.0;
  Tc.at<double>(0, 1) = 0.0;
  Tc.at<double>(0, 2) = 0.0;
  Tc.at<double>(1, 0) = 0.0;
  Tc.at<double>(1, 1) = -1.0;
  Tc.at<double>(1, 2) = 0.0;
  Tc.at<double>(2, 0) = 0.0;
  Tc.at<double>(2, 1) = 0.0;
  Tc.at<double>(2, 2) = 1.0;

}

void projectLidarToCamera2() {
  // load image from file
  //cv::Mat img = cv::imread("../images/0000000000.png");
  cv::Mat img = cv::imread("../test_data/svl_camera.png");

  // load Lidar points from file
  std::vector<LidarPoint> lidarPoints;
  readLidarPts("../test_data/svl_lidar.pcd", lidarPoints);

  // store calibration data in OpenCV matrices
  cv::Mat L(
      3, 1,
      cv::DataType<double>::type);  // 3x4 projection matrix after rectification
  cv::Mat C(
      3, 1,
      cv::DataType<double>::
          type);  // 3x3 rectifying rotation to make image planes co-planar
  cv::Mat Tc(
      3, 3,
      cv::DataType<double>::type);  // rotation matrix and translation vector

  double CONVERSION_CONSTANT_X;
  double CONVERSION_CONSTANT_Y;

  loadCalibrationData(L, C, Tc, CONVERSION_CONSTANT_X, CONVERSION_CONSTANT_Y);

  // TODO: project lidar points
  cv::Mat visImg = img.clone();
  cv::Mat overlay = visImg.clone();

  cv::Mat Cam(3, 1, cv::DataType<double>::type);
  cv::Mat X(3, 1, cv::DataType<double>::type);
  cv::Mat Y(2, 1, cv::DataType<double>::type);

  // Added for ensuring the LiDAR points that get visualised are those
  // we can actually see in the camera plane
  const float maxX = 25.0f, maxY = 6.0f, minZ = -1.4f, minR = 0.01f;//x->25, y->6, z->-1.4,
  
  for (auto it = lidarPoints.begin(); it != lidarPoints.end(); ++it) {
    // Check to be sure this is a LiDAR point we actually want to look at
    // If any of the checks below are true, skip
    // Check #1 - If LiDAR point is behind the vehicle
    // Check #2 - If LiDAR point is too far
    // Check #3 - If the difference horizontally is too far away from the vehicle
    // Check #4 - If the point is on the road
    // Check #5 - If the reflectivity is low (low reliability)
  
    if (it->z < 0.0f || it->z > maxX || std::abs(it->x) > maxY ||
        it->y < minZ) { //|| it->r < minR
      continue;
    }
    // 1. Convert current Lidar point into homogeneous coordinates and store it in the 4D variable X.
    X.at<double>(0, 0) = it->x;
    X.at<double>(1, 0) = it->y;
    X.at<double>(2, 0) = it->z;

    // 2. Then, apply the projection equation as detailed in lesson 5.1 to map X onto the image plane of the camera.
    // Store the result in Y.
    Cam = Tc*(X + (L-C));
    Y.at<double>(0,0) = CONVERSION_CONSTANT_X*(Cam.at<double>(0,0)/Cam.at<double>(2,0));
    Y.at<double>(1,0) = CONVERSION_CONSTANT_Y*(Cam.at<double>(1,0)/Cam.at<double>(2,0));

    // 3. Once this is done, transform Y back into Euclidean coordinates and store the result in the variable pt.
    cv::Point pt(Y.at<double>(0,0), Y.at<double>(1,0));

    float val = it->z;
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