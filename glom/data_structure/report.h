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
 
#ifndef GLOM_DATASTRUCTURE_REPORT_H
#define GLOM_DATASTRUCTURE_REPORT_H

#include "layout/report_parts/layoutitem_groupby.h"
#include <glibmm/ustring.h>


class Report
{
public:
  Report();
  Report(const Report& src);
  Report& operator=(const Report& src);

  Glib::ustring get_name() const; //For the predicate, when using std::find_if().

  Glib::ustring get_title_or_name() const;

  Glib::ustring m_name;
  Glib::ustring m_title;

  LayoutGroup m_layout_group;
};

#endif //GLOM_DATASTRUCTURE_REPORT_H



