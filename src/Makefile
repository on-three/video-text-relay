COMPILER=g++
INCLUDES=`pkg-config --cflags glib-2.0 gstreamer-1.0 cairo` -I/usr/include/pango-1.0
LIBS=`pkg-config --libs glib-2.0 gstreamer-1.0 cairo pangocairo` -lgstvideo-1.0
FLAGS=-std=c++11
SOURCE=relay.cpp video_overlay_rpc_server.cpp main.cpp ScrollingMsg.cpp StaticMessage.cpp utilities.cpp text.cpp NikoNikoDisplay.cpp
HEADERS=relay.hpp abstract_video_overlay_rpc_server.h colors.h threadsafe_queue.hpp video_overlay_rpc_server.hpp ScrollingMsg.hpp StaticMessage.hpp utilities.hpp text.hpp NikoNikoDisplay.hpp
EXE=video-text-relay

all: $(EXE)

debug: $(SOURCE) $(HEADERS)
	$(COMPILER) $(FLAGS) -g $(INCLUDES) $(SOURCE) $(LIBS) -ljsonrpc -o $(EXE)-debug

rebuild: clean all

clean:
	rm -f video-text-relay

abstract_video_overlay_rpc_server.h: rpc_spec.json
	jsonrpcstub -s -o ./ rpc_spec.json _video_overlay_rpc_

$(EXE): $(SOURCE) $(HEADERS)
	$(COMPILER) $(FLAGS) $(INCLUDES) $(SOURCE) $(LIBS) -ljsonrpc -o $@
