#include "ScrollingMsg.hpp"
#include "utilities.hpp"
#include <iostream>
using std::cout;
using std::endl;

ScrollingMsg::ScrollingMsg()
  :m_current_w(0)
  ,m_current_h(0)
  ,m_friendly_name("None")
  ,m_loops(0)
  ,m_current_loop(0)
  ,m_scroll_time(12.0f)
  ,m_text()
{

};
ScrollingMsg::ScrollingMsg( const int width, 
                            const int height,
                            const std::string& font, 
                            const std::string& friendly_name, 
                            const int& loop, 
                            const std::string& msg, 
                            const double& scroll_time,
                            const int& y_pos,
                            const bool dropshadow,
                            const bool underlay)
  :m_current_w(width)
  ,m_current_h(height)
  ,m_friendly_name(friendly_name)
  ,m_loops(loop)
  ,m_current_loop(0)
  ,m_scroll_time(scroll_time)
  ,m_text(msg, width, y_pos, font, dropshadow, underlay)
{

};

ScrollingMsg::~ScrollingMsg() {

}

void ScrollingMsg::Resize(const int width, const int height) {
  m_current_w = width;
  m_current_h = height;
};
bool ScrollingMsg::Update(const float dt)
{
  //d_pos = d/t * dt
  int xpos = m_text.X();
  xpos -= ((m_current_w + m_text.width())/m_scroll_time)*dt;
  if(xpos<(-1.0f*m_text.width())) {//wraparound
    xpos = m_current_w;
    m_current_loop += 1;
  }
  m_text.X(xpos);
  return (m_loops > 0 && m_current_loop >= m_loops);
};

void ScrollingMsg::Draw(cairo_t* context, const float dt)
{
  m_text.Draw(context, dt);
}


ScrollingMsgController::ScrollingMsgController()
{

}

void ScrollingMsgController::AddMsg(const int width, 
  const int height, 
  const std::string& font, 
  const std::string& friendly_name, 
  const int& loop, 
  const std::string& msg, 
  const double& scroll_time, 
  const int& y_pos,
  const bool dropshadow,
  const bool underlay)
{
    m_msgs[friendly_name]=ScrollingMsg(width, height, font, friendly_name, loop, msg, scroll_time, y_pos, dropshadow, underlay);
}

void ScrollingMsgController::RemoveMsg(const std::string& friendly_name)
{
  std::map< std::string, ScrollingMsg >::iterator msg = m_msgs.find(friendly_name);
  if(msg!=m_msgs.end())
  {
    m_msgs.erase(msg);
  }
};

void ScrollingMsgController::Update(float dt) {
  for(std::map< std::string, ScrollingMsg >::iterator imsg=m_msgs.begin();
    imsg!=m_msgs.end();)
  {
    if(imsg->second.Update(dt)) {
      imsg = m_msgs.erase(imsg);
    }else{
      ++imsg;
    }
  }
};
void ScrollingMsgController::Draw(cairo_t* context, const float dt) {
  for(std::map< std::string, ScrollingMsg >::iterator imsg=m_msgs.begin();
    imsg!=m_msgs.end(); ++imsg)
  {
    imsg->second.Draw(context, dt);
  }
}

void ScrollingMsgController::Resize(const int width, const int height) {
  for(std::map< std::string, ScrollingMsg >::iterator imsg=m_msgs.begin();
    imsg!=m_msgs.end(); ++imsg)
  {
    imsg->second.Resize(width, height);
  }
}
