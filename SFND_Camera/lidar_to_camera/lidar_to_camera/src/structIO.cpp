#include "structIO.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <pcl/io/pcd_io.h>    //NR
#include <pcl/point_types.h>  //NR

using namespace std;

/* TEMPLATES */
template <typename T>
void write_pod(std::ofstream& out, T& t) {
  out.write(reinterpret_cast<char*>(&t), sizeof(T));
}

template <typename T>
void read_pod(std::ifstream& in, T& t) {
  in.read(reinterpret_cast<char*>(&t), sizeof(T));
}

template <typename T>
void read_pod_vector(std::ifstream& in, std::vector<T>& vect) {
  long size;

  read_pod(in, size);
  for (int i = 0; i < size; ++i) {
    T t;
    read_pod(in, t);
    vect.push_back(t);
  }
}

template <typename T>
void write_pod_vector(std::ofstream& out, std::vector<T>& vect) {
  long size = vect.size();
  write_pod<long>(out, size);
  for (auto it = vect.begin(); it != vect.end(); ++it) {
    write_pod<T>(out, *it);
  }
}

/* DATATYPE WRAPPERS */

void writeLidarPts(std::vector<LidarPoint>& input, const char* fileName) {
  std::ofstream out(fileName);
  write_pod_vector(out, input);
  out.close();
}

void readLidarPts(const char* fileName, std::vector<LidarPoint>& output) {
  //std::ifstream in(fileName);
  //read_pod_vector(in, output);

  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

  if (pcl::io::loadPCDFile<pcl::PointXYZ>(fileName, *cloud) ==
      -1)  //* load the file
  {
    PCL_ERROR("Couldn't read file test_pcd.pcd \n");
    return;
  }

  int i = 0;
  for (const auto& point : *cloud) {
    output.push_back(LidarPoint());
    output.at(i).x = point.x;  //z
    output.at(i).y = point.y;  //-x
    output.at(i).z = point.z;  //y

    i++;
  }
}

void writeKeypoints(std::vector<cv::KeyPoint>& input, const char* fileName) {
  std::ofstream out(fileName);
  write_pod_vector(out, input);
  out.close();
}

void readKeypoints(const char* fileName, std::vector<cv::KeyPoint>& output) {
  std::ifstream in(fileName);
  read_pod_vector(in, output);
}

void writeKptMatches(std::vector<cv::DMatch>& input, const char* fileName) {
  std::ofstream out(fileName);
  write_pod_vector(out, input);
  out.close();
}

void readKptMatches(const char* fileName, std::vector<cv::DMatch>& output) {
  std::ifstream in(fileName);
  read_pod_vector(in, output);
}

void writeDescriptors(cv::Mat& input, const char* fileName) {
  cv::FileStorage opencv_file(fileName, cv::FileStorage::WRITE);
  opencv_file << "desc_matrix" << input;
  opencv_file.release();
}

void readDescriptors(const char* fileName, cv::Mat& output) {
  cv::FileStorage opencv_file(fileName, cv::FileStorage::READ);
  opencv_file["desc_matrix"] >> output;
  opencv_file.release();
}
