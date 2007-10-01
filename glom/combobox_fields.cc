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

#include "combobox_fields.h"
#include <gtk/gtkcomboboxentry.h>
#include <glibmm/i18n.h>

namespace Glom
{

ComboBox_Fields::ComboBox_Fields(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::ComboBox(cobject),
  m_renderer_title(0)
{
  m_model = Gtk::TreeStore::create(m_model_columns);

  set_model(m_model);

  //Add name column:
  //m_renderer_name = Gtk::manage(new Gtk::CellRendererText());
  //pack_start(*m_renderer_name);
  //set_cell_data_func(*m_renderer_name, sigc::mem_fun(*this, &ComboBox_Fields::on_cell_data_name));

  //Add title column:
  m_renderer_title = Gtk::manage(new Gtk::CellRendererText());
  pack_start(*m_renderer_title);
  set_cell_data_func(*m_renderer_title, sigc::mem_fun(*this, &ComboBox_Fields::on_cell_data_title));

  set_row_separator_func(sigc::mem_fun(*this, &ComboBox_Fields::on_row_separator));
}

ComboBox_Fields::~ComboBox_Fields()
{

}

sharedptr<Field> ComboBox_Fields::get_selected_field() const
{
  Gtk::TreeModel::iterator iter = get_active();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    return row[m_model_columns.m_field];
  }
  else
    return sharedptr<Field>();
}

Glib::ustring ComboBox_Fields::get_selected_field_name() const
{
  sharedptr<Field> field = get_selected_field();
  return glom_get_sharedptr_name(field);
}

void ComboBox_Fields::set_selected_field(const sharedptr<const Field>& field)
{
  if(field)
    set_selected_field(field->get_name());
  else
    set_selected_field(Glib::ustring());
}

void ComboBox_Fields::set_selected_field(const Glib::ustring& field_name)
{
  //Look for the row with this text, and activate it:
  Glib::RefPtr<Gtk::TreeModel> model = get_model();
  if(model)
  {
    for(Gtk::TreeModel::iterator iter = model->children().begin(); iter != model->children().end(); ++iter)
    {
      Gtk::TreeModel::Row row = *iter;
      sharedptr<Field> field = row[m_model_columns.m_field];
      const Glib::ustring this_name = glom_get_sharedptr_name(field);

      //(An empty name means Select the parent table item.)
      if(this_name == field_name)
      {
        set_active(iter);
        return; //success
      }
    }
  }

  //Not found, so mark it as blank:
  //std::cerr << "ComboBox_Fields::set_selected_field(): field not found in list: " << field_name << std::endl;

  //Avoid calling unset_active() if nothing is selected, because it triggers the changed signal unnecessarily.
  if(get_active()) //If something is active (selected).
    unset_active();
}

void ComboBox_Fields::set_fields(Document_Glom* document, const Glib::ustring parent_table_name)
{
  if(!document)
    return;

  const Document_Glom::type_vecFields fields = document->get_table_fields(parent_table_name);

  m_model->clear();

  //Fill the model:
  for(type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
  {
    Gtk::TreeModel::iterator tree_iter = m_model->append();
    Gtk::TreeModel::Row row = *tree_iter;

    sharedptr<Field> rel = *iter;
    row[m_model_columns.m_field] = rel;
    row[m_model_columns.m_separator] = false;
  }
}

void ComboBox_Fields::set_fields(const type_vecFields& fields, bool with_none_item)
{
  m_model->clear();

  if(with_none_item)
  {
    //Add a special "None" item, so the user can clear the GtkComboBox:
    Gtk::TreeModel::iterator tree_iter = m_model->append();
    Gtk::TreeModel::Row row = *tree_iter;

    row[m_model_columns.m_field] = sharedptr<Field>(); 
    row[m_model_columns.m_separator] = false;

    //Add a separator after the "None" item:
    tree_iter = m_model->append();
    row = *tree_iter;

    row[m_model_columns.m_field] = sharedptr<Field>(); 
    row[m_model_columns.m_separator] = true;
  }

  //Fill the model:
  for(type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
  {
    Gtk::TreeModel::iterator tree_iter = m_model->append();
    Gtk::TreeModel::Row row = *tree_iter;

    row[m_model_columns.m_field] = *iter;
    row[m_model_columns.m_separator] = false;
  }
}

void ComboBox_Fields::on_cell_data_title(const Gtk::TreeModel::const_iterator& iter)
{
  Gtk::TreeModel::Row row = *iter;
  sharedptr<Field> field = row[m_model_columns.m_field];
  if(field)
  {
    m_renderer_title->set_property("text", field->get_title_or_name());
    //m_renderer_title->property_text() = field->get_title_or_name();
  }
  else
  {
    // A special "None" item, allowing the user to do the equivalent of clearing the combobox,
    // which is not normally possible with the GtkComboBox UI:
    m_renderer_title->set_property("text", _("(None)"));
    //m_renderer_title->property_text() = _("(None)");
  }
}

bool ComboBox_Fields::on_row_separator(const Glib::RefPtr<Gtk::TreeModel>& /* model */, const Gtk::TreeModel::const_iterator& iter)
{
  Gtk::TreeModel::Row row = *iter;
  const bool separator = row[m_model_columns.m_separator];
  return separator;
}


} //namespace Glom




