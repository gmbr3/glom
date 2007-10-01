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

#include "fieldtypes.h"
#include <iostream> //For debug output
#include <libgda/gda-util.h> // For gda_g_type_to_string
#include <glom/libglom/connectionpool.h>

namespace Glom
{

FieldTypes::FieldTypes(const Glib::RefPtr<Gnome::Gda::Connection>& gda_connection)
{
  // These are documented here:
  // http://www.gnome-db.org/docs/libgda/libgda-provider-class.html#LIBGDA-PROVIDER-GET-SCHEMA
  enum GlomGdaDataModelTypesColumns
  {
    DATAMODEL_FIELDS_COL_NAME = 0,
    DATAMODEL_FIELDS_COL_OWNER = 1,
    DATAMODEL_FIELDS_COL_COMMENTS = 2,
    DATAMODEL_FIELDS_COL_GDATYPE = 3
  };
  
  if(gda_connection && gda_connection->is_opened())
  {
    //Read the Types information, so that we can map the string representation of the type (returned by CONNECTION_SCHEMA_FIELDS) to
    //the Gda::ValueType used by Glib::RefPtr<Gnome::Gda::Column>.
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    Glib::RefPtr<Gnome::Gda::DataModel> data_model_tables = gda_connection->get_schema(Gnome::Gda::CONNECTION_SCHEMA_TYPES);
#else
    std::auto_ptr<Glib::Error> error;
    Glib::RefPtr<Gnome::Gda::DataModel> data_model_tables = gda_connection->get_schema(Gnome::Gda::CONNECTION_SCHEMA_TYPES, error);
    // Ignore error here, we do not process data_model_tables if it is NULL
    // anyway
#endif // GLIBMM_EXCEPTIONS_ENABLED
    if(data_model_tables && (data_model_tables->get_n_columns() == 0))
    {
      std::cerr << "FieldTypes::FieldTypes(): get_schema(Gnome::Gda::CONNECTION_SCHEMA_TYPES) failed." << std::endl;
    }
    else if(data_model_tables)
    {
      int rows = data_model_tables->get_n_rows();
      for(int i = 0; i < rows; ++i)
      {
        Gnome::Gda::Value value_name = data_model_tables->get_value_at(DATAMODEL_FIELDS_COL_NAME, i);

        //Get the types's string representation:
        Glib::ustring schema_type_string;
        if(value_name.get_value_type() == G_TYPE_STRING)
          schema_type_string = value_name.get_string();
    
        if(!schema_type_string.empty())
        {
          Gnome::Gda::Value value_gdatype = data_model_tables->get_value_at(DATAMODEL_FIELDS_COL_GDATYPE, i);
          if(value_gdatype.get_value_type() == G_TYPE_ULONG) // TODO: I think this might change to G_TYPE_GTYPE in a future libgda
          {
            const GType gdatype = static_cast<GType>(value_gdatype.get_ulong());

            //Save it for later:
            m_mapSchemaStringsToGdaTypes[schema_type_string] = gdatype;

            Glib::ustring gdatypestring = gda_g_type_to_string(gdatype); // TODO: What is this actually used for?
            //std::cout << "schema type: " << schema_type_string << " = gdatype " << (guint)gdatype << "(" << gdatypestring << ")" << std::endl;
            
            m_mapGdaTypesToSchemaStrings[gdatype] = schema_type_string; //We save it twice, to just to make searching easier, without using a predicate.
            
            //g_warning("debug: schema type: %s = gdatype %d", schema_type_string.c_str(), gdatype);
          }
        }
          
      }
    }
  }
}

FieldTypes::~FieldTypes()
{
}

GType FieldTypes::get_gdavalue_for_schema_type_string(const Glib::ustring& schema_type_string) const
{
  type_mapSchemaStringsToGdaTypes::const_iterator iterFind = m_mapSchemaStringsToGdaTypes.find(schema_type_string);
  if(iterFind == m_mapSchemaStringsToGdaTypes.end())
    return GDA_TYPE_NULL;
  else
    return iterFind->second;
}

Glib::ustring FieldTypes::get_string_name_for_gdavaluetype(GType field_type) const
{
  type_mapGdaTypesToSchemaStrings::const_iterator iterFind = m_mapGdaTypesToSchemaStrings.find(field_type);
  if(iterFind == m_mapGdaTypesToSchemaStrings.end())
  {
    g_warning("FieldTypes::get_string_name_for_gdavaluetype(): returning unknowntype for field_type=%ld", static_cast<long>(field_type));

    g_warning("  possible types are: ");
    for(type_mapGdaTypesToSchemaStrings::const_iterator iter = m_mapGdaTypesToSchemaStrings.begin(); iter != m_mapGdaTypesToSchemaStrings.end(); ++iter)
    {
      g_warning("    gdatype=%ld, sqltype=%s", static_cast<long>(iter->first), iter->second.c_str());
    }
    
    return "unknowntype";
  }
  else
    return iterFind->second;
}

} //namespace Glom


