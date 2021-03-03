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