#include "StaticMessage.hpp"
#include "utilities.hpp"

#include <iostream>
using std::cout;
using std::endl;

StaticMsg::StaticMsg()
  :m_current_w(0)
  ,m_current_h(0)
  ,m_friendly_name("None")
  ,m_elapsedtime(0.0f)
  ,m_timeout(0.0f)
  ,m_text()
{

};
StaticMsg::StaticMsg(const int width, 
    const int height,
    const std::string& font, 
    const std::string& friendly_name, 
    const std::string& msg, 
    const int x, const int y,
    const float timeout,
    const bool dropshadow,
    const bool underlay)
  :m_current_w(width)
  ,m_current_h(height)
  ,m_friendly_name(friendly_name)
  ,m_elapsedtime(0.0f)
  ,m_timeout(timeout)
  ,m_text(msg, x, y, font, dropshadow, underlay)
{

};

StaticMsg::~StaticMsg() {
  
}

void StaticMsg::Resize(const int width, const int height) {
  m_current_w = width;
  m_current_h = height;

  //TODO: Invalidate lazy initialization so we can adapt to new resolutions.
}

bool StaticMsg::Update(const float dt)
{
  m_text.Update(dt);

  m_elapsedtime += dt;
  if(m_timeout>0.0f && m_elapsedtime>m_timeout) {
    return true;
  }
  return false;
}

void StaticMsg::Draw(cairo_t* context, const float dt)
{
  m_text.Draw(context, dt);
}



StaticMsgController::StaticMsgController()
{

}

void StaticMsgController::AddMsg(const int width, 
    const int height,
    const std::string& font, 
    const std::string& friendly_name, 
    const std::string& msg, 
    const int x, const int y,
    const float timeout,
    const bool dropshadow,
    const bool underlay)
{
  m_msgs[friendly_name]=StaticMsg(width, height, font, friendly_name, msg, x, y, timeout, dropshadow, underlay);
}

void StaticMsgController::RemoveMsg(const std::string& friendly_name)
{
  std::map< std::string, StaticMsg >::iterator msg = m_msgs.find(friendly_name);
  if(msg!=m_msgs.end())
  {
    m_msgs.erase(msg);
  }
};

void StaticMsgController::Update(float dt) {
  for(std::map< std::string, StaticMsg >::iterator imsg=m_msgs.begin();
    imsg!=m_msgs.end();)
  {
    if(imsg->second.Update(dt)) {
      imsg = m_msgs.erase(imsg);
    }else{
      ++imsg;
    }
  }
};
void StaticMsgController::Draw(cairo_t* context, const float dt) {
  for(std::map< std::string, StaticMsg >::iterator imsg=m_msgs.begin();
    imsg!=m_msgs.end(); ++imsg)
  {
    imsg->second.Draw(context, dt);
  }
}

void StaticMsgController::Resize(const int width, const int height) {
  for(std::map< std::string, StaticMsg >::iterator imsg=m_msgs.begin();
    imsg!=m_msgs.end(); ++imsg)
  {
    imsg->second.Resize(width, height);
  }
}
