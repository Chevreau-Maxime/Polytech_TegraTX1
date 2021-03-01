# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#	 notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#	 notice, this list of conditions and the following disclaimer in the
#	 documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#	 contributors may be used to endorse or promote products derived
#	 from this software without specific prior written permission.
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

!INCLUDE ..\..\config.nmake

OS = WinNT
OS_ARCH = x64

BASE_CPPFLAGS = /EHsc /nologo /wd4351

# Debug build flags
!IF $(dbg)
BASE_CPPFLAGS = $(BASE_CPPFLAGS) /Zi /MDd
TARGET = debug
OPENCV_LIB_SUFFIX = d
!ELSE
BASE_CPPFLAGS = $(BASE_CPPFLAGS) /O2 /MD
TARGET = release
OPENCV_LIB_SUFFIX =
!ENDIF

VISION_WORKS_CFLAGS = /I "$(VISIONWORKS_DIR)\include"
VISION_WORKS_LIBS = "$(VISIONWORKS_DIR)\lib\visionworks.lib"
EIGEN_CFLAGS = /I "..\..\3rdparty\eigen"

CUDA_CFLAGS = /I "$(CUDA_DIR)\include" -DUSE_NPP=1
CUDA_LIBS = /LIBPATH:"$(CUDA_DIR)\lib\x64" cudart.lib cublas.lib cufft.lib nppc.lib nppi.lib npps.lib

!IF EXIST("$(OPENCV_DIR)")
OPENCV_CFLAGS = /I "$(OPENCV_DIR)\include" -DUSE_OPENCV=1
OPENCV_LIBS = "$(OPENCV_DIR)\x64\vc12\lib\opencv_*$(OPENCV_VERSION)$(OPENCV_LIB_SUFFIX).lib"
!ENDIF

NVXIO_CFLAGS = -DCUDA_API_PER_THREAD_DEFAULT_STREAM -DUSE_GUI=1 -DUSE_GLFW=1 /I ..\..\nvxio\include -DNVXIO_STATIC_DEFINE
NVXIO_LIBS = ..\..\libs\$(OS_ARCH)\$(OS)\$(TARGET)\nvxio.lib

INCLUDES = $(NVXIO_CFLAGS) $(CUDA_CFLAGS) $(OPENCV_CFLAGS) $(VISION_WORKS_CFLAGS) $(EIGEN_CFLAGS) 

LIBRARIES = $(NVXIO_LIBS) ..\..\3rdparty\freetype\libs\freetype.lib opengl32.lib ..\..\3rdparty\glfw3\libs\glfw3.lib user32.lib gdi32.lib  winmm.lib  $(VISION_WORKS_LIBS) $(CUDA_LIBS) $(OPENCV_LIBS) 

OBJ_DIR = obj\$(TARGET)
OBJ_FILES =  \
	$(OBJ_DIR)\main_player.obj

OUTPUT_DIR = ..\..\bin\$(OS_ARCH)\$(OS)\$(TARGET)

################################################################################

# Target rules
all: build

build: $(OUTPUT_DIR)\nvx_sample_player.exe

.cpp{$(OBJ_DIR)}.obj::
	if NOT EXIST "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	$(CPP) $(BASE_CPPFLAGS) $(INCLUDES) $(CPPFLAGS) /Fo"$(OBJ_DIR)\\" /c $<

$(OUTPUT_DIR)\nvx_sample_player.exe: $(OBJ_FILES) $(NVXIO_LIBS)
	if NOT EXIST "$(OUTPUT_DIR)" mkdir "$(OUTPUT_DIR)"
	xcopy /d /y "$(OPENCV_DIR)\x64\vc12\bin\*.dll" "$(OUTPUT_DIR)"
	xcopy /d /y "$(VISIONWORKS_DIR)\bin\*.dll" "$(OUTPUT_DIR)"
	$(CPP) $(BASE_CPPFLAGS) /Fe"$@" $** /link $(LIBRARIES)

run: build
	$(OUTPUT_DIR)\nvx_sample_player.exe

clean:
	-del $(OUTPUT_DIR)\nvx_sample_player.exe
	-rd /S /Q "$(OBJ_DIR)"

$(NVXIO_LIBS):
	cd ../../nvxio && $(MAKE) /$(MAKEOPTS) /nologo
