#pragma once
#include "headers.h"

/** Fonction test qui genere une image de synthese ou lit une image de test */
void FausseImage(const FrameSource::Parameters& config, ContextGuard& context, Application &app, vx_image& image, const char* name){
  vx_image base_image;
  std::string fileName1 = app.findSampleFilePath(name);
  cv::Mat cvmat = cv::imread(fileName1, cv::IMREAD_GRAYSCALE);
  base_image = nvx_cv::createVXImageFromCVMat(context, cvmat);
  vxuScaleImage(context, base_image, image, NVXCU_INTERPOLATION_TYPE_BILINEAR);
  vxReleaseImage(&base_image);
  cvmat.release();
}

void ResetImage(const FrameSource::Parameters& config, ContextGuard& context, vx_image image){
  vx_rectangle_t rect;
  rect.start_x = 0; rect.start_y = 0; rect.end_x = config.frameWidth; rect.end_y = config.frameHeight;
  //vx_image tmp1 = vxCreateImageFromROI(image, &rect);
  vx_uint32 width; vx_uint32 height; vx_df_image format;
  vxQueryImage(image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
  vxQueryImage(image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));
  vxQueryImage(image, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(vx_df_image));
  vx_image tmp1 = vxCreateImage(context, width, height, format);
  vx_image tmp2 = vxCreateImage(context, width, height, format);
  vxuAbsDiff(context, tmp1, tmp2, image);
  vxReleaseImage(&tmp1);
  vxReleaseImage(&tmp2);
}

void CopyPartialImage(const FrameSource::Parameters& config, ContextGuard& context, const vx_image& src, vx_image dst, vx_uint32 x, vx_uint32 y, vx_uint32 w, vx_uint32 h){
  vx_rectangle_t rect;
  rect.start_x = x; rect.start_y = y; rect.end_x = x + w; rect.end_y = y + h;
  vx_image tmp1 = vxCreateImageFromROI(src, &rect);
  vx_image tmp2 = vxCreateImageFromROI(dst, &rect);
  nvxuCopyImage(context, tmp1, tmp2);
  vxReleaseImage(&tmp1);
  vxReleaseImage(&tmp2);
}

void InitThreshHold(vx_threshold thr, vx_uint8 threshold = 75)
{
  vx_int32 true_value = 255, false_value = 0, lower_value = threshold, upper_value = 255;
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

void SubplotCopy(ContextGuard& context, const vx_image& src, vx_image& dst, vx_uint32 n_width, vx_uint32 n_height, vx_uint32 i){
  if (i < 0 || i >= n_width*n_height){
    printf("\nErreur, Index invalide pour le subplot\n");
    return;
  }
  /// Init
  vx_rectangle_t rect;
  vx_uint32 width; vx_uint32 height; vx_df_image format;
  vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
  vxQueryImage(dst, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

  vx_uint32 w = width / n_width; vx_uint32 h = height / n_height;
  vx_uint32 x = (i % n_width); vx_uint32 y = (i / n_width);
  rect.start_x = x * w; rect.end_x = (x + 1) * w;
  rect.start_y = y * h; rect.end_y = (y + 1) * h;
  vx_image target_image = vxCreateImageFromROI(dst, &rect);

  vxuScaleImage(context, src, target_image, NVXCU_INTERPOLATION_TYPE_BILINEAR);

  vxReleaseImage(&target_image);
}