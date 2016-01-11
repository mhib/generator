#include <gtk/gtk.h>
#include <stdlib.h>
#include "gen.h"

static GtkWidget *text;
GtkWidget *window;

static void choose_folder(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          (GtkWindow*)window,
                                          GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                          "Cancel", GTK_RESPONSE_CANCEL,
                                          "Open", GTK_RESPONSE_ACCEPT,
                                          NULL);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        gtk_entry_set_text(GTK_ENTRY(text), filename);
        g_free (filename);
    }
    gtk_widget_destroy (dialog);
}

static void generate(GtkWidget *widget, gpointer data) {
    GtkWidget * dialog;
    char * result = generate_site((char*)gtk_entry_get_text(GTK_ENTRY(text)));
    if(!strcmp(result, "Generated.")) {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                        "%s", "Generated");
        gtk_window_set_title(GTK_WINDOW(dialog), "Success");
    } else {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                        "%s", result);
        gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    }
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void
activate(GtkApplication *app,
         gpointer user_data) {
    window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 100);
    gtk_window_set_title(GTK_WINDOW(window), "Generator");
    gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    GtkWidget *box1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box1);
    text = gtk_entry_new();
    gtk_editable_select_region(GTK_EDITABLE(text),
                               0, gtk_entry_get_text_length(GTK_ENTRY(text)));
    gtk_entry_set_placeholder_text((GtkEntry*)text, (const gchar*)"Choose index path");
    gtk_box_pack_start(GTK_BOX(box1), text, TRUE, TRUE, 0);
    GtkWidget *button;
    button = gtk_button_new_with_label("Chose folder");
    g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(choose_folder), NULL);
    gtk_box_pack_start(GTK_BOX(box1), button, TRUE, FALSE, 0);
    GtkWidget *button2;
    button2 = gtk_button_new_with_label("Generate");
    g_signal_connect(G_OBJECT(button2), "clicked",G_CALLBACK(generate), NULL);
    gtk_box_pack_start(GTK_BOX(box1), button2, TRUE, FALSE, 0);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_box_pack_start(GTK_BOX(box1), grid, TRUE, TRUE, 0);
    gtk_widget_show_all(window);
}
int
main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    app = gtk_application_new("org.gtk.example",
                              G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate",
                     G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return (status);
}
