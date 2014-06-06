#include "video_overlay_rpc_server.hpp"


VideoOverlayRPCServer::VideoOverlayRPCServer()
  :Abstract_video_overlay_rpc_Server(new HttpServer(8080))
  ,m_width(0)
  ,m_height(0)
{
}

std::string VideoOverlayRPCServer::ShowMessage(const std::string& friendly_name, 
  const std::string& msg, const int& x, const int& y)
{
  std::lock_guard<std::mutex> l(m_mutex);

  //TODO: REMOVE THIS LEGACY INTERFACE
  return friendly_name;
}

std::string VideoOverlayRPCServer::add_scrolling_msg(const std::string& font, 
  const std::string& friendly_name, 
  const int& loop, 
  const std::string& msg, 
  const double& scroll_time, 
  const int& y_pos)
{
  std::lock_guard<std::mutex> l(m_mutex);

  m_scrollingMsgController.AddMsg(m_width, m_height,
    font, friendly_name, loop, msg, scroll_time, y_pos);
  return friendly_name;
}
std::string VideoOverlayRPCServer::remove_scrolling_msg(const std::string& friendly_name)
{
  std::lock_guard<std::mutex> l(m_mutex);

  m_scrollingMsgController.RemoveMsg(friendly_name);
  return friendly_name;
}

void VideoOverlayRPCServer::Resize(const int width, const int height)
{
  m_width = width;
  m_height = height;
  m_scrollingMsgController.Resize(width, height);
}

void VideoOverlayRPCServer::Initialize(void) {
  StartListening();
}
void VideoOverlayRPCServer::Update(float dt) {
  std::lock_guard<std::mutex> l(m_mutex);
  m_scrollingMsgController.Update(dt);
}
void VideoOverlayRPCServer::Draw(cairo_t * cr, float dt) {
  if(m_width==0 or m_height==0) {
    return;
  }
  std::lock_guard<std::mutex> l(m_mutex);
  m_scrollingMsgController.Draw(cr, dt);
}