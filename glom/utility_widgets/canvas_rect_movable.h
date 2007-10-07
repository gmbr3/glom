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

#ifndef GLOM_UTILITY_WIDGETS_CANVAS_RECT_MOVABLE_H
#define GLOM_UTILITY_WIDGETS_CANVAS_RECT_MOVABLE_H

#include <libgoocanvasmm/rect.h>

namespace Glom
{

class CanvasRectMovable : public Goocanvas::Rect
{
protected:
  CanvasRectMovable();
  virtual ~CanvasRectMovable();

public:
  static Glib::RefPtr<CanvasRectMovable> create();

  typedef sigc::signal<void> type_signal_moved;
  type_signal_moved signal_moved();

protected:
  
  bool on_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  bool on_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  bool on_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event);

  bool m_dragging;
  double m_drag_x, m_drag_y;

  type_signal_moved m_signal_moved;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_RECT_MOVABLE_H

