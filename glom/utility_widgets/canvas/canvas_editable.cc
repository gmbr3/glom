/* Glom
 *
 * Copyright (C) 2007 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "canvas_editable.h"
#include "canvas_group_resizable.h"
#include "canvas_rect_movable.h"
#include <math.h>
#include <iostream>

namespace Glom
{

CanvasEditable::CanvasEditable()
: m_dragging(false),
  m_drag_x(0.0), m_drag_y(0.0)
{
}

CanvasEditable::~CanvasEditable()
{
}

void CanvasEditable::add_item(const Glib::RefPtr<Goocanvas::Item>& item, bool resizable)
{
  Glib::RefPtr<Goocanvas::Rect> rect = Glib::RefPtr<Goocanvas::Rect>::cast_dynamic(item);
  if(rect)
    add_item_rect(rect, resizable);
  else
  {
    Glib::RefPtr<Goocanvas::Path> path = Glib::RefPtr<Goocanvas::Path>::cast_dynamic(item);
    if(rect)
      add_item_line(path, resizable);
    else
    {
      Glib::RefPtr<Goocanvas::Item> root = get_root_item();

      Glib::RefPtr<Goocanvas::Group> root_group = Glib::RefPtr<Goocanvas::Group>::cast_dynamic(root);
      if(root_group)
      {
        root_group->add_child(item);
      }
    }
  }
}

void CanvasEditable::add_item_group(const Glib::RefPtr<Goocanvas::Group>& item)
{
  if(!item)
    return;

  Glib::RefPtr<Goocanvas::Item> root = get_root_item();

  Glib::RefPtr<Goocanvas::Group> root_group = Glib::RefPtr<Goocanvas::Group>::cast_dynamic(root);
  if(root_group)
  {
    root_group->add_child(item);
  }
}

void CanvasEditable::add_item_line(const Glib::RefPtr<Goocanvas::Path>& item, bool resizable)
{
  if(!item)
    return;

  Glib::RefPtr<Goocanvas::Item> root = get_root_item();

  Glib::RefPtr<Goocanvas::Group> root_group = Glib::RefPtr<Goocanvas::Group>::cast_dynamic(root);
  if(root_group)
  {
    root_group->add_child(item);
  }
}

void CanvasEditable::add_item_rect(const Glib::RefPtr<Goocanvas::Rect>& item, bool resizable)
{
  if(!item)
    return;

  ItemInfo info;
  info.m_resizable = resizable;

  Glib::RefPtr<Goocanvas::Item> root = get_root_item();

  Glib::RefPtr<Goocanvas::Group> root_group = Glib::RefPtr<Goocanvas::Group>::cast_dynamic(root);
  if(root_group)
  {
    if(resizable)
    {
      Glib::RefPtr<CanvasGroupResizable> resizable = CanvasGroupResizable::create();
      resizable->set_grid(&m_grid);

      root_group->add_child(resizable); //We must do this before calling set_child(), so that set_child() can discover the bounds.
      resizable->set_child(item); //Puts draggable corners and edges around it. 
    }
    else
      root_group->add_child(item);
  }

  //item->signal_motion_notify_event().connect(sigc::mem_fun(*this, &CanvasEditable::on_item_motion_notify_event));
  //item->signal_button_press_event().connect(sigc::mem_fun(*this, &CanvasEditable::on_item_button_press_event));
  //item->signal_button_release_event().connect(sigc::mem_fun(*this, &CanvasEditable::on_item_button_release_event));

  m_map_item_info[item] = info;
}

void CanvasEditable::remove_all_items()
{
  Glib::RefPtr<Goocanvas::Item> root = get_root_item();
  while(root && root->get_n_children())
      root->remove_child(0);
}

//static:
Glib::RefPtr<Goocanvas::Item> CanvasEditable::get_parent_container_or_self(const Glib::RefPtr<Goocanvas::Item>& item)
{
  return item;

  Glib::RefPtr<Goocanvas::Item> result = item; 
  while(result && !result->is_container())
    result = result->get_parent();

  return result;
}

bool CanvasEditable::on_item_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  printf("%s: button=%d\n", __FUNCTION__, event->button);
  switch(event->button)
  {
    case 3:
    {
      //This method is virtual:
      on_show_context_menu(event->button, event->time);
      break;
    }

    default:
      break;
  }
  
  return true;
}

void CanvasEditable::on_show_context_menu(guint button, guint32 activate_time)
{
  printf("%s\n", __FUNCTION__);
}

Glib::RefPtr<Goocanvas::Polyline> CanvasEditable::create_grid_or_rule_line(double x1, double y1, double x2, double y2, bool is_rule)
{
  Glib::RefPtr<Goocanvas::Polyline> line = 
      Glib::wrap((GooCanvasPolyline*)goo_canvas_polyline_new_line(NULL, x1, y1, x2, y2, NULL));
  line->property_line_width() = 1.0f;

  if(is_rule)
    line->property_stroke_color() = "green";
  else
    line->property_stroke_color() = "gray";

  return line;
}

void CanvasEditable::add_vertical_rule(double x)
{
  m_grid.m_rules_x.push_back(x);
}

void CanvasEditable::add_horizontal_rule(double y)
{
  m_grid.m_rules_y.push_back(y);
}

void CanvasEditable::set_grid_gap(double gap)
{
  m_grid.m_grid_gap = gap;

  m_grid_lines_group = Goocanvas::Group::create();
  double left, top, right, bottom = 0;
  get_bounds(left, top, right, bottom);
  const double width = right - left;
  const double height = bottom - top;

  const double count_vertical_grid_lines = width / m_grid.m_grid_gap;
  const double count_horizontal_grid_lines = height / m_grid.m_grid_gap;
  
  //Vertical grid lines:
  for(double i = 0; i < count_vertical_grid_lines; ++i)
  {
    const double x = i * m_grid.m_grid_gap;
   
    Glib::RefPtr<Goocanvas::Polyline> line = create_grid_or_rule_line(x, top, x, bottom);
    m_grid_lines_group->add_child(line);
  }

  //Horizontal grid lines:
  for(double i = 0; i < count_horizontal_grid_lines; ++i)
  {
    const double y = i * m_grid.m_grid_gap;

    Glib::RefPtr<Goocanvas::Polyline> line = create_grid_or_rule_line(left, y, right, y);
    m_grid_lines_group->add_child(line);
  }

  add_item_group(m_grid_lines_group);

  //Vertical rules:
  for(CanvasGrid::type_vec_double::const_iterator iter = m_grid.m_rules_x.begin(); iter != m_grid.m_rules_x.end(); ++iter)
  {
    const double x = *iter;
    Glib::RefPtr<Goocanvas::Polyline> line = create_grid_or_rule_line(x, top, x, bottom, true);
    m_grid_lines_group->add_child(line);
  }

  //Horizontal rules:
  for(CanvasGrid::type_vec_double::const_iterator iter = m_grid.m_rules_y.begin(); iter != m_grid.m_rules_y.end(); ++iter)
  {
    const double y = *iter;
    Glib::RefPtr<Goocanvas::Polyline> line = create_grid_or_rule_line(left, y, right, y, true);
    m_grid_lines_group->add_child(line);
  }
}

void CanvasEditable::remove_grid()
{
  m_grid.m_grid_gap = 0.0;
}

} //namespace Glom

