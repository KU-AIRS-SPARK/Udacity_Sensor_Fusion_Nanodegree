#include <iostream>
#include <numeric>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "structIO.hpp"

using namespace std;

void loadCalibrationData(cv::Mat &P_rect_00, cv::Mat &R_rect_00, cv::Mat &RT)
{
    // RT.at<double>(0,0) = 7.533745e-03; RT.at<double>(0,1) = -9.999714e-01; RT.at<double>(0,2) = -6.166020e-04; RT.at<double>(0,3) = -4.069766e-03;
    // RT.at<double>(1,0) = 1.480249e-02; RT.at<double>(1,1) = 7.280733e-04; RT.at<double>(1,2) = -9.998902e-01; RT.at<double>(1,3) = -7.631618e-02;
    // RT.at<double>(2,0) = 9.998621e-01; RT.at<double>(2,1) = 7.523790e-03; RT.at<double>(2,2) = 1.480755e-02; RT.at<double>(2,3) = -2.717806e-01;
    // RT.at<double>(3,0) = 0.0; RT.at<double>(3,1) = 0.0; RT.at<double>(3,2) = 0.0; RT.at<double>(3,3) = 1.0;
    
    RT.at<double>(0,0) = 1.000000000000e+00; RT.at<double>(0,1) = 0.000000000000e+00; RT.at<double>(0,2) = 0.000000000000e+00; RT.at<double>(0,3) = 1.0679201;
    RT.at<double>(1,0) = 0.000000000000e+00; RT.at<double>(1,1) = 1.000000000000e+00; RT.at<double>(1,2) = 0.000000000000e+00; RT.at<double>(1,3) = 0.000000000000e+00;
    RT.at<double>(2,0) = 0.000000000000e+00; RT.at<double>(2,1) = 0.000000000000e+00; RT.at<double>(2,2) = 1.000000000000e+00; RT.at<double>(2,3) = 0.812;
    RT.at<double>(3,0) = 0.0; RT.at<double>(3,1) = 0.0; RT.at<double>(3,2) = 0.0; RT.at<double>(3,3) = 1.0;
    
    P_rect_00.at<double>(0,0) = 1.158033737075e+03; P_rect_00.at<double>(0,1) = 0.000000e+00; P_rect_00.at<double>(0,2) = 9.600000000000e+02; P_rect_00.at<double>(0,3) = 0.000000e+00;
    P_rect_00.at<double>(1,0) = 0.000000e+00; P_rect_00.at<double>(1,1) = 1.158033737075e+03; P_rect_00.at<double>(1,2) = 5.400000000000e+02; P_rect_00.at<double>(1,3) = 0.000000e+00;
    P_rect_00.at<double>(2,0) = 0.000000e+00; P_rect_00.at<double>(2,1) = 0.000000e+00; P_rect_00.at<double>(2,2) = 1.000000e+00; P_rect_00.at<double>(2,3) = 0.000000e+00;

}

void projectLidarToCamera2()
{
    // load image from file
    cv::Mat img = cv::imread("../test_data/camera.png");

    // load Lidar points from file
    std::vector<LidarPoint> lidarPoints;
    readLidarPts("../test_data/lidar.pcd", lidarPoints);

    // store calibration data in OpenCV matrices
    cv::Mat P_rect_00(3,4,cv::DataType<double>::type); // 3x4 projection matrix after rectification
    cv::Mat R_rect_00(4,4,cv::DataType<double>::type); // 3x3 rectifying rotation to make image planes co-planar
    cv::Mat RT(4,4,cv::DataType<double>::type); // rotation matrix and translation vector
    loadCalibrationData(P_rect_00, R_rect_00, RT);
    // project lidar points
    cv::Mat visImg = img.clone();
    cv::Mat overlay = visImg.clone();

    cv::Mat X(4,1,cv::DataType<double>::type);
    cv::Mat Y(3,1,cv::DataType<double>::type);
    for(auto it=lidarPoints.begin(); it!=lidarPoints.end(); ++it) {

      // if (it->x < 0.0f) { continue; }

      X.at<double>(0, 0) = it->x;
      X.at<double>(1, 0) = it->y;
      X.at<double>(2, 0) = it->z;
      X.at<double>(3, 0) = 1;

      Y = P_rect_00 * RT * X;
      cv::Point pt;
      pt.x = Y.at<double>(0, 0) / (Y.at<double>(2, 0) * 1920);
      pt.y = Y.at<double>(1, 0) / (Y.at<double>(2, 0) * 1080);

      cout << "x: " << pt.x << " y: " << pt.y << std::endl;

      float val = it->x;
      float maxVal = 20.0;
      int red = min(255, (int)(255 * abs((val - maxVal) / maxVal)));
      int green = min(255, (int)(255 * (1 - abs((val - maxVal) / maxVal))));
      cv::circle(overlay, pt, 5, cv::Scalar(0, green, red), -1);
    }

    float opacity = 0.6;
    cv::addWeighted(overlay, opacity, visImg, 1 - opacity, 0, visImg);
    
    string windowName = "LiDAR data on image overlay";
    cv::namedWindow( windowName, 3 );
    cv::imshow( windowName, visImg );
    cv::waitKey(0); // wait for key to be pressed
}

int main()
{
    projectLidarToCamera2();
}
