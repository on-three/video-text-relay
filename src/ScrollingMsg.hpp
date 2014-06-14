/*
ScrollingMsg.hpp
on_three
Tuesday, June 3rd 2014

Scrolling msg display superimposed on video stream.

Controller allows multiple messages to be managed.


*/
#ifndef __MSG_SCROLL_HPP__
#define __MSG_SCROLL_HPP__

#include <string>
#include <map>
#include <cairo.h>
#include <pango/pangocairo.h>
#include "text.hpp"


class ScrollingMsg
{
public:
  ScrollingMsg();
  ScrollingMsg(const int width, 
    const int height,
    const std::string& font, 
    const std::string& friendly_name, 
    const int& loop, 
    const std::string& msg, 
    const double& scroll_time, 
    const int& y_pos,
    const bool dropshadow,
    const bool underlay);
  ~ScrollingMsg();


public:
  int CurrentLoop(void)const{return m_current_loop;};

public:
  void Resize(const int width, const int height);
  bool Update(const float dt);
  void Draw(cairo_t* context);

private:
  int m_current_w, m_current_h;
  std::string m_friendly_name;
  int m_loops;
  int m_current_loop;
  int m_scroll_time;
  overlay::Text m_text;
};

class ScrollingMsgController
{
public:
  ScrollingMsgController();

public:
  void AddMsg(const int width, 
    const int height, 
    const std::string& font, 
    const std::string& friendly_name, 
    const int& loop, 
    const std::string& msg, 
    const double& scroll_time, 
    const int& y_pos,
    const bool dropshadow,
    const bool underlay);
  void RemoveMsg(const std::string& friendly_name);

public:
  void Update(float dt);
  void Draw(cairo_t* context);
  void Resize(const int width, const int height);

private:
  std::map< std::string, ScrollingMsg > m_msgs;
};

#endif
