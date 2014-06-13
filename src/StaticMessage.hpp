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
    const bool dropshadow,
    const bool underlay);
  ~StaticMsg();

public:
  void Resize(const int width, const int height);
  void Update(const float dt);;
  void Draw(cairo_t* context, const float dt);

private:
  void LazyInitialization(cairo_t* context);

private:
  int m_current_w, m_current_h;
  std::string m_friendly_name;
  std::string m_msg;
  std::string m_fontfamily;
  int m_ypos;
  int m_xpos;
  bool m_dropshadow;
  bool m_underlay;
  PangoLayout *pango_layout;
  PangoFontDescription *pango_fontdesc;
  PangoAttrList* pTextAttributes;
  PangoAttrList* no_color_attributes;
  std::string displayed_text;
  PangoRectangle ink_rect, logical_rect;
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
