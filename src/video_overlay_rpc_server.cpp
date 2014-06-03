#include "video_overlay_rpc_server.hpp"

VideoOverlayRPCServer::VideoOverlayRPCServer(Queue<std::string>* queue)
  :Abstract_video_overlay_rpc_Server(new HttpServer(8080))
  ,m_queue(queue)
{
}

std::string VideoOverlayRPCServer::ShowMessage(const std::string& friendly_name, 
  const std::string& msg, const int& x, const int& y)
{
  //cout<<"Showing message\'"<<msg<<"\' at "<<x<<","<<y<<" named "<<friendly_name<<endl;
  m_queue->push(msg);
  return friendly_name;
}

std::string VideoOverlayRPCServer::add_scrolling_msg(const std::string& friendly_name, const int& loop, const std::string& msg, const int& size, const int& y_pos)
{
  return friendly_name;
}
std::string VideoOverlayRPCServer::remove_scrolling_msg(const std::string& friendly_name)
{
  return friendly_name;
}