// vim:ts=4:sw=4:expandtab
#include "all.h"
#include <getopt.h>

#ifndef __VERSION
#define __VERSION "unknown"
#endif

/* Forward declarations */
static void at_exit_cb(void);
static void parse_args(int argc, char *argv[]);
static void print_usage(void);

Display *display;
xcb_connection_t *conn;
xcb_screen_t *screen;
xcb_xrm_context_t *ctx;

char *res_name;
char *res_class;
int use_int = 0;

void get_xcb(void) {
    if (xcb_xrm_context_new(conn, screen, &ctx) < 0) {
        fprintf(stderr, "Failed to initialize xcb-xrm context.\n");
        return;
    }

    if (xcb_xrm_database_from_resource_manager(ctx) < 0) {
        fprintf(stderr, "Failed to load resource database.\n");
        return;
    }

    xcb_xrm_resource_t *resource;
    if (xcb_xrm_resource_get(ctx, res_name, res_class, &resource) < 0) {
        fprintf(stdout, "xcb-xrm: <nomatch>\n");
        return;
    }

    if (use_int) {
        fprintf(stdout, "xcb-xrm: %d\n", xcb_xrm_resource_value_int(resource));
    } else {
        fprintf(stdout, "xcb-xrm: %s\n", xcb_xrm_resource_value(resource));
    }

    xcb_xrm_resource_free(resource);
}

void get_xlib(void) {
    XrmInitialize();

    char *rm = XResourceManagerString(display);
    XrmDatabase db = XrmGetStringDatabase(rm);

    int res_code;
    char *res_type;
    XrmValue res_value;
    res_code = XrmGetResource(db, res_name, res_class, &res_type, &res_value);

    if (!res_code) {
        fprintf(stdout, "   xlib: <nomatch>\n");
    } else {
        fprintf(stdout, "   xlib: %s\n", (char *)res_value.addr);
    }
}

int main(int argc, char *argv[]) {
    atexit(at_exit_cb);
    parse_args(argc, argv);

    if (res_name == NULL) {
        fprintf(stderr, "Please specify a query name.\n");
        exit(EXIT_FAILURE);
    }

    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Failed to connect to X11 server.\n");
        exit(EXIT_FAILURE);
    }

    conn = XGetXCBConnection(display);

    screen = xcb_aux_get_screen(conn, DefaultScreen(display));
    if (screen == NULL) {
        fprintf(stderr, "Failed to get screen.\n");
        exit(EXIT_FAILURE);
    }

    get_xcb();
    get_xlib();

    exit(EXIT_SUCCESS);
}

static void at_exit_cb(void) {
    FREE(res_name);
    FREE(res_class);

    if (ctx != NULL)
        xcb_xrm_context_free(ctx);

    if (conn != NULL)
        xcb_disconnect(conn);
}

static void parse_args(int argc, char *argv[]) {
    int c,
        opt_index = 0;
    static struct option long_options[] = {
        { "version", no_argument, 0, 'v' },
        { "help", no_argument, 0, 'h' },
        { "name", required_argument, 0, 'n' },
        { "class", required_argument, 0, 'c' },
        { 0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "n:c:ivh", long_options, &opt_index)) != -1) {
        switch (c) {
            case 0:
                /* Example for a long-named option.
                if (strcmp(long_options[opt_index].name, "parameter") == 0) {
                    break;
                }
                */

                print_usage();
                break;
            case 'n':
                res_name = strdup(optarg);
                break;
            case 'c':
                res_class = strdup(optarg);
                break;
            case 'i':
                use_int = 1;
                break;
            case 'v':
                fprintf(stderr, "xrmq version %s\n", __VERSION);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
            default:
                print_usage();
                break;
        }
    }
}

static void print_usage(void) {
    fprintf(stderr, "Usage: xrmq [-v|--version] [-h|--help]");
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}
