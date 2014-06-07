COMPILER=g++
INCLUDES=`pkg-config --cflags glib-2.0 gstreamer-1.0 cairo` -I/usr/include/pango-1.0
LIBS=`pkg-config --libs glib-2.0 gstreamer-1.0 cairo pangocairo` -lgstvideo-1.0
FLAGS=-std=c++11
SOURCE=relay.cpp video_overlay_rpc_server.cpp main.cpp ScrollingMsg.cpp StaticMessage.cpp
HEADERS=relay.hpp abstract_video_overlay_rpc_server.h colors.h threadsafe_queue.hpp video_overlay_rpc_server.hpp ScrollingMsg.hpp StaticMessage.hpp

all: relay

rebuild: clean all

clean:
	rm -f relay

abstract_video_overlay_rpc_server.h: rpc_spec.json
	jsonrpcstub -s -o ./ rpc_spec.json _video_overlay_rpc_

relay: $(SOURCE) $(HEADERS)
	$(COMPILER) $(FLAGS) $(INCLUDES) $(SOURCE) $(LIBS) -ljsonrpc -o $@
