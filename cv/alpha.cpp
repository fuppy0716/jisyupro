#include <cv.h>
#include <highgui.h>
#include <iostream>

using namespace cv;
using namespace std;

bool isBrack(Mat img, int x, int y) {
  if (img.data[4*(y * img.cols + x)] + img.data[4*(y * img.cols + x) + 1] + img.data[4*(y * img.cols + x) + 2] == 0) {
    return true;
  }
  else {
    return false;
  }

}

void make_alpha(char* name) {
   Mat source = imread(name);
   imshow("source", source);
   cvWaitKey();
   Mat alpha_image = Mat(source.size(), CV_8UC4);
   cvtColor(source, alpha_image, CV_RGB2RGBA);
   imshow("alpha1", alpha_image);
   cvWaitKey();
   for (int y = 0; y < alpha_image.rows; ++y) {
     for (int x = 0; x < alpha_image.cols; ++x) {
       if (isBrack(alpha_image, x, y)) {
	 alpha_image.data[4*(y * alpha_image.cols + x) + 3] = 0;
       }
     }
   }
   imshow("alpha2", alpha_image);
   cvWaitKey();
   char newname[30];
   int i = 0;
   while (name[i] != '.') {
     newname[i] = name[i];
     i++;
   }
   newname[i] = '_';
   newname[i + 1] = 'a';
   newname[i + 2] = 'l';
   newname[i + 3] = 'p';
   newname[i + 4] = 'h';
   newname[i + 5] = 'a';

   while (name[i - 1] != '\0') {
     newname[i + 6] = name[i];
     i++;
   }
   imwrite(newname, alpha_image);
   waitKey(0);
   
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    cout << "画像欲しい" << endl;
    return 0;
  }
  make_alpha(argv[1]);
}
