/*
# Copyright (c) 2014-2016, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>

#define WINDOWS

#ifdef WINDOWS

#include "NVXIO/Application.hpp"
#include "NVXIO/FrameSource.hpp"
#include "NVXIO/Render.hpp"
#include "NVXIO/SyncTimer.hpp"
#include "NVXIO/Utility.hpp"
#include <NVX\nvx.h>
#include <NVX\nvx_timer.hpp>

using namespace nvxio;
#else

#include "NVX/Application.hpp"
#include "OVX/FrameSourceOVX.hpp"
#include "OVX/RenderOVX.hpp"
#include "NVX/SyncTimer.hpp"
#include "OVX/UtilityOVX.hpp"

using namespace nvxio;
using namespace ovxio;
#endif

#include "traitement.h"


/*
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
*/

bool flag_capture_bkg;

struct EventData
{
    EventData(): alive(true), pause(false) {}

    bool alive;
    bool pause;
};

static void keyboardEventCallback(void* context, vx_char key, vx_uint32 /*x*/, vx_uint32 /*y*/)
{
    EventData* eventData = static_cast<EventData*>(context);
    if (key == 27) // escape
    {
        eventData->alive = false;
    }
    else if (key == 32)
    {
        eventData->pause = !eventData->pause;
    }
    else if (key == 'c')
    {
        flag_capture_bkg=true;
    }
    InputTraitement(key);
}

//
// main - Application entry point
//

int main(int argc, char** argv)
{
    try
    {
        Application &app = Application::get();
        std::string input = app.findSampleFilePath("cars.mp4");

        app.setDescription("This sample plays a video from video-file or camera");
        app.addOption('s', "source", "Input URI", nvxio::OptionHandler::string(&input));
        app.init(argc, argv);

        //
        // Create OpenVX context
        //

        ContextGuard context;
        vxRegisterLogCallback(context, &stdoutLogCallback, vx_false_e);

        //
        // Create a Frame Source
        //

        std::unique_ptr<FrameSource> source(createDefaultFrameSource(context, input));
        if (!source || !source->open())
        {
            std::cerr << "Error: Can't open source URI " << input << std::endl;
            return nvxio::Application::APP_EXIT_CODE_NO_RESOURCE;
        }
        FrameSource::Parameters config = source->getConfiguration();

        //
        // Create a Render
        //

        std::unique_ptr<Render> render(createDefaultRender(
                    context, "Player Sample", config.frameWidth*2, config.frameHeight));
        if (!render)
        {
            std::cout << "Error: Cannot open default render!" << std::endl;
            return nvxio::Application::APP_EXIT_CODE_NO_RENDER;
        }

        EventData eventData;
        render->setOnKeyboardEventCallback(keyboardEventCallback, &eventData);

        vx_image frame = vxCreateImage(context, config.frameWidth, config.frameHeight, config.format);
        vx_image defaultFrame = vxCreateImage(context, config.frameWidth, config.frameHeight, config.format);
        flag_capture_bkg = true;


        NVXIO_CHECK_REFERENCE(frame);

        Render::TextBoxStyle style = {{255,255,255,255}, {0,0,127,127}, {10,10}};

        std::unique_ptr<SyncTimer> syncTimer = createSyncTimer();
        syncTimer->arm(1. / app.getFPSLimit());

        nvx::Timer totalTimer;
        totalTimer.tic();

		///Init
		vx_image dstImg = vxCreateImage(context, 2 * config.frameWidth, config.frameHeight, config.format);
		InitTraitement(context);

        while(eventData.alive)
        {
            FrameSource::FrameStatus status = FrameSource::OK;
            if (!eventData.pause)
            {
                status = source->fetch(frame);
                if (flag_capture_bkg){
                  defaultFrame = vxCreateImage(context, config.frameWidth, config.frameHeight, config.format);
                  nvxuCopyImage(context, frame, defaultFrame);
                  flag_capture_bkg = false;
                }
            }

            switch(status)
            {
            case nvxio::FrameSource::OK:
            {
                double total_ms = totalTimer.toc();

                //std::cout << "NO PROCESSING" << std::endl;
                //std::cout << "Display Time : " << total_ms << " ms" << std::endl << std::endl;

                syncTimer->synchronize();

                total_ms = totalTimer.toc();
                totalTimer.tic();

                std::ostringstream txt;
                txt << std::fixed << std::setprecision(1);

                txt << "Source size: " << config.frameWidth << 'x' << config.frameHeight << std::endl;
                txt << "Algorithm: " << "No Processing" << std::endl;
                txt << "Display: " << total_ms  << " ms / " << 1000.0 / total_ms << " FPS" << std::endl;

                txt << std::setprecision(6);
                txt.unsetf(std::ios_base::floatfield);

                txt << "LIMITED TO " << app.getFPSLimit() << " FPS FOR DISPLAY" << std::endl;
                txt << "Space - pause/resume" << std::endl;
                txt << "Esc - close the demo";


				//Traitement
				MainTraitement(config, context, app, dstImg, frame);

				//Render
                render->putImage(dstImg);
                render->putTextViewport(txt.str(), style);

                if (!render->flush())
                    eventData.alive = false;
            } break;
            case nvxio::FrameSource::TIMEOUT:
            {
                // Do nothing
            } break;
            case nvxio::FrameSource::CLOSED:
            {
                // Reopen
                if (!source->open())
                {
                    std::cerr << "Error: Failed to reopen the source" << std::endl;
                    eventData.alive = false;
                }
            } break;
            }

        }

        //
        // Release all objects
        //
        vxReleaseImage(&frame);

    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return nvxio::Application::APP_EXIT_CODE_ERROR;
    }

    return nvxio::Application::APP_EXIT_CODE_SUCCESS;
}
