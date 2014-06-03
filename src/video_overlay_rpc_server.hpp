

#ifndef __VIDEO_OVERLAY_RPC_SERVER_HPP__
#define __VIDEO_OVERLAY_RPC_SERVRE_HPP__

#include "colors.h"
#include "abstract_video_overlay_rpc_server.h"
#include "threadsafe_queue.hpp"

using namespace jsonrpc;
using namespace std;

class VideoOverlayRPCServer : public Abstract_video_overlay_rpc_Server {
public:
  VideoOverlayRPCServer(Queue<std::string>* queue);

  virtual std::string ShowMessage(const std::string& friendlyName, const std::string& msg, const int& x, const int& y);
  virtual std::string add_scrolling_msg(const std::string& friendly_name, const int& loop, const std::string& msg, const int& size, const int& y_pos);
  virtual std::string remove_scrolling_msg(const std::string& friendly_name);
private:
  Queue<std::string>* m_queue;
};

#endif