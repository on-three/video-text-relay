#ifndef __MSG_SCROLL_HPP__
#define __MSG_SCROLL_HPP__

#include <string>
#include <map>
#include <cairo.h>
#include <iostream>

using std::cout;
using std::endl;

class ScrollingMsg
{
public:
  ScrollingMsg()
  :m_current_w(0)
  ,m_current_h(0)
  ,m_friendly_name("None")
  ,m_msg("None")
  ,m_loops(0)
  ,m_current_loop(0)
  ,m_size(0)
  ,m_ypos(0)
  ,m_xpos(0)
  ,m_scroll_time(12.0f)
  {

  };
  ScrollingMsg(const std::string& friendly_name,
    const std::string& msg, 
    const int loop, const int size, const int ypos,
    const int current_w, int current_h, int scroll_time)
  :m_current_w(current_w)
  ,m_current_h(current_h)
  ,m_friendly_name(friendly_name)
  ,m_msg(msg)
  ,m_loops(loop)
  ,m_current_loop(-1)
  ,m_size(size)
  ,m_ypos(ypos)
  ,m_xpos(current_w)
  ,m_scroll_time(scroll_time)
  {

  };

  int CurrentLoop(void)const{return m_current_loop;};

  void Resize(const int width, const int height) {
    m_current_w = width;
    m_current_h = height;
  };
  void Update(const float dt)
  {

  };
  void Draw(cairo_t* context, const float dt)
  {

  //TODO:In current initial state, the previous_timestamp value is invalid.
  //if(s->previous_timestamp<0) {
  //  s->previous_timestamp = timestamp;
  //}
  
  cairo_text_extents_t te;
  cairo_set_source_rgb (context, 1.0, 1.0, 0.0);
  cairo_select_font_face (context, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size (context, 35.0);
  cairo_text_extents (context, m_msg.c_str(), &te);
  //double dt = ((timestamp-s->previous_timestamp)/(double)1e9);//time format is in nanoseconds, so we convert to seconds
  //double dt = ((duration)/(double)1e9);//time format is in nanoseconds, so we convert to seconds
  //s->previous_timestamp = timestamp;
  //printf("elapsed time %f", elapsed_time);
  m_xpos -= ((m_current_w+te.width)/12.0)*dt;//full scroll in 12 seconds.
  if(m_xpos<(-1.0*m_current_w))//wraparound.
  {
    m_xpos = m_current_w;
  }
  //cairo_move_to (context, 0.5 - te.width / 2 - te.x_bearing, 0.5 - te.height / 2 - te.y_bearing);
  cairo_set_source_rgb (context, 0.0, 0.0, 0.0);
  cairo_move_to(context, m_xpos+3, (2*m_current_h/3)+3);
  cairo_show_text (context, m_msg.c_str());
  cairo_set_source_rgb (context, 1.0, 1.0, 0.0);
  cairo_move_to(context, m_xpos, 2*m_current_h/3);
  cairo_show_text (context, m_msg.c_str());
  }

private:
  int m_current_w, m_current_h;
  std::string m_friendly_name;
  std::string m_msg;
  int m_loops;
  int m_current_loop;
  int m_size;
  int m_ypos;
  int m_xpos;
  int m_scroll_time;

private:
  //ScrollingMsg();
  //ScrollingMsg(const ScrollingMsg&);
  //ScrollingMsg operator=(const ScrollingMsg&);

};

class ScrollingMsgController
{
public:
  ScrollingMsgController()
  {

  }

private:
  std::map< std::string, ScrollingMsg > m_msgs;

public:
  void AddMsg(const std::string& friendly_name,
    const std::string& msg, 
    const int loop, const int size, const int ypos,
    const int current_w, int current_h, int scroll_time)
  {
    cout<<__FUNCTION__<<endl;
    if(m_msgs.find(friendly_name)==m_msgs.end()) {
      m_msgs[friendly_name]=ScrollingMsg(friendly_name, msg, loop, size, ypos, current_w, current_h, scroll_time);
    }
  };

  void RemoveMsg(const std::string& friendly_name)
  {
    cout<<__FUNCTION__<<endl;
    std::map< std::string, ScrollingMsg >::iterator msg = m_msgs.find(friendly_name);
    if(msg!=m_msgs.end())
    {
      m_msgs.erase(msg);
    }
  };

  void Update(float dt) {
    cout<<__FUNCTION__<<" "<<dt<<endl;
    for(std::map< std::string, ScrollingMsg >::iterator imsg=m_msgs.begin();
      imsg!=m_msgs.end(); ++imsg)
    {
      //imsg->second.Update(dt);
      //remove those msgs that are 'done'
      //if(imsg->second.CurrentLoop()<0) {
      //  imsg = m_msgs.erase(imsg);
      //}
    }
  };
  void Draw(cairo_t* context, const float dt) {
    cout<<__FUNCTION__<<" "<<dt<<endl;
    for(std::map< std::string, ScrollingMsg >::iterator imsg=m_msgs.begin();
      imsg!=m_msgs.end(); ++imsg)
    {
      imsg->second.Draw(context, dt);
    }
  }



};

#endif