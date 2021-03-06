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
#define block 10
#define xMax colNum / block
#define yMax rowNum / block

const string windowName = "window";
Color color;

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

void butterfly(int x, int y, Mat image, const int* color) {
  for (int i = 0; i < 2 * block; i++) {
    for (int j = 0; j < 2 * block; j++) {
      if (y*block + i > rowNum || x*block + j > colNum) {
	continue;
      }
      if (min(j, 2*block - j) <= i && i <= max(j, 2*block - j)) {
	for (int c = 0; c < 3; c++) {
	  image.data[3 * ((y * block + i) * colNum + x * block + j) + c] = color[c];
	}
      }
    }
  }
}
	

int main(int argc, char** argv) {
  int cnt = 0;
  int offset;
  Mat image(Size(rowNum, colNum), CV_8UC3, Scalar(0, 0, 0));
  namedWindow(windowName);
  for (int flyx = 0; flyx < xMax; flyx++) {
    cnt++;
    offset = (cnt / 10) % 2;
    int flyy = 5 * sin(flyx / 2.0) + 10;
    for (int y = 0; y < yMax; y++) {
      for (int x = 0; x < xMax; x++) {
	if ((x - xMax/2) * (x - xMax/2) + (y - offset - yMax/2) * (y - offset - yMax/2) < xMax*xMax/9) {
	  if ((x - xMax/2) * (x - xMax/2) + (y - offset - yMax/2) * (y - offset - yMax/2) > xMax*xMax/10.0) {
	    draw(x, y, image, Color::blue);
	  }
	  else {
	    draw(x, y, image, Color::lightBlue);
	  }
	}
	else {
	  draw(x, y, image, Color::black);
	}
      }
    }
    butterfly(flyx, flyy, image, Color::red);
    display(image);
  }
}

