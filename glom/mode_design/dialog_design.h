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

#ifndef GLOM_DIALOG_DESIGN_H
#define GLOM_DIALOG_DESIGN_H

#include <gtkmm.h>
#include <libglademm.h>
#include "../document/document_glom.h"

class Dialog_Design
: public Gtk::Dialog,
  public View_Composite_Glom
{
public:
  Dialog_Design(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_Design();

  virtual void init_db_details(const Glib::ustring& strDatabaseName, const Glib::ustring& strTableName);

protected:
  virtual void on_button_close();

  Gtk::Label* m_label_table;
  Gtk::Label* m_label_frame;
};

#endif //GLOM_DIALOG_DESIGN_H
