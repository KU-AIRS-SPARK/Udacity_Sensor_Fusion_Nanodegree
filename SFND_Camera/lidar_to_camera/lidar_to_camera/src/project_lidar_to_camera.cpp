#include <iostream>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <cmath>

#include "structIO.hpp"

using namespace std;

int W = 640;
int H = 480;

double HORIZONTAL_FOV = 47.9440551;
double VERTICAL_FOV = 36.7129106;
//double OFFSET_COEFF = 0.1;
double OFFSET_X = 336.20585 - W/2;
double OFFSET_Y = 264.25822 - H/2;

double CONVERSION_CONSTANT_X = (W/2)*(1/(tan((HORIZONTAL_FOV/2)*(M_PI/180))));
double CONVERSION_CONSTANT_Y = (H/2)*(1/(tan((VERTICAL_FOV/2)*(M_PI/180))));

void loadIntrinsicData(cv::Mat& camera_matrix, cv::Mat& dist_coefficients) {

  camera_matrix.at<double>(0, 0) = 719.67715;
  camera_matrix.at<double>(0, 1) = 0.0;
  camera_matrix.at<double>(0, 2) = 336.20585;
  camera_matrix.at<double>(1, 0) = 0.0;
  camera_matrix.at<double>(1, 1) = 723.3015;
  camera_matrix.at<double>(1, 2) = 264.25822;
  camera_matrix.at<double>(2, 0) = 0.0;
  camera_matrix.at<double>(2, 1) = 0.0;
  camera_matrix.at<double>(2, 2) = 1.0;

  dist_coefficients.at<double>(0,0) = 0.065220;
  dist_coefficients.at<double>(0,1) = -0.013585;
  dist_coefficients.at<double>(0,2) = 0.010870;
  dist_coefficients.at<double>(0,3) = 0.001598;
  dist_coefficients.at<double>(0,4) = 0.000000;
}

void loadExtrinsicData(cv::Mat& L, cv::Mat& C, cv::Mat& Tc) {

  L.at<double>(0, 0) = 0.215;
  L.at<double>(1, 0) = 0.04;
  L.at<double>(2, 0) = 0.19;

  C.at<double>(0, 0) = 0.4135;
  C.at<double>(1, 0) = 0.0;
  C.at<double>(2, 0) = 0.0283;

  Tc.at<double>(0, 0) = 0.0;
  Tc.at<double>(0, 1) = -1.0;
  Tc.at<double>(0, 2) = 0.0;
  Tc.at<double>(1, 0) = 0.0;
  Tc.at<double>(1, 1) = 0.0;
  Tc.at<double>(1, 2) = -1.0;
  Tc.at<double>(2, 0) = 1.0;
  Tc.at<double>(2, 1) = 0.0;
  Tc.at<double>(2, 2) = 0.0;

}

void projectLidarToCamera2() {
  // load image from file
  cv::Mat img = cv::imread("../test_data/test_with_durak.jpg");
  cv::Size imageSize(cv::Size(img.cols, img.rows));

  cv::Mat camera_matrix(
    3, 3,
    cv::DataType<double>::type);
  cv::Mat dist_coefficients(
    1, 5,
    cv::DataType<double>::type);

  loadIntrinsicData(camera_matrix, dist_coefficients);

  cv::Mat undistorted_img, new_camera_matrix;
  new_camera_matrix = cv::getOptimalNewCameraMatrix(camera_matrix, dist_coefficients, imageSize, 1, imageSize, 0);

  cv::undistort(img, undistorted_img, new_camera_matrix, dist_coefficients, new_camera_matrix);

  // load Lidar points from file
  std::vector<LidarPoint> lidarPoints;
  readLidarPts("../test_data/test_with_durak.pcd", lidarPoints);

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
      
  loadExtrinsicData(L, C, Tc);

  // TODO: project lidar points
  cv::Mat visImg = undistorted_img.clone();
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
  
    if (it->x < 0.0f || it->x > maxX || std::abs(it->y) > maxY ||
        it->z < minZ) { //|| it->r < minR
      continue;
    }
    // 1. Convert current Lidar point into homogeneous coordinates and store it in the 4D variable X.
    X.at<double>(0, 0) = it->x;
    X.at<double>(1, 0) = it->y;
    X.at<double>(2, 0) = it->z;

    // 2. Then, apply the projection equation as detailed in lesson 5.1 to map X onto the image plane of the camera.
    // Store the result in Y.
    Cam = Tc*(X + (L-C));
    //Cam.at<double>(0,0) += (OFFSET_X + W/2)*0.001;
    //Cam.at<double>(1,0) -= (OFFSET_Y + H/2)*0.001;

    Y.at<double>(0,0) = (int) (W/2 + CONVERSION_CONSTANT_X*(Cam.at<double>(0,0)/Cam.at<double>(2,0)) + OFFSET_X);
    Y.at<double>(1,0) = (int) (H/2 + CONVERSION_CONSTANT_Y*(Cam.at<double>(1,0)/Cam.at<double>(2,0)) - OFFSET_Y);

    // 3. Once this is done, transform Y back into Euclidean coordinates and store the result in the variable pt.
    cv::Point pt(Y.at<double>(0,0), Y.at<double>(1,0));

    float val = it->x;
    float maxVal = 10.0;
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

int main()
{
    projectLidarToCamera2();
}
