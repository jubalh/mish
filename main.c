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

static gchar* base_prompt(void) {
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

	return g_strdup_printf("\033[%dm\033[1m%s\033[0m@%s> ", color_code, pw->pw_name, host);
}

int main(int argc, char** argv) {
	char *buffer = NULL;
	gchar *prompt = base_prompt();

	do {
		if (buffer) free(buffer);

		buffer = readline(prompt);

		if (g_strcmp0(buffer, "exit") == 0)
			break;

		char ** tokens = g_strsplit(buffer, " ", 0);

		if (g_strcmp0(tokens[0], "cd") == 0) {
			if (chdir(tokens[1]) == -1) {
				perror(pname);
			}
		} else {
			execute(tokens);
		}

		g_strfreev(tokens);
	} while(1);

	free(buffer);
	g_free(prompt);

	return EXIT_SUCCESS;
}
