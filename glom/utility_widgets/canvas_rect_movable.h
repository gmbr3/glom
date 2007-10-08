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
#include <gdkmm/cursor.h>

namespace Glom
{

class CanvasRectMovable : public Goocanvas::Rect
{
protected:
  CanvasRectMovable();
  virtual ~CanvasRectMovable();

public:
  static Glib::RefPtr<CanvasRectMovable> create();

  void set_drag_cursor(Gdk::CursorType cursor);
  void set_drag_cursor(const Gdk::Cursor& cursor);

  typedef sigc::signal<void> type_signal_moved;
  type_signal_moved signal_moved();

protected:

  void set_cursor(const Gdk::Cursor& cursor);
  void unset_cursor();
  
  bool on_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  bool on_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event);
  bool on_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event);

  bool on_enter_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event);
  bool on_leave_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventCrossing* event);

  bool m_dragging;
  double m_drag_start_cursor_x, m_drag_start_cursor_y;
  double m_drag_start_position_x, m_drag_start_position_y;
  Gdk::Cursor m_drag_cursor;

  type_signal_moved m_signal_moved;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CANVAS_RECT_MOVABLE_H

