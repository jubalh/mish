/*
 * vim: noexpandtab:ts=2:sts=2:sw=2
 *
 * Copyright (C) 2020 Michael Vetter <jubalh@iodoru.org>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <glib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include "config.h"

#define pname "mish"

static void execute(char **args) {
	pid_t pid, wpid;
	int status;

	if (args[0] == NULL)
		return;

	pid = fork();

	if (pid == 0) {
		// child
		if (execvp(args[0], args) == -1) {
			perror(pname);
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// Error forking
		perror(pname);
	} else {
		// parent
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
}

static gchar* get_base_prompt(void) {
	uid_t uid = getuid();
	int color_code = 34;

	if (uid == 0)
		color_code = 31;

	struct passwd *pw = getpwuid(uid);
	if (!pw) {
		perror(pname);
		exit(EXIT_FAILURE);
	}

	char host[1024];
	if (gethostname(host, 1023) == -1) {
		perror(pname);
		exit(EXIT_FAILURE);
	}

	return g_strdup_printf("\033[%dm\033[1m%s\033[0m@%s", color_code, pw->pw_name, host);
}

static gchar* get_prompt(gchar *prompt) {
	char path[PATH_MAX];

	if (!getcwd(path, PATH_MAX-1)) {
		perror(pname);
	}

	struct passwd *pw = getpwuid(getuid());
	if (!pw) {
		perror(pname);
		exit(EXIT_FAILURE);
	}

	char *home = NULL;
	if (g_str_has_prefix(path, pw->pw_dir)) {
		home = &path[strlen(pw->pw_dir)];
	}

	return g_strdup_printf("%s \033[1m%s%s\033[0m > ",
			prompt,
			home ? "~" : "",
			home ? home : path);
}

static int shell(void) {
	char *buffer = NULL;
	gchar *baseprompt = get_base_prompt();
	gchar *prompt = get_prompt(baseprompt);

	do {
		if (buffer) free(buffer);

		buffer = readline(prompt);

		if (g_strcmp0(buffer, "exit") == 0)
			break;

		char ** tokens = g_strsplit(buffer, " ", 0);

		if (g_strcmp0(tokens[0], "cd") == 0) {
			if (chdir(tokens[1]) == -1) {
				perror(pname);
			} else {
				g_free(prompt);
				prompt = get_prompt(baseprompt);
			}
		} else {
			execute(tokens);
		}

		g_strfreev(tokens);
	} while(1);

	free(buffer);
	g_free(baseprompt);
	g_free(prompt);

	return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
  static gboolean version = FALSE;

	static GOptionEntry entries[] =
	{
		{ "version", 'v', 0, G_OPTION_ARG_NONE, &version, "Show version", NULL },
		{ NULL }
	};

	GError *error = NULL;
	GOptionContext *context;

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, entries, NULL);
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print("%s\n", error->message);
		g_option_context_free(context);
		g_error_free(error);
		return 1;
	}
	g_option_context_free(context);

	if (version) {
		g_print("mish shell, version %s\n", VERSION);
		g_print("Copyright (C) 2020 Michael Vetter <jubalh@iodoru.org>.\n");
		g_print("License GPLv3+: GNU GPL version 3 or later <https://www.gnu.org/licenses/gpl.html>\n\n");
		g_print("This is free software; you are free to change and redistribute it.\n");
		g_print("There is NO WARRANTY, to the extent permitted by law.\n");
	}

	return shell();
}
