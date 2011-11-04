/* Glom
 *
 * Copyright (C) 2007 Johannes Schmid <johannes.schmid@openismus.com>
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

#include "sidebar.h"
#include <iostream>

namespace Glom
{

SideBar::SideBar()
: m_width(0), m_height(0)
{ 
  set_handle_position(Gtk::POS_TOP);
  set_snap_edge(Gtk::POS_TOP);
  
  m_palette.set_style(Gtk::TOOLBAR_BOTH_HORIZ);
  
  add(m_palette);
  show_all_children();
}

SideBar::~SideBar()
{
}

void SideBar::add_group(Gtk::ToolItemGroup& group)
{
  m_palette.add(group);
}

void SideBar::remove_group(Gtk::ToolItemGroup& group)
{
  m_palette.remove(group);
}

void SideBar::set_drag_source()
{
  // It's important to call this AFTER all groups have been added
  m_palette.set_drag_source(Gtk::TOOL_PALETTE_DRAG_ITEMS);
}

void SideBar::on_child_detached(Gtk::Widget* child)
{
  get_size_request(m_width, m_height);
  child->set_size_request(m_width, m_height);
  set_size_request(0, 0);
}

void SideBar::on_child_attached(Gtk::Widget* /* child */)
{
  set_size_request(m_width, m_height);
}

} // namespace Glom
