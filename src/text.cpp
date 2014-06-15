#include "text.hpp"
#include "utilities.hpp"
#include <iostream>
using std::cout;
using std::endl;

namespace overlay
{
Text::Text()
  :m_msg("None")
  ,m_fontfamily("Sans Bold 12")
  ,m_ypos(0)
  ,m_xpos(0)
  ,m_dropshadow(false)
  ,m_underlay(false)
  ,pango_layout(0)
  ,pango_fontdesc(0)
  ,pTextAttributes(0)
  ,no_color_attributes(0)
  ,displayed_text("None")
{

}

Text::Text(const std::string& msg, const int x, const int y,
  const std::string& fontfamily,
  const bool dropshadow, const bool underlay)
  :m_msg(msg)
  ,m_fontfamily(fontfamily)
  ,m_ypos(y)
  ,m_xpos(x)
  ,m_dropshadow(dropshadow)
  ,m_underlay(underlay)
  ,pango_layout(0)
  ,pango_fontdesc(0)
  ,pTextAttributes(0)
  ,no_color_attributes(0)
  ,displayed_text(msg)
{

}

Text::~Text() {
  //cleanup messy as fuck C api
  if(pTextAttributes) {
    pango_attr_list_unref(pTextAttributes);
  }
  if(no_color_attributes) {
  pango_attr_list_unref(no_color_attributes);
  }
  if(pango_fontdesc) {
    pango_font_description_free(pango_fontdesc);
  }
  if(pango_layout) {
    g_object_unref(pango_layout);
  }
}

int Text::width(void)const {
  if(!pango_layout) {
    return -1;
  }
  return logical_rect.width;
}
int Text::height(void)const {
  if(!pango_layout) {
    return -1;
  }
  return logical_rect.height;
}

//void Resize(const int width, const int height);
bool Text::Update(const float dt) {
  return true;
}

void Text::Draw(cairo_t* context) {

  LazyInitialization(context);

  cairo_save(context);

  //possibly draw a transparent text underlay of fixed color+alpha
  if(m_underlay) {
    cairo_set_source_rgba(context, 0.0, 0.0, 0.0, 0.4);
    cairo_rectangle(context, m_xpos, m_ypos, logical_rect.width, logical_rect.height);
    cairo_fill(context);
  }

  //possibly draw simple dropshadow to make text readable
  if(m_dropshadow) {
    pango_layout_set_attributes(pango_layout, no_color_attributes);
    cairo_set_source_rgb (context, 0.0, 0.0, 0.0);
    cairo_move_to(context, m_xpos+2, m_ypos+2);
    pango_cairo_update_layout(context, pango_layout);
    pango_cairo_show_layout(context, pango_layout);
  }

  pango_layout_set_attributes (pango_layout, pTextAttributes);
  cairo_set_source_rgb (context, 1.0, 1.0, 1.0);
  cairo_move_to(context, m_xpos, m_ypos);
  pango_cairo_update_layout(context, pango_layout);
  pango_cairo_show_layout(context, pango_layout);

  cairo_restore(context);
}

void Text::LazyInitialization(cairo_t* context) {
  if(pango_layout || pango_fontdesc || pTextAttributes || no_color_attributes) {
    return;
  }
  pango_layout = pango_cairo_create_layout(context);
  pango_fontdesc = pango_font_description_from_string(m_fontfamily.c_str());
  pTextAttributes = pango_attr_list_new();
  gchar *text = 0;
  if(!pango_parse_markup(m_msg.c_str(),
                    -1,//null terminated text string above
                    0,//no accellerated marker
                    &pTextAttributes,
                    &text,
                    NULL,
                    NULL)) 
  {
    //Failure to parse markup, so displayed text is empty. Reset it to default text
    displayed_text = m_msg;
  }else{
    displayed_text = text;
  }
  no_color_attributes = utilities::remove_color_attributes(pTextAttributes);

  pango_layout_set_text(pango_layout, displayed_text.c_str(), -1);
  pango_layout_set_attributes (pango_layout, pTextAttributes);
  pango_layout_set_font_description(pango_layout, pango_fontdesc);
  pango_layout_get_pixel_extents(pango_layout, &ink_rect, &logical_rect);
}



}