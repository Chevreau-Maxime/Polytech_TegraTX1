/*
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
*/


#pragma once
#include "headers.h"
#include "fonctions_utilitaires.h"
#include "classification.h"

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

//frame RGBX, oldFrame RGBX, old_thresh U8, outputFrame RGBX
void newContour(vx_context context, vx_uint32 Width, vx_uint32 Height, vx_image frame, vx_image oldFrame, vx_image old_thresh_GS, vx_image outputFrame)
{
  vx_threshold thr = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8); 
  InitThreshHold(thr, (vx_uint8)75);
  vx_threshold thrR = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8);
  vx_threshold thrG = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8);
  vx_threshold thrB = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8);
  vx_image buffer_U8 = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image frameR_U8 = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image frameG_U8 = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image frameB_U8 = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image oldframeR_U8 = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image oldframeG_U8 = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image oldframeB_U8 = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image thresh_R = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image thresh_B = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image thresh_G = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image thresh_GS = vxCreateImage(context, Width, Height, VX_DF_IMAGE_U8);
  vx_image thresh_RGBX = vxCreateImage(context, Width, Height, VX_DF_IMAGE_RGBX);
  vx_float32 meanR, meanG, meanB, devR, devG, devB;

  //extraction et normalization de l'image actuelle
  vxuChannelExtract(context, frame, VX_CHANNEL_R, frameR_U8);
  vxuEqualizeHist(context, frameR_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, frameR_U8); vxuMeanStdDev(context, frameR_U8, &meanR, &devR);
  vxuChannelExtract(context, frame, VX_CHANNEL_G, frameG_U8);
  vxuEqualizeHist(context, frameR_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, frameR_U8); vxuMeanStdDev(context, frameG_U8, &meanG, &devG);
  vxuChannelExtract(context, frame, VX_CHANNEL_B, frameB_U8);
  vxuEqualizeHist(context, frameR_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, frameR_U8); vxuMeanStdDev(context, frameB_U8, &meanB, &devB);

  //extraction et normalization de l'ancienne image
  vxuChannelExtract(context, oldFrame, VX_CHANNEL_R, oldframeR_U8);
  vxuEqualizeHist(context, oldframeR_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, oldframeR_U8);
  vxuChannelExtract(context, oldFrame, VX_CHANNEL_G, oldframeG_U8);
  vxuEqualizeHist(context, oldframeR_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, oldframeR_U8);
  vxuChannelExtract(context, oldFrame, VX_CHANNEL_B, oldframeB_U8);
  vxuEqualizeHist(context, oldframeR_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, oldframeR_U8);

  //difference entre chaque canal e chaque image et seuillage avec pour seuil la moitie de la moyenne de l'image
  vxuAbsDiff(context, frameR_U8, oldframeR_U8, buffer_U8); InitThreshHold(thrR, (vx_uint8)(0.5f * meanR)); vxuThreshold(context, buffer_U8, thrR, thresh_R);
  vxuAbsDiff(context, frameG_U8, oldframeG_U8, buffer_U8); InitThreshHold(thrG, (vx_uint8)(0.5f * meanG)); vxuThreshold(context, buffer_U8, thrG, thresh_G);
  vxuAbsDiff(context, frameB_U8, oldframeB_U8, buffer_U8); InitThreshHold(thrB, (vx_uint8)(0.5f * meanB)); vxuThreshold(context, buffer_U8, thrB, thresh_B);
  vxuChannelExtract(context, oldFrame, VX_CHANNEL_A, buffer_U8); vxuChannelCombine(context, thresh_R, thresh_G, thresh_B, buffer_U8, thresh_RGBX);
  vxuColorConvert(context, thresh_RGBX, buffer_U8); nvxuCopyImage(context, buffer_U8, thresh_GS);

  //fermeture
  vxuDilate3x3(context, thresh_GS, buffer_U8); nvxuCopyImage(context, buffer_U8, thresh_GS);
  vxuErode3x3(context, thresh_GS, buffer_U8); nvxuCopyImage(context, buffer_U8, thresh_GS);

  //seuillage
  vxuThreshold(context, thresh_GS, thr, buffer_U8); nvxuCopyImage(context, buffer_U8, thresh_GS);

  //OU logique entre l'ancienne image et la nouvelle
  vxuOr(context, thresh_GS, old_thresh_GS, buffer_U8);
  nvxuCopyImage(context, thresh_GS, old_thresh_GS);
  nvxuCopyImage(context, buffer_U8, thresh_GS);

  //dilatation pour fermer les contours
  vxuDilate3x3(context, thresh_GS, buffer_U8); nvxuCopyImage(context, buffer_U8, thresh_GS);
  vxuDilate3x3(context, thresh_GS, buffer_U8); nvxuCopyImage(context, buffer_U8, thresh_GS);
  vxuColorConvert(context, thresh_GS, outputFrame);

  vxReleaseThreshold(&thr);
  vxReleaseThreshold(&thrR);
  vxReleaseThreshold(&thrG);
  vxReleaseThreshold(&thrB);
  vxReleaseImage(&buffer_U8);
  vxReleaseImage(&frameR_U8);
  vxReleaseImage(&frameG_U8);
  vxReleaseImage(&frameB_U8);
  vxReleaseImage(&oldframeR_U8);
  vxReleaseImage(&oldframeG_U8);
  vxReleaseImage(&oldframeB_U8);
  vxReleaseImage(&thresh_R);
  vxReleaseImage(&thresh_G);
  vxReleaseImage(&thresh_B);
  vxReleaseImage(&thresh_GS);
  vxReleaseImage(&thresh_RGBX);
}

/** Traitement par soustraction de fond */
void Traitement_SoustractionDeFond(const FrameSource::Parameters& config, ContextGuard& context, vx_image& dstImg, const vx_image& frame, vx_image& output){
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

    vxuColorConvert(context, tmpFrame, right_image);
    vxuColorConvert(context, defaultFrameU8, left_image);
    nvxuCopyImage(context, tmpFrame, output);
    
    //Free
    vxReleaseImage(&frameU8);
    vxReleaseImage(&frameResult); 
    vxReleaseImage(&defaultFrameU8);
    vxReleaseImage(&tmpFrame);
    vxReleaseThreshold(&thresh);
}


/** Fonction main qui appelle les autres traitements */
void MainTraitement(const FrameSource::Parameters& config, ContextGuard& context, Application &app, vx_image& dstImg, const vx_image& frame){
  //vx_image image_traitee = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  //Traitement_SoustractionDeFond(config, context, dstImg, frame, image_traitee);
  //descripteur_objet* desc = 0;
  //Test_Classification(config, context, app, dstImg, desc);


  /// Init
  //vx_rectangle_t l_rect, r_rect;
  //l_rect.start_x = 0;  l_rect.start_y = 0; l_rect.end_x = config.frameWidth; l_rect.end_y = config.frameHeight;
  //r_rect.start_x = config.frameWidth; r_rect.start_y = 0; r_rect.end_x = 2 * config.frameWidth;   r_rect.end_y = config.frameHeight;
  //vx_image base = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  //vx_image left_image = vxCreateImageFromROI(dstImg, &l_rect);
  //vx_image right_image = vxCreateImageFromROI(dstImg, &r_rect);
  
  static vx_image image_n_1 = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_RGBX);
  static vx_image old_thresh_GS = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  static vx_image output_rgb = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_RGBX);

  /// Execution
  newContour(context, config.frameWidth, config.frameHeight, frame, image_n_1, old_thresh_GS, output_rgb);
  vx_image tmp = Classification_image_traitee(config, context, output_rgb);

  /// Display
  SubplotCopy(context, output_rgb, dstImg, 2, 1, 0);
  SubplotCopy(context, tmp, dstImg, 2, 1, 1);
  //SubplotCopy(context, output_rgb, dstImg, 2, 1, i);


  //nvxuCopyImage(context, output_rgb, left_image);
  //nvxuCopyImage(context, Classification_image_traitee(config, context, output_rgb), right_image);
  /// End
  nvxuCopyImage(context, frame, image_n_1);
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
