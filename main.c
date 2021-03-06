/*
 * 
 * Copyright (C) appelblim 2011 <docvanrock@googlemail.com>
 *
 * Various other sources were used for creating this. I do not claim any legal
 * ownership of those various lines I copied from someone. This softwares
 * sole purpose is of educational nature.
 * 
 * flv2mp3 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * flv2mp3 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include "spawn_process.h"
#include "struct.h"


static void destroy( GtkWidget*, gpointer );
static void quality_changed( GtkComboBoxText*, Data* );
static void meta_okay_clicked( GtkButton*, Data* );
static void file_changed( GtkFileChooser*, Data* );

int main (int argc, char *argv[])
{
	GtkWidget *window, *notebook;
	GtkWidget *label1, *label2, *btn_close, *btn_go, *btn_meta_ok, *meta_table, *meta_track_title;
	GtkWidget *meta_track_artist, *meta_track_year, *meta_track_genre, *meta_track_album, *label_title, *label_artist, *label_quality;
	GtkWidget *label_kbs, *meta_vbox, *main_vbox, *main_hbox, *main_hbox2, *combobox_quality, *picker, *label_pwd, *text_view_ffmpeg, *sw, *vpaned;
	GtkWidget *about_vbox, *label3, *about_text;
	GtkWidget *label_output_file, *output_file, *main_hbox_output, *spinner;
	GtkWidget *label_genre, *label_year, *label_album, *label_pwd_meta;
	Data *data;
	GtkTextBuffer *about_buffer;
	GtkTextIter iter;

	data = g_slice_new( Data );

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Flv2Mp3");
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_resizable( GTK_WINDOW(window), FALSE );
	gtk_window_set_icon_from_file( GTK_WINDOW(window), "/usr/share/pixmaps/faces/baseball.png", NULL );

	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);

	notebook = gtk_notebook_new ();
	label1 = gtk_label_new ("Transcode");
	label2 = gtk_label_new ("Metadata");
	label3 = gtk_label_new ("About");
	
	spinner = gtk_spinner_new();
	data->spinner = spinner;
	
	label_output_file = gtk_label_new( "Output:" );
	output_file = gtk_entry_new();
	main_hbox_output = gtk_hbox_new( FALSE, 5 );
	data->output_entry = GTK_ENTRY(output_file);

	combobox_quality = gtk_combo_box_text_new();
	btn_close = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
	btn_go = gtk_button_new_with_label ("Start");
	btn_meta_ok = gtk_button_new_with_label ("Apply");

	meta_table = gtk_table_new (2, 2, TRUE);

	meta_track_title = gtk_entry_new();
	meta_track_artist = gtk_entry_new();
	meta_track_year = gtk_entry_new();
	meta_track_genre = gtk_entry_new();
	meta_track_album = gtk_entry_new();

	label_title = gtk_label_new( "Title:" );
	label_artist = gtk_label_new( "Artist:" );
	label_genre = gtk_label_new( "Genre:" );
	label_year = gtk_label_new( "Year:" );
	label_album = gtk_label_new( "Album:" );
	label_kbs = gtk_label_new( " kb/s" );
	label_quality = gtk_label_new( "Quality: " );
	label_pwd = gtk_label_new( "" );
	label_pwd_meta = gtk_label_new( "" );
	gtk_label_set_selectable( GTK_LABEL(label_pwd_meta), TRUE );
	
	/* Set the ellipsize mode - omit leading characters if the text is too long. */
	gtk_label_set_ellipsize( GTK_LABEL(label_pwd), PANGO_ELLIPSIZE_START );

	data->quality = "256";
	data->label_pwd = GTK_LABEL(label_pwd);
	data->label_pwd_meta = GTK_LABEL(label_pwd_meta);

	/* The file picker. */
	picker = gtk_file_chooser_button_new ("Pick a File", GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER (picker), g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD) );


	text_view_ffmpeg = gtk_text_view_new();
	gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(text_view_ffmpeg), FALSE );
	
	/* Set the line breaking behaviour. */
	gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(text_view_ffmpeg), GTK_WRAP_WORD );

	vpaned = gtk_vpaned_new ();
	gtk_container_set_border_width (GTK_CONTAINER(vpaned), 5);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_paned_add2 (GTK_PANED (vpaned), sw);

	gtk_container_add (GTK_CONTAINER (sw), text_view_ffmpeg);

	data->out = gtk_text_view_get_buffer( GTK_TEXT_VIEW( text_view_ffmpeg ) );
	data->err = gtk_text_view_get_buffer( GTK_TEXT_VIEW( text_view_ffmpeg ) );

	data->parent = window;

	about_text = gtk_text_view_new();
	gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW(about_text), FALSE );
	gtk_text_view_set_justification( GTK_TEXT_VIEW(about_text), GTK_JUSTIFY_CENTER );
	gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(about_text), GTK_WRAP_WORD );
	about_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( about_text ) );
	gtk_text_buffer_get_iter_at_offset (about_buffer, &iter, 0);
	gtk_text_buffer_insert (about_buffer, &iter, "\nFlv2Mp3 - 1.3 \n\n by appelblim \n 2011 \n\n requires any recent FFmpeg version & gstreamer1.x-ugly Plugins \n\n Licence: GPL", -1);

	gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combobox_quality), "128" );
	gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combobox_quality), "192" );
	gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combobox_quality), "256" );  
	gtk_combo_box_text_append_text( GTK_COMBO_BOX_TEXT(combobox_quality), "320" );

	gtk_combo_box_set_active( GTK_COMBO_BOX(combobox_quality), 1 );

	meta_vbox = gtk_vbox_new (FALSE, 5);
	main_hbox = gtk_hbox_new (FALSE, 5);
	main_vbox = gtk_vbox_new (FALSE, 5);
	main_hbox2 = gtk_hbox_new (FALSE, 5);
	about_vbox = gtk_vbox_new (FALSE, 5);

	gtk_box_pack_start( GTK_BOX(main_vbox), vpaned, TRUE, TRUE, 0 );

	/* Pack the table. */
	gtk_table_attach( GTK_TABLE(meta_table), label_title, 0, 1, 0, 1, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), label_artist, 0, 1, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), meta_track_title, 1, 2, 0,1, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), meta_track_artist, 1, 2, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), label_album, 0, 1, 2, 3, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), label_genre, 0, 1, 3, 4, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), label_year, 0, 1, 4, 5, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), meta_track_year, 1, 2, 4, 5, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), meta_track_genre, 1, 2, 3, 4, GTK_EXPAND, GTK_SHRINK, 0, 0 );
	gtk_table_attach( GTK_TABLE(meta_table), meta_track_album, 1, 2, 2, 3, GTK_EXPAND, GTK_SHRINK, 0, 0 );

	/* Set the spacings for the table. */
	gtk_table_set_row_spacings(GTK_TABLE(meta_table), 5);
	gtk_table_set_col_spacings(GTK_TABLE(meta_table), 5);

	g_object_set_data(G_OBJECT(btn_meta_ok), "entry1", meta_track_title);
	g_object_set_data(G_OBJECT(btn_meta_ok), "entry2", meta_track_artist);
	g_object_set_data(G_OBJECT(btn_meta_ok), "entry3", meta_track_album);
	g_object_set_data(G_OBJECT(btn_meta_ok), "entry4", meta_track_genre);
	g_object_set_data(G_OBJECT(btn_meta_ok), "entry5", meta_track_year);

	/* Signal connections. */
	g_signal_connect( G_OBJECT(btn_meta_ok), "clicked", G_CALLBACK(meta_okay_clicked), data );

	g_signal_connect_swapped (G_OBJECT(btn_close), "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)window );
	g_signal_connect( G_OBJECT(btn_go), "clicked", G_CALLBACK(button_clicked), data );

	g_signal_connect( G_OBJECT(combobox_quality), "changed", G_CALLBACK(quality_changed), data );

	g_signal_connect( G_OBJECT(picker), "selection_changed", G_CALLBACK(file_changed), data );



	/* Pack the table and the button into the main_vbox. */
	gtk_box_pack_start( GTK_BOX(main_hbox), btn_close, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_hbox), spinner, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_hbox), btn_go, TRUE, TRUE, 0 );
	
	gtk_box_pack_start( GTK_BOX(main_hbox_output), label_output_file, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_hbox_output), output_file, TRUE, TRUE, 0 );

	gtk_box_pack_start( GTK_BOX(main_hbox2), label_quality, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_hbox2), combobox_quality, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_hbox2), label_kbs, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), picker, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), label_pwd, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), sw, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), main_hbox2, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), main_hbox_output, TRUE, TRUE, 0 );
	gtk_box_pack_start( GTK_BOX(main_vbox), main_hbox, TRUE, TRUE, 0 );

	/* Pack the table and the button into the meta_vbox. */
	gtk_box_pack_start(GTK_BOX(meta_vbox), label_pwd_meta, TRUE, TRUE, 0 );
	gtk_box_pack_start(GTK_BOX(meta_vbox), meta_table, TRUE, TRUE, 0 );
	gtk_box_pack_start(GTK_BOX(meta_vbox), btn_meta_ok, FALSE, FALSE, 0 );

	/* Pack the stuff into the about_vbox. */
	gtk_box_pack_start( GTK_BOX(about_vbox), about_text, TRUE, TRUE, 0 );

	/* Append to pages to the notebook container. */
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), main_vbox, label1);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), meta_vbox, label2);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), about_vbox, label3);

	gtk_container_add (GTK_CONTAINER (window), notebook);
	gtk_widget_show_all(window);
	
	gtk_widget_set_visible(data->spinner, FALSE);


	gtk_main();
	

	g_slice_free( Data, data );

	return 0;
}


static void destroy( GtkWidget *window, gpointer data )
{
	gtk_main_quit();
}


/* Function handling changes in the combo box for the quality. */
static void quality_changed( GtkComboBoxText *combobox_quality, Data* data )
{	
	data->quality = gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT(combobox_quality) );
}

/* Callback function for when the Apply button on the meta page is clicked. */
static void meta_okay_clicked( GtkButton *btn_meta_ok, Data *data )
{	
	GtkEntry *entry1 = g_object_get_data(G_OBJECT(btn_meta_ok), "entry1");
	GtkEntry *entry2 = g_object_get_data(G_OBJECT(btn_meta_ok), "entry2");
	GtkEntry *entry3 = g_object_get_data(G_OBJECT(btn_meta_ok), "entry3");
	GtkEntry *entry4 = g_object_get_data(G_OBJECT(btn_meta_ok), "entry4");
	GtkEntry *entry5 = g_object_get_data(G_OBJECT(btn_meta_ok), "entry5");

	data->title = (gchar*)gtk_entry_get_text(GTK_ENTRY(entry1));
	data->artist = (gchar*)gtk_entry_get_text(GTK_ENTRY(entry2));
	data->album = (gchar*)gtk_entry_get_text(GTK_ENTRY(entry3));
	data->genre = (gchar*)gtk_entry_get_text(GTK_ENTRY(entry4));
	data->year = (gchar*)gtk_entry_get_text(GTK_ENTRY(entry5));
	
	/* If $TITLE and $ARTIST are not void - set the outputfilename to "$PATH/$ARTIST - $TITLE.mp3". */
	if( g_utf8_strlen( data->artist, 1) != 0 && g_utf8_strlen( data->title, 1) != 0 )
	{
		gchar *outputfilename = g_strconcat( data->inputfile_directory, "/", data->artist, " - ", data->title, ".mp3", NULL );
		gtk_entry_set_text( data->output_entry, outputfilename );
	}
}


/* Handle the signal of the file chooser. */
static void file_changed( GtkFileChooser *picker, Data *data )
{
	gchar *file = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(picker) );
	gchar *directory = gtk_file_chooser_get_current_folder( GTK_FILE_CHOOSER(picker) );
	gchar *outputfilename = g_strconcat( file, ".mp3", NULL );
	
	/* Dunno whether the "if" is needed - note: this does not purge the text from
	 * the entry boxes on the meta page, just the saved data ! */
	if( file != data->inputfilename )
	{
	data->title = NULL;
	data->artist = NULL;
	data->album = NULL;
	data->genre = NULL;
	data->year = NULL;
	}
	
	gtk_label_set_text( data->label_pwd, file );
	gtk_label_set_text( data->label_pwd_meta, g_path_get_basename(file) );
	data->inputfilename = file;
	data->inputfile_directory = directory;
	
	/* Set the default output filename. */
	gtk_entry_set_text( data->output_entry, outputfilename );
}
