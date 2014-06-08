/*
file: utilities.hpp
author: John O'Neil
date: Sunday June 8th 2014
Desc: utilities dealing with GTK, pango, cairo and GStreamer
*/
#ifndef __UTILITIES_HPP__
#define __UTILITIES_HPP__

#include <pango/pangocairo.h>

namespace utilities
{
//filter color attributes out of a pango attribute list
gboolean filter_colors(PangoAttribute *attribute, gpointer user_data);
PangoAttrList* remove_color_attributes(PangoAttrList* const list);
};


#endif