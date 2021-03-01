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

#define DESCRIPTOR_OK 0
#define DESCRIPTOR_SIZE_PB 1
#define DESCRIPTOR_RATIO_PB 2

#define NB_SOURCE_VIDEOS 3 
bool flag_capture;
char** source_videos;
int selected_video;
int display_mode;


struct descripteur_objet {
  unsigned int xmin, xmax, ymin, ymax;
  float ratiohw; // ratio h/w
  float spacing; // distance from mid
  float spacing_irregularity; //avg diff from average spacing
};




