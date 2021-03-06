using namespace std;

#include <SerialStream.h>
#include "MySerial.hpp"

#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include "Mycv.hpp"

#include <AL/alut.h>

#include <pthread.h>

#include <string.h>
#include <vector>
#include <iostream>
#include <stdio.h> 
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <string>

const int bairitsu = 5;
const int summernum = 2;
const int winternum = 1;

bool tempFlag;
bool lightFlag;
int lefthandVal;
int righthandVal;
bool windFlag;

int offset = 0;
long long cnt = 0;
int snowcnt = 0;
int backcnt = 0;
int windcnt = 0;
int clothcnt = 0;
int sleepcnt = 0;
int girlX = 35 * bairitsu;
int girlY = 15 * bairitsu;

//ADD
ALuint buffer[2], source[2];
int soundcnt = 0;

void initAl(int argc, char** argv) {
  char wavname[2][30] = {"loop_137.wav", "kyaa_02.wav"};
  alutInit(&argc,argv);
  for (int i = 0; i < 2; i++) {
    alGenBuffers( 1, &buffer[i] );
    alGenSources( 1, &source[i] );
    buffer[i] = alutCreateBufferFromFile( wavname[i] );
    if (i == 0) {
      alSourcei( source[i], AL_LOOPING, AL_TRUE);
      alSourcef( source[i], AL_GAIN, 0.3);
    }
    alSourcei( source[i], AL_BUFFER, buffer[i] );
  }
  return;
}

void *loopBGM(void *arg) {
  ALuint *src;
  src = (ALuint *)arg;
  alSourcePlay(*src);
  return arg;
}

void *kya(void *arg) {
  ALuint *src;
  src = (ALuint *)arg;
  alSourcePlay(*src);
  alutSleep(2);
  return arg;
}

//ADDEND



void setFlag(char res) {
  tempFlag = (bool)(res & 1);
  lightFlag = (bool)(res & (1 << 1));
  lefthandVal = (int)((res & (3 << 4)) >> 4);
  righthandVal = (int)((res & (3 << 2)) >> 2);
  windFlag = (bool)(res & (1 << 6));
  return;
}

void drawGirl(void) {
  cnt++;
  if (cnt % 5 == 0) {
    offset = 10 - offset;
    snowcnt = (snowcnt + 1) % 3;
    backcnt = (backcnt + 1) % 2;
  }
  if (cnt % 3 == 0) {
    windcnt = (windcnt + 1) % 2;
    sleepcnt = (sleepcnt + 1) % 4;
  }
    
  //背景
  Mat back;
  char backname[30];
  if (lightFlag) {
    sprintf(backname, "back_night%d.png", backcnt + 1);
  }
  else {
    sprintf(backname, "back_morning%d.png", backcnt + 1);
  }
  back = imread(backname, 1);
  back = Mycv::enlarge(back, bairitsu);
  Mat res = back.clone();

  vector<cv::Point2f> allPt;
  allPt.push_back(cv::Point2f(0, 0));
  allPt.push_back(cv::Point2f(back.cols, 0));
  allPt.push_back(cv::Point2f(back.cols, back.rows));
  allPt.push_back(cv::Point2f(0, back.rows));

  
  //冬なら雪
  if (tempFlag) {
    char snowname[30];
    sprintf(snowname, "snow%d.png", snowcnt + 1);
    Mat snowimg = imread(snowname, IMREAD_UNCHANGED);
    snowimg = Mycv::enlarge(snowimg, bairitsu);
;
    Mycv::DrawTransPinP(res, snowimg, res, allPt);
  }

  char season[10];
  Mat land;
  if (tempFlag) {
    sprintf(season, "winter%d", clothcnt % winternum + 1);
    land = imread("winter_land.png", IMREAD_UNCHANGED);
  }
  else {
    sprintf(season, "summer%d", clothcnt % summernum + 1);
    land = imread("summer_land.png", IMREAD_UNCHANGED);
  }
  land = Mycv::enlarge(land, bairitsu);
  Mycv::DrawTransPinP(res, land, res, allPt);
  
  //女の子
  Mat girl = imread("base.png", IMREAD_UNCHANGED);
  girl = Mycv::enlarge(girl, bairitsu);
  vector<cv::Point2f> girlPt;
  girlPt.push_back(cv::Point2f(girlX, girlY));
  girlPt.push_back(cv::Point2f(girlX + girl.cols, girlY));
  girlPt.push_back(cv::Point2f(girlX + girl.cols, girlY + girl.rows));
  girlPt.push_back(cv::Point2f(girlX, girlY + girl.rows));
  
  if (lightFlag) {
    //ADD
    soundcnt = 0;
    //ADDEND
    char sleepname[30];
    sprintf(sleepname, "sleep%d.png", sleepcnt + 1);
    Mat sleep = imread(sleepname, IMREAD_UNCHANGED);
    sleep = Mycv::enlarge(sleep, bairitsu);
    Mycv::DrawTransPinP(res, sleep, res, girlPt);
  }
  else if (windFlag) {
    //ADD
    soundcnt++;
    //ADDEND
    char windname[30];
    sprintf(windname, "wind_wind%d.png", windcnt + 1);
    Mat wind = imread(windname, IMREAD_UNCHANGED);
    wind = Mycv::enlarge(wind, bairitsu);
    Mycv::DrawTransPinP(res, wind, res, allPt);  
    
    char girlname[30];
    sprintf(girlname, "wind%d_base.png", windcnt + 1);
    girl = imread(girlname, IMREAD_UNCHANGED);
    girl = Mycv::enlarge(girl, bairitsu);
    Mycv::DrawTransPinP(res, girl, res, girlPt);

    char clothname[30];
    sprintf(clothname, "wind%d_%s.png", windcnt + 1, season);
    Mat cloth = imread(clothname, IMREAD_UNCHANGED);
    cloth = Mycv::enlarge(cloth, bairitsu);
    Mycv::DrawTransPinP(res, cloth, res, girlPt);
  }
  else{
    //ADD
    soundcnt = 0;
    //ADDEND
    for (int i = 0; i < 4; i++) {
      girlPt[i].y += offset;
    }
    Mycv::DrawTransPinP(res, girl, res, girlPt);

    char left[30];
    sprintf(left, "lefthand%d.png", lefthandVal + 1);
    Mat lefthand = imread(left, IMREAD_UNCHANGED);
    lefthand = Mycv::enlarge(lefthand, bairitsu);
    Mycv::DrawTransPinP(res, lefthand, res, girlPt);

    char right[30];
    sprintf(right, "righthand%d.png", righthandVal + 1);
    Mat righthand = imread(right, IMREAD_UNCHANGED);
    righthand = Mycv::enlarge(righthand, bairitsu);
    Mycv::DrawTransPinP(res, righthand, res, girlPt);

    char clothname[30];
    sprintf(clothname, "%s_base.png", season);
    Mat cloth = imread(clothname, IMREAD_UNCHANGED);
    cloth = Mycv::enlarge(cloth, bairitsu);
    Mycv::DrawTransPinP(res, cloth, res, girlPt);

    char leftcloth[30];
    sprintf(leftcloth, "%s_%s", season, left);
    lefthand = imread(leftcloth, IMREAD_UNCHANGED);
    lefthand = Mycv::enlarge(lefthand, bairitsu);
    Mycv::DrawTransPinP(res, lefthand, res, girlPt);

    char rightcloth[30];
    sprintf(rightcloth, "%s_%s", season, right);
    righthand = imread(rightcloth, IMREAD_UNCHANGED);
    righthand = Mycv::enlarge(righthand, bairitsu);
    Mycv::DrawTransPinP(res, righthand, res, girlPt);
  }

  imshow("res", res);
  
}



int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "ポート名ほしい" << endl;
    return 0;
  }

  //ADD
  initAl(argc, argv);
  pthread_t th[2];
  pthread_create(&th[0], NULL, loopBGM, (void *)&source[0]);
  //ADDEND
  
  MySerial serial(argv[1]);
  while (true) {
    char res = serial.readSerial();
    //printf("%d\n", res);
    setFlag(res);

    drawGirl();
    //ADD
    if (soundcnt % 5 == 1) {
      pthread_create(&th[0], NULL, kya, (void *)&source[1]);
    }
    //ADDEND
    char key = cvWaitKey(100);
    if (key == 'q') {
      return 0;
    }
    else if (key == 'c') {
      clothcnt++;
    }
  }
}
