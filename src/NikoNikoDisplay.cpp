#include "NikoNikoDisplay.hpp"
#include "utilities.hpp"
#include <algorithm>

bool NikoNikoMsg::sort_by_ypos(std::shared_ptr< NikoNikoMsg> first, std::shared_ptr<NikoNikoMsg> second) {
  return (first->m_text.Y()<second->m_text.Y());
}

NikoNikoMsg::NikoNikoMsg()
  :m_current_w(0)
  ,m_current_h(0)
  ,m_scroll_time(12.0f)
  ,m_text()
{

};
NikoNikoMsg::NikoNikoMsg(const int width, 
    const int height,
    const std::string& font, 
    const std::string& msg,
    const std::string& nick,
    const std::string& channel,
    const double& scroll_time, 
    const int& y_pos,
    const bool dropshadow,
    const bool underlay)
  :m_current_w(width)
  ,m_current_h(height)
  ,m_scroll_time(scroll_time)
  ,m_text(msg, width, -1, font, dropshadow, underlay)
  ,m_nick(nick)
  ,m_channel(channel)
{

};

NikoNikoMsg::~NikoNikoMsg() {

}

void NikoNikoMsg::Resize(const int width, const int height) {
  m_current_w = width;
  m_current_h = height;
};
bool NikoNikoMsg::Update(const float dt)
{
  
  int xpos = m_text.X();
  const int text_width = m_text.width();
  if(text_width<0) {
    return false;
  }
  //d_pos = d/t * dt
  int dx = ((m_current_w + text_width)/m_scroll_time)*dt;
  if(dx<0) {
    dx = 0;
  }
  xpos-=dx;
  if(xpos<(-1.0f*m_text.width())) {
    return true;//this message is "complete" when it's scrolled past the left window edge
  }
  m_text.X(xpos);
  return false;
};

void NikoNikoMsg::Draw(cairo_t* context)
{
  m_text.Draw(context);
}

void NikoNikoMsg::Initialize(cairo_t* context)
{
  m_text.LazyInitialization(context);
}


NikoNikoMsgController::NikoNikoMsgController()
{

}

void NikoNikoMsgController::AddMsg(
    const int width, 
    const int height,
    const std::string& msg,
    const std::string& nick,
    const std::string& channel)
{
  //we add these messages asynchronously, so this new element's true width
  //and height can't be figured until the next 'Draw' method call (lazy initialization)
  //so we just assign y_pos here to -1, and we'll assign a correct value asap.
  m_pending_msgs.push_back(std::shared_ptr<NikoNikoMsg>(new NikoNikoMsg(width, height, std::string("Sans Bold 24"), 
      msg, nick, channel, 10.0f, -1, true/*dropshadow*/, false/*underlay*/)));
}

void NikoNikoMsgController::Update(float dt) {
  for(std::list< std::shared_ptr< NikoNikoMsg > >::iterator imsg=m_msgs.begin();
    imsg!=m_msgs.end();)
  {
    if((*imsg)->Update(dt)) {
      imsg = m_msgs.erase(imsg);
    }else{
      ++imsg;
    }
  }
};
void NikoNikoMsgController::Draw(cairo_t* context) {
  for(std::list< std::shared_ptr< NikoNikoMsg > >::iterator imsg=m_msgs.begin();
    imsg!=m_msgs.end(); ++imsg)
  {
    (*imsg)->Draw(context);
  }
  //Okay. at this point all new messages will have gone through lazy init,
  //so their height and width for the current drawing context will be correct.
  //we can now correctly set their y_pos value (initialized to -1) to interact
  //with the currently displayed messages correctly.
  if(!m_pending_msgs.empty()) {

    for(std::list< std::shared_ptr< NikoNikoMsg > >::iterator imsg= m_pending_msgs.begin();
      imsg!=m_pending_msgs.end(); ++imsg) {
      (*imsg)->Initialize(context);
      const int w = (*imsg)->width();
      const int h = (*imsg)->height();
      const int y = FindMsgYPos(w,h);
      (*imsg)->Y(y);

      m_msgs.push_back(*imsg);
    }
    m_pending_msgs.clear();
  }
}

int NikoNikoMsgController::FindMsgYPos(const int width, const int height) {
  m_msgs.sort(NikoNikoMsg::sort_by_ypos);
  int new_y=20;
  for(std::list< std::shared_ptr< NikoNikoMsg > >::iterator imsg=m_msgs.begin();
      imsg!=m_msgs.end(); ++imsg) {
    const int y = (*imsg)->Y();
    const int h = (*imsg)->height();
    if(y>new_y+height) {
      return new_y;
    }else{
      new_y = y+h;
    }
  }
  return new_y;
}

void NikoNikoMsgController::Resize(const int width, const int height) {
  for(std::list< std::shared_ptr< NikoNikoMsg > >::iterator imsg=m_msgs.begin();
    imsg!=m_msgs.end(); ++imsg)
  {
    (*imsg)->Resize(width, height);
  }
}
