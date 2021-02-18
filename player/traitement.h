
#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgcodecs.hpp>
#include <opencv/cv.h>


#define WINDOWS

#ifdef WINDOWS

#include "NVXIO/Application.hpp"
#include "NVXIO/FrameSource.hpp"
#include "NVXIO/Render.hpp"
#include "NVXIO/SyncTimer.hpp"
#include "NVXIO/Utility.hpp"
#include <NVX\nvx.h>
#include <NVX\nvx_opencv_interop.hpp>
#include <NVX\nvxcu.h>

using namespace nvxio;
#else


#include <VX/vx.h>
#include <NVX/nvx_timer.hpp>

#include "NVX/Application.hpp"
#include "OVX/FrameSourceOVX.hpp"
#include "OVX/RenderOVX.hpp"
#include "NVX/FrameSource.hpp"
#include "NVX/Render.hpp"
#include "NVX/SyncTimer.hpp"
#include "OVX/UtilityOVX.hpp"
#include "NVX/Utility.hpp"
#include "NVX/nvx_opencv_interop.hpp"

using namespace nvxio;
using namespace ovxio;
#endif


#define SIZE_PYM 3

bool flag_capture;






void HelloWorld(){
	printf("\nHello there !");
}

//Images entrée et sortie en U8
//renvoie la petite image
//rapport =4 pour une pym de taille 3
//vx_pyramid pympym = vxCreatePyramid(context, SIZE_PYM, VX_SCALE_PYRAMID_HALF, config.frameWidth, config.frameHeight, VX_DF_IMAGE_S16);
void Pyramin(const FrameSource::Parameters& config, ContextGuard& context, const vx_image& InImg, vx_image& OutImg, vx_pyramid& pympym, int rapport )
{
  vx_image tmpFrame = vxCreateImage(context,config.frameWidth/rapport);
  vxuLaplacianPyramid(context,InImg,pympym,tmpFrame);
  vxuConvertDepth(context,tmpFrame,OutImg,VX_CONVERT_POLICY_WRAP,0);
  vxReleaseImage(&tmpFrame);
}

//Images entrée et sorties en U8
//renvoie la grande image
//rapport =4 pour pyramide taille 3
void PyramOut(const FrameSource::Parameters& config, ContextGuard& context, const vx_image& InImg, vx_image& OutImg, vx_pyramid& pympym, int rapport)
{
  vx_image tmpFrame = vxCreateImage(context, config.frameWidth/rapport);
  vxuConvertDepth(context, InImg, tmpFrame, VX_CONVERT_POLICY, 0);
  vxuLaplacianReconstruct(context, pympym, tmpFrame, OutImg);
  vxReleaseImage(&tmpFrame);
}

/** Fonction d'initialisation des differents parametres */
void InitTraitement(ContextGuard& context){
	flag_capture = true;
}

/** Fonction a appeler pour saisir un input */
void InputTraitement(vx_char c){
	if (c == 'c'){
		flag_capture = true;
	}
}

/** Traitement par soustraction de fond */
void Traitement_SoustractionDeFond(const FrameSource::Parameters& config, ContextGuard& context, vx_image& dstImg, const vx_image& frame){
    printf("\nProcessus : Background Substraction");
    
    //Init
    vx_rectangle_t l_rect, r_rect;
	l_rect.start_x = 0;  l_rect.start_y = 0; l_rect.end_x = config.frameWidth; l_rect.end_y = config.frameHeight;
	r_rect.start_x = config.frameWidth; r_rect.start_y = 0; r_rect.end_x = 2 * config.frameWidth; r_rect.end_y = config.frameHeight;
	vx_image left_image = vxCreateImageFromROI(dstImg, &l_rect);
	vx_image right_image = vxCreateImageFromROI(dstImg, &r_rect);
    
    //Background
    static vx_image defaultFrame;
    if (flag_capture) {
        defaultFrame = vxCreateImage(context, config.frameWidth, config.frameHeight, config.format);
        nvxuCopyImage(context, frame, defaultFrame);
        flag_capture = false;
    }

    //Execution
    vx_image frameU8 =          vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
    vx_image tmpFrame =         vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
    vx_image frameResult =      vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
    vx_image defaultFrameU8 =   vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);

    vxuColorConvert(context, frame, frameU8);
    vxuColorConvert(context, defaultFrame, defaultFrameU8);
    vxuAbsDiff(context, frameU8, defaultFrameU8, frameResult);

    vx_threshold thresh = vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, VX_TYPE_UINT8);
    
    vx_int32 thresh_value = 30, true_value = 255, false_value = 0;
    vxSetThresholdAttribute(thresh, VX_THRESHOLD_TRUE_VALUE, &true_value, sizeof(vx_int32));
    vxSetThresholdAttribute(thresh, VX_THRESHOLD_FALSE_VALUE, &false_value, sizeof(vx_int32));
    vxSetThresholdAttribute(thresh, VX_THRESHOLD_THRESHOLD_VALUE, &thresh_value, sizeof(vx_int32));
    vxuThreshold(context, frameResult, thresh, tmpFrame);

    //nvxuCopyImage(context, tmpFrame, frameResult);

    vxuColorConvert(context, tmpFrame, right_image);
    vxuColorConvert(context, defaultFrameU8, left_image);
    
    //Free
    vxReleaseImage(&frameU8);
    vxReleaseImage(&frameResult); 
    vxReleaseImage(&defaultFrameU8);
    vxReleaseImage(&tmpFrame);
    vxReleaseThreshold(&thresh);
}

/** Fonction test qui genere une image de synthese ou lit une image de test */
void FausseImage(const FrameSource::Parameters& config, ContextGuard& context, Application &app, vx_image& image){
  vx_image base_image;
  
  std::string fileName1 = app.findSampleFilePath("synthese2.jpg");
  //cv::Mat cvmat = nvx_cv::VXImageToCVMatMapper(base_image, 0, 0, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST).getMat(); 
  cv::Mat cvmat = cv::imread(fileName1, cv::IMREAD_GRAYSCALE);
  base_image = nvx_cv::createVXImageFromCVMat(context, cvmat);
  vxuScaleImage(context, base_image, image, NVXCU_INTERPOLATION_TYPE_BILINEAR);
  vxReleaseImage(&base_image);
  cvmat.release();
  
  
  
  
  //cv::gpu::GpuMat cv_src1;
  //= cv::imread(fileName1, cv::IMREAD_GRAYSCALE);
  //base_image = nvx_cv::createVXImageFromCVGpuMat(context, cv_src1);
  //nvxcu_border_t border = {0, NVXCU_BORDER_MODE_REPLICATE};
  //nvxcu_exec_target_t exec = {};
  //nvxcuScaleImage(base_image, image, NVXCU_INTERPOLATION_TYPE_BILINEAR, border);
  
}

/** Fonction qui recoit une image seuillee et affiche dans la console les descripteurs*/
void FiltrageObjets(const FrameSource::Parameters& config, ContextGuard& context, Application &app, vx_image& image){
  printf("\n Filtrage des objets");
  /// Init
  vx_image base = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  //FausseImage(config, context, base);
    
  /// Copy to right place
  vx_rectangle_t l_rect, r_rect;
  l_rect.start_x = 0;  l_rect.start_y = 0; l_rect.end_x = config.frameWidth; l_rect.end_y = config.frameHeight;
  r_rect.start_x = config.frameWidth; r_rect.start_y = 0; r_rect.end_x = 2 * config.frameWidth;   r_rect.end_y = config.frameHeight;
  vx_image left_image = vxCreateImageFromROI(image, &l_rect);
  vx_image right_image = vxCreateImageFromROI(image, &r_rect);
  FausseImage(config, context, app, base);
  vxuColorConvert(context, base, left_image);

  /// Analyse
  cv::Mat cvmat = nvx_cv::VXImageToCVMatMapper(base, 0, 0, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST).getMat();
  
  //cv::cvtColor(cvmat, cvmat, CV_BGR2GRAY);
  cv::threshold(cvmat, cvmat, 50, 255, CV_THRESH_BINARY);
  std::vector<std::vector<cv::Point> > contours;
  cv::Mat contourOutput = cvmat.clone();
  cv::findContours(contourOutput, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

  //Draw the contours
  cv::Mat contourImage(cvmat.size(), CV_8UC3, cv::Scalar(0, 0, 0));
  cv::Scalar colors[3];
  colors[0] = cv::Scalar(255, 0, 0);
  colors[1] = cv::Scalar(0, 255, 0);
  colors[2] = cv::Scalar(0, 0, 255);
  for (size_t idx = 0; idx < contours.size(); idx++) {
    cv::drawContours(contourImage, contours, idx, colors[idx % 3]);
  }

  base = nvx_cv::createVXImageFromCVMat(context, contourImage);
  vxuColorConvert(context, base, right_image);
  
  
  //Prepare the image for findContours
  //cv::cvtColor(cvmat, cvmat, CV_BGR2GRAY);
  //cv::threshold(cvmat, cvmat, 50, 255, CV_THRESH_BINARY);

  //Find the contours. Use the contourOutput Mat so the original image doesn't get overwritten
  /*std::vector<std::vector<cv::Point> > contours;
  cv::Mat contourOutput = cvmat.clone();
  cv::findContours(contourOutput, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

  //Draw the contours
  cv::Mat contourImage(cvmat.size(), CV_8UC3, cv::Scalar(0, 0, 0));
  cv::Scalar colors[3];
  colors[0] = cv::Scalar(255, 0, 0);
  colors[1] = cv::Scalar(0, 255, 0);
  colors[2] = cv::Scalar(0, 0, 255);
  for (size_t idx = 0; idx < contours.size(); idx++) {
    cv::drawContours(contourImage, contours, idx, colors[idx % 3]);
  }*/

  

	/// Free
  //contourImage.release();
	vxReleaseImage(&base);
  cvmat.release();
}


void InitThreshHold(vx_threshold thr, vx_uint8 threshold = 25)
{
  vx_int32 true_value = 255, false_value = 0, lower_value = threshold, upper_value = 150;
  vxSetThresholdAttribute(thr, VX_THRESHOLD_THRESHOLD_LOWER, &lower_value, sizeof(vx_int32));
  vxSetThresholdAttribute(thr, VX_THRESHOLD_THRESHOLD_UPPER, &upper_value, sizeof(vx_int32));
  vxSetThresholdAttribute(thr, VX_THRESHOLD_TRUE_VALUE, &true_value, sizeof(vx_int32));
  vxSetThresholdAttribute(thr, VX_THRESHOLD_FALSE_VALUE, &false_value, sizeof(vx_int32));
}

//oldFrame XRGB, frame XRGB, outputFrame U8, newOldFrame XRGB, (lower) threshold [0,255]
bool Seuillage(vx_context context, FrameSource::Parameters config, vx_image oldframe, vx_image frame, vx_image outputFrame, vx_image newOldFrame, vx_float32 oldWeight = 0.1, vx_uint8 threshold = 25)
{
  //initialisation
  vx_uint8 one = 1;
  vx_threshold thr = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8);
  vx_image frame_U8, oldFrame_U8, buffer_U8, buffer_U8_2, ones; 
  buffer_U8 = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  buffer_U8_2 = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  frame_U8 = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  oldFrame_U8 = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);     
  ones = vxCreateUniformImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8, (vx_pixel_value_t*)&one);
  InitThreshHold(thr, threshold);

  //conversion rgb->niveau de gris
  vxuColorConvert(context, frame, frame_U8);
  vxuColorConvert(context, oldframe, oldFrame_U8);

  //normalization
  vxuEqualizeHist(context, frame_U8, buffer_U8);
  vxuEqualizeHist(context, oldFrame_U8, buffer_U8_2);

  //valeur absolu de la difference
  vxuAbsDiff(context, buffer_U8, buffer_U8_2, frame_U8);

  //seuillage
  vxuThreshold(context, frame_U8, thr, oldFrame_U8);
  vxuDilate3x3(context, oldFrame_U8, frame_U8);
  vxuErode3x3(context, frame_U8, outputFrame);

  //moyenne temporelle
  vxuMultiply(context, buffer_U8, ones, (float)(1 - oldWeight), VX_CONVERT_POLICY_WRAP, VX_ROUND_POLICY_TO_ZERO, frame_U8);
  vxuMultiply(context, buffer_U8_2, ones, (float)oldWeight, VX_CONVERT_POLICY_WRAP, VX_ROUND_POLICY_TO_ZERO, oldFrame_U8);
  vxuAdd(context, frame_U8, oldFrame_U8, VX_CONVERT_POLICY_WRAP, buffer_U8);
  vxuColorConvert(context, buffer_U8, newOldFrame);

  //destruction
  vxReleaseImage(&frame_U8);
  vxReleaseImage(&oldFrame_U8);
  vxReleaseImage(&buffer_U8);
  vxReleaseImage(&buffer_U8_2);
  vxReleaseImage(&ones);
  vxReleaseThreshold(&thr);
  return 1;
}



/** Fonction main qui appelle les autres traitements */
void MainTraitement(const FrameSource::Parameters& config, ContextGuard& context, Application &app, vx_image& dstImg, const vx_image& frame){
    //Traitement_SoustractionDeFond(config, context, dstImg, frame);
    FiltrageObjets(config, context, app, dstImg);
}




/** Reserve d'exemples de fonctions */

//vx_image nvx_cv::createVXImageFromCVGpuMat 	( 	vx_context  	context,
//		const cv::gpu::GpuMat &  	mat 
//	) 	
//vxuChannelExtract(context, frame, VX_CHANNEL_R, chR);
//vxuChannelExtract(context, frame, VX_CHANNEL_G, chG);
//vxuChannelExtract(context, frame, VX_CHANNEL_B, chB);
//vxuChannelCombine(context, chR, chR, chR, chR, frame);
//vxuChannelExtract(context, frame, );
//vxuChannelCombine(context, frameU8, 0, 0);
//nvxuCopyImage(context, defaultFrameU8, orig);
//vxuAbsDiff(context, frame, defaultFrame, frame);
