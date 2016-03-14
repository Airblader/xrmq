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

xcb_connection_t *conn;
xcb_screen_t *screen;
xcb_xrm_context_t *ctx;

char *res_name;
char *res_class;

int main(int argc, char *argv[]) {
    atexit(at_exit_cb);
    parse_args(argc, argv);

    if (res_name == NULL) {
        fprintf(stderr, "Please specify a query name.\n");
        exit(EXIT_FAILURE);
    }

    int screennr;
    conn = xcb_connect(NULL, &screennr);
    if (conn == NULL || xcb_connection_has_error(conn)) {
        fprintf(stderr, "Failed to connect to X11 server.\n");
        exit(EXIT_FAILURE);
    }

    screen = xcb_aux_get_screen(conn, screennr);
    if (screen == NULL) {
        fprintf(stderr, "Failed to get screen.\n");
        exit(EXIT_FAILURE);
    }

    if (xcb_xrm_context_new(conn, screen, &ctx) < 0) {
        fprintf(stderr, "Failed to initialize xcb-xrm context.\n");
        exit(EXIT_FAILURE);
    }

    if (xcb_xrm_database_load(ctx) < 0) {
        fprintf(stderr, "Failed to load resource database.\n");
        exit(EXIT_FAILURE);
    }

    xcb_xrm_resource_t *resource;
    const char *type;
    if (xcb_xrm_resource_get(ctx, res_name, res_class, &type, &resource) < 0) {
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "%s\n", resource->value);
    xcb_xrm_resource_free(resource);
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

    while ((c = getopt_long(argc, argv, "n:c:vh", long_options, &opt_index)) != -1) {
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
