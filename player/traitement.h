
#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>

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

bool flag_capture;






void HelloWorld(){
	printf("\nHello there !");
}

/** Fonction d'initialisation des differents parametres */
void InitTraitement(ovxio::ContextGuard& context){
	flag_capture = true;
}

/** Fonction a appeler pour saisir un input */
void InputTraitement(vx_char c){
	if (c == 'c'){
		flag_capture = true;
	}
}

/** Traitement par soustraction de fond */
void Traitement_SoustractionDeFond(const ovxio::FrameSource::Parameters& config, ovxio::ContextGuard& context, vx_image& dstImg, const vx_image& frame){
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

    vx_threshold thresh = vxCreateThreshold(context, VX_THRESHOLD_TYPE_BINARY, VX_DF_IMAGE_U8);
    
    vx_uint8 thresh_value = 150, true_value = 0, false_value = 250;
    vxSetThresholdAttribute(thresh, VX_THRESHOLD_TRUE_VALUE, &true_value, sizeof(vx_uint8));
    vxSetThresholdAttribute(thresh, VX_THRESHOLD_FALSE_VALUE, &false_value, sizeof(vx_uint8));
    vxSetThresholdAttribute(thresh, VX_THRESHOLD_THRESHOLD_VALUE, &thresh_value, sizeof(vx_uint8));
    vxuThreshold(context, frameResult, thresh, tmpFrame);

    //nvxuCopyImage(context, tmpFrame, frameResult);

    vxuColorConvert(context, frameResult, right_image);
    vxuColorConvert(context, defaultFrameU8, left_image);
    
    //Free
    vxReleaseImage(&frameU8);
    vxReleaseImage(&frameResult); 
    vxReleaseImage(&defaultFrameU8);
    vxReleaseThreshold(&thresh);
}


/** Fonction main qui appelle les autres traitements */
void MainTraitement(const ovxio::FrameSource::Parameters& config, ovxio::ContextGuard& context, vx_image& dstImg, const vx_image& frame){
    Traitement_SoustractionDeFond(config, context, dstImg, frame);
}


/** Reserve d'exemples de fonctions */
//vxuChannelExtract(context, frame, VX_CHANNEL_R, chR);
//vxuChannelExtract(context, frame, VX_CHANNEL_G, chG);
//vxuChannelExtract(context, frame, VX_CHANNEL_B, chB);
//vxuChannelCombine(context, chR, chR, chR, chR, frame);
//vxuChannelExtract(context, frame, );
//vxuChannelCombine(context, frameU8, 0, 0);
//nvxuCopyImage(context, defaultFrameU8, orig);
//vxuAbsDiff(context, frame, defaultFrame, frame);

