
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

#include "window_print_layout_edit.h"
#include <glom/box_db_table.h>
#include <glom/print_layout/canvas_layout_item.h>
#include <glom/utils_ui.h>
#include <libglom/data_structure/layout/layoutitem_line.h>
#include <libglom/data_structure/layout/layoutitem_portal.h>
#include <libglom/utils.h> //For bold_message()).
#include <gtkmm/scrolledwindow.h>
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

const char* Window_PrintLayout_Edit::glade_id("window_print_layout_edit");
const bool Window_PrintLayout_Edit::glade_developer(true);

Window_PrintLayout_Edit::Window_PrintLayout_Edit(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Window(cobject),
  m_entry_name(0),
  m_entry_title(0),
  m_label_table_name(0),
  m_button_close(0),
  m_box(0),
  m_box_item_position(0),
  m_spinbutton_x(0),
  m_spinbutton_y(0),
  m_spinbutton_width(0),
  m_spinbutton_height(0),
  m_ignore_spinbutton_signals(false),
  m_drag_preview_requested(false),
  m_vruler(0),
  m_hruler(0),
  m_context_menu(0)
{
  set_default_size(640, 480);

  add_view(&m_canvas);

  builder->get_widget("vbox_menu", m_box_menu);
  builder->get_widget("vbox_canvas", m_box_canvas);
  builder->get_widget("vbox_inner", m_box);

  builder->get_widget("label_table_name", m_label_table_name);
  builder->get_widget("entry_name", m_entry_name);
  builder->get_widget("entry_title", m_entry_title);

  builder->get_widget("box_item_position", m_box_item_position);
  builder->get_widget("spinbutton_x", m_spinbutton_x);
  m_spinbutton_x->signal_value_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_spinbutton_x));
  builder->get_widget("spinbutton_y", m_spinbutton_y);
  m_spinbutton_y->signal_value_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_spinbutton_y));
  builder->get_widget("spinbutton_width", m_spinbutton_width);
  m_spinbutton_width->signal_value_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_spinbutton_width));
  builder->get_widget("spinbutton_height", m_spinbutton_height);
  m_spinbutton_height->signal_value_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_spinbutton_height));

  //The rulers are not in the glade file because they use an unusual widget 
  //that Glade wouldn't usually know about:
  m_vruler = GIMP_RULER(gimp_ruler_new(GTK_ORIENTATION_VERTICAL));
  gtk_widget_show(GTK_WIDGET(m_vruler));
  m_hruler = GIMP_RULER(gimp_ruler_new(GTK_ORIENTATION_HORIZONTAL));
  gtk_widget_show(GTK_WIDGET(m_hruler));
 
  //Add the ruler widgets to the table at the left and top:
  Gtk::Table* table = 0;
  builder->get_widget("table_canvas", table);
  gtk_table_attach(table->gobj(), GTK_WIDGET(m_vruler), 
    0, 1, 1, 2, GTK_SHRINK, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_table_attach(table->gobj(), GTK_WIDGET(m_hruler), 
    1, 2, 0, 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_SHRINK, 0, 0);

  gimp_ruler_set_unit(m_hruler, GIMP_UNIT_MM);
  gimp_ruler_set_unit(m_vruler, GIMP_UNIT_MM);

  builder->get_widget("handle_box", m_palette_handle_box);


  builder->get_widget("button_close", m_button_close);
  m_button_close->signal_clicked().connect( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_button_close) );

  m_scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  m_scrolled_window.add(m_canvas);
  m_scrolled_window.show_all();
  m_box_canvas->pack_start(m_scrolled_window);
  m_canvas.show();

  //Make the canvas a drag-and-drop destination:
  const Gtk::TargetEntry toolbar_target = Gtk::ToolPalette::get_drag_target_item();
  m_drag_targets.push_back(toolbar_target);

  //Note that we don't use Gtk::DEST_DEFAULT_DEFAULTS because that would prevent our signal handlers from being used:
  m_canvas.drag_dest_set(m_drag_targets, Gtk::DEST_DEFAULT_HIGHLIGHT, Gdk::ACTION_COPY);
  m_canvas.signal_drag_drop().connect(
      sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_drag_drop) );
  m_canvas.signal_drag_motion().connect(
      sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_drag_motion) );
  m_canvas.signal_drag_data_received().connect(
      sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_drag_data_received) );
  m_canvas.signal_drag_leave().connect(
      sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_drag_leave) );

  init_menu();

  m_palette_handle_box->add(m_toolbar);
  m_toolbar.show();

  m_scrolled_window.get_hadjustment()->signal_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_scroll_value_changed) );
  m_scrolled_window.get_hadjustment()->signal_value_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_scroll_value_changed) );
  m_scrolled_window.get_vadjustment()->signal_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_scroll_value_changed) );
  m_scrolled_window.get_vadjustment()->signal_value_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_scroll_value_changed) );

  //Fill composite view:
  //add_view(m_box);

  setup_context_menu();
  m_canvas.signal_show_context().connect(sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_show_context_menu));

  m_canvas.signal_selection_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_selection_changed));
  on_canvas_selection_changed(); //Disable relevant widgets or actions by default.
  
  show_all_children();
}

void Window_PrintLayout_Edit::init_menu()
{
  m_action_group = Gtk::ActionGroup::create();

  m_action_group->add(Gtk::Action::create("Menu_File", _("_File")));
  m_action_group->add(Gtk::Action::create("Action_Menu_File_PageSetup", _("Page _Setup")),
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_file_page_setup));


  m_action_group->add(Gtk::Action::create("Menu_Edit", Gtk::Stock::EDIT));

  m_action_edit_cut = Gtk::Action::create("Action_Menu_Edit_Cut", Gtk::Stock::CUT);
  m_action_group->add(m_action_edit_cut,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_edit_cut) );

  m_action_edit_copy = Gtk::Action::create("Action_Menu_Edit_Copy", Gtk::Stock::COPY);
  m_action_group->add(m_action_edit_copy,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_edit_copy) );
  
  m_action_edit_paste = Gtk::Action::create("Action_Menu_Edit_Paste", Gtk::Stock::PASTE);
  m_action_group->add(m_action_edit_paste,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_edit_paste) );
  m_action_edit_paste->set_sensitive(false); //This is enable when something is copied or cut.

  m_action_edit_delete = Gtk::Action::create("Action_Menu_Edit_Delete", Gtk::Stock::DELETE);
  m_action_group->add(m_action_edit_delete,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_edit_delete) );

  m_action_group->add(
    Gtk::Action::create("Action_Menu_Edit_SelectAll", Gtk::Stock::SELECT_ALL),
    Gtk::AccelKey("<control>A"), //TODO: Suggest this as part of the stock item in GTK+?
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_edit_selectall) );
  m_action_group->add(
    Gtk::Action::create("Action_Menu_Edit_UnselectAll", _("Unselect All")), //TODO: Propose a new stock item for GTK+.
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_edit_unselectall) );

  m_action_group->add(Gtk::Action::create("Menu_Insert", _("_Insert")));
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_Field", _("Insert _Field")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_field) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_Text", _("Insert _Text")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_text) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_Image", _("Insert _Image")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_image) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_RelatedRecords", _("Insert _Related Records")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_relatedrecords) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_LineHorizontal", _("Insert _Horizontal Line")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_line_horizontal) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_LineVertical", _("Insert _Vertical Line")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_line_vertical) );

  m_action_group->add(Gtk::Action::create("Menu_View", _("_View")));
  m_action_showgrid = Gtk::ToggleAction::create("Action_Menu_View_ShowGrid", _("Show Grid"));
  m_action_group->add(m_action_showgrid, sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_show_grid));
  m_action_showrules = Gtk::ToggleAction::create("Action_Menu_View_ShowRules", _("Show Rules"));
  m_action_group->add(m_action_showrules, sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_show_rules));
  m_action_showoutlines = Gtk::ToggleAction::create("Action_Menu_View_ShowOutlines", _("Show Outlines"));
  m_action_group->add(m_action_showoutlines, sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_show_outlines));

  Gtk::RadioAction::Group group_zoom;
  m_action_zoom_fit_page_width = Gtk::RadioAction::create(group_zoom, "Action_Menu_View_ZoomFitPageWidth", _("Fit Page _Width"));
  m_action_group->add(m_action_zoom_fit_page_width,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_fitpagewidth));

  m_action_group->add(Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom200", _("Zoom 200%")),
    sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 200));

  Glib::RefPtr<Gtk::RadioAction> action_zoom100 = 
    Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom100", Gtk::Stock::ZOOM_100);
  m_action_group->add(action_zoom100,
    sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 100));

  Glib::RefPtr<Gtk::Action> action_50 = Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom50", _("Zoom 50%"));
  m_action_group->add(action_50,
    sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 50));

  m_action_group->add(Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom25", _("Zoom 25%")),
    sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 25));

  action_zoom100->activate(); //This seems like a sane default.

  //Build part of the menu structure, to be merged in by using the "PH" placeholders:
  static const Glib::ustring ui_description =
    "<ui>"
    "  <menubar name='Menubar'>"
    "      <menu action='Menu_File'>"
    "        <menuitem action='Action_Menu_File_PageSetup' />"
    "      </menu>"
    "      <menu action='Menu_Edit'>"
    "        <menuitem action='Action_Menu_Edit_Cut' />"
    "        <menuitem action='Action_Menu_Edit_Copy' />"
    "        <menuitem action='Action_Menu_Edit_Paste' />"
    "        <menuitem action='Action_Menu_Edit_Delete' />"
    "        <separator />"
    "        <menuitem action='Action_Menu_Edit_SelectAll' />"
    "        <menuitem action='Action_Menu_Edit_UnselectAll' />"
    "      </menu>"
    "      <menu action='Menu_Insert'>"
    "        <menuitem action='Action_Menu_Insert_Field' />"
    "        <menuitem action='Action_Menu_Insert_Text' />"
    "        <menuitem action='Action_Menu_Insert_Image' />"
    "        <menuitem action='Action_Menu_Insert_RelatedRecords' />"
    "        <menuitem action='Action_Menu_Insert_LineHorizontal' />"
    "        <menuitem action='Action_Menu_Insert_LineVertical' />"
    "      </menu>"
    "      <menu action='Menu_View'>"
    "        <menuitem action='Action_Menu_View_ShowGrid' />"
    "        <menuitem action='Action_Menu_View_ShowRules' />"
    "        <menuitem action='Action_Menu_View_ShowOutlines' />"
    "        <separator />"
    "        <menuitem action='Action_Menu_View_ZoomFitPageWidth' />"
    "        <menuitem action='Action_Menu_View_Zoom200' />"
    "        <menuitem action='Action_Menu_View_Zoom100' />"
    "        <menuitem action='Action_Menu_View_Zoom50' />"
    "        <menuitem action='Action_Menu_View_Zoom25' />"
    "      </menu>"
    "  </menubar>"
    "</ui>";

  //Add menu:
  m_uimanager = Gtk::UIManager::create();
  m_uimanager->insert_action_group(m_action_group);
  m_uimanager->add_ui_from_string(ui_description);

  //Menubar:
  Gtk::MenuBar* pMenuBar = static_cast<Gtk::MenuBar*>(m_uimanager->get_widget("/Menubar"));
  m_box_menu->pack_start(*pMenuBar, Gtk::PACK_SHRINK);
  pMenuBar->show();

  //TODO: Add a toolbar if it would be useful:
  //Gtk::Toolbar* pToolBar = static_cast<Gtk::Toolbar*>(m_uimanager->get_widget("/Bakery_ToolBar"));
  //m_HandleBox_Toolbar.add(*pToolBar);
  //m_HandleBox_Toolbar.show();

  add_accel_group(m_uimanager->get_accel_group());
}

Glib::RefPtr<Gdk::Pixbuf> Window_PrintLayout_Edit::get_icon_for_toolbar_item(Gtk::ToolItem& item)
{
  Glib::RefPtr<Gdk::Pixbuf> result;

  //Set the icon to show when dragging:
  Glib::RefPtr<Gtk::Action> action = item.get_related_action();
  if(!action)
    return result;

  const Gtk::StockID stock_id = action->property_stock_id();
  if(stock_id)
  {
    result = item.render_icon_pixbuf(stock_id, Gtk::ICON_SIZE_LARGE_TOOLBAR);
  }
  else
  {
    const Glib::ustring icon_name = action->property_icon_name();

    Glib::RefPtr<Gdk::Screen> screen = item.get_screen();
    if(!screen)
      return result;

    int width = 0;
    int height = 0;
    if(!Gtk::IconSize::lookup(Gtk::ICON_SIZE_LARGE_TOOLBAR, width, height))
    {
      //An arbitrary default:
      width = height = 24;
    }

    Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_for_screen(screen);
    if(!icon_theme)
      return result;

    result = icon_theme->load_icon(icon_name, MIN(width, height), (Gtk::IconLookupFlags)0);
  }

  return result;
}

/*
void Window_PrintLayout_Edit::make_toolbar_items_draggable()
{
  const int count = m_toolbar->get_n_items();
  for(int i = 0; i < count; ++i)
  {
    Gtk::ToolItem* item = m_toolbar->get_nth_item(i);
    if(!item)
      continue;

    //Allow this widget to be dragged:
    item->set_use_drag_window();
    item->drag_source_set(m_drag_targets, Gdk::BUTTON1_MASK, Gdk::ACTION_COPY);

    //Set the icon to be shown when dragging:
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = get_icon_for_toolbar_item(*item);
    if(pixbuf)
      item->drag_source_set_icon(pixbuf);

    //item->signal_drag_begin().connect(
    //  sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_toolbar_item_drag_begin) );

    //item->signal_drag_end().connect(
    //  sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_toolbar_item_drag_end) );

    //Let the item supply some data when the destination asks for it:
    Glib::RefPtr<Gtk::Action> action = item->get_action();
    item->signal_drag_data_get().connect(
      sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_toolbar_item_drag_data_get), action) );
  }
}
*/

/*
void Window_PrintLayout_Edit::on_toolbar_item_drag_begin(const Glib::RefPtr<Gdk::DragContext>& drag_context)
{
  std::cout << "Window_PrintLayout_Edit::on_toolbar_item_drag_begin" << std::endl;
}

void Window_PrintLayout_Edit::on_toolbar_item_drag_end(const Glib::RefPtr<Gdk::DragContext>& drag_context)
{
  std::cout << "Window_PrintLayout_Edit::on_toolbar_item_drag_end" << std::endl;
}
*/


//TODO: I don't know what this really means. murrayc.
const int DRAG_DATA_FORMAT = 8; // 8 bits format

void Window_PrintLayout_Edit::on_toolbar_item_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& drag_context, Gtk::SelectionData& selection_data, guint /* info */, guint /* time */)
{
  PrintLayoutToolbarButton::enumItems type = PrintLayoutToolbarButton::get_item_type_from_selection_data(drag_context, selection_data);

  selection_data.set(selection_data.get_target(), DRAG_DATA_FORMAT,
          (const guchar*)&type,
          1 /* 1 byte */);
}


bool Window_PrintLayout_Edit::on_canvas_drag_drop(const Glib::RefPtr<Gdk::DragContext>& drag_context, int /* x */, int /* y */, guint timestamp)
{
  const Glib::ustring target = m_canvas.drag_dest_find_target(drag_context);
  if(target.empty())
    return false;

  //Cause our drag_data_received callback to be called:
  //Note that this isn't necessary when using DEST_DEFAULT_DEFAULTS (or DEST_DEFAULT_DROP), 
  //because that would allow us to just return true to make this happen automatically.
  m_canvas.drag_get_data(drag_context, target, timestamp);

  return true; //Allow the drop.
}

bool Window_PrintLayout_Edit::on_canvas_drag_motion(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint timestamp)
{
  const Glib::ustring target = m_canvas.drag_dest_find_target(drag_context);
  if(target.empty())
    return false;

  m_canvas.drag_highlight();

  //Create the temporary canvas item if necesary:
  if(!m_layout_item_dropping)
  {
    //We need to examine the SelectionData:
    //This will cause our drag_data_received callback to be called, with that information.
    //Note: This does not work (and grabs the cursor) if we call dest_set() with the flags for default actions, such as DEST_DEFAULT_DEFAULTS.
    m_drag_preview_requested = true;
    m_canvas.drag_get_data(drag_context, target, timestamp);
    return true;
  }

  drag_context->drag_status(Gdk::ACTION_COPY, timestamp);

  //Move the temporary canvas item to the new position:
  double item_x = x;
  double item_y = y;
  canvas_convert_from_drag_pixels(item_x, item_y);
  m_layout_item_dropping->snap_position(item_x, item_y);

  m_layout_item_dropping->set_xy(item_x, item_y);

  return true; //Allow the drop.
}

sharedptr<LayoutItem> Window_PrintLayout_Edit::create_empty_item(PrintLayoutToolbarButton::enumItems item_type)
{
  sharedptr<LayoutItem> layout_item;

  if(item_type == PrintLayoutToolbarButton::ITEM_FIELD)
  {
    layout_item = sharedptr<LayoutItem_Field>::create();
    layout_item->set_print_layout_position(0, 0, 50, 10);
  }
  else if(item_type == PrintLayoutToolbarButton::ITEM_TEXT)
  {
    sharedptr<LayoutItem_Text> layout_item_derived = sharedptr<LayoutItem_Text>::create();

    // Note to translators: This is the default contents of a text item on a print layout: 
    layout_item_derived->set_text(_("text")); //TODO: Choose some other longer default because this is hidden under the drag icon?
    layout_item = layout_item_derived;
    layout_item->set_print_layout_position(0, 0, 50, 10);
  }
  else if(item_type == PrintLayoutToolbarButton::ITEM_IMAGE)
  {
    layout_item = sharedptr<LayoutItem_Image>::create();
    layout_item->set_print_layout_position(0, 0, 50, 50);
  }
  else if(item_type == PrintLayoutToolbarButton::ITEM_LINE_HORIZONTAL)
  {
    sharedptr<LayoutItem_Line> layout_item_derived = sharedptr<LayoutItem_Line>::create();
    layout_item_derived->set_coordinates(0, 0, 100, 0);
    layout_item = layout_item_derived;
  }
  else if(item_type == PrintLayoutToolbarButton::ITEM_LINE_VERTICAL)
  {
    sharedptr<LayoutItem_Line> layout_item_derived = sharedptr<LayoutItem_Line>::create();
    layout_item_derived->set_coordinates(0, 0, 0, 100);
    layout_item = layout_item_derived;
  }
  else if(item_type == PrintLayoutToolbarButton::ITEM_PORTAL)
  {
    sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::create();
    portal->set_print_layout_row_height(10); //Otherwise it will be 0, which is useless.
    layout_item = portal;
    layout_item->set_print_layout_position(0, 0, 100, 50);
  }
  else
  {
    std::cerr << G_STRFUNC << ": Unhandled item type: " << item_type << std::endl;
  }

  return layout_item;
}

void Window_PrintLayout_Edit::canvas_convert_from_drag_pixels(double& x, double& y) const
{
  //The canvas might be scrolled down in the viewport/scrolledwindow,
  //so deal with that:
  const double scroll_x = m_scrolled_window.get_hadjustment()->get_value();
  const double scroll_y = m_scrolled_window.get_vadjustment()->get_value();
  
  x += scroll_x;
  y += scroll_y;

  m_canvas.convert_from_pixels(x, y);
}


void Window_PrintLayout_Edit::on_canvas_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, const Gtk::SelectionData& selection_data, guint /* info */, guint timestamp)
{
  //This is called when an item is dropped on the canvas,
  //or after our drag_motion handler has called drag_get_data()): 
  
  //Discover what toolbar item was dropped:
  const PrintLayoutToolbarButton::enumItems item_type = PrintLayoutToolbarButton::get_item_type_from_selection_data(drag_context, selection_data);
  
  if(m_drag_preview_requested)
  {
    //Create the temporary drag item if necessary:
    if(!m_layout_item_dropping)
    {
      sharedptr<LayoutItem> layout_item = create_empty_item(item_type);

      //Show it on the canvas, at the position:
      if(layout_item)
      {
        m_layout_item_dropping = CanvasLayoutItem::create(layout_item);
        m_canvas.add_canvas_layout_item(m_layout_item_dropping);

        double item_x = x;
        double item_y = y;
        canvas_convert_from_drag_pixels(item_x, item_y);
        m_layout_item_dropping->snap_position(item_x, item_y);
        m_layout_item_dropping->set_xy(item_x, item_y);
      }
    }

    drag_context->drag_status(Gdk::ACTION_COPY, timestamp);
    m_drag_preview_requested = false;
  }
  else
  {
    //Drop an item:
    drag_context->drag_finish(false, false, timestamp);
    m_canvas.drag_unhighlight();

    //Add the item to the canvas:
    sharedptr<LayoutItem> layout_item = create_empty_item(item_type);
    Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
    m_canvas.add_canvas_layout_item(item);
    double item_x = x;
    double item_y = y;
    canvas_convert_from_drag_pixels(item_x, item_y);
    item->snap_position(item_x, item_y);
    item->set_xy(item_x, item_y);
   
    if(m_layout_item_dropping)
    {
      m_layout_item_dropping->remove();
      m_layout_item_dropping.reset();
    }
  }
}


void Window_PrintLayout_Edit::on_canvas_drag_leave(const Glib::RefPtr<Gdk::DragContext>& /* drag_context */, guint /* timestamp */)
{
  //Remove the temporary drag item if the cursor was dragged out of the drop widget:
  if(m_layout_item_dropping)
  {
    m_layout_item_dropping->remove();
    m_layout_item_dropping.reset();
  }
}


Window_PrintLayout_Edit::~Window_PrintLayout_Edit()
{
  remove_view(&m_canvas);
}

void Window_PrintLayout_Edit::update_table_title()
{
  const Document* document = dynamic_cast<const Document*>(get_document());
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document was null" << std::endl;
    return;
  }

  Glib::ustring table_label = _("None selected");

  //Show the table title (if any) and name:
  Glib::ustring table_title = document->get_table_title(m_table_name);
  if(table_title.empty())
    table_label = m_table_name;
  else
    table_label = table_title + " (" + m_table_name + ')';

  if(m_label_table_name)
  {
    m_label_table_name->set_markup(
      Utils::bold_message(table_label));
  }
}

bool Window_PrintLayout_Edit::init_db_details(const Glib::ustring& table_name)
{
  m_table_name = table_name;
  update_table_title();

  return true;
}


Glib::ustring Window_PrintLayout_Edit::get_original_name() const
{
  return m_name_original;
}

void Window_PrintLayout_Edit::set_print_layout(const Glib::ustring& table_name, const sharedptr<const PrintLayout>& print_layout)
{
  m_modified = false;

  m_name_original = print_layout->get_name();
  m_print_layout = sharedptr<PrintLayout>(new PrintLayout(*print_layout)); //Copy it, so we only use the changes when we want to.
  m_canvas.set_print_layout(table_name, m_print_layout);
  m_table_name = table_name;

  //Dialog_Layout::set_document(layout, document, table_name, table_fields);

  //Set the table name and title:
  update_table_title();

  m_entry_name->set_text(print_layout->get_name()); 
  m_entry_title->set_text(print_layout->get_title());

  set_ruler_sizes();

  m_modified = false;
}



void Window_PrintLayout_Edit::enable_buttons()
{

}

sharedptr<PrintLayout> Window_PrintLayout_Edit::get_print_layout()
{
  m_print_layout = m_canvas.get_print_layout();
  m_print_layout->set_name( m_entry_name->get_text() );
  m_print_layout->set_title( m_entry_title->get_text() );

/*
  m_print_layout->m_layout_group->remove_all_items();

  m_print_layout->m_layout_group->remove_all_items();

  //The Header and Footer parts are implicit (they are the whole header or footer treeview)
  sharedptr<LayoutItem_Header> header = sharedptr<LayoutItem_Header>::create();
  sharedptr<LayoutGroup> group_temp = header;
  fill_print_layout_parts(group_temp, m_model_parts_header);
  if(header->get_items_count())
    m_print_layout->m_layout_group->add_item(header);

  fill_print_layout_parts(m_print_layout->m_layout_group, m_model_parts_main);

  sharedptr<LayoutItem_Footer> footer = sharedptr<LayoutItem_Footer>::create();
  group_temp = footer;
  fill_print_layout_parts(group_temp, m_model_parts_footer);
  if(footer->get_items_count())
    m_print_layout->m_layout_group->add_item(footer);

*/
  return m_print_layout;
}

void Window_PrintLayout_Edit::on_context_menu_insert_field()
{
  on_menu_insert_field();
}

void Window_PrintLayout_Edit::on_context_menu_insert_text()
{
  on_menu_insert_text();
}

void Window_PrintLayout_Edit::setup_context_menu()
{
  m_context_menu_action_group = Gtk::ActionGroup::create();

  m_context_menu_action_group->add(Gtk::Action::create("ContextMenu", "Context Menu") );
  m_context_menu_action_group->add(Gtk::Action::create("ContextMenuInsert", _("Insert")) );

  Glib::RefPtr<Gtk::Action> action =  Gtk::Action::create("ContextInsertField", _("Field"));
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_context_menu_insert_field) );

  action = Gtk::Action::create("ContextInsertText", _("Text"));
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_context_menu_insert_text) );

  /*
  action =  Gtk::Action::create("ContextDelete", Gtk::Stock::DELETE);
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_context_menu_delete) );
  */

  m_context_menu_uimanager = Gtk::UIManager::create();
  m_context_menu_uimanager->insert_action_group(m_context_menu_action_group);

  try
  {
    Glib::ustring ui_info = 
      "<ui>"
      "  <popup name='ContextMenu'>"
      "    <menu action='ContextMenuInsert'>"
      "      <menuitem action='ContextInsertField'/>"
      "      <menuitem action='ContextInsertText'/>"
      "    </menu>"
      "  </popup>"
      "</ui>";

    m_context_menu_uimanager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }

  //Get the menu:
  m_context_menu = dynamic_cast<Gtk::Menu*>( m_context_menu_uimanager->get_widget("/ContextMenu") ); 
}


void Window_PrintLayout_Edit::on_canvas_show_context_menu(guint button, guint32 activate_time)
{
  if(m_context_menu)
    m_context_menu->popup(button, activate_time);
}

bool Window_PrintLayout_Edit::get_is_item_at(double x, double y)
{
  Glib::RefPtr<Goocanvas::Item> item_hit = m_canvas.get_item_at(x, y, false);
  if(!item_hit)
   return false;

  Glib::RefPtr<CanvasLayoutItem> layout_item = Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(item_hit);
  return layout_item;
}

void Window_PrintLayout_Edit::set_default_position(const sharedptr<LayoutItem>& item)
{
  if(!item)
    return;

  double item_x = 10;
  double item_y = 10;
  canvas_convert_from_drag_pixels(item_x, item_y);
  
  //TODO: This doesn't seem to actually work:
  while(get_is_item_at(item_x, item_y))
  {
    item_x += 10;
    item_y += 10;
  }

  //Get the old position so we can preserve the width and height:
  double old_x = 0;
  double old_y = 0;
  double old_width = 0;
  double old_height = 0;
  item->get_print_layout_position(old_x, old_y, old_width, old_height);

  item->set_print_layout_position(item_x, item_y, old_width, old_height);
}

void Window_PrintLayout_Edit::on_menu_insert_field()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_FIELD);

  // Note to translators: This is the default contents of a text item on a print layout: 
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_text()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_TEXT);
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_image()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_IMAGE);
  // Note to translators: This is the default contents of a text item on a print layout: 
  //layout_item->set_text(_("text"));
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_relatedrecords()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_PORTAL);
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_line_horizontal()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_LINE_HORIZONTAL);

  /*
  double item_x = m_drop_x;
  double item_y = m_drop_y;
  m_canvas.convert_from_pixels(item_x, item_y);
  */

  // Note to translators: This is the default contents of a text item on a print layout: 
  //layout_item->set_text(_("text"));
  //layout_item->set_coordinates(item_x, item_y, item_x + 100, item_y);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_line_vertical()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_LINE_VERTICAL);

  /*
  double item_x = m_drop_x;
  double item_y = m_drop_y;
  canvas_convert_from_drag_pixels(item_x, item_y);
  */

  // Note to translators: This is the default contents of a text item on a print layout: 
  //layout_item->set_text(_("text"));
  //layout_item->set_coordinates(item_x, item_y, item_x, item_y + 100);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_button_close()
{
  hide();
}

void Window_PrintLayout_Edit::on_menu_view_show_grid()
{
  if(m_action_showgrid->get_active())
  {
    m_canvas.set_grid_gap(5.0f);
  }
  else
  {
    m_canvas.remove_grid();
  }
}

void Window_PrintLayout_Edit::on_menu_view_show_rules()
{
  //TODO:
}


void Window_PrintLayout_Edit::on_menu_view_show_outlines()
{
  m_canvas.set_outlines_visibility(
    m_action_showoutlines->get_active());
}

void Window_PrintLayout_Edit::on_menu_view_zoom(guint percent)
{
  m_canvas.set_zoom_percent(percent);
}

void Window_PrintLayout_Edit::on_menu_view_fitpagewidth()
{
  //Get the canvas size:
  Goocanvas::Bounds bounds;
  m_canvas.get_bounds(bounds);
  
  double canvas_width_pixels = bounds.get_x2() - bounds.get_x1();
  double canvas_height_pixels = bounds.get_y2() - bounds.get_y1();
  m_canvas.convert_to_pixels(canvas_width_pixels, canvas_height_pixels);
  canvas_width_pixels = canvas_width_pixels / m_canvas.property_scale();

  //Get the viewport size:
  Gtk::Widget* child = m_scrolled_window.get_child();
  if(child)
  {
    Gtk::Allocation widget_allocation = child->get_allocation();
    const double viewport_width = widget_allocation.get_width();
    if(canvas_width_pixels)
    {
      //scale the canvas so it fits perfectly in the viewport:
      const double scale = viewport_width / canvas_width_pixels;
      m_canvas.set_zoom_percent((guint)(scale * 100));
    }
  }
}

void Window_PrintLayout_Edit::on_menu_file_page_setup()
{
  Glib::RefPtr<Gtk::PageSetup> page_setup = m_canvas.get_page_setup();

  //Show the page setup dialog, asking it to start with the existing settings:
  Glib::RefPtr<Gtk::PrintSettings> print_settings = Gtk::PrintSettings::create(); //TODO: Do we really need to get this from the user and store it?
  page_setup = Gtk::run_page_setup_dialog(*this, page_setup, print_settings);

  //Save the chosen page setup dialog for use when printing, previewing, or
  //showing the page setup dialog again:
  m_canvas.set_page_setup(page_setup);

  set_ruler_sizes();
}

void Window_PrintLayout_Edit::on_menu_edit_cut()
{
  on_menu_edit_copy();
  on_menu_edit_delete();
}

void Window_PrintLayout_Edit::on_menu_edit_copy()
{
  if(m_layout_items_selected.empty())
    return;

  m_layout_items_to_paste.clear();

  for(type_vec_canvas_items::iterator iter = m_layout_items_selected.begin();
    iter != m_layout_items_selected.end(); ++iter)
  {
    Glib::RefPtr<CanvasLayoutItem> item = *iter;
    if(item)
      item->update_layout_position_from_canvas();

    sharedptr<LayoutItem> cloned = 
      glom_sharedptr_clone(item->get_layout_item());

    m_layout_items_to_paste.push_back(cloned);
  }

  m_action_edit_paste->set_sensitive();
}

void Window_PrintLayout_Edit::on_menu_edit_paste()
{
  if(m_layout_items_to_paste.empty())
    return;

  for(type_list_items::iterator iter = m_layout_items_to_paste.begin();
    iter != m_layout_items_to_paste.end(); ++iter)
  {
    sharedptr<LayoutItem> item = *iter;
    if(!item)
      continue;

    //TODO: Add x,y offset and add.
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;
    item->get_print_layout_position(
      x, y, width, height);

    const double offset = 5;
    x += offset;
    y += offset;
    item->set_print_layout_position(x, y, width, height);

    Glib::RefPtr<CanvasLayoutItem> canvas_item = 
      CanvasLayoutItem::create(item);
    m_canvas.add_canvas_layout_item(canvas_item);
  }
}

void Window_PrintLayout_Edit::on_menu_edit_delete()
{
  while(!m_layout_items_selected.empty())
  {
    Glib::RefPtr<CanvasLayoutItem> item = m_layout_items_selected[0];
    if(item)
      m_canvas.remove_canvas_layout_item(item);
  }

  m_layout_items_selected.clear();
}

void Window_PrintLayout_Edit::on_menu_edit_selectall()
{
  m_canvas.select_all();
}

void Window_PrintLayout_Edit::on_menu_edit_unselectall()
{
  m_canvas.select_all(false);
}

static void spinbutton_set_max(Gtk::SpinButton& spinbutton, double max)
{
  spinbutton.set_range(0, max);
  spinbutton.set_increments(1, 10);
}

void Window_PrintLayout_Edit::set_ruler_sizes()
{
  //Note: We should use the page size if we decide not to make the canvas bounds == page size.
  on_scroll_value_changed();

  double x1 = 0;
  double y1 = 0;
  double x2 = 0;
  double y2 = 0;
  m_canvas.get_bounds(x1, y1, x2, y2);

  gimp_ruler_set_range(m_hruler, x1, x2, x2);
  gimp_ruler_set_range(m_vruler, y1, y2, x2);

  //Set the limits for the SpinButtons too:
  spinbutton_set_max(*m_spinbutton_x, x2);
  spinbutton_set_max(*m_spinbutton_y, y2);
  spinbutton_set_max(*m_spinbutton_width, x2);
  spinbutton_set_max(*m_spinbutton_height, y2);
}

void Window_PrintLayout_Edit::on_scroll_value_changed()
{
  double width = m_scrolled_window.get_hadjustment()->get_page_size();
  double height = m_scrolled_window.get_vadjustment()->get_page_size();
  double x = m_scrolled_window.get_hadjustment()->get_value();
  double y = m_scrolled_window.get_vadjustment()->get_value();
  
  //This definitely seems to give the correct mm values.
  //(It understands the canvas units and scale):
  m_canvas.convert_from_pixels(width, height); 
  m_canvas.convert_from_pixels(x, y);

  //std::cout << "DEBUG: Calling m_hruler->set_range(" << x << ", " << x + width << ", 0, " <<  x + width << std::endl;

  gimp_ruler_set_range(m_hruler, x, x + width, x + width);
  gimp_ruler_set_range(m_vruler, y, y + height, y + height);
}

bool Window_PrintLayout_Edit::on_configure_event(GdkEventConfigure* event)
{
  const bool result = Gtk::Window::on_configure_event(event);

  //If we are in fit-page-width then rescale the canvas:
  if(m_action_zoom_fit_page_width->get_active())
    on_menu_view_fitpagewidth();

  return result;
}

void Window_PrintLayout_Edit::get_dimensions_of_multiple_selected_items(double& x, double& y, double& width, double& height)
{
 //Get the selected items, and their dimensions as a group:
  x = 0;
  y = 0;
  double x2 = 0;
  double y2 = 0;
  bool first = true;
  for(type_vec_canvas_items::iterator iter = m_layout_items_selected.begin();
    iter != m_layout_items_selected.end(); ++iter)
  {
    Glib::RefPtr<CanvasLayoutItem> item = Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(*iter);
    if(!item)
      continue;

    //Get the position:
    double item_x = 0;
    double item_y = 0;
    item->get_xy(item_x, item_y);
    double item_width = 0;
    double item_height = 0;
    item->get_width_height(item_width, item_height);
    const double item_x2 = item_x + item_width;
    const double item_y2 = item_y + item_height;

    //Store the outermost positions of the group of items:
    if(first || item_x < x)
      x = item_x;

    if(first || item_y < y)
      y = item_y;

    if(item_x2 > x2)
      x2 = item_x2;

    if(item_y2 > y2)
      y2 = item_y2;

    first = false;
  }

  width = x2 - x;
  height = y2 - y;
}

void Window_PrintLayout_Edit::on_canvas_selection_changed()
{
  Canvas_PrintLayout::type_vec_items items = m_canvas.get_selected_items();

  //Forget about any previously selected items:
  m_layout_items_selected.clear();
  for(type_vec_connections::iterator iter = m_connections_items_selected_moved.begin();
    iter != m_connections_items_selected_moved.end(); ++iter)
  {
    iter->disconnect();
  }
  m_connections_items_selected_moved.clear();
  

  for(Canvas_PrintLayout::type_vec_items::const_iterator iter = items.begin();
    iter != items.end(); ++iter)
  {
    Glib::RefPtr<CanvasLayoutItem> item = Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(*iter);
    if(!item)
      continue;

    //Cache the selected items and handle their signal_moved signals:
    m_layout_items_selected.push_back(item);
    m_connections_items_selected_moved.push_back(
      item->signal_moved().connect(
        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_selected_item_moved)));
  }

  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;
  get_dimensions_of_multiple_selected_items(x, y, width, height);

  //Update the SpinButton values,
  //but don't respond to the SpinButton changes that we cause programatically:
  const bool old_ignore = m_ignore_spinbutton_signals;
  m_ignore_spinbutton_signals = true;
  m_spinbutton_x->set_value(x);
  m_spinbutton_y->set_value(y);
  m_spinbutton_width->set_value(width);
  m_spinbutton_height->set_value(height);
  m_ignore_spinbutton_signals = old_ignore;

  //Disable the spinbuttons if there are no items selected,
  //or if there are more than 1.
  //TODO: Let the user resize groups of items.
  const bool one_selected = (m_layout_items_selected.size() == 1);
  const bool some_selected = !m_layout_items_selected.empty();

  //Allow x/y editing via the numbers for multiple items,
  //but not width/height for multiple items (TODO: Stretch them in that case?)
  //and only allow any editing when at least one item is selected:
  m_box_item_position->set_sensitive(some_selected);
  m_spinbutton_x->set_sensitive(some_selected);
  m_spinbutton_y->set_sensitive(some_selected);
  m_spinbutton_width->set_sensitive(one_selected);
  m_spinbutton_height->set_sensitive(one_selected);

  if(m_action_edit_cut)
    m_action_edit_cut->set_sensitive(some_selected);

  if(m_action_edit_copy)
    m_action_edit_copy->set_sensitive(some_selected);

  if(m_action_edit_delete)
    m_action_edit_delete->set_sensitive(some_selected);
}

void Window_PrintLayout_Edit::on_selected_item_moved(const Glib::RefPtr<CanvasItemMovable>& item, double x_offset, double y_offset)
{
  //Move the other selected items too:
  for(type_vec_canvas_items::iterator iter = m_layout_items_selected.begin();
    iter != m_layout_items_selected.end(); ++iter)
  {
    Glib::RefPtr<CanvasLayoutItem> selected_item = *iter;
    if(!selected_item || (item == selected_item))
      continue;

    double x = 0;
    double y = 0;
    selected_item->get_xy(x, y);
    selected_item->set_xy(x + x_offset, y + y_offset);
  }
  
  //Show the new positions in the spinbuttons:
  on_canvas_selection_changed();
}

void Window_PrintLayout_Edit::on_spinbutton_x()
{
  if(m_ignore_spinbutton_signals)
    return;

  if(m_layout_items_selected.empty())
    return;

  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;
  get_dimensions_of_multiple_selected_items(x, y, width, height);

  //Discover the offset:
  const double offset_x = m_spinbutton_x->get_value() - x;

  //Apply the offset to all items:
  for(type_vec_canvas_items::iterator iter = m_layout_items_selected.begin();
    iter != m_layout_items_selected.end(); ++iter)
  {
    Glib::RefPtr<CanvasLayoutItem> item = *iter;
    if(!item)
      continue;

    double item_x = 0;
    double item_y = 0;
    item->get_xy(item_x, item_y);

    item->set_xy(
      item_x + offset_x,
      item_y);
  }
}

void Window_PrintLayout_Edit::on_spinbutton_y()
{
  if(m_ignore_spinbutton_signals)
    return;

  if(m_layout_items_selected.empty())
    return;

   double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;
  get_dimensions_of_multiple_selected_items(x, y, width, height);

  //Discover the offset:
  const double offset_y = m_spinbutton_y->get_value() - y;

  //Apply the offset to all items:
  for(type_vec_canvas_items::iterator iter = m_layout_items_selected.begin();
    iter != m_layout_items_selected.end(); ++iter)
  {
    Glib::RefPtr<CanvasLayoutItem> item = *iter;
    if(!item)
      continue;

    double item_x = 0;
    double item_y = 0;
    item->get_xy(item_x, item_y);

    item->set_xy(
      item_x,
      item_y + offset_y);
  }
}

void Window_PrintLayout_Edit::on_spinbutton_width()
{
  if(m_ignore_spinbutton_signals)
    return;

  if(m_layout_items_selected.empty())
    return;

  Glib::RefPtr<CanvasLayoutItem> item = m_layout_items_selected[0];

  double width = 0;
  double height = 0;
  item->get_width_height(width, height);

  item->set_width_height(
    m_spinbutton_width->get_value(),
    height);
}

void Window_PrintLayout_Edit::on_spinbutton_height()
{
  if(m_ignore_spinbutton_signals)
    return;

  if(m_layout_items_selected.empty())
    return;

  Glib::RefPtr<CanvasLayoutItem> item = m_layout_items_selected[0];

  double width = 0;
  double height = 0;
  item->get_width_height(width, height);

  item->set_width_height(
    width,
    m_spinbutton_height->get_value());
}


} //namespace Glom


