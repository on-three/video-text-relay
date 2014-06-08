#include "utilities.hpp"

namespace utilities
{

gboolean filter_colors(PangoAttribute *attribute, gpointer user_data)
{
  return (attribute->klass->type != PANGO_ATTR_BACKGROUND && 
    attribute->klass->type != PANGO_ATTR_FOREGROUND);
}

PangoAttrList* remove_color_attributes(PangoAttrList * const list) {
  PangoAttrList* tmp = pango_attr_list_copy(list);
  PangoAttrList* no_color = pango_attr_list_filter(tmp, filter_colors, NULL);
  pango_attr_list_unref(tmp);
  return no_color;
}

}