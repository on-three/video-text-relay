#include "text.hpp"
#include "utilities.hpp"
#include <boost/regex.hpp>
#include <sstream>
#include <chrono>
#include <memory>

boost::regex time_markup_regex("\\{\\{(.*?)\\}\\}");
const size_t MAX_TIME_FORMAT_MARKUP = 256;

std::string replace_time_markup(const boost::smatch& match){

  std::string format = match[1];
  std::stringstream now;
 
  auto tp = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>( tp.time_since_epoch() );
  size_t modulo = ms.count() % 1000;
  time_t seconds = std::chrono::duration_cast<std::chrono::seconds>( ms ).count();
 
#if HAS_STD_PUT_TIME
    now << std::put_time(localtime(&seconds), format);
#else
  char buffer[MAX_TIME_FORMAT_MARKUP];
  if(strftime(buffer, MAX_TIME_FORMAT_MARKUP, format.c_str(), localtime(&seconds))) {
      now << buffer;
  }
 
#endif // HAS_STD_PUT_TIME
  return now.str();
}

namespace overlay
{
Text::Text()
  :m_msg("None")
  ,m_displayedText("None")
  ,m_timeFormatText("None")
  ,m_fontfamily("Sans Bold 12")
  ,m_ypos(0)
  ,m_xpos(0)
  ,m_dropshadow(false)
  ,m_underlay(false)
  ,m_updateTime(false)
  ,pango_layout(0)
  ,pango_fontdesc(0)
  ,pTextAttributes(0)
  ,no_color_attributes(0)
  
{

}

Text::Text(const std::string& msg, const int x, const int y,
  const std::string& fontfamily,
  const bool dropshadow, const bool underlay)
  :m_msg(msg)
  ,m_displayedText("None")
  ,m_timeFormatText("None")
  ,m_fontfamily(fontfamily)
  ,m_ypos(y)
  ,m_xpos(x)
  ,m_dropshadow(dropshadow)
  ,m_underlay(underlay)
  ,m_updateTime(false)
  ,pango_layout(0)
  ,pango_fontdesc(0)
  ,pTextAttributes(0)
  ,no_color_attributes(0)
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

bool Text::Update(const float dt) {
  return true;
}

void Text::Draw(cairo_t* context) {

  LazyInitialization(context);

  //reformatting markup and time display is expensive, so we only do so
  //when needed.
  if(m_updateTime) {
    UpdateTextMarkup();
  }

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

  //redoing the time display and markup is expensive, so we record the need
  //to do so here.
  if(boost::regex_search(m_msg, time_markup_regex))
  {
    m_updateTime = true;
  }

  UpdateTextMarkup();
}

void Text::UpdateTextMarkup(void) {
  gchar *text = 0;
  m_timeFormatText = boost::regex_replace(m_msg, time_markup_regex, replace_time_markup);
  
  if(!pango_parse_markup(m_timeFormatText.c_str(),
                    -1,//null terminated text string above
                    0,//no accellerated marker
                    &pTextAttributes,
                    &text,
                    NULL,
                    NULL)) 
  {
    //Failure to parse markup, so displayed text is empty. Reset it to default text
    m_displayedText = m_timeFormatText;
  }else{
    m_displayedText = text;
  }

  no_color_attributes = utilities::remove_color_attributes(pTextAttributes);

  pango_layout_set_text(pango_layout, m_displayedText.c_str(), -1);
  pango_layout_set_attributes (pango_layout, pTextAttributes);
  pango_layout_set_font_description(pango_layout, pango_fontdesc);
  pango_layout_get_pixel_extents(pango_layout, &ink_rect, &logical_rect);
}

}