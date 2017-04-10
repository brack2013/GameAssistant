/*
 * =====================================================================================
 *
 *       Filename:  native_lib.c
 *
 *    Description:  JNI functions including
 *                  1. screen shot from frame buffer
 *                  2. simulate inputs
 *
 *
 *        Version:  1.0
 *        Created:  2015/6/23 17:27:47
 *       Revision:  none
 *       Compiler:  ndk
 *
 *         Author:  Jiang Lei
 *   Organization:
 *
 * =====================================================================================
 */


#include <jni.h>

#include <linux/fb.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "log.h"

#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"

using namespace cv;


// Screenshot

extern "C" {
    jint Java_com_monect_gameassistant_NativeLib_setTarget( JNIEnv* env, jobject thiz, jstring targetPath, jfloat roiX, jfloat roiY, jfloat roiWidth, jfloat roiHeight);
    jint Java_com_monect_gameassistant_NativeLib_findTargetInScreen( JNIEnv* env, jobject thiz, jfloatArray xArray, jfloatArray yArray);
    jboolean JNICALL Java_com_monect_gameassistant_NativeLib_OpenFrameBuffer(JNIEnv * env, jobject obj);
    jint Java_com_monect_gameassistant_NativeLib_openInputs( JNIEnv* env, jobject thiz, jint idevicetype, jint screenWidth, jint screenHeight);
    jint Java_com_monect_gameassistant_NativeLib_touchend( JNIEnv* env, jobject thiz);
    jint Java_com_monect_gameassistant_NativeLib_touchevent( JNIEnv* env, jobject thiz, jint id, jint ix, jint iy );
    jint Java_com_monect_gameassistant_NativeLib_mouseevent( JNIEnv* env, jobject thiz , jboolean bleft, jboolean bright, jint ix, jint iy);
    void Java_com_monect_gameassistant_NativeLib_saveScreenShot( JNIEnv* env, jobject thiz);
};

IplImage* origin_img;
Mat screenImg;

int l_fd_fb = 0;
unsigned char *l_pscreenvideo = 0;
int l_istride = 0;
int l_iscreensize = 0;

int screenWidth = 0;
int screenHeight = 0;

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

void coordinateTransform(float &x, float &y, float &width, float &height, float roiX, float roiY, float roiWidth, float roiHeight)
{
    x = 1 - roiY;
    y = roiX;
    width = roiHeight;
    height = roiWidth;
    x -= roiHeight;
}

jboolean JNICALL Java_com_monect_gameassistant_NativeLib_OpenFrameBuffer(JNIEnv * env, jobject obj)
{
    
    l_fd_fb = open("/dev/graphics/fb0", O_RDWR);
    if (l_fd_fb <= 0) {
        LOGE("Error: cannot open framebuffer device");
        return 0;
    }
    
    // Get fixed screen information
    if (ioctl(l_fd_fb, FBIOGET_FSCREENINFO, &finfo)) {
        LOGE("Error reading fixed information");
        return 0;
    }
    
    // Get variable screen information
    if (ioctl(l_fd_fb, FBIOGET_VSCREENINFO, &vinfo)) {
        LOGE("Error reading variable information");
        return 0;
    }
    
    LOGD("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
    LOGD(
         "xoffset:%d, yoffset:%d, line_length: %d", vinfo.xoffset, vinfo.yoffset, finfo.line_length);
    
    int ioffset = vinfo.xoffset * vinfo.bits_per_pixel / 8 + (vinfo.yoffset * finfo.line_length);
    
    l_iscreensize = finfo.line_length * vinfo.yres;
    l_istride = finfo.line_length;
    
    
    // Map the device to memory
    unsigned char *pfbuffer = (unsigned char *) mmap(0, ioffset + l_iscreensize,
                                                     PROT_READ | PROT_WRITE, MAP_SHARED, l_fd_fb, 0);
    if (pfbuffer <= 0) {
        LOGE("Error mmap");
        return 0;
    }
    
    l_pscreenvideo = pfbuffer + ioffset;
    
    // determin screen pixel format
    //int iscreenformat = PIX_FMT_RGBA;
    int ired_offset = vinfo.red.offset;
    int igreen_offset = vinfo.green.offset;
    int iblue_offset = vinfo.blue.offset;
    int ialpha_offset = vinfo.transp.offset;
    LOGE("red %d, green %d, blue %d, alpha %d", ired_offset, igreen_offset, iblue_offset, ialpha_offset);
    if(ired_offset == 24 && igreen_offset == 16 && iblue_offset == 8 && ialpha_offset == 0)
    {
        //iscreenformat = PIX_FMT_RGBA;
        LOGE("PIX_FMT_RGBA");
    }
    else if(ired_offset == 8 && igreen_offset == 16 && iblue_offset == 24 && ialpha_offset == 0)
    {
        //iscreenformat = PIX_FMT_ABGR;
        LOGE("PIX_FMT_ABGR");
    }
    else if(ired_offset == 0 && igreen_offset == 8 && iblue_offset == 16 && ialpha_offset == 24)
    {
        //iscreenformat = PIX_FMT_ARGB;
        LOGE("PIX_FMT_ARGB");
    }
    else if(ired_offset == 16 && igreen_offset == 8 && iblue_offset == 0 && ialpha_offset == 24)
    {
        //iscreenformat = PIX_FMT_BGRA;
        LOGE("PIX_FMT_BGRA");
    }
    else if(ired_offset == 16 && igreen_offset == 8 && iblue_offset == 0 && ialpha_offset == 0)
    {
        //iscreenformat = PIX_FMT_RGB24;
        LOGE("PIX_FMT_RGB24");
    }
    else if(ired_offset == 0 && igreen_offset == 8 && iblue_offset == 16 && ialpha_offset == 0)
    {
        //iscreenformat = PIX_FMT_BGR24;
        LOGE("PIX_FMT_BGR24");
    }
    
    
    
    screenWidth = vinfo.xres;
    screenHeight = vinfo.yres;
    
    origin_img = cvCreateImageHeader(cvSize(vinfo.xres, vinfo.yres), IPL_DEPTH_8U, 4);
    cvSetData(origin_img, l_pscreenvideo, finfo.line_length);
    screenImg = Mat(origin_img);
    
    return 1;
}






#include "input.h"
#include "uinput.h"

#define XINPUTS_MOUSE      		 0
#define XINPUTS_TOUCHSCREEN		 1


int                    l_fd;
jint Java_com_monect_gameassistant_NativeLib_openInputs( JNIEnv* env, jobject thiz, jint idevicetype, jint screenWidth, jint screenHeight )
{
    struct uinput_user_dev uidev;
    l_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    
    switch(idevicetype)
    {
        case XINPUTS_MOUSE:
        {
            if(ioctl(l_fd, UI_SET_EVBIT, EV_KEY) < 0)
                LOGE("error: ioctl");
            if(ioctl(l_fd, UI_SET_KEYBIT, BTN_LEFT) < 0)
                LOGE("error: ioctl");
            if(ioctl(l_fd, UI_SET_KEYBIT, BTN_RIGHT) < 0)
                LOGE("error: ioctl");
            
            if(ioctl(l_fd, UI_SET_EVBIT, EV_REL) < 0)
                LOGE("error: ioctl");
            
            if(ioctl(l_fd, UI_SET_RELBIT, REL_X) < 0)
                LOGE("error: ioctl");
            
            if(ioctl(l_fd, UI_SET_RELBIT, REL_Y) < 0)
                LOGE("error: ioctl");
            
            memset(&uidev, 0, sizeof(uidev));
            snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "monect_mouse");
            uidev.id.bustype = BUS_USB;
            uidev.id.vendor  = 0x1;
            uidev.id.product = 0x1;
            uidev.id.version = 1;
            //uidev.absmin[ABS_X] = 0;
            //uidev.absmax[ABS_X] = 1023;
            //uidev.absmin[ABS_Y] = 0;
            //uidev.absmax[ABS_Y] = 1023;
        }
            break;
            
        case XINPUTS_TOUCHSCREEN:
        {
            if(ioctl(l_fd, UI_SET_PROPBIT, INPUT_PROP_DIRECT) < 0)
                LOGE("error: ioctl");
            
            if(ioctl(l_fd, UI_SET_EVBIT, EV_ABS) < 0)
                LOGE("error: ioctl");
            
            if(ioctl(l_fd, UI_SET_ABSBIT, ABS_MT_SLOT) < 0)
                LOGE("error: ioctl");
            
            if(ioctl(l_fd, UI_SET_ABSBIT, ABS_MT_TOUCH_MAJOR) < 0)
                LOGE("error: ioctl");
            
            if(ioctl(l_fd, UI_SET_ABSBIT, ABS_MT_POSITION_X) < 0)
                LOGE("error: ioctl");
            
            if(ioctl(l_fd, UI_SET_ABSBIT, ABS_MT_POSITION_Y) < 0)
                LOGE("error: ioctl");
            
            if(ioctl(l_fd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID) < 0)
                LOGE("error: ioctl");
            
            if(ioctl(l_fd, UI_SET_ABSBIT, ABS_MT_PRESSURE) < 0)
                LOGE("error: ioctl");
            
            
            memset(&uidev, 0, sizeof(uidev));
            snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "monect_mtouch");
            uidev.id.bustype = BUS_VIRTUAL;
            uidev.id.vendor  = 0x1;
            uidev.id.product = 0x1;
            uidev.id.version = 1;
            uidev.absmin[ABS_MT_SLOT] = 0;
            uidev.absmax[ABS_MT_SLOT] = 9;
            uidev.absmin[ABS_MT_TOUCH_MAJOR] = 0;
            uidev.absmax[ABS_MT_TOUCH_MAJOR] = 30;
            uidev.absmin[ABS_MT_POSITION_X] = 0;
            uidev.absmax[ABS_MT_POSITION_X] = screenHeight - 1;
            uidev.absmin[ABS_MT_POSITION_Y] = 0;
            uidev.absmax[ABS_MT_POSITION_Y] = screenWidth - 1;
            uidev.absmin[ABS_MT_TRACKING_ID] = 0;
            uidev.absmax[ABS_MT_TRACKING_ID] = 65535;
            uidev.absmin[ABS_MT_PRESSURE] = 0;
            uidev.absmax[ABS_MT_PRESSURE] = 255;
        }
            break;
    }
    
    if(write(l_fd, &uidev, sizeof(uidev)) < 0)
        LOGE("error: write");
    
    if(ioctl(l_fd, UI_DEV_CREATE) < 0)
        LOGE("error: ioctl");
    
    return l_fd;
}

jint Java_com_monect_gameassistant_NativeLib_touchend( JNIEnv* env, jobject thiz)
{
    
    struct input_event     ev;
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
        LOGE("error: write");
}

jint Java_com_monect_gameassistant_NativeLib_touchevent( JNIEnv* env, jobject thiz, jint id, jint ix, jint iy )
{
    struct input_event     ev;
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_ABS;
    ev.code = ABS_MT_TRACKING_ID;
    ev.value = id;
    if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
        LOGE("error: write");
    
    if(ix >= 0 && iy >= 0)
    {
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_ABS;
    ev.code = ABS_MT_POSITION_X;
    ev.value = ix;
    if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
        LOGE("error: write");
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_ABS;
    ev.code = ABS_MT_POSITION_Y;
    ev.value = iy;
    if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
        LOGE("error: write");
        
        
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_ABS;
        ev.code = ABS_MT_PRESSURE;
        ev.value = 0x33;
        if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
            LOGE("error: write");
        
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_ABS;
        ev.code = ABS_MT_TOUCH_MAJOR;
        ev.value = id;
        if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
            LOGE("error: write");
    
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
        LOGE("error: write");
    }
    
    return 0;
}



jint Java_com_monect_gameassistant_NativeLib_mouseevent( JNIEnv* env, jobject thiz , jboolean bleft, jboolean bright, jint ix, jint iy)
{
    struct input_event     ev;
    
    if(bleft)
    {
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_KEY;
        ev.code = BTN_LEFT;
        ev.value = 1;
        if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
            LOGE("error: write");
        
        
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_SYN;
        ev.code = 0;
        ev.value = 0;
        if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
            LOGE("error: write");
        
        
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_KEY;
        ev.code = BTN_LEFT;
        ev.value = 0;
        if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
            LOGE("error: write");
        
        
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_SYN;
        ev.code = 0;
        ev.value = 0;
        if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
            LOGE("error: write");
    }
    
    
    if(bright)
    {
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_KEY;
        ev.code = BTN_RIGHT;
        ev.value = 1;
        if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
            LOGE("error: write");
        
        
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_SYN;
        ev.code = 0;
        ev.value = 0;
        if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
            LOGE("error: write");
        
        
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_KEY;
        ev.code = BTN_RIGHT;
        ev.value = 0;
        if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
            LOGE("error: write");
        
        
        memset(&ev, 0, sizeof(struct input_event));
        ev.type = EV_SYN;
        ev.code = 0;
        ev.value = 0;
        if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
            LOGE("error: write");
    }
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_REL;
    ev.code = REL_X;
    ev.value = ix;
    if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
        LOGE("error: write");
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_REL;
    ev.code = REL_Y;
    ev.value = iy;
    if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
        LOGE("error: write");
    
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    if(write(l_fd, &ev, sizeof(struct input_event)) < 0)
        LOGE("error: write");
}

void Java_com_monect_gameassistant_NativeLib_closeInputs( JNIEnv* env, jobject thiz )
{
    if(ioctl(l_fd, UI_DEV_DESTROY) < 0)
        LOGE("error: ioctl");
    close(l_fd);
}



Mat img_object;
float roi_x = 0;
float roi_y = 0;
float roi_width = 0;
float roi_height = 0;

void rot90(cv::Mat &matImage, int rotflag)
{
    //1=CW, 2=CCW, 3=180
    if (rotflag == 1){
        transpose(matImage, matImage);
        flip(matImage, matImage,1); //transpose+flip(1)=CW
    } else if (rotflag == 2) {
        transpose(matImage, matImage);
        flip(matImage, matImage,0); //transpose+flip(0)=CCW
    } else if (rotflag ==3){
        flip(matImage, matImage,-1);    //flip(-1)=180
    } else if (rotflag != 0){ //if not 0,1,2,3:
        //cout  << "Unknown rotation flag(" << rotflag << ")" << endl;
    }
}

Mat rotateImage(const Mat& source, double angle)
{
    Point2f src_center(source.cols / 2.0, source.rows / 2.0);
    Mat rot_mat = getRotationMatrix2D(src_center, 270, 1.0);
    Mat dst;
    warpAffine(source, dst, rot_mat, Size(source.rows, source.cols));
    return dst;
}

jint Java_com_monect_gameassistant_NativeLib_setTarget( JNIEnv* env, jobject thiz, jstring targetPath, jfloat roiX, jfloat roiY, jfloat roiWidth, jfloat roiHeight)
{
    origin_img = cvCreateImageHeader(cvSize(vinfo.xres, vinfo.yres), IPL_DEPTH_8U, 4);
    
    cvSetData(origin_img, l_pscreenvideo, finfo.line_length);
    
    
    coordinateTransform(roi_x, roi_y, roi_width, roi_height, roiX, roiY, roiWidth, roiHeight);
    
    roi_x = roi_x * screenWidth;
    roi_y = roi_y * screenHeight;
    roi_width = roi_width * screenWidth;
    roi_height = roi_height * screenHeight;
    
    
    LOGE("cvSetImageROI %f, %f, %f, %f", roi_x, roi_y, roi_width, roi_height);
    
    cvSetImageROI(origin_img, cvRect(roi_x, roi_y, roi_width, roi_height));
    
    screenImg = Mat(origin_img);
    
    const char *str = (env)->GetStringUTFChars(targetPath, 0);
    
    
    img_object = imread(str);
    //LOGE("a type %d", a.type());
    //a.convertTo(img_object, screenImg.type());
    //img_object = a;
    //cvtColor(a, img_object, CV_BGR2GRAY);
    
    rot90(img_object, 1);
    //img_object = rotateImage(img_object, 270);
    
    (env)->ReleaseStringUTFChars(targetPath, str);
}

//jint Java_com_monect_gameassistant_NativeLib_findTargetInScreen( JNIEnv* env, jobject thiz, jfloatArray xArray, jfloatArray yArray)
//{
//    
//    Mat gray;
//    cvtColor(screenImg, gray, CV_BGR2GRAY);
//   // imwrite("/sdcard/3target.jpg", img_object);
//   // imwrite("/sdcard/3scrimg.jpg", gray);
//    
//    float *xs = (env)->GetFloatArrayElements( xArray, NULL);
//    float *ys = (env)->GetFloatArrayElements( yArray, NULL);
//    
//    /// Create the result matrix
//    //int match_method = CV_TM_CCOEFF_NORMED;
//    int match_method = CV_TM_SQDIFF_NORMED;
//    Mat result;
//    int result_cols =  gray.cols - img_object.cols + 1;
//    int result_rows = gray.rows - img_object.rows + 1;
//    
//    result.create( result_rows, result_cols, CV_32FC1 );
//    
////    
////    LOGE("screen channels %d", screenImg.channels());
////    LOGE("img_object channels %d", img_object.channels());
//    
//    /// Do the Matching and Normalize
//    matchTemplate( gray, img_object, result, match_method );
//    //normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );
//    
//    
//    /// Localizing the best match with minMaxLoc
//    double minVal; double maxVal; Point minLoc; Point maxLoc;
//    Point matchLoc;
//    
//    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
//    
//    
//    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
//    if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
//    { matchLoc = minLoc; }
//    else
//    { matchLoc = maxLoc; }
//    
//    
//    //-- Show detected matches
//    xs[0] = matchLoc.x + roi_x;
//    xs[1] = matchLoc.x + img_object.cols + roi_x;
//    xs[2] = matchLoc.x + img_object.cols + roi_x;
//    xs[3] = matchLoc.x + roi_x;
//    
//    ys[0] = matchLoc.y + roi_y;
//    ys[1] = matchLoc.y + roi_y;
//    ys[2] = matchLoc.y + img_object.rows + roi_y;
//    ys[3] = matchLoc.y + img_object.rows + roi_y;
//    
//    LOGE("max min : (%f, %f)", maxVal, minVal);
//    
//    env->ReleaseFloatArrayElements( xArray, xs, 0);
//    env->ReleaseFloatArrayElements( yArray, ys, 0);
//    
//    return maxVal * 100;
//}


jint Java_com_monect_gameassistant_NativeLib_findTargetInScreen( JNIEnv* env, jobject thiz, jfloatArray xArray, jfloatArray yArray)
{
    
    Mat gray = screenImg;
    //cvtColor(screenImg, gray, CV_BGRA2GRAY);
//    imwrite("/sdcard/3target.jpg", img_object);
//    imwrite("/sdcard/3scrimg.jpg", gray);
    
    float *xs = (env)->GetFloatArrayElements( xArray, NULL);
    float *ys = (env)->GetFloatArrayElements( yArray, NULL);
    
    if( !img_object.data || !gray.data )
    { LOGE(" --(!) Error reading images "); return -1; }
    
    //LOGE(" Step 1");
    //-- Step 1: Detect the keypoints using SURF Detector
    int minHessian = 50;
    SurfFeatureDetector detector( minHessian );
    
    //FastFeatureDetector detector(15);
    
    std::vector<KeyPoint> keypoints_object, keypoints_scene;
    
    detector.detect( img_object, keypoints_object );
    detector.detect( gray, keypoints_scene );
    
    
    //LOGE(" Step 2");
    //-- Step 2: Calculate descriptors (feature vectors)
    SurfDescriptorExtractor extractor;
    
    Mat descriptors_object, descriptors_scene;
    
    
    extractor.compute( img_object, keypoints_object, descriptors_object );
    extractor.compute( gray, keypoints_scene, descriptors_scene );
    
    
    if(descriptors_object.type() != CV_32F)
    {
        env->ReleaseFloatArrayElements( xArray, xs, 0);
        env->ReleaseFloatArrayElements( yArray, ys, 0);
        return 100;
    }
    
    if(descriptors_scene.type() != CV_32F)
    {
        env->ReleaseFloatArrayElements( xArray, xs, 0);
        env->ReleaseFloatArrayElements( yArray, ys, 0);
        return 100;
    }
    
    //LOGE(" Step 3");
    //-- Step 3: Matching descriptor vectors using FLANN matcher
    FlannBasedMatcher matcher;
    std::vector< DMatch > matches;
    matcher.match( descriptors_object, descriptors_scene, matches );
    
    double max_dist = 0; double min_dist = 100;
    
    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_object.rows; i++ )
    {
        double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }
    
    
    LOGE("-- Max dist : %f \n", max_dist );
    LOGE("-- Min dist : %f \n", min_dist );
    
    //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
    std::vector< DMatch > good_matches;
    
    for( int i = 0; i < descriptors_object.rows; i++ )
    {
        if( matches[i].distance < 3 * min_dist )
        {
            good_matches.push_back( matches[i]);
        }
    }
    
    if(good_matches.size() < 4)
    {
        LOGE("Not enough good matches : %d \n", good_matches.size() );
        env->ReleaseFloatArrayElements( xArray, xs, 0);
        env->ReleaseFloatArrayElements( yArray, ys, 0);
        return 100;
    }
    
//    Mat img_matches;
//    drawMatches( img_object, keypoints_object, screenImg, keypoints_scene,
//                good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
//                vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    
    //-- Localize the object
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    
    for( int i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
    }
    
    
    Mat H = findHomography( obj, scene, CV_RANSAC );
    
    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( img_object.cols, 0 );
    obj_corners[2] = cvPoint( img_object.cols, img_object.rows ); obj_corners[3] = cvPoint( 0, img_object.rows );
    std::vector<Point2f> scene_corners(4);
    
    perspectiveTransform( obj_corners, scene_corners, H);
    
    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
//    line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 4 );
//    line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
//    line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
//    line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
    
    //-- Show detected matches
    xs[0] = scene_corners[0].x + roi_x;
    xs[1] = scene_corners[1].x + roi_x;
    xs[2] = scene_corners[2].x + roi_x;
    xs[3] = scene_corners[3].x + roi_x;
    
    ys[0] = scene_corners[0].y + roi_y;
    ys[1] = scene_corners[1].y + roi_y;
    ys[2] = scene_corners[2].y + roi_y;
    ys[3] = scene_corners[3].y + roi_y;
    
    LOGE("good_matches num = %d", good_matches.size());
    
    env->ReleaseFloatArrayElements( xArray, xs, 0);
    env->ReleaseFloatArrayElements( yArray, ys, 0);
    
    return min_dist * 100;
}

void Java_com_monect_gameassistant_NativeLib_saveScreenShot( JNIEnv* env, jobject thiz)
{
    Mat gray;
    cvtColor(screenImg, gray, CV_BGRA2GRAY);
    imwrite("/sdcard/11111111.jpg", gray);
}



