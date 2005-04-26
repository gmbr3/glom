/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#ifndef GLOM_UTILITY_WIDGETS_TEXTVIEW_GLOM_H
#define GLOM_UTILITY_WIDGETS_TEXTVIEW_GLOM_H

#include <gtkmm.h>
#include "../data_structure/field.h"
#include "layoutwidgetfield.h"
#include <libglademm.h>

class App_Glom;

class TextViewGlom
: public Gtk::ScrolledWindow,
  public LayoutWidgetField
{
public:
  explicit TextViewGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  explicit TextViewGlom(Field::glom_field_type glom_type = Field::TYPE_TEXT);
  virtual ~TextViewGlom();

  void set_glom_type(Field::glom_field_type glom_type);

  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  void set_text(const Glib::ustring& text); //override

  /** Set the text from a Gnome::Gda::Value.
   */
  virtual void set_value(const Gnome::Gda::Value& value);

  virtual Gnome::Gda::Value get_value() const;

protected:
  void init();

  //Overrides of default signal handlers:
  virtual void on_buffer_changed();
  //virtual void on_activate(); //From Gtk::Entry.
  virtual bool on_textview_focus_out_event(GdkEventFocus* event); //From Gtk::Widget
  //virtual void on_insert_text(const Glib::ustring& text, int* position); //From Gtk::Editable

  virtual void check_for_change();

  virtual bool on_button_press_event(GdkEventButton *event); //override

  virtual App_Glom* get_application();

  Glib::ustring m_old_text;
  Field::glom_field_type m_glom_type; //Store the type so we can validate the text accordingly.

  //Gnome::Gda::Value m_value; //The last-stored value. We have this because the displayed value might be unparseable.
  Gtk::TextView m_TextView;
};

#endif //GLOM_UTILITY_WIDGETS_TEXTVIEW_GLOM_H

