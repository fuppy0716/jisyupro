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
using namespace std;
using namespace cv;

#define rowNum 50
#define colNum 50
#define block 10

const string windowName = "window";

void display(Mat image) {
  imshow(windowName, image);
  waitKey(100);
}

void draw(int x, int y, Mat image, int* color) {
  for (int i = 0; i < block; i++) {
    for (int j = 0; j < block; j++) {
      for (int c = 0; c < 3; c++) {
	image.data[3 * ((y * block + i) * colNum + x * block + j) + c] = color[c];      }
    }
  }
}

int main(int argc, char** argv) {
  Mat image(Size(rowNum, colNum), CV_8UC3, Scalar(0, 0, 0));
  namedWindow(windowName);
  int color[3];
  color[0] = 0;
  color[1] = 255;
  color[2] = 0;
  for (int y = 0; y < rowNum / block; y++) {
    for (int x = 0; x < colNum / block; x++) {
      swap(color[0], color[1]);
      swap(color[1], color[2]);
      draw(x, y, image, color);
      display(image);
    }
  }
}
