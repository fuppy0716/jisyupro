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

Mat src;
Mat mini;

vector<vector<int> > initPallete(int n) {
  vector<vector<int> > pallete(pow(n + 2, 3), vector<int>(3));
  int now = 0;
  for (int i = 0; i <= n; i++) {
    for (int j = 0; j <= n; j++) {
      for (int k = 0; k <= n; k++) {
	pallete[now][0] = min(i*(255 / n), 255);
	pallete[now][1] = min(j*(255 / n), 255);
	pallete[now][2] = min(k*(255 / n), 255);
	now++;
      }
    }
  }
  
  return pallete;
}

vector<int> nearColor(vector<vector<int> > pallete, vector<double> color) {
  double dist = 10000000000;
  int resi;
  for (int i = 0; i < pallete.size(); i++) {
    double temp = pow((pallete[i][0] - color[0]), 2) + pow((pallete[i][1] - color[1]), 2) + pow((pallete[i][2] - color[2]), 2);
    if (temp < dist) {
      dist = temp;
      resi = i;
    }
  }
  return pallete[resi];
}


int main(int argc, char** argv) {
  if(argc>=2){
    src=imread(argv[1],1);
  }else{
    cout<<"画像を指定してください"<<endl;
  }
  namedWindow("src", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  imshow("src", src);
  cvWaitKey();
  // destroyWindow("src");

  vector<vector<int> > pallete = initPallete(5);
  
  int rows = src.rows;
  int cols = src.cols;
  const int blockNum = rows / 5;
  cout << rows << " " << cols << endl;
  Mat mini(blockNum, blockNum, CV_8UC3);
  vector<vector<vector<double> > > color(blockNum, vector<vector<double> >(blockNum, vector<double>(4, 0)));

  // srcとminiの位置の対応準備
  map<int, int> src2colorI, src2colorJ;
  int y = 0;
  for (int i = 0; i < blockNum; i++) {
    for (int j = 0; j < rows / blockNum; j++) {
      src2colorI[y] = i;
      y++;
    }
    if (i < rows % blockNum) {
      src2colorI[y] = i;
      y++;
    }
  }
  int x = 0;
  for (int i = 0; i < blockNum; i++) {
    for (int j = 0; j < cols / blockNum; j++) {
	src2colorJ[x] = i;
	x++;
    }
    if (i < cols % blockNum) {
      src2colorJ[x] = i;
      x++;
    }
  }

  //srcの色をcolorに縮小
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      y = src2colorI[i];
      x = src2colorJ[j];
      color[y][x][0] += src.data[3*(i*cols + j)];
      color[y][x][1] += src.data[3*(i*cols + j) + 1];
      color[y][x][2] += src.data[3*(i*cols + j) + 2];
      color[y][x][3] += 1;
    }
  }
  for (y = 0; y < blockNum; y++) {
    for (x = 0; x < blockNum; x++) {
      color[y][x][0] /= color[y][x][3];
      color[y][x][1] /= color[y][x][3];
      color[y][x][2] /= color[y][x][3];
    }
  }
  
  
  
  for (y = 0; y < blockNum; y++) {
    for (x = 0; x < blockNum; x++) {
      mini.data[3*(y*blockNum + x)] = color[y][x][0];
      mini.data[3*(y*blockNum + x) + 1] = color[y][x][1];
      mini.data[3*(y*blockNum + x) + 2] = color[y][x][2];
    }
  }

  int bairitsu = 5;
  
  Mat deka(bairitsu * blockNum, bairitsu * blockNum, CV_8UC3);
  for (int y = 0; y < blockNum; y++) {
    for (int x = 0; x < blockNum; x++) {      
      vector<int> temp = nearColor(pallete, color[y][x]);
      for (int i = 0; i < bairitsu; i++) {
	for (int j = 0; j < 10; j++) {
	  deka.data[3*((y * bairitsu + i)*bairitsu*blockNum + x*bairitsu + j)] = temp[0];
	  deka.data[3*((y * bairitsu + i)*bairitsu*blockNum + x*bairitsu + j) + 1] = temp[1];
	  deka.data[3*((y * bairitsu + i)*bairitsu*blockNum + x*bairitsu + j) + 2] = temp[2];
	}
      }
    }
  }
  namedWindow("deka", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  imshow("deka", deka);
  char key = cvWaitKey();
  if (key == 's') {
    imwrite("deka.jpg", deka);
  }
}
