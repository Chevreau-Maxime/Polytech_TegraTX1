#pragma once
#include "headers.h"
#include "fonctions_utilitaires.h"

/* Fonction qui construit les descripteurs */
void getDescripteurs(const std::vector<std::vector<cv::Point> >& contours, descripteur_objet*& desc){
  desc = new descripteur_objet[contours.size()];

  /// First Loop : Get amplitude, min, max
  for (size_t idx = 0; idx < contours.size(); idx++) {
    desc[idx].xmin = contours[idx][0].x;
    desc[idx].xmax = contours[idx][0].x;
    desc[idx].ymin = contours[idx][0].y;
    desc[idx].ymax = contours[idx][0].y;
    for (size_t p = 0; p < contours[idx].size(); p++){
      if (desc[idx].xmin > contours[idx][p].x) desc[idx].xmin = contours[idx][p].x;
      if (desc[idx].xmax < contours[idx][p].x) desc[idx].xmax = contours[idx][p].x;
      if (desc[idx].ymin > contours[idx][p].y) desc[idx].ymin = contours[idx][p].y;
      if (desc[idx].ymax < contours[idx][p].y) desc[idx].ymax = contours[idx][p].y;
    }
    float tmp = (float)(desc[idx].ymax - desc[idx].ymin) / (float)(desc[idx].xmax - desc[idx].xmin);
    desc[idx].ratiohw = tmp;
  }
  /// Second Loop : Get distance from center (spacing)
  for (size_t idx = 0; idx < contours.size(); idx++){
    desc[idx].spacing = 0;
    float midx = desc[idx].xmin + ((desc[idx].xmax - desc[idx].xmin) / 2);
    float midy = desc[idx].ymin + ((desc[idx].ymax - desc[idx].ymin) / 2);
    for (size_t p = 0; p < contours[idx].size(); p++){
      float tmp1 = abs(midx - contours[idx][p].x) / (desc[idx].xmax - desc[idx].xmin);
      float tmp2 = abs(midy - contours[idx][p].y) / (desc[idx].ymax - desc[idx].ymin);
      desc[idx].spacing += sqrt((tmp1*tmp1) + (tmp2*tmp2));
    }
    desc[idx].spacing = desc[idx].spacing / contours[idx].size();
  }
  /// Third Loop : Get standard deviation from spacing
  for (size_t idx = 0; idx < contours.size(); idx++){
    desc[idx].spacing_irregularity = 0;
    float midx = desc[idx].xmin + ((desc[idx].xmax - desc[idx].xmin) / 2);
    float midy = desc[idx].ymin + ((desc[idx].ymax - desc[idx].ymin) / 2);
    for (size_t p = 0; p < contours[idx].size(); p++){
      float tmp1 = abs(midx - contours[idx][p].x) / (desc[idx].xmax - desc[idx].xmin);
      float tmp2 = abs(midy - contours[idx][p].y) / (desc[idx].ymax - desc[idx].ymin);
      desc[idx].spacing_irregularity += abs(desc[idx].spacing - sqrt((tmp1*tmp1) + (tmp2*tmp2)));
    }
    desc[idx].spacing_irregularity /= contours[idx].size();
  }
}


/* Fonction qui affiche les descripteurs dans la console */
void printDescripteurs(descripteur_objet*& desc, size_t nb_desc){
  for (size_t idx = 0; idx < nb_desc; idx++) {
    printf("\n Object %d : ", idx);
    printf("\n Amplitude en X : %d (%d-%d)", desc[idx].xmax - desc[idx].xmin, desc[idx].xmin, desc[idx].xmax);
    printf("\n Amplitude en Y : %d (%d-%d)", desc[idx].ymax - desc[idx].ymin, desc[idx].ymin, desc[idx].ymax);
    printf("\n Ratio H/W      : %f", desc[idx].ratiohw);
    printf("\n Spacing        : %f", desc[idx].spacing);
    printf("\n Spacing irregularity : %f", desc[idx].spacing_irregularity);
  }
}


/* Fonction de test de descripteur */
int validDescripteur(const FrameSource::Parameters& config, const descripteur_objet& d){
  //Ratio :
  if (d.spacing_irregularity > 0.12) return DESCRIPTOR_RATIO_PB;
  if (abs(d.ratiohw - 1.f) > 0.5) return DESCRIPTOR_RATIO_PB;
  //Size
  if ((d.xmax - d.xmin < config.frameWidth / 15) || (d.ymax - d.ymin < config.frameHeight / 15)) return DESCRIPTOR_SIZE_PB;
  if ((d.xmax - d.xmin > config.frameWidth * 0.4) || (d.ymax - d.ymin > config.frameHeight * 0.4)) return DESCRIPTOR_SIZE_PB;
  //Ok
  return DESCRIPTOR_OK;
}


/* Fonction qui dessine les contours sur l'image output_rgb_image */
void drawDescripteurs(const FrameSource::Parameters& config, ContextGuard& context, descripteur_objet*& desc, size_t nb_desc, 
                      vx_image& output_rgb_image, const std::vector<std::vector<cv::Point> >& contours, const cv::Mat& input){
  vx_df_image format; vx_uint32 width; vx_uint32 height;
  vxQueryImage(output_rgb_image, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
  vxQueryImage(output_rgb_image, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));
  vxQueryImage(output_rgb_image, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(vx_df_image));
  vx_image tmp = vxCreateImage(context, width, height, format);
  cv::Mat contourImage(input.size(), CV_8UC3, cv::Scalar(0, 0, 0));
  cv::Scalar colors[4];
  colors[0] = cv::Scalar(0, 255, 0);
  colors[1] = cv::Scalar(0, 0, 255);
  colors[2] = cv::Scalar(255, 0, 0);
  colors[3] = cv::Scalar(200, 0, 200);
  for (size_t idx = 0; idx < nb_desc; idx++) {
	int color_choice = validDescripteur(config, desc[idx]);

	if (display_mode == 0){
		cv::drawContours(contourImage, contours, idx, colors[color_choice]);
	}
	else if (color_choice == DESCRIPTOR_OK){
		cv::drawContours(contourImage, contours, idx, colors[color_choice]);
	}
  }

  //Display in dstImage
  tmp = nvx_cv::createVXImageFromCVMat(context, contourImage);
  vxuColorConvert(context, tmp, output_rgb_image);
  vxReleaseImage(&tmp);
  contourImage.release();
}

/* Fonction qui dessine les zones des descripteurs valides de l'image input dans l'image output */
void drawValidObjects(const FrameSource::Parameters& config, ContextGuard& context, descripteur_objet*& desc, size_t nb_desc,
  vx_image& output_image, vx_image& input_image){
  for (size_t idx = 0; idx < nb_desc; idx++) {
    if (validDescripteur(config, desc[idx]) == 0) CopyPartialImage(config, context, input_image, output_image, 
      desc[idx].xmin, desc[idx].ymin, desc[idx].xmax - desc[idx].xmin, desc[idx].ymax - desc[idx].ymin);
  }
}

/* Fonction qui prend une vx_image filtree binarisee format rgb en entree, et renvoie une vx_image rgb en sortie */
vx_image Classification_image_traitee(const FrameSource::Parameters& config, ContextGuard& context, vx_image input_RGB){
  /// Init
  vx_df_image format; vx_uint32 width; vx_uint32 height;
  vxQueryImage(input_RGB, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(vx_uint32));
  vxQueryImage(input_RGB, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));
  vx_image input_U8 = vxCreateImage(context, width, height, VX_DF_IMAGE_U8);
  vxuColorConvert(context, input_RGB, input_U8);
  vx_image output_rgb = vxCreateImage(context, width, height, VX_DF_IMAGE_RGBX);
  
  /// Analyse
  cv::Mat cvmat = nvx_cv::VXImageToCVMatMapper(input_U8, 0, 0, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST).getMat();
  cv::threshold(cvmat, cvmat, 50, 255, CV_THRESH_BINARY);
  std::vector<std::vector<cv::Point> > contours;
  cv::Mat contourOutput = cvmat.clone();
  cv::findContours(contourOutput, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

  /// Descripteurs
  descripteur_objet* desc = 0;
  getDescripteurs(contours, desc);
  drawDescripteurs(config, context, desc, contours.size(), output_rgb, contours, cvmat);
  //drawValidObjects(config, context, desc, contours.size(), output_rgb, input_RGB);

  vxReleaseImage(&input_U8);
  return output_rgb;
}








/** Fonction qui recoit une image seuillee et affiche dans la console les descripteurs*/
void Test_Classification(const FrameSource::Parameters& config, ContextGuard& context, Application &app, vx_image& image, descripteur_objet* desc){
  /// Init
  vx_rectangle_t l_rect, r_rect;
  l_rect.start_x = 0;  l_rect.start_y = 0; l_rect.end_x = config.frameWidth; l_rect.end_y = config.frameHeight;
  r_rect.start_x = config.frameWidth; r_rect.start_y = 0; r_rect.end_x = 2 * config.frameWidth;   r_rect.end_y = config.frameHeight;
  vx_image base = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);
  vx_image left_image = vxCreateImageFromROI(image, &l_rect);
  vx_image right_image = vxCreateImageFromROI(image, &r_rect);
  vx_image tmp_rgb = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_RGBX);
  vx_image tmp_gray = vxCreateImage(context, config.frameWidth, config.frameHeight, VX_DF_IMAGE_U8);

  ResetImage(config, context, tmp_gray);
  vxuColorConvert(context, tmp_gray, right_image);
  FausseImage(config, context, app, base, "synthese_pont.jpg");
  vxuColorConvert(context, base, tmp_rgb);

  /// Analyse
  cv::Mat cvmat = nvx_cv::VXImageToCVMatMapper(base, 0, 0, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST).getMat();
  cv::threshold(cvmat, cvmat, 50, 255, CV_THRESH_BINARY);
  std::vector<std::vector<cv::Point> > contours;
  cv::Mat contourOutput = cvmat.clone();
  cv::findContours(contourOutput, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

  /// Descripteurs
  getDescripteurs(contours, desc);

  /// Display in console :
  if (true){
    static int countimage = 0;
    if (countimage == 0) printDescripteurs(desc, contours.size());
    countimage = (countimage + 1) % 30;
  }

  //Draw the contours
  drawDescripteurs(config, context, desc, contours.size(), left_image, contours, cvmat);
  drawValidObjects(config, context, desc, contours.size(), right_image, tmp_rgb);
  
	// Free
  vxReleaseImage(&base);
  vxReleaseImage(&left_image);
  vxReleaseImage(&right_image);
  vxReleaseImage(&tmp_gray);
  vxReleaseImage(&tmp_rgb);
  cvmat.release();
  contourOutput.release();
}