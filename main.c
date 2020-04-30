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
#include <sys/wait.h>

#define pname "mish"

static void execute(char **args) {
	pid_t pid, wpid;
	int status;

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

int main(int argc, char** argv) {
	char *buffer = NULL;
	uid_t uid;

	uid = getuid();

	do {
		if (buffer) free(buffer);

		if (uid == 0)
			buffer = readline("\033[31m>\033[0m");
		else
			buffer = readline("\033[34m>\033[0m ");

		if (g_strcmp0(buffer, "exit") == 0)
			break;

		char ** tokens = g_strsplit(buffer, " ", 0);
		execute(tokens);
		g_strfreev(tokens);
	} while(1);

	free(buffer);

	return EXIT_SUCCESS;
}
