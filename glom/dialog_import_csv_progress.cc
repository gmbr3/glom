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

#include "dialog_import_csv_progress.h"
#include <libglom/libglom_config.h>

#include <libglom/data_structure/glomconversions.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_Import_CSV_Progress::Dialog_Import_CSV_Progress(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject), m_data_source(NULL), m_current_row(0)
{
  builder->get_widget("import_csv_progress_progress_bar", m_progress_bar);
  builder->get_widget("import_csv_progress_textview", m_text_view);

  if(!m_progress_bar || !m_text_view)
    throw std::runtime_error("Missing widgets from glade file for Dialog_Import_CSV_Progress");
}

bool Dialog_Import_CSV_Progress::init_db_details(const Glib::ustring& table_name)
{
  const bool result = Base_DB_Table_Data::init_db_details(table_name);
  m_field_primary_key = get_field_primary_key_for_table(table_name);
  return result;
}

void Dialog_Import_CSV_Progress::import(Dialog_Import_CSV& data_source)
{
  // Cancel any running operations
  clear();

  // Begin with first row from source
  m_data_source = &data_source;
  m_current_row = 0;

  switch(data_source.get_state())
  {
  case Dialog_Import_CSV::PARSING:
    // Wait for the parsing to finish. We do not start importing before the file has been
    // parsed completely since we would not be able to roll back our changes in case of a
    // parsing error.
    m_progress_bar->set_text(Glib::ustring::compose(_("Parsing CSV file %1"), data_source.get_filename()));
    m_ready_connection = data_source.signal_state_changed().connect(sigc::mem_fun(*this, &Dialog_Import_CSV_Progress::on_state_changed));
    break;
  case Dialog_Import_CSV::PARSED:
    begin_import();
    break;
  default:
    // This function should not be called with data_source being in other states
    g_assert_not_reached();
    break;
  }

  set_response_sensitive(Gtk::RESPONSE_CANCEL, true);
  set_response_sensitive(Gtk::RESPONSE_OK, false);
}

void Dialog_Import_CSV_Progress::clear()
{
  // Cancel any running import
  m_progress_connection.disconnect();
  m_ready_connection.disconnect();

  m_data_source = NULL;
  m_current_row = 0;
}

void Dialog_Import_CSV_Progress::add_text(const Glib::ustring& text)
{
  m_text_view->get_buffer()->insert(m_text_view->get_buffer()->end(), text);
  m_text_view->scroll_to(m_text_view->get_buffer()->get_insert());
}

void Dialog_Import_CSV_Progress::begin_import()
{
  //Note to translators: This is a progress indication, showing how many rows have been imported, of the total number of rows.
  m_progress_bar->set_text(Glib::ustring::compose(_("%1 / %2"), m_current_row, m_data_source->get_row_count()));
  m_progress_bar->set_fraction(0.0);

  m_progress_connection = Glib::signal_idle().connect(sigc::mem_fun(*this, &Dialog_Import_CSV_Progress::on_idle_import));
}

void Dialog_Import_CSV_Progress::on_state_changed()
{
  switch(m_data_source->get_state())
  {
  case Dialog_Import_CSV::ENCODING_ERROR:
    // Cancel on error
    response(Gtk::RESPONSE_CANCEL);
    break;
  case Dialog_Import_CSV::PARSED:
    // Begin importing when fully parsed
    begin_import();
    //m_progress_connection = Glib::signal_idle().connect(sigc::mem_fun(*this, &Dialog_Import_CSV_Progress::on_idle_import));
    break;
  default:
    // We only install the signal in state PARSING, and we should not change
    // back to NONE state, so we must have changed to one of the states handled
    // above, otherwise something went wrong.
    g_assert_not_reached();
    break;
  }
}

bool Dialog_Import_CSV_Progress::on_idle_import()
{
  //Note to translators: This is a progress indication, showing how many rows have been imported, of the total number of rows.
  m_progress_bar->set_text(Glib::ustring::compose(_("%1 / %2"), m_current_row, m_data_source->get_row_count()));
  m_progress_bar->set_fraction(static_cast<double>(m_current_row) / static_cast<double>(m_data_source->get_row_count()));

  if(m_current_row == m_data_source->get_row_count())
  {
    // Don't do the response immediately, so the user has a chance to read the
    // warnings and errors, if any.
    set_response_sensitive(Gtk::RESPONSE_CANCEL, false);
    set_response_sensitive(Gtk::RESPONSE_OK, true);
    add_text(_("Import complete\n"));
    return false;
  }

  // Update the current row values map:
  for(unsigned int i = 0; i < m_data_source->get_column_count(); ++ i)
  {
    const sharedptr<Field>& field = m_data_source->get_field_for_column(i);
    if(field)
    {
      // We always assume exported data is in postgres format, since
      // we export it this way.
      // TODO: Document what that format is.
      bool success = false;
      Gnome::Gda::Value value = field->from_file_format(m_data_source->get_data(m_current_row, i), success);

      if(success)
      {
        // Make the value empty if the value is not unique.
        if(field->get_unique_key())
        {
          sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
          layout_item->set_full_field_details(field);
          if(!get_field_value_is_unique(m_table_name, layout_item, value))
          {
            value = Gnome::Gda::Value();

            const Glib::ustring message(Glib::ustring::compose(_("Warning: Importing row %1: The value for field %2 must be unique, but is already in use. The value will not be imported.\n"), m_current_row + 1, field->get_name()));
            add_text(message);
          }
        }

        m_current_row_values[field->get_name()] = value;
      }
      else
      {
        const Glib::ustring message(Glib::ustring::compose(_("Warning: Importing row %1: The value for field %2, \"%3\" could not be converted to the field's type. The value will not be imported.\n"), m_current_row + 1, field->get_name(), m_data_source->get_data(m_current_row, i)));
        add_text(message);
      }
    }
  }

  // Choose the primary key value
  Gnome::Gda::Value primary_key_value;
  if(m_field_primary_key->get_auto_increment())
  {
    primary_key_value = get_next_auto_increment_value(m_table_name, m_field_primary_key->get_name());
  }
  else
  {
    // No auto-increment primary key: Check for uniqueness
    Gnome::Gda::Value primary_key_value = m_current_row_values[m_field_primary_key->get_name()];
    sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
    layout_item->set_full_field_details(m_field_primary_key);

    if(!get_field_value_is_unique(m_table_name, layout_item, primary_key_value))
      primary_key_value = Gnome::Gda::Value();
  }
  
  if(Glom::Conversions::value_is_empty(primary_key_value))
  {
    const Glib::ustring message(Glib::ustring::compose(_("Error importing row %1: Cannot import the row because the primary key is empty.\n"), m_current_row + 1));
    add_text(message);
  }
  else
  {
    std::cout << "Dialog_Import_CSV_Progress::on_idle_import(): Calling record_new() with primary_key_value=" << primary_key_value.to_string() << " ..." << std::endl;
    record_new(m_table_name, true /* use_entered_data */, primary_key_value);
    std::cout << "Dialog_Import_CSV_Progress::on_idle_import(): ... Finished calling record_new()" << std::endl;
  }

  m_current_row_values.clear();

  ++ m_current_row;
  return true;
}

void Dialog_Import_CSV_Progress::on_response(int /* response_id */)
{
  // Don't continue importing when the user already cancelled, or closed the
  // window via delete event.
  clear();
}

Gnome::Gda::Value Dialog_Import_CSV_Progress::get_entered_field_data(const sharedptr<const LayoutItem_Field>& field) const
{
  type_mapValues::const_iterator iter = m_current_row_values.find(field->get_name());
  if(iter == m_current_row_values.end())
    return Gnome::Gda::Value();

  return iter->second;
}

void Dialog_Import_CSV_Progress::set_entered_field_data(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value&  value)
{
  m_current_row_values[field->get_name()] = value;
}

sharedptr<Field> Dialog_Import_CSV_Progress::get_field_primary_key() const
{
  return m_field_primary_key;
}

Gnome::Gda::Value Dialog_Import_CSV_Progress::get_primary_key_value_selected() const
{
  type_mapValues::const_iterator iter = m_current_row_values.find(m_field_primary_key->get_name());
  if(iter == m_current_row_values.end())
    return Gnome::Gda::Value();

  return iter->second;
}

// These don't make sense in Dialog_Import_CSV_Progress, and thus should not
// be called. We need to implement them though, because they are pure abstract
// in Base_DB_Table_Data.
void Dialog_Import_CSV_Progress::set_primary_key_value(const Gtk::TreeModel::iterator& /* row */, const Gnome::Gda::Value& /* value */)
{
  // This is actually called by Base_DB_Table_Data::record_new(), but we can safely ignore it.
  //throw std::logic_error("Dialog_Import_CSV_Progress::set_primary_key_value() called");
}

Gnome::Gda::Value Dialog_Import_CSV_Progress::get_primary_key_value(const Gtk::TreeModel::iterator& /* row */) const
{
  throw std::logic_error("Dialog_Import_CSV_Progress::get_primary_key_value() called");
}

} //namespace Glom
