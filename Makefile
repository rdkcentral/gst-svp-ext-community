##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2019 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
CXXFLAGS += -Wno-attributes -Wall -g -fpermissive -std=c++1y -fPIC
EXTRA_LDFLAGS = -lglib-2.0 -Wl,-rpath=../../,-rpath=./ -L./

# Link to RDKPerf component
EXTRA_LDFLAGS += -lrdkperf -lperftool

SOURCES = \
	gst_svp_meta.cpp \
	gst_svp_performance.cpp \
	gst_svp_logging.cpp \
	gst_svp_scopedlock.cpp \
	gst_svp_secure_buffers_default.cpp \
	gst_svp_header.cpp

include device/platform.inc

OBJS=$(addsuffix .o, $(basename $(SOURCES)))
LIBCOMSVPMETA_LIB=libgstsvpext.so

.phony: lib clean

ifeq ($(PLATFORM_SVP),AMLOGIC)
lib: $(LIBCOMSVPMETA_LIB) libgstsvppay.so
else
lib: $(LIBCOMSVPMETA_LIB)
endif
%.o: %.cpp
	 @echo Compiling $<...
	 $(CXX) -c $< $(CXXFLAGS) -o $@

$(LIBCOMSVPMETA_LIB):  $(OBJS)
	@echo Dynamic library creating $(OBJS) ...
	$(CXX) $(OBJS) $(EXTRA_LDFLAGS) -shared -fPIC -o $@

ifeq ($(PLATFORM_SVP),AMLOGIC)
libgstsvppay.so: Makefile plugins/gst-svp-payload.cpp $(LIBCOMSVPMETA_LIB)
	$(CXX) $(CXXFLAGS) -DPLATFORM_SVP_$(PLATFORM_SVP)=1 plugins/gst-svp-payload.cpp  $(EXTRA_LDFLAGS) -lgstsvpext -shared -Wl,-soname,libgstsvppay.so -o libgstsvppay.so  
endif

clean:
	@rm -rf $(LIBCOMSVPMETA_LIB)
ifeq ($(PLATFORM_SVP),AMLOGIC)
	@rm -rf libgstsvppay.so
endif

cleanall:
	@rm -rf $(LIBCOMSVPMETA_LIB) *.o device/*.o
ifeq ($(PLATFORM_SVP),AMLOGIC2)
	@rm -rf libgstsvppay.so
endif

