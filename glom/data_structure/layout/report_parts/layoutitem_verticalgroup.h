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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_VERTICALGROUP_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_VERTICALGROUP_H

#include "../layoutgroup.h"
#include "../../field.h"

/** The child items are arranged vertically in a row on a report.
 */
class LayoutItem_VerticalGroup
: public LayoutGroup
{
public:

  LayoutItem_VerticalGroup();
  LayoutItem_VerticalGroup(const LayoutItem_VerticalGroup& src);
  LayoutItem_VerticalGroup& operator=(const LayoutItem_VerticalGroup& src);
  virtual ~LayoutItem_VerticalGroup();

  virtual LayoutItem* clone() const;

  virtual Glib::ustring get_part_type_name() const;

protected:

};

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_VERTICALGROUP_H



