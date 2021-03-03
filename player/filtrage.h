#pragma once
#include "headers.h"

// fermeture horizontale
void fermeture_horizontale(const FrameSource::Parameters& config, ContextGuard& context, vx_image& src){
  vx_uint32 width; vx_uint32 height;
  vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
  vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));
  vx_matrix mask = vxCreateMatrixFromPattern(context, VX_PATTERN_CROSS, 5, 1);
  vx_image tmp = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
  vxuNonLinearFilter(context, VX_NONLINEAR_FILTER_MEDIAN, src, mask, tmp);
  nvxuCopyImage(context, tmp, src);
}


// frame RGBX, oldFrame RGBX, old_thresh U8, outputFrame RGBX
void newContour2(vx_context context, vx_uint32 Width, vx_uint32 Height, vx_image frame, vx_image oldFrame, vx_image old_thresh_GS, vx_image outputFrame, std::ostringstream& txt)
{
  timer_mini.tic();
  vx_threshold thr = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8); InitThreshHold(thr, (vx_uint8)75);
  vx_threshold thrR = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8);
  vx_threshold thrG = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8);
  vx_threshold thrB = vxCreateThreshold(context, VX_THRESHOLD_TYPE_RANGE, VX_TYPE_UINT8);
  vx_image buffer_U8 = buf1;
  vx_image frameR_U8 = buf2;
  vx_image frameG_U8 = buf3;
  vx_image frameB_U8 = buf4;
  vx_image oldframeR_U8 = buf5;
  vx_image oldframeG_U8 = buf6;
  vx_image oldframeB_U8 = buf7;
  vx_image thresh_R = buf8;
  vx_image thresh_B = buf9;
  vx_image thresh_G = buf10;
  vx_image thresh_GS = buf11;
  vx_image thresh_RGBX = buf12;
  vx_float32 meanR = 0, meanG = 0, meanB = 0, devR = 0, devG = 0, devB = 0;

  //extraction et normalization de l'image actuelle
  vxuChannelExtract(context, frame, VX_CHANNEL_R, frameR_U8);
  vxuEqualizeHist(context, frameR_U8, buffer_U8);
  nvxuCopyImage(context, buffer_U8, frameR_U8);
  vxuMeanStdDev(context, frameR_U8, &meanR, &devR);
  vxuChannelExtract(context, frame, VX_CHANNEL_G, frameG_U8);
  vxuEqualizeHist(context, frameG_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, frameG_U8); vxuMeanStdDev(context, frameG_U8, &meanG, &devG);
  vxuChannelExtract(context, frame, VX_CHANNEL_B, frameB_U8);
  vxuEqualizeHist(context, frameB_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, frameB_U8); vxuMeanStdDev(context, frameB_U8, &meanB, &devB);
  
  //extraction et normalization de l'ancienne image
  vxuChannelExtract(context, oldFrame, VX_CHANNEL_R, oldframeR_U8);
  vxuEqualizeHist(context, oldframeR_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, oldframeR_U8);
  vxuChannelExtract(context, oldFrame, VX_CHANNEL_G, oldframeG_U8);
  vxuEqualizeHist(context, oldframeG_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, oldframeG_U8);
  vxuChannelExtract(context, oldFrame, VX_CHANNEL_B, oldframeB_U8);
  vxuEqualizeHist(context, oldframeB_U8, buffer_U8); nvxuCopyImage(context, buffer_U8, oldframeB_U8);
  AddTimeToString("-extraction and equalization : ", txt, timer_mini);

  //difference entre chaque canal e chaque image et seuillage avec pour seuil la moitie de la moyenne de l'image
  vxuAbsDiff(context, frameR_U8, oldframeR_U8, buffer_U8); InitThreshHold(thrR, (vx_uint8)(0.5f * meanR)); vxuThreshold(context, buffer_U8, thrR, thresh_R);
  vxuAbsDiff(context, frameG_U8, oldframeG_U8, buffer_U8); InitThreshHold(thrG, (vx_uint8)(0.5f * meanG)); vxuThreshold(context, buffer_U8, thrG, thresh_G);
  vxuAbsDiff(context, frameB_U8, oldframeB_U8, buffer_U8); InitThreshHold(thrB, (vx_uint8)(0.5f * meanB)); vxuThreshold(context, buffer_U8, thrB, thresh_B);
  vxuChannelExtract(context, oldFrame, VX_CHANNEL_A, buffer_U8); vxuChannelCombine(context, thresh_R, thresh_G, thresh_B, buffer_U8, thresh_RGBX);
  vxuColorConvert(context, thresh_RGBX, buffer_U8); nvxuCopyImage(context, buffer_U8, thresh_GS);
  AddTimeToString("-abs diff : ", txt, timer_mini);

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
  AddTimeToString("-operations : ", txt, timer_mini);
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
  vx_image frameU8 = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  vx_image tmpFrame = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  vx_image frameResult = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  vx_image defaultFrameU8 = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);

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