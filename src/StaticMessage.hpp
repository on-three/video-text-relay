/*
StaticMsg.hpp
on_three
Tuesday, June 3rd 2014

Static msg display superimposed on video stream.

Controller allows multiple messages to be managed.


*/
#ifndef __MSG_STATIC_HPP__
#define __MSG_STATIC_HPP__

#include <string>
#include <map>
#include <cairo.h>
#include <pango/pangocairo.h>
#include "text.hpp"


class StaticMsg
{
public:
  StaticMsg();
  StaticMsg(const int width, 
    const int height,
    const std::string& font, 
    const std::string& friendly_name, 
    const std::string& msg, 
    const int x, const int y,
    const float timeout,
    const bool dropshadow,
    const bool underlay);
  ~StaticMsg();

public:
  void Resize(const int width, const int height);
  bool Update(const float dt);;
  void Draw(cairo_t* context, const float dt);

private:
  int m_current_w, m_current_h;
  std::string m_friendly_name;
  float m_elapsedtime;
  float m_timeout;
  overlay::Text m_text;
};

class StaticMsgController
{
public:
  StaticMsgController();

public:
  void AddMsg(const int width, 
    const int height,
    const std::string& font, 
    const std::string& friendly_name, 
    const std::string& msg, 
    const int x, const int y,
    const float timeout,
    const bool dropshadow, const bool underlay);
  void RemoveMsg(const std::string& friendly_name);

public:
  void Update(float dt);
  void Draw(cairo_t* context, const float dt);
  void Resize(const int width, const int height);

private:
  std::map< std::string, StaticMsg > m_msgs;
};

#endif
