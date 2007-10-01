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

#include "dialog_connection.h"
#include "box_db.h" //For Box_DB::connect_to_server().
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_Connection::Dialog_Connection(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  Base_DB(),
  m_entry_host(0),
  m_entry_user(0),
  m_entry_password(0),
  m_label_database(0)
{
  refGlade->get_widget("entry_host", m_entry_host);
  refGlade->get_widget("entry_user", m_entry_user);
  refGlade->get_widget("entry_password", m_entry_password);
  refGlade->get_widget("label_database", m_label_database);

#ifdef ENABLE_MAEMO
  // Make the bold title the window title (which cannot be empty in maemo
  // because it displays <Untitled window> instead). This also helps to
  // make the dialog smaller in height, so we save a bit screen space required
  // by the onscreen keyboard.
  Gtk::Label* title;
  Gtk::Label* note;

  refGlade->get_widget("connection_title", title);
  refGlade->get_widget("connection_note", note);

  set_title(title->get_text());
  title->hide();

  // Without size request, this label enlarges the dialog significantly,
  // and the text is still truncated.
  note->set_size_request(400, -1);
#endif
}

Dialog_Connection::~Dialog_Connection()
{
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED
sharedptr<SharedConnection> Dialog_Connection::connect_to_server_with_connection_settings() const
#else
sharedptr<SharedConnection> Dialog_Connection::connect_to_server_with_connection_settings(std::auto_ptr<ExceptionConnection>& error) const
#endif
{
  //std::cout << "debug: Dialog_Connection::connect_to_server_with_connection_settings()" << std::endl;

  //TODO: Bakery::BusyCursor busy_cursor(get_app_window());

  sharedptr<SharedConnection> result(0);

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
    //Set the connection details in the ConnectionPool singleton.
    //The ConnectionPool will now use these every time it tries to connect.

    const Document_Glom* document = get_document();
    if(document)
    {
      //std::cout << "debug: Dialog_Connection::connect_to_server_with_connection_settings(): m_database_name=" << m_database_name << std::endl;
      connection_pool->set_database(m_database_name);

#ifndef ENABLE_CLIENT_ONLY
      if(document->get_connection_is_self_hosted())
        connection_pool->set_host("localhost");
      else
#endif // !ENABLE_CLIENT_ONLY
        connection_pool->set_host(m_entry_host->get_text());
 
      connection_pool->set_user(m_entry_user->get_text());
      connection_pool->set_password(m_entry_password->get_text());
      //if(document)
      //{
      //  connection_pool->set_database(document->get_connection_database());
      //}
    }

    connection_pool->set_ready_to_connect(); //Box_DB::connect_to_server() will now attempt the connection-> Shared instances of m_Connection will also be usable.

#ifdef GLIBMM_EXCEPTIONS_ENABLED
    result = Box_DB::connect_to_server(const_cast<Dialog_Connection*>(this));
#else
    result = Box_DB::connect_to_server(const_cast<Dialog_Connection*>(this), error);
#endif

    /*
    if(document)
    {
      document->set_connection_server(m_entry_host->get_text());
      document->set_connection_user(m_entry_user->get_text());
    }
    */
  }
  else
     std::cerr << "Dialog_Connection::connect_to_server_with_connection_settings(): ConnectionPool::get_instance() failed." << std::endl;

  return result;
}

void Dialog_Connection::load_from_document()
{
  Document_Glom* document = get_document();
  if(document)
  {
#ifndef ENABLE_CLIENT_ONLY
    //Load server and user:
    if(document->get_connection_is_self_hosted())
    {
       m_entry_host->set_text("(self hosted)");
       m_entry_host->set_sensitive(false);
    }
    else
#endif // !ENABLE_CLIENT_ONLY
    {
      Glib::ustring host = document->get_connection_server();
      if(host.empty())
        host = "localhost";
     
      m_entry_host->set_text(host);
      m_entry_host->set_sensitive(true);
    }

    Glib::ustring user = document->get_connection_user(); //TODO: Offer a drop-down list of users.

    if(user.empty())
    {
      //Default to the UNIX user name, which is often the same as the Postgres user name:
      const char* pchUser = getenv("USER"); 
      if(pchUser)
        user = pchUser;
    }

    m_entry_user->set_text(user);

    //Show the database to be opened, or created.
    //TODO: In future, we can hide this completely.
    set_database_name(document->get_connection_database());
  }
  else
    g_warning("Dialog_Connection::load_from_document(): no document");

}

void Dialog_Connection::set_self_hosted_user_and_password(const Glib::ustring& user, const Glib::ustring& password)
{
  m_entry_user->set_text(user);
  m_entry_password->set_text(password);
}

void Dialog_Connection::set_database_name(const Glib::ustring& name)
{
  m_database_name = name;
  if(m_database_name.empty())
    m_label_database->set_text(_("Not yet created."));
  else
    m_label_database->set_text(name);
}

} //namespace Glom
