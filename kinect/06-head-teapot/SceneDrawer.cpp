using namespace std;
#include <vector>
#include <math.h>
#include <iostream>

#include "SceneDrawer.h"

#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif

#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
using namespace cv;


extern xn::UserGenerator g_UserGenerator;
extern xn::DepthGenerator g_DepthGenerator;

extern XnBool g_bDrawBackground;
extern XnBool g_bDrawPixels;
extern XnBool g_bDrawSkeleton;
extern XnBool g_bPrintID;
extern XnBool g_bPrintState;
extern XnBool g_bDrawTexture;
extern XnBool g_bDrawPerson;
extern XnBool g_bPrintFrameID;
extern XnBool g_bMarkJoints;

#include <map>
std::map<XnUInt32, std::pair<XnCalibrationStatus, XnPoseDetectionStatus> > m_Errors;

//追加プロトタイプ宣言
void DrawPerson(XnUserID player);
void DrawPoint(XnUserID player);
void DrawFace(double x, double y, double radius);

void XN_CALLBACK_TYPE MyCalibrationInProgress(xn::SkeletonCapability& /*capability*/, XnUserID id, XnCalibrationStatus calibrationError, void* /*pCookie*/)
{
    m_Errors[id].first = calibrationError;
}

void XN_CALLBACK_TYPE MyPoseInProgress(xn::PoseDetectionCapability& /*capability*/, const XnChar* /*strPose*/, XnUserID id, XnPoseDetectionStatus poseError, void* /*pCookie*/)
{
    m_Errors[id].second = poseError;
}

unsigned int getClosestPowerOfTwo(unsigned int n)
{
    unsigned int m = 2;
    while(m < n) m<<=1;

    return m;
}
GLuint initTexture(void** buf, int& width, int& height)
{
    GLuint texID = 0;
    glGenTextures(1,&texID);

    width = getClosestPowerOfTwo(width);
    height = getClosestPowerOfTwo(height);
    *buf = new unsigned char[width*height*4];
    glBindTexture(GL_TEXTURE_2D,texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texID;
}

GLfloat texcoords[8];


void DrawRectangle(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
{
    GLfloat verts[8] = {
        topLeftX,     topLeftY,
        topLeftX,     bottomRightY,
        bottomRightX, bottomRightY,
        bottomRightX, topLeftY};
    glVertexPointer(2, GL_FLOAT, 0, verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glFlush();
}


void DrawTexture(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
{
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

    DrawRectangle(topLeftX, topLeftY, bottomRightX, bottomRightY);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

XnFloat Colors[][3] =
{
    {0,1,1},
    {0,0,1},
    {0,1,0},
    {1,1,0},
    {1,0,0},
    {1,.5,0},
    {.5,1,0},
    {0,.5,1},
    {.5,0,1},
    {1,1,.5},
    {1,1,1}
};
XnUInt32 nColors = 10;



void glPrintString(void *font, char *str)
{
    int i, l = (int)strlen(str);

    for (i = 0; i < l; ++i) {
        glutBitmapCharacter(font, *str++);
    }
}

bool DrawLimb(XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2)
{
    if (!g_UserGenerator.GetSkeletonCap().IsTracking(player)) {
        printf("not tracked!\n");
        return true;
    }

    if (!g_UserGenerator.GetSkeletonCap().IsJointActive(eJoint1) ||
        !g_UserGenerator.GetSkeletonCap().IsJointActive(eJoint2)) {
        return false;
    }

    XnSkeletonJointPosition joint1, joint2;
    g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
    g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);

    if (joint1.fConfidence < 0.5 || joint2.fConfidence < 0.5) {
        return true;
    }

    XnPoint3D pt[2];
    pt[0] = joint1.position;
    pt[1] = joint2.position;

    g_DepthGenerator.ConvertRealWorldToProjective(2, pt, pt);

    glVertex3i(pt[0].X, pt[0].Y, 0);
    glVertex3i(pt[1].X, pt[1].Y, 0);

    return true;
}

static const float DEG2RAD = 3.14159/180;



void drawCircle(float x, float y, float radius)
{
   glBegin(GL_TRIANGLE_FAN);

   for (int i = 0; i < 360; ++i) {
      float degInRad = i * DEG2RAD;
      glVertex2f(x + cos(degInRad) * radius, y + sin(degInRad) * radius);
   }

   glEnd();
}



void DrawJoint(XnUserID player, XnSkeletonJoint eJoint)
{
    if (!g_UserGenerator.GetSkeletonCap().IsTracking(player)) {
        printf("not tracked!\n");
        return;
    }

    if (!g_UserGenerator.GetSkeletonCap().IsJointActive(eJoint)) {
        return;
    }

    XnSkeletonJointPosition joint;
    g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint, joint);

    if (joint.fConfidence < 0.5) {
        return;
    }

    XnPoint3D pt;
    pt = joint.position;

    g_DepthGenerator.ConvertRealWorldToProjective(1, &pt, &pt);

    drawCircle(pt.X, pt.Y, 2);
}

const XnChar* GetCalibrationErrorString(XnCalibrationStatus error)
{
    switch (error) {
    case XN_CALIBRATION_STATUS_OK:
        return "OK";
    case XN_CALIBRATION_STATUS_NO_USER:
        return "NoUser";
    case XN_CALIBRATION_STATUS_ARM:
        return "Arm";
    case XN_CALIBRATION_STATUS_LEG:
        return "Leg";
    case XN_CALIBRATION_STATUS_HEAD:
        return "Head";
    case XN_CALIBRATION_STATUS_TORSO:
        return "Torso";
    case XN_CALIBRATION_STATUS_TOP_FOV:
        return "Top FOV";
    case XN_CALIBRATION_STATUS_SIDE_FOV:
        return "Side FOV";
    case XN_CALIBRATION_STATUS_POSE:
        return "Pose";
    default:
        return "Unknown";
    }
}

const XnChar* GetPoseErrorString(XnPoseDetectionStatus error)
{
    switch (error) {
    case XN_POSE_DETECTION_STATUS_OK:
        return "OK";
    case XN_POSE_DETECTION_STATUS_NO_USER:
        return "NoUser";
    case XN_POSE_DETECTION_STATUS_TOP_FOV:
        return "Top FOV";
    case XN_POSE_DETECTION_STATUS_SIDE_FOV:
        return "Side FOV";
    case XN_POSE_DETECTION_STATUS_ERROR:
        return "General error";
    default:
        return "Unknown";
    }
}

void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, Mat img)
{
    static bool bInitialized = false;
    static GLuint depthTexID;
    static unsigned char* pDepthTexBuf;
    static int texWidth, texHeight;

    float topLeftX;
    float topLeftY;
    float bottomRightY;
    float bottomRightX;
    float texXpos;
    float texYpos;
    if(!bInitialized) {
        texWidth =  getClosestPowerOfTwo(dmd.XRes());
        texHeight = getClosestPowerOfTwo(dmd.YRes());
	
//      printf("Initializing depth texture: width = %d, height = %d\n", texWidth, texHeight);
        depthTexID = initTexture((void**)&pDepthTexBuf, texWidth, texHeight) ;
	
//      printf("Initialized depth texture: width = %d, height = %d\n", texWidth, texHeight);
        bInitialized = true;
	
        topLeftX = dmd.XRes();
        topLeftY = 0;
        bottomRightY = dmd.YRes();
        bottomRightX = 0;
        texXpos =(float)dmd.XRes()/texWidth;
        texYpos  =(float)dmd.YRes()/texHeight;
	
        memset(texcoords, 0, 8*sizeof(float));
        texcoords[0] = texXpos, texcoords[1] = texYpos, texcoords[2] = texXpos, texcoords[7] = texYpos;
    }
    
    unsigned int nValue = 0;
    unsigned int nHistValue = 0;
    unsigned int nIndex = 0;
    unsigned int nX = 0;
    unsigned int nY = 0;
    unsigned int nNumberOfPoints = 0;
    XnUInt16 g_nXRes = dmd.XRes();
    XnUInt16 g_nYRes = dmd.YRes();

    unsigned char* pDestImage = pDepthTexBuf;

    const XnDepthPixel* pDepth = dmd.Data();
    const XnLabel* pLabels = smd.Data();

    static unsigned int nZRes = dmd.ZRes();
    static float* pDepthHist = (float*)malloc(nZRes* sizeof(float));

    // Calculate the accumulative histogram
    memset(pDepthHist, 0, nZRes*sizeof(float));
    for (nY = 0; nY < g_nYRes; ++nY) {
        for (nX = 0; nX < g_nXRes; ++nX) {
            nValue = *pDepth;

            if (nValue != 0) {
	        pDepthHist[nValue]++;
                nNumberOfPoints++;
            }

            pDepth++;
        }
    }

    for (nIndex = 1; nIndex < nZRes; nIndex++) {
        pDepthHist[nIndex] += pDepthHist[nIndex-1];
    }
    if (nNumberOfPoints) {
        for (nIndex = 1; nIndex < nZRes; nIndex++) {
            pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (pDepthHist[nIndex] / nNumberOfPoints)));
        }
    }

    
    pDepth = dmd.Data();
    if (g_bDrawPixels) {
        XnUInt32 nIndex = 0;
        // Prepare the texture map
        for (nY = 0; nY < g_nYRes; nY++) {
            for (nX = 0; nX < g_nXRes; nX++, nIndex++) {
                pDestImage[0] = 0;
                pDestImage[1] = 0;
                pDestImage[2] = 0;
                if (g_bDrawBackground || *pLabels != 0) {
                    nValue = *pDepth;
                    XnLabel label = *pLabels;
                    XnUInt32 nColorID = label % nColors;
                    if (label == 0) {
                        nColorID = nColors;
                    }

                    if (nValue != 0) {
                        nHistValue = pDepthHist[nValue];

                        pDestImage[0] = nHistValue * Colors[nColorID][0];
                        pDestImage[1] = nHistValue * Colors[nColorID][1];
                        pDestImage[2] = nHistValue * Colors[nColorID][2];
                    }
                }

                pDepth++;
                pLabels++;
                pDestImage+=3;
            }

            pDestImage += (texWidth - g_nXRes) * 3;
        }
    } else {
        xnOSMemSet(pDepthTexBuf, 0, 3 * 2 * g_nXRes * g_nYRes);
    }

    glBindTexture(GL_TEXTURE_2D, depthTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pDepthTexBuf);

    // Display the OpenGL texture map
    if (g_bDrawTexture) {
      glColor4f(0.75, 0.75, 0.75, 1);

      glEnable(GL_TEXTURE_2D);
      glPushMatrix();
      glTranslated(0.0, 0.0, -100.0);
      DrawTexture(dmd.XRes(), dmd.YRes(), 0, 0);
      glPopMatrix();
      glDisable(GL_TEXTURE_2D);
    }
    char strLabel[50] = "";
    XnUserID aUsers[15];
    XnUInt16 nUsers = 15;
    g_UserGenerator.GetUsers(aUsers, nUsers);
    for (int i = 0; i < nUsers; ++i) {
        if (g_bPrintID) {
            XnPoint3D com;
            g_UserGenerator.GetCoM(aUsers[i], com);
            g_DepthGenerator.ConvertRealWorldToProjective(1, &com, &com);

            XnUInt32 nDummy = 0;

            xnOSMemSet(strLabel, 0, sizeof(strLabel));
            if (!g_bPrintState) {
                // Tracking
                xnOSStrFormat(strLabel, sizeof(strLabel), &nDummy, "%d", aUsers[i]);
            } else if (g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])) {
                // Tracking
                xnOSStrFormat(strLabel, sizeof(strLabel), &nDummy, "%d - Tracking", aUsers[i]);
            } else if (g_UserGenerator.GetSkeletonCap().IsCalibrating(aUsers[i])) {
                // Calibrating
                xnOSStrFormat(strLabel, sizeof(strLabel), &nDummy, "%d - Calibrating [%s]", aUsers[i], GetCalibrationErrorString(m_Errors[aUsers[i]].first));
            } else {
                // Nothing
                xnOSStrFormat(strLabel, sizeof(strLabel), &nDummy, "%d - Looking for pose [%s]", aUsers[i], GetPoseErrorString(m_Errors[aUsers[i]].second));
            }

            glColor4f(1-Colors[i%nColors][0], 1-Colors[i%nColors][1], 1-Colors[i%nColors][2], 1);

            glRasterPos2i(com.X, com.Y);
            glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
        }
	////////////////////////////////////
	if (g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i]) && g_bDrawPerson ) {
	  DrawPerson(aUsers[i]);
	}

        if (g_bDrawSkeleton && g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])) {
            glColor4f(1-Colors[aUsers[i]%nColors][0], 1-Colors[aUsers[i]%nColors][1], 1-Colors[aUsers[i]%nColors][2], 1);

            // Draw Joints
            if (g_bMarkJoints) {
	        // Try to draw all joints
                DrawJoint(aUsers[i], XN_SKEL_HEAD);
                DrawJoint(aUsers[i], XN_SKEL_NECK);
                DrawJoint(aUsers[i], XN_SKEL_TORSO);
                DrawJoint(aUsers[i], XN_SKEL_WAIST);

                DrawJoint(aUsers[i], XN_SKEL_LEFT_COLLAR);
                DrawJoint(aUsers[i], XN_SKEL_LEFT_SHOULDER);
                DrawJoint(aUsers[i], XN_SKEL_LEFT_ELBOW);
                DrawJoint(aUsers[i], XN_SKEL_LEFT_WRIST);
                DrawJoint(aUsers[i], XN_SKEL_LEFT_HAND);
                DrawJoint(aUsers[i], XN_SKEL_LEFT_FINGERTIP);

                DrawJoint(aUsers[i], XN_SKEL_RIGHT_COLLAR);
                DrawJoint(aUsers[i], XN_SKEL_RIGHT_SHOULDER);
                DrawJoint(aUsers[i], XN_SKEL_RIGHT_ELBOW);
                DrawJoint(aUsers[i], XN_SKEL_RIGHT_WRIST);
                DrawJoint(aUsers[i], XN_SKEL_RIGHT_HAND);
                DrawJoint(aUsers[i], XN_SKEL_RIGHT_FINGERTIP);

                DrawJoint(aUsers[i], XN_SKEL_LEFT_HIP);
                DrawJoint(aUsers[i], XN_SKEL_LEFT_KNEE);
                DrawJoint(aUsers[i], XN_SKEL_LEFT_ANKLE);
                DrawJoint(aUsers[i], XN_SKEL_LEFT_FOOT);

                DrawJoint(aUsers[i], XN_SKEL_RIGHT_HIP);
                DrawJoint(aUsers[i], XN_SKEL_RIGHT_KNEE);
                DrawJoint(aUsers[i], XN_SKEL_RIGHT_ANKLE);
                DrawJoint(aUsers[i], XN_SKEL_RIGHT_FOOT);
            }

            //DrawTeapot(aUsers[i], XN_SKEL_HEAD, img);
	    //DrawPoint(aUsers[i]);
	    glBegin(GL_LINES);

            // Draw Limbs
            DrawLimb(aUsers[i], XN_SKEL_HEAD, XN_SKEL_NECK);

            DrawLimb(aUsers[i], XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
            DrawLimb(aUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
            if (!DrawLimb(aUsers[i], XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_WRIST)) {
                DrawLimb(aUsers[i], XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);
            } else {
                DrawLimb(aUsers[i], XN_SKEL_LEFT_WRIST, XN_SKEL_LEFT_HAND);
                DrawLimb(aUsers[i], XN_SKEL_LEFT_HAND, XN_SKEL_LEFT_FINGERTIP);
            }

            DrawLimb(aUsers[i], XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
            DrawLimb(aUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
            if (!DrawLimb(aUsers[i], XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_WRIST)) {
                DrawLimb(aUsers[i], XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);
            } else {
                DrawLimb(aUsers[i], XN_SKEL_RIGHT_WRIST, XN_SKEL_RIGHT_HAND);
                DrawLimb(aUsers[i], XN_SKEL_RIGHT_HAND, XN_SKEL_RIGHT_FINGERTIP);
            }

            DrawLimb(aUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
            DrawLimb(aUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);

            DrawLimb(aUsers[i], XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
            DrawLimb(aUsers[i], XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
            DrawLimb(aUsers[i], XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);

            DrawLimb(aUsers[i], XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
            DrawLimb(aUsers[i], XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
            DrawLimb(aUsers[i], XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);

            DrawLimb(aUsers[i], XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP);

	    
            glEnd();
        }
    }
}

void DrawFace(double x, double y, double radius) {
  glPushMatrix();
  {
    glColor4d(1.0, 0.0, 0.0, 1.0);
    glTranslated(x, y, -1);
    drawCircle(0, 0, radius);
    glPushMatrix();
    {
      glTranslated(0, -10, 1);
      glScalef(1, 2, 1);
      glColor4d(0, 0, 0, 1.0);
      drawCircle(-15, 0, 7);
      drawCircle(15, 0, 7);
    }
    glPopMatrix();
  }
  glPopMatrix();
    
}

void DrawPoint(XnUserID player) {
    if (!g_UserGenerator.GetSkeletonCap().IsTracking(player)) {
        printf("not tracked!\n");
        return;
    }

    XnSkeletonJoint Points[25];
    for (int i = 0; i < 25; i++) {
      Points[i] = (XnSkeletonJoint)i;
    }

    for (int i = 0; i < 25; i++) {
      if (!g_UserGenerator.GetSkeletonCap().IsJointActive(Points[i])) {
        continue;
      }
      XnSkeletonJointPosition Joint;
      g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, Points[i], Joint);
      if (Joint.fConfidence < 0.5) {
	continue;
      }
      XnPoint3D Pt;
      Pt = Joint.position;
      g_DepthGenerator.ConvertRealWorldToProjective(1, &Pt, &Pt);
      glPushMatrix();
      {
        glColor4d(1.0, 1.0, 1.0, 1.0);
	glTranslated(Pt.X, Pt.Y, 0);
	glRotated(180.0, 1.0, 0.0, 0.0);
	glutSolidCube(10);
      }
      glPopMatrix();
    }
}


void DrawPerson(XnUserID player)
{
  if (!g_UserGenerator.GetSkeletonCap().IsTracking(player)) {
    printf("not tracked!\n");
    return;
  }

  
  //腕
  vector<vector<XnSkeletonJoint> > arm(4, vector<XnSkeletonJoint>());
  arm[0] = {XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND};
  arm[1] = {XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND};
  arm[2] = {XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT};
  arm[3] = {XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT};
  
  for (int i = 0; i < 4; i++) {
    vector<XnSkeletonJointPosition> armJoint(3);
    vector<XnPoint3D> armPt(3);
    for (int j = 0; j < 3; j++) {
      if (!g_UserGenerator.GetSkeletonCap().IsJointActive(arm[i][j])) {
	cout << i << " " << j << endl;
	return;
      }
      g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, arm[i][j], armJoint[j]);
      if (armJoint[j].fConfidence < 0.5) {
	continue;
      }
      armPt[j] = armJoint[j].position;
      g_DepthGenerator.ConvertRealWorldToProjective(1, &armPt[j], &armPt[j]);
    }

    vector<double> theta(3), dist(2);
    for (int j = 0; j < 2; j++) {
      theta[j + 1] = atan2(armPt[j + 1].Y - armPt[j].Y, armPt[j + 1].X - armPt[j].X);
      theta[j + 1] *= (180 / M_PI);
      dist[j] = sqrt(pow(armPt[j + 1].Y - armPt[j].Y, 2) + pow(armPt[j + 1].X - armPt[j].X, 2));
      dist[j] /= 1.8;
    }
    
    glPushMatrix();
    {
      glColor4d(1.0, 1.0, 1.0, 1.0);
      glTranslated(armPt[0].X, armPt[0].Y, 0);
      for (int j = 0; j < 2; j++) {
	glPushMatrix();
	{
	  glRotated(theta[j + 1] - theta[j], 0, 0, 1);
	  glTranslated(dist[j] * 1.8 / 4.0, 0, 0);
	  glPushMatrix();
	  {
	    glScalef(dist[j] / 15, 1, 1);
	    glutSolidCube(15);
	  }
	  glPopMatrix();
	  glTranslated(dist[j] * 1.8 / 4.0, 0, 0);
	}
      }
      for (int j = 0; j < 2; j++) {
	glPopMatrix();
      }
    }
    glPopMatrix();
  }

  // 胴体
  XnSkeletonJoint bodyParts[4] = {XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_HIP, XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_SHOULDER};
  XnSkeletonJointPosition bodyJoint[4];
  XnPoint3D bodyPt[4];
  for (int i = 0; i < 4; i++) {
      
    if (!g_UserGenerator.GetSkeletonCap().IsJointActive(bodyParts[i])) {
      cout << "not Active" << endl;
      return;
    }
    g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, bodyParts[i], bodyJoint[i]);
    if (bodyJoint[i].fConfidence < 0.5) {
      return;
    }
    bodyPt[i] = bodyJoint[i].position;
    g_DepthGenerator.ConvertRealWorldToProjective(1, &bodyPt[i], &bodyPt[i]);
  }

  
  glPushMatrix();
  {
    glColor4d(1.0, 1.0, 0.0, 1.0);
    double minX = 100000000, minY = 100000000, maxX = -1, maxY = -1;
    for (int i = 0; i < 4; i++) {
      minX = min(minX, (double)bodyPt[i].X);
      minY = min(minY, (double)bodyPt[i].Y);
      maxX = max(maxX, (double)bodyPt[i].X);
      maxY = max(maxY, (double)bodyPt[i].Y);
    }
    DrawRectangle(minX, minY, maxX, maxY);
  }
  glPopMatrix();


  
  // 頭
  if (!g_UserGenerator.GetSkeletonCap().IsJointActive(XN_SKEL_HEAD)) {
    return;
  }
  XnSkeletonJointPosition headJoint;
  g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, XN_SKEL_HEAD, headJoint);
  if (headJoint.fConfidence < 0.5) {
    return;
  }
  XnPoint3D headPt;
  headPt = headJoint.position;
  g_DepthGenerator.ConvertRealWorldToProjective(1, &headPt, &headPt);
  DrawFace(headPt.X, headPt.Y + 20, 50);
  
  
}
