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

#include "notebook_find.h"
#include <libintl.h>

Notebook_Find::Notebook_Find()
{

  pages().push_back(Gtk::Notebook_Helpers::TabElem(m_Box_List, gettext("List")));
  pages().push_back(Gtk::Notebook_Helpers::TabElem(m_Box_Details, gettext("Details")));

  set_current_page(1); //Show the details page by default. It's more obvious for a Find.
  //TODO: Show the same layout that is being edited at the time that the mode was changed.

  //Connect Signals:
  m_Box_List.signal_find.connect(sigc::mem_fun(*this, &Notebook_Find::on_page_find));
  m_Box_Details.signal_find.connect(sigc::mem_fun(*this, &Notebook_Find::on_page_find));

  //Fill composite view:
  add_view(&m_Box_List);
  add_view(&m_Box_Details);
  
  show_all_children();
}

Notebook_Find::~Notebook_Find()
{
}

void Notebook_Find::init_db_details(const Glib::ustring& strTableName)
{
  m_Box_List.init_db_details(strTableName);

  Gnome::Gda::Value primary_key_value; //It's ignored anyway.
  m_Box_Details.init_db_details(strTableName, primary_key_value);
}

void Notebook_Find::on_page_find(Glib::ustring strWhereClause)
{
  signal_find(strWhereClause);
}
