

#ifndef __VIDEO_OVERLAY_RPC_SERVER_HPP__
#define __VIDEO_OVERLAY_RPC_SERVRE_HPP__

#include "abstract_video_overlay_rpc_server.h"
#include "ScrollingMsg.hpp"
#include <cairo.h>
#include <thread>
#include <mutex>

using namespace jsonrpc;
//using namespace std;

class VideoOverlayRPCServer : public Abstract_video_overlay_rpc_Server {
public:
  VideoOverlayRPCServer();

  virtual std::string ShowMessage(const std::string& friendlyName, const std::string& msg, const int& x, const int& y);
  virtual std::string add_scrolling_msg(const std::string& font, const std::string& friendly_name, const int& loop, const std::string& msg, const double& scroll_time, const int& y_pos);
  virtual std::string remove_scrolling_msg(const std::string& friendly_name);

public:
  void Initialize(void);
  void Resize(const int width, const int height);
  void Update(float dt);
  void Draw(cairo_t * cr, float dt);
private:
  ScrollingMsgController m_scrollingMsgController;
  int m_width;
  int m_height;
  std::mutex m_mutex;
};

#endif