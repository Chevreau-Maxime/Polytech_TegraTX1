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
#include/FrameSourceOVX.hpp"
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
#include "filtrage.h"


void HelloWorld(){
	printf("\nHello there !");
}

/** Fonction d'initialisation des differents parametres */
void InitTraitement(const FrameSource::Parameters& config, ContextGuard& context){
	flag_capture = true;
	source_videos = new char*[NB_SOURCE_VIDEOS];
	source_videos[0] = "cars.mp4";
	source_videos[1] = "pedestrians.mp4";
	source_videos[2] = "signs.avi";
	selected_video = 0;
	display_mode = 0;
  pause = 0;

  InitBuffer(context, config.frameWidth*SOURCE_DOWNSCALING, config.frameHeight*SOURCE_DOWNSCALING);
}

/** Fonction a appeler pour saisir un input */
void InputTraitement(vx_char c){
	if (c == 'c'){
		flag_capture = true;
	}
	if (c >= '1' && c < '1' + NB_SOURCE_VIDEOS){
		selected_video = c - '1';
	}
	if (c == 'm'){
		display_mode = 1 - display_mode;
	}
  if (c == 32){
    pause = !pause;
  }
}

void EndTraitement(){
  ReleaseBuffer();
}

/* Demo */
void Demo1(const FrameSource::Parameters& config, ContextGuard& context, Application &app, vx_image& dstImg, const vx_image& frame, std::ostringstream& txt){
  
  txt.str(""); txt.clear(); 
  txt << "Demo 1 : Traitement et classification" << std::endl << std::endl;
  
  vx_uint32 width = config.frameWidth*SOURCE_DOWNSCALING;
  vx_uint32 height = config.frameHeight*SOURCE_DOWNSCALING;

  static vx_image image_n_1 = vxCreateImage(context, width, height, VX_DF_IMAGE_RGBX);
  static vx_image old_thresh_GS = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
  static vx_image output_rgb = vxCreateImage(context, width, height, VX_DF_IMAGE_RGBX);
  vx_image frame_downscale = vxCreateImage(context, width, height, VX_DF_IMAGE_RGBX);

  /// Execution
  vxuScaleImage(context, frame, frame_downscale, NVXCU_INTERPOLATION_TYPE_BILINEAR);
  timer.tic();
  newContour2(context, width, height, frame_downscale, image_n_1, old_thresh_GS, output_rgb, txt);
  AddTimeToString("---> total filtering time : ", txt, timer);
  vx_image tmp = Classification_image_traitee(config, context, output_rgb, frame, display_mode, txt);
  AddTimeToString("---> total classification time : ", txt, timer);

  /// Display
  if (display_mode == 0)  {
    SubplotCopy(context, output_rgb, dstImg, 2, 1, 0);
    SubplotCopy(context, tmp, dstImg, 2, 1, 1);
  } else {
    SubplotCopy(context, frame, dstImg, 2, 1, 0);
    SubplotCopy(context, tmp, dstImg, 2, 1, 1);
  }

  /// End
  nvxuCopyImage(context, frame_downscale, image_n_1);
  vxReleaseImage(&frame_downscale); 
  vxReleaseImage(&tmp);
}

/* Demo avec pyramide (WIP / TODO)*/
void Demo2(const FrameSource::Parameters& config, ContextGuard& context, Application &app, vx_image& dstImg, const vx_image& frame, std::ostringstream& txt){
  vx_uint32 width_down = config.frameWidth / 4;
  vx_uint32 height_down = config.frameHeight / 4;
  vx_uint32 width = config.frameWidth;
  vx_uint32 height = config.frameHeight;

  static vx_image image_n_1 = vxCreateImage(context, width_down, height_down, VX_DF_IMAGE_RGBX);
  static vx_image old_thresh_GS = vxCreateImage(context, width_down, height_down, VX_DF_IMAGE_U8);
  static vx_image output_rgb = vxCreateImage(context, width_down, height_down, VX_DF_IMAGE_RGBX);

  vx_image frame_U8 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
  vx_image frame_downscale = vxCreateImage(context, width_down, height_down, VX_DF_IMAGE_RGBX);
  vx_image frame_downscale_U8 = vxCreateImage(context, width_down, height_down, VX_DF_IMAGE_U8);
  vx_pyramid pympym = vxCreatePyramid(context, SIZE_PYM, VX_SCALE_PYRAMID_HALF, width, height, VX_DF_IMAGE_S16);

  /// Execution
  vxuColorConvert(context, frame, frame_U8);
  PyramIn(config, context, frame_U8, frame_downscale_U8, pympym);
  vxuColorConvert(context, frame_downscale_U8, frame_downscale);
  //vxuScaleImage(context, frame, frame_downscale, NVXCU_INTERPOLATION_TYPE_BILINEAR);
  newContour2(context, width, height, frame_downscale, image_n_1, old_thresh_GS, output_rgb, txt);
  vx_image tmp = Classification_image_traitee(config, context, output_rgb, frame, true, txt);

  /// Display
  if (display_mode == 0)  {
    SubplotCopy(context, output_rgb, dstImg, 2, 1, 0);
    SubplotCopy(context, tmp, dstImg, 2, 1, 1);
  } else {
    SubplotCopy(context, frame, dstImg, 2, 1, 0);
    SubplotCopy(context, tmp, dstImg, 2, 1, 1);
  }

  /// End
  nvxuCopyImage(context, frame_downscale, image_n_1);
  vxReleaseImage(&frame_downscale);
  vxReleaseImage(&frame_downscale_U8);
  vxReleaseImage(&frame_U8);
}

/* Test traitement custom kernel */
void Demo3(const FrameSource::Parameters& config, ContextGuard& context, Application &app, vx_image& dstImg, const vx_image& frame, std::ostringstream& txt){
  vx_uint32 width = config.frameWidth*SOURCE_DOWNSCALING;
  vx_uint32 height = config.frameHeight*SOURCE_DOWNSCALING;

  static vx_image image_n_1 = vxCreateImage(context, width, height, VX_DF_IMAGE_RGBX);
  static vx_image old_thresh_GS = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
  static vx_image output_rgb = vxCreateImage(context, width, height, VX_DF_IMAGE_RGBX);
  vx_image tmp_U8 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
  vx_image tmp_rgb = vxCreateImage(context, width, height, VX_DF_IMAGE_RGBX);
  vx_image frame_downscale = vxCreateImage(context, width, height, VX_DF_IMAGE_RGBX);

  /// Execution
  vxuScaleImage(context, frame, frame_downscale, NVXCU_INTERPOLATION_TYPE_BILINEAR);
  newContour2(context, width, height, frame_downscale, image_n_1, old_thresh_GS, output_rgb, txt);
  vxuColorConvert(context, output_rgb, tmp_U8);
  fermeture_horizontale(config, context, tmp_U8);
  vxuColorConvert(context, tmp_U8, output_rgb);
  vx_image tmp = Classification_image_traitee(config, context, output_rgb, frame, display_mode, txt);

  /// Display
  if (display_mode == 0)  {
    SubplotCopy(context, output_rgb, dstImg, 2, 1, 0);
    SubplotCopy(context, tmp, dstImg, 2, 1, 1);
  }
  else {
    SubplotCopy(context, frame, dstImg, 2, 1, 0);
    SubplotCopy(context, tmp, dstImg, 2, 1, 1);
  }

  /// End
  nvxuCopyImage(context, frame_downscale, image_n_1);
  vxReleaseImage(&frame_downscale);


  /*// init
  vx_image src_RGBX = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_RGBX);
  vx_image src_U8 = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  vx_image res_RGBX = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_RGBX);
  vx_image res_U8 = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  vx_matrix mask = vxCreateMatrixFromPattern(context, VX_PATTERN_CROSS, 9, 1);
  // source
  FausseImage(config, context, app, src_U8, "synthese_pont_bruit.jpg");
  vxuColorConvert(context, src_U8, src_RGBX);
  SubplotCopy(context, src_RGBX, dstImg, 2, 1, 0);
  // test
  vxuNonLinearFilter(context, VX_NONLINEAR_FILTER_MAX, src_U8, mask, res_U8);
  vxuColorConvert(context, res_U8, res_RGBX);
  SubplotCopy(context, res_RGBX, dstImg, 2, 1, 1);

  vxReleaseImage(&src_RGBX);
  vxReleaseImage(&src_U8);
  vxReleaseImage(&res_RGBX);
  vxReleaseImage(&res_U8);*/
}


/** Fonction main qui appelle les autres traitements */
void MainTraitement(const FrameSource::Parameters& config, ContextGuard& context, Application &app, vx_image& dstImg, const vx_image& frame, std::ostringstream& txt){
  if (!pause){
    Demo1(config, context, app, dstImg, frame, txt);
  }
  else {
  }
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


 /// Init
  //vx_rectangle_t l_rect, r_rect;
  //l_rect.start_x = 0;  l_rect.start_y = 0; l_rect.end_x = config.frameWidth; l_rect.end_y = config.frameHeight;
  //r_rect.start_x = config.frameWidth; r_rect.start_y = 0; r_rect.end_x = 2 * config.frameWidth;   r_rect.end_y = config.frameHeight;
  //vx_image base = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  //vx_image left_image = vxCreateImageFromROI(dstImg, &l_rect);
  //vx_image right_image = vxCreateImageFromROI(dstImg, &r_rect);
  //nvxuCopyImage(context, output_rgb, left_image);
  //nvxuCopyImage(context, Classification_image_traitee(config, context, output_rgb), right_image);


  //vx_image image_traitee = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  //Traitement_SoustractionDeFond(config, context, dstImg, frame, image_traitee);
  //descripteur_objet* desc = 0;
  //Test_Classification(config, context, app, dstImg, desc);
