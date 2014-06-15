#include "video_overlay_rpc_server.hpp"


VideoOverlayRPCServer::VideoOverlayRPCServer(const int port)
  :Abstract_video_overlay_rpc_Server(new HttpServer(port))
  ,m_width(0)
  ,m_height(0)
{
}

std::string VideoOverlayRPCServer::add_scrolling_msg(const bool& dropshadow, 
    const std::string& font, 
    const std::string& friendly_name, 
    const int& loop, 
    const std::string& msg, 
    const double& scroll_time, 
    const bool& underlay, 
    const int& y_pos)
{
  m_scrollingMsgController.AddMsg(m_width, m_height,
    font, friendly_name, loop, msg, scroll_time, y_pos, dropshadow, underlay);
  return friendly_name;
}
std::string VideoOverlayRPCServer::remove_scrolling_msg(const std::string& friendly_name)
{

  m_scrollingMsgController.RemoveMsg(friendly_name);
  return friendly_name;
}

std::string VideoOverlayRPCServer::add_msg(const bool& dropshadow, 
    const std::string& font, 
    const std::string& friendly_name, 
    const std::string& msg,
    const double& timeout,
    const bool& underlay, 
    const int& x, const int& y)
{

  m_staticMsgController.AddMsg(m_width, m_height,
      font, 
      friendly_name, 
      msg, 
      x, y,
      static_cast<float>(timeout),
      dropshadow, underlay);
  
  return friendly_name;
}
std::string VideoOverlayRPCServer::remove_msg(const std::string& friendly_name) {

  m_staticMsgController.RemoveMsg(friendly_name);

  return friendly_name;
}

std::string VideoOverlayRPCServer::irc_privmsg(const std::string& channel, 
  const std::string& msg,
  const std::string& nick)
{
  m_nikoNikoMsgController.AddMsg(m_width, m_height,
    msg,
    nick,
    channel);
  return channel;
}

void VideoOverlayRPCServer::Resize(const int width, const int height)
{
  m_width = width;
  m_height = height;
  m_scrollingMsgController.Resize(width, height);
  m_staticMsgController.Resize(width, height);
  m_nikoNikoMsgController.Resize(width, height);
}

void VideoOverlayRPCServer::Initialize(void) {
  StartListening();
}
void VideoOverlayRPCServer::Update(float dt) {
  m_scrollingMsgController.Update(dt);
  m_staticMsgController.Update(dt);
  m_nikoNikoMsgController.Update(dt);
}
void VideoOverlayRPCServer::Draw(cairo_t * cr) {
  if(m_width==0 or m_height==0) {
    return;
  }
  //TODO: better handling for layering of text
  m_scrollingMsgController.Draw(cr);
  m_staticMsgController.Draw(cr);
  m_nikoNikoMsgController.Draw(cr);
}