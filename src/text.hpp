/*
text.hpp
John O'Neil
Friday June 6th 2014

Refactoring cairo text display atop video stream to single
implementation

*/
#ifndef __TEXT_HPP__
#define __TEXT_HPP__

#include <string>
#include <cairo.h>
#include <pango/pangocairo.h>

namespace overlay
{

class Text {
public:
  Text();
  Text(const std::string& msg, const int x, const int y,
  const std::string& fontfamily,
  const bool dropshadow, const bool underlay);
  ~Text();

public:
  int width(void)const;
  int height(void)const;

public:
  bool Update(const float dt);
  void Draw(cairo_t* context);

public:
  int X(void)const{return m_xpos;};
  void X(const int x){m_xpos=x;};
  int Y(void)const{return m_ypos;};
  void Y(const int y){m_ypos=y;};

public:
  void LazyInitialization(cairo_t* context);

private:
  void UpdateTextMarkup(void);

private:
  std::string m_msg;
  std::string m_timeFormatText;
  std::string m_displayedText;
  int m_ypos;
  int m_xpos;
  std::string m_fontfamily;
  bool m_dropshadow;
  bool m_underlay;
  bool m_updateTime;//update time display on each render
  PangoLayout *pango_layout;
  PangoFontDescription *pango_fontdesc;
  PangoAttrList* pTextAttributes;
  PangoAttrList* no_color_attributes;
  PangoRectangle ink_rect, logical_rect;
};

}

#endif