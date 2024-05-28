/* dwmcmd */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wayland-client.h>
#include "wlr-foreign-toplevel-management-unstable-v1-client-protocol.h"

char fullTitle[4096];
static int handleCounter = 0; /// Used to story more handles if the title if found muliple times
static bool listAll = false;
static bool wrongTitle = true;

// main state struct
struct client_state {
	/* Globals */
	struct wl_seat *wl_seat;
	struct wl_display *wl_display;
	struct wl_registry *wl_registry;
	struct zwlr_foreign_toplevel_manager_v1 *zwlr_foreign_toplevel_manager_v1;
	struct zwlr_foreign_toplevel_handle_v1 *toplevel[512];
	/* Objects */
	/* State */
	const char *title;
};

/* Dummy function */
static void noop() {}

/* Global functions */
static struct zwlr_foreign_toplevel_handle_v1_listener zwlr_foreign_toplevel_handle_v1_listener;

/********************************* TOPLEVEL MANAGEMENT ****************************************/
static void zwlr_foreign_toplevel_handle_v1_handle_title(void *data,
												struct zwlr_foreign_toplevel_handle_v1 *toplevel,
																				const char *title) {
	(void)data;
	(void)toplevel;
	(void)title;
	struct client_state *state = data;
	state->title = strdup(title);
}

static void zwlr_foreign_toplevel_handle_v1_handle_app_id(void *data,
												struct zwlr_foreign_toplevel_handle_v1 *toplevel,
																			const char *app_id) {
	(void)data;
	(void)toplevel;
	(void)app_id;
	struct client_state *state = data;
	if (listAll) {
		fprintf(stdout, "app_id: %s\ntitle: %s\n\n", app_id, state->title);
	}
	if (strcmp(state->title, fullTitle) == 0 || strcmp(app_id, fullTitle) == 0) {
		wrongTitle = false;
		state->toplevel[handleCounter] = toplevel;
		handleCounter = handleCounter + 1;
	}
	free((void *)state->title);
}

static struct zwlr_foreign_toplevel_handle_v1_listener zwlr_foreign_toplevel_handle_v1_listener = {
	.title = zwlr_foreign_toplevel_handle_v1_handle_title,
	.app_id = zwlr_foreign_toplevel_handle_v1_handle_app_id,
	.output_enter = noop,
	.output_leave = noop,
	.state = noop,
	.done = noop,
	.closed = noop,
	.parent = noop,
};

static void zwlr_foreign_toplevel_manager_v1_handle_toplevel(void *data,
			 struct zwlr_foreign_toplevel_manager_v1 *zwlr_foreign_toplevel_manager_v1,
			 struct zwlr_foreign_toplevel_handle_v1 *toplevel) {
	(void)zwlr_foreign_toplevel_manager_v1;
	struct client_state *state = data;
	zwlr_foreign_toplevel_handle_v1_add_listener(toplevel,
												&zwlr_foreign_toplevel_handle_v1_listener, state);
}

static void zwlr_foreign_toplevel_manager_v1_handle_finished(void *data,
												struct zwlr_foreign_toplevel_manager_v1 *manager) {
	(void)data;
	(void)manager;
	printf("Finishes toplevel\n");
	zwlr_foreign_toplevel_manager_v1_destroy(manager);
}

static const struct zwlr_foreign_toplevel_manager_v1_listener foreign_toplevel_listener = {
	.toplevel = zwlr_foreign_toplevel_manager_v1_handle_toplevel,
	.finished = zwlr_foreign_toplevel_manager_v1_handle_finished,
};

/// binding objects to globals
static void registry_global(void *data, struct wl_registry *wl_registry, uint32_t name,
													const char *interface, uint32_t version) {
	(void)version;
	struct client_state *state = data;
	if (strcmp(interface, wl_seat_interface.name) == 0) {
		state->wl_seat = wl_registry_bind(wl_registry, name, &wl_seat_interface, 7);
	}
    else if (strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
        state->zwlr_foreign_toplevel_manager_v1 = wl_registry_bind(wl_registry, name,
        											&zwlr_foreign_toplevel_manager_v1_interface, 3);
        zwlr_foreign_toplevel_manager_v1_add_listener(state->zwlr_foreign_toplevel_manager_v1,
																&foreign_toplevel_listener, state);
    }
}

static void registry_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
	(void)data;
	(void)name;
	(void)wl_registry;
	/* This space deliberately left blank */
}

static const struct wl_registry_listener wl_registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove,
};

int main(int argc, char *argv[]) {
	fullTitle[0] = '\0';
	if (argc < 2) {
		fprintf(stderr, "No arguments provided, please see: --help\n");
	}
	if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		printf("This application activates a window that is minimized or covered by other windows\n");
		printf("List all:\n\t--list-all\tLists add_id and title of all opened windows\n");
		printf("Examples:\n\tdwmcmd title: New Tab\n");
		printf("\tdwmcmd app_id: brave-browser\n");
		printf("Hint:\nFor some titles you need to put it in double quotes e.g.:\n");
		printf("\tdwmcmd title: \"Some long title with special characters\"\n");
	}
	if (argc == 2 && strcmp(argv[1], "--list-all") == 0) {
		listAll = true;
	}
	if (argc == 2 && strcmp(argv[1], "--help") != 0 && strcmp(argv[1], "--list-all") != 0) {
		strcpy(fullTitle, argv[1]);
	}
	if (argc > 2 && strcmp(argv[1], "--help") != 0 && strcmp(argv[1], "--list-all") != 0) {
		if (strcmp(argv[1], "title:") == 0 || strcmp(argv[1], "app_id:") == 0) {
			strcpy(fullTitle, argv[2]);
			// Count all the args and create a strung with all the args
			for (int i = 3; argv[i] != NULL; i++) {
				strcat(fullTitle, " ");
				strcat(fullTitle, argv[i]);
			}
		}
		else {
			fprintf(stderr, "Incorrect value provided, please see: --help\n");
			exit(1);
		}
	}

	struct client_state state = { 0 };

	state.wl_display = wl_display_connect(NULL);
	state.wl_registry = wl_display_get_registry(state.wl_display);
	wl_registry_add_listener(state.wl_registry, &wl_registry_listener, &state);
	wl_display_roundtrip(state.wl_display);
	wl_display_dispatch(state.wl_display);
	// Activating the given window
	if (fullTitle[0] != '\0' && !wrongTitle) {
		for (int i = 0; i < handleCounter; i++) {
			zwlr_foreign_toplevel_handle_v1_activate(state.toplevel[i], state.wl_seat);
			wl_display_dispatch(state.wl_display);
		}
	}
	if (argc >= 2 && wrongTitle && strcmp(argv[1], "title:") == 0 && \
				strcmp(argv[1], "--help") != 0 && strcmp(argv[1], "--list-all") != 0) {
		fprintf(stderr, "Incorrect title provided, please see: --list-all\n");
	}
	else if (argc >= 2 && wrongTitle && strcmp(argv[1], "app_id:") == 0 && \
				strcmp(argv[1], "--help") != 0 && strcmp(argv[1], "--list-all") != 0) {
		fprintf(stderr, "Incorrect app_id provided, please see: --list-all\n");
	}
	else if (argc >= 2 && wrongTitle && strcmp(argv[1], "app_id:") != 0 && \
						strcmp(argv[1], "title:") != 0 && \
						strcmp(argv[1], "--help") != 0 && strcmp(argv[1], "--list-all") != 0) {
		fprintf(stderr, "Incorrect provided, please see: --help\n");
	}
	/// free resources
	zwlr_foreign_toplevel_manager_v1_stop(state.zwlr_foreign_toplevel_manager_v1);
	zwlr_foreign_toplevel_manager_v1_destroy(state.zwlr_foreign_toplevel_manager_v1);
	wl_seat_destroy(state.wl_seat);
	wl_registry_destroy(state.wl_registry);
	wl_display_disconnect(state.wl_display);

    return 0;
}
