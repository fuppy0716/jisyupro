#include <pthread.h>
#include <unistd.h>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <algorithm>
#include <omp.h>
#include "Mycv.hpp"
using namespace std;
using namespace cv;




int main(int argc, char** argv) {
  /*
  if (argc <= 2) {
    cout << "画像!!" << endl;
    return 0;
  }
  //*/
  Mat alpha = imread("cat.png", 1);
  Mat base = imread("base.png", IMREAD_UNCHANGED);
  Mat grayone = imread("grayone11.png", IMREAD_UNCHANGED);
  Mat lefthand1 = imread("lefthand1.png", IMREAD_UNCHANGED);
  Mat righthand1 = imread("righthand1.png", IMREAD_UNCHANGED);
  vector<Mat> vm = {base, lefthand1, righthand1, grayone};

  
  for (int i = 0; i < 4; i++) {
    vm[i] = Mycv::enlarge(vm[i], 5);
  }
  
  vector<cv::Point2f> tgtPt1, tgtPt2(4);;
  tgtPt1.push_back(cv::Point2f(0, 0));
  tgtPt1.push_back(cv::Point2f(vm[0].rows, 0));
  tgtPt1.push_back(cv::Point2f(vm[0].rows, vm[0].cols));
  tgtPt1.push_back(cv::Point2f(0, vm[0].cols));

  for (int i = 0; i < 4; i++) {
    tgtPt2[i].x = tgtPt1[i].x;
    tgtPt2[i].y = tgtPt1[i].y + 10;
  }

  for (int i = 0; i < 50; i++) {
    Mat res = alpha.clone();
    for (int j = 0; j < 4; j++) {
      Mycv::DrawTransPinP(res, vm[j], res, tgtPt1);
    }
    imshow("alpha", res);
    cvWaitKey(100);
    res = alpha.clone();
    for (int j = 0; j < 4; j++) {
      Mycv::DrawTransPinP(res, vm[j], res, tgtPt2);
    }
    imshow("alpha", res);
    cvWaitKey(100);
  }  
}
  
