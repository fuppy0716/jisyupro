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
#include "Color.cpp"
using namespace std;
using namespace cv;

#define rowNum 500
#define colNum 500
#define block 5
#define xMax colNum / block
#define yMax rowNum / block

const string windowName = "girl";

void display(Mat image) {
  imshow(windowName, image);
  waitKey(50);
}

void draw(int x, int y, Mat image, const int* color) {
  for (int i = 0; i < block; i++) {
    for (int j = 0; j < block; j++) {
      for (int c = 0; c < 3; c++) {
	image.data[3 * ((y * block + i) * colNum + x * block + j) + c] = color[c];
      }
    }
  }
}

int main() {
  char key = 0;
  Mat image(Size(rowNum, colNum), CV_8UC3, Scalar(0, 0, 0));
  namedWindow(windowName);
  while (true) {
    for (int y = 0; y < yMax; y++) {
      for (int x = 0; x < xMax; x++) {
	if (xMax/4.0 < x && x < xMax*3/4.0) {
	  if (pow((x - 0.5 * xMax), 2) / pow(xMax/5, 2) + pow((y - 0.3 * yMax), 2) / pow(yMax/4, 2) < 1) {
	    draw(x, y, image, Color::lightBlue);
	  }
	}
	
      }
    }
    display(image);
  }
}
