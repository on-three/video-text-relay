/*
NikoNikoDisplay.hpp
on_three
Saturday June 14th 2014

Replicate a simple niko niko IRC display atop a video stream

*/
#ifndef __NIKO_NIKO_DISPLAY_HPP__
#define __NIKO_NIKO_DISPLAY_HPP__

#include <string>
#include <list>
#include <cairo.h>
#include <pango/pangocairo.h>
#include <memory>
#include "text.hpp"


class NikoNikoMsg
{
public:
  NikoNikoMsg();
  NikoNikoMsg(const int width, 
    const int height,
    const std::string& font, 
    const std::string& msg,
    const std::string& nick,
    const std::string& channel,
    const double& scroll_time, 
    const int& y_pos,
    const bool dropshadow,
    const bool underlay);
  ~NikoNikoMsg();

public:
  void Resize(const int width, const int height);
  bool Update(const float dt);
  void Draw(cairo_t* context);
public:
  void Initialize(cairo_t* context);

public:
  int X(void)const{return m_text.X();};
  void X(const int value){m_text.X(value);};
  int Y(void)const{return m_text.Y();};
  void Y(const int value){m_text.Y(value);};
  int width(void)const{return m_text.width();};
  int height(void)const{return m_text.height();};

public:
  static bool sort_by_ypos(std::shared_ptr< NikoNikoMsg> first, std::shared_ptr<NikoNikoMsg> second);

private:
  int m_current_w, m_current_h;
  //std::string m_friendly_name;
  //int m_loops;
  //int m_current_loop;
  int m_scroll_time;
  overlay::Text m_text;
  std::string m_nick;
  std::string m_channel;
};

class NikoNikoMsgController
{
public:
  NikoNikoMsgController();

public:
  void AddMsg(const int width, 
    const int height,
    const std::string& msg,
    const std::string& nick,
    const std::string& channel);
public:
  void Update(float dt);
  void Draw(cairo_t* context);
  void Resize(const int width, const int height);

private:
  int FindMsgYPos(const int width, const int height);

private:
  std::list< std::shared_ptr< NikoNikoMsg > > m_msgs;
  //the text displayed width and height are not immediately available
  //so we store them in a pending list before integration into the proper list above.
  std::list< std::shared_ptr< NikoNikoMsg > > m_pending_msgs;
};

#endif
