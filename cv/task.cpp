/*
標準入力で入れ替えをクリックで入れ替えにする
並列処理で敵を作る
*/

#include <cv.h>
#include <cvaux.h>
#include <highgui.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <algorithm>
using namespace std;
using namespace cv;
#define font FONT_HERSHEY_DUPLEX
#define N 2
#define P pair<Mat,int>

Mat src;

Mat renketu(vector< vector<P> >);
void on_mouse(int event,int x,int y,int flags,void *param);

int main(int argc,char** argv){
  srand((unsigned)time(NULL));
  int i,j;
  if(argc>=2){
    src=imread(argv[1],1);
  }else{
    cout<<"画像を指定してください"<<endl;
  }
  
  // N*N分割するための点を定義
  vector< vector<Point> > p(N,vector<Point>(N));
  for(i=0;i<N;i++){
    for(j=0;j<N;j++){
      p[i][j]={src.cols*j/N, src.rows*i/N};
    }
  }
  
  // N*N分割し、N*Nの二次元配列に分割された画像を入れる。
  vector< vector<P> > a(N,vector<P>(N));
  for(i=0;i<N;i++){
    j=0;
    for (auto itr = p[i].begin(); itr != p[i].end(); ++itr) {
      // 関心領域を元の画像から矩形で切り出す
      Mat roi = src(Rect(itr->x, itr->y, src.cols / N, src.rows / N));
      a[i][j]=make_pair(roi,N*i+j);
      j++;
    }
  }

  // 分割された画像を一つ一つ表示
  /*
    for(i=0;i<N;i++){
    for(j=0;j<N;j++){
    cv::namedWindow("part", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
    cv::imshow("part",a[i][j].first);
    cv::waitKey(0);
    }
    }
  */

  //分割された画像をランダムに並び替える。
  for(i=0;i<10000;i++){
    int i1,i2,j1,j2;
    i1=rand()%N;
    i2=rand()%N;
    j1=rand()%N;
    j2=rand()%N;
    swap(a[i1][j1],a[i2][j2]);
  }

  //画像を連結する。
  Mat my_puz,enemy_puz;
  my_puz=enemy_puz=renketu(a);
  namedWindow("my_puzzle", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  imshow("my_puzzle",my_puz);
  //マウスイベントに対するコールバック関数を登録
  unsigned char c;
  while(1){
    c=cvWaitKey(2);
    if(c=='\x0a'){
      break;
    }
  }
  
  //連結した画像を表示
  /*
  namedWindow("my_puzzle", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
  imshow("my_puzzle",my_puz);
  unsigned char c;
  while(1){
    c=cvWaitKey(2);
    if(c=='\x0a'){
      break;
    }
  }
  */
  

  //全て合うまで入れ替える。
  bool flag;
  while(1){
    flag=true;
    for(i=0;i<N;i++){
      for(j=0;j<N;j++){
	if(a[i][j].second!=N*i+j){
	  flag=false;
	}
      }
    }
    if(flag){
      cout<<"finished!"<<endl;
      break;
    }
    int ix1,iy1,ix2,iy2;
    cout<<"入れ替えて"<<endl;
    cin>>ix1>>iy1>>ix2>>iy2;
    swap(a[ix1][iy1],a[ix2][iy2]);
    my_puz=renketu(a);
    imshow("my_puzzle",my_puz);
    while(1){
      c=cvWaitKey(2);
      if(c=='\x0a'){
	break;
      }
    }
  }
}

Mat renketu(vector< vector<P> > a){
  int i,j;
  Mat my_puz(Size(src.cols,src.rows), CV_8UC3);
  vector<Mat> b(N);
  for(i=0;i<N;i++){
    auto itr=a[i].begin();
    auto itr_end=a[i].end();
    Mat temp(Size(src.cols,src.rows/N),CV_8UC3);
    Rect roi_rect1;
    for(; itr!=itr_end; ++itr){
      roi_rect1.width=itr->first.cols;
      roi_rect1.height=itr->first.rows;
      Mat roi(temp,roi_rect1);
      itr->first.copyTo(roi);
      roi_rect1.x += itr->first.cols;
    }
    b[i]=temp;
  }
  auto itr=b.begin();
  auto itr_end=b.end();
  Rect roi_rect;
  for(; itr!=itr_end; ++itr){
    roi_rect.width=itr->cols;
    roi_rect.height=itr->rows;
    Mat roi(my_puz,roi_rect);
    itr->copyTo(roi);
    roi_rect.y += itr->rows;
  }
  
  return my_puz;
  
}

void on_mouse(int event,int x,int y,int flags,void *param){
  if(event==CV_EVENT_LBUTTONDOWN){
    cout<<src.rows<<" "<<src.cols<<" "<<x<<" "<<y<<endl;
  }
  int i=0;
  if(event==CV_EVENT_LBUTTONDOWN){
    cout<<"aaa"<<endl;
  }
}
  
//max_x=src.cols,max_y=src.rows
