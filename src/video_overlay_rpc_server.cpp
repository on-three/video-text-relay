#include "video_overlay_rpc_server.hpp"
#include <iostream>

using std::cout;
using std::endl;

VideoOverlayRPCServer::VideoOverlayRPCServer()
  :Abstract_video_overlay_rpc_Server(new HttpServer(8080))
  ,m_width(0)
  ,m_height(0)
{
}

std::string VideoOverlayRPCServer::ShowMessage(const std::string& friendly_name, 
  const std::string& msg, const int& x, const int& y)
{
  cout<<"locking show message"<<endl;
  std::lock_guard<std::mutex> l(m_mutex);
  cout<<"Show message locked"<<endl;

  m_scrollingMsgController.AddMsg(friendly_name,
    msg, 
    0, 32, y,
    m_width, m_height, 12.0f);
  return friendly_name;
}

std::string VideoOverlayRPCServer::add_scrolling_msg(const std::string& friendly_name, const int& loop, const std::string& msg, const int& size, const int& y_pos)
{
  std::lock_guard<std::mutex> l(m_mutex);

  m_scrollingMsgController.AddMsg(friendly_name,
    msg, 
    loop, size, y_pos,
    m_width, m_height, 12.0f);
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
}

void VideoOverlayRPCServer::Initialize(void) {
  StartListening();
}
void VideoOverlayRPCServer::Update(float dt) {
  m_scrollingMsgController.Update(dt);
}
void VideoOverlayRPCServer::Draw(cairo_t * cr, float dt) {
  if(m_width==0 or m_height==0) {
    return;
  }
  m_scrollingMsgController.Draw(cr, dt);
}