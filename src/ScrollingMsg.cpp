#include "ScrollingMsg.hpp"

ScrollingMsg::ScrollingMsg()
  :m_current_w(0)
  ,m_current_h(0)
  ,m_friendly_name("None")
  ,m_msg("None")
  ,m_loops(0)
  ,m_current_loop(0)
  ,m_size(0)
  ,m_ypos(300)
  ,m_xpos(0)
  ,m_scroll_time(12.0f)
{

};
ScrollingMsg::ScrollingMsg(const std::string& friendly_name,
  const std::string& msg, 
  const int loop, const int size, const int ypos,
  const int current_w, int current_h, int scroll_time)
  :m_current_w(current_w)
  ,m_current_h(current_h)
  ,m_friendly_name(friendly_name)
  ,m_msg(msg)
  ,m_loops(loop)
  ,m_current_loop(0)
  ,m_size(size)
  ,m_ypos(ypos)
  ,m_xpos(current_w)
  ,m_scroll_time(scroll_time)
{

};

void ScrollingMsg::Resize(const int width, const int height) {
  m_current_w = width;
  m_current_h = height;
};
void ScrollingMsg::Update(const float dt)
{
  //TODO: separate update and drawing of msg to facilitate different ordering.
};
void ScrollingMsg::Draw(cairo_t* context, const float dt)
{

  cairo_text_extents_t te;
  cairo_set_source_rgb (context, 1.0, 1.0, 0.0);
  cairo_select_font_face (context, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size (context, 35.0);
  cairo_text_extents (context, m_msg.c_str(), &te);

  //d_pos = d/t * dt
  m_xpos -= ((m_current_w + te.width)/m_scroll_time)*dt;
  if(m_xpos<(-1.0f*te.width)) {//wraparound
    m_xpos = m_current_w;
    m_current_loop += 1;
    if(m_current_loop==m_loops) {
      m_current_loop = -1;//indicates controller should remove this msg.
    }
  }
  cairo_set_source_rgb (context, 0.0, 0.0, 0.0);
  cairo_move_to(context, m_xpos+3, m_ypos+3);
  cairo_show_text (context, m_msg.c_str());
  cairo_set_source_rgb (context, 1.0, 1.0, 0.0);
  cairo_move_to(context, m_xpos, m_ypos);
  cairo_show_text (context, m_msg.c_str());
}



ScrollingMsgController::ScrollingMsgController()
{

}

void ScrollingMsgController::AddMsg(const std::string& friendly_name,
  const std::string& msg, 
  const int loop, const int size, const int ypos,
  const int current_w, int current_h, int scroll_time)
{
  if(m_msgs.find(friendly_name)==m_msgs.end()) {
    m_msgs[friendly_name]=ScrollingMsg(friendly_name, msg, loop, size, ypos, current_w, current_h, scroll_time);
  }
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
    imsg->second.Update(dt);
    //remove those msgs that are 'done'
    if(imsg->second.CurrentLoop()<0) {
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
