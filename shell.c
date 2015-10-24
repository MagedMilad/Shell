/*
 ============================================================================
 Name        : Maged Milad
 ID	         : 54
 Description : Lab 1
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <errno.h>

struct var { // variable struct to save at most 1000 variables
	char *name;
	char *val;
}*vars[1000];

const int INTERACTIVE = 0;
const int BATCH = 1;
const int BUFFER_SIZE = 512;
char *args[512], *bin_paths[512], *env_var[512];
char *line, *history_path, *log_path;
FILE *batch_file, *log_file, *history_file;
int var_count = 0;

char* get_var_val(char *var_name) { // function to get variable value
	int i = 0;
	for (; vars[i] != NULL; i++) {
		if (strcmp(vars[i]->name, var_name) == 0) {
			return vars[i]->val;
		}
	}
	return "";
}

int is_env_var(char *var_name) { // to check to the variable name is an environment variable
	int i = 0;
	for (; env_var[i] != NULL; i++) {
		if (strcmp(var_name, env_var[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

void assign_var(char *line) { // to add new variable to the struct at most 1000 variable
	if (var_count == 100) {
		fprintf(stderr, "excited variable number limit\n");
		return;
	}
	int i = 0;
	char *var_name = strtok(line, "=");
	char *var_val = strtok(NULL, "=");
	for (; vars[i] != NULL; i++) {
		if (strcmp(vars[i]->name, var_name) == 0) {
			strcpy(vars[i]->val, var_val);
			return;
		}
	}
	vars[var_count] = (struct var*) malloc(sizeof(struct var));
	vars[var_count]->name = malloc(sizeof(char) * BUFFER_SIZE);
	bzero(vars[var_count]->name, BUFFER_SIZE);
	vars[var_count]->val = malloc(sizeof(char) * BUFFER_SIZE);
	bzero(vars[var_count]->val, BUFFER_SIZE);
	strcpy(vars[var_count]->name, var_name);
	strcpy(vars[var_count]->val, var_val);
	var_count++;
}

int is_seperator(char ch) {
	return (ch == '+') || (ch == '-') || (ch == '*') || (ch == '/')
			|| (ch == ' ') || (ch == '\t') || (ch == '$') || (ch == '(')
			|| (ch == ')');
}

void fix_echo_line(char *line) { // to substitute the variables with their values
	char *pointer = line;
	char *ret = malloc(sizeof(char) * BUFFER_SIZE);
	bzero(ret, BUFFER_SIZE);
	int in_match = 0;
	char *var_name = malloc(sizeof(char) * BUFFER_SIZE);
	while (*pointer != '\0') {
		if (!in_match) {
			if (*pointer == '$') {
				in_match = 1;
				bzero(var_name, BUFFER_SIZE);
			} else {
				strncat(ret, pointer, 1);
			}
		} else {
			if (is_seperator(*pointer)) {
				if (is_env_var(var_name)) {
					char *val = getenv(var_name);
					strncat(ret, val, strlen(val));
				} else {
					char *val = get_var_val(var_name);
					strncat(ret, val, strlen(val));
				}
				in_match = 0;
				if (*pointer != '$') {
					strncat(ret, pointer, 1);
				} else {
					in_match = 1;
				}
			} else {
				strncat(var_name, pointer, 1);
			}
		}
		pointer++;
	}
	if (in_match) {
		if (is_env_var(var_name)) {
			char *val = getenv(var_name);
			strncat(ret, val, strlen(val));
		} else {
			char *val = get_var_val(var_name);
			strncat(ret, val, strlen(val));
		}
		in_match = 0;
		if (*pointer != '$') {
			strncat(var_name, pointer, 1);
		} else {
			in_match = 1;
		}

	}
	strncat(ret, "\0", 1);
	strcpy(line, ret);
}

void add_to_history(char *line) { // add the command to history
	history_file = fopen(history_path, "a");
	fprintf(history_file, "%s\n", line);
	fclose(history_file);
}

void append_home_dir(char *path) { // to replace ~ with home directory
	char *home = getenv("HOME");
	char *pointer = path;
	char *ret = malloc(sizeof(char) * BUFFER_SIZE);
	bzero(ret, BUFFER_SIZE);
	while (*pointer != '\0') {
		if (*pointer == '~') {
			strcat(ret, home);
		} else {
			strncat(ret, pointer, 1);
		}
		pointer++;
	}
	strncat(ret, "\0", 1);
	strcpy(path, ret);
}

void execute_history() { // to execute the history command
	int num = 1;
	if (open(history_path, O_RDONLY) > 0) {
		history_file = fopen(history_path, "r");
		char *line = NULL;
		size_t len = 0;
		ssize_t read = 0;
		while ((read = getline(&line, &len, history_file)) != -1) {
			printf("%d  %s", num++, line);
		}
		fclose(history_file);
		free(line);
	} else {
		fprintf(stderr, "not history yet\n");
	}
}

int is_command(char * line) { // check if the given line is a command by looping in all environment directories
	char path[512];
	bzero(path, BUFFER_SIZE);
	int i, fd;
	for (i = 0; bin_paths[i] != NULL; i++) {
		strcpy(path, bin_paths[i]);
		strcat(path, line);
		if ((fd = open(path, O_RDONLY)) > 0) {
			strcpy(line, path);
			close(fd);
			return 1;
		}
	}
	return 0;
}

void fill_bin_paths() { // to fill array by the bin paths of the current system
	char *paths = getenv("PATH");
	char *pointer = paths;
	char ret[512];
	bzero(ret, BUFFER_SIZE);
	int idx = 0;
	while (*pointer != '\0') {
		if (*pointer == ':') {
			strncat(ret, "/", 1);
			bin_paths[idx] = (char *) malloc(sizeof(char) * (strlen(ret) + 1));
			strncat(bin_paths[idx], ret, strlen(ret));
			strncat(bin_paths[idx], "\0", 1);
			bzero(ret, BUFFER_SIZE);
			idx++;
		} else {
			strncat(ret, pointer, 1);
		}
		pointer++;
	}
}

void write_to_log(int pid) { // to write to log if child terminate and get it's PID
	log_file = fopen(log_path, "a");
	if (pid == 0) {
		int status;
		pid_t corpse;
		while ((corpse = waitpid(0, &status, WNOHANG)) > 0) {
			fprintf(log_file, "child %d has terminated\n", corpse);

		}
	} else
		fprintf(log_file, "child %d has terminated\n", pid);
	fclose(log_file);
}

void handle_signal(int sig) {
	write_to_log(0);
}

int is_comment(char *line) { // to fill the arguments of the given command
	char *pointer = line;
	while (*pointer == ' ' || *pointer == '\t')
		pointer++;
	if (*pointer == '#')
		return 1;
	return 0;
}

void fill_argv(char *line) { // to fill the arguments of the given command
	char *pointer = line;
	while (*pointer == ' ' || *pointer == '\t')
		pointer++;
	int idx = 0;
	char ret[512];
	bzero(ret, BUFFER_SIZE);
	int is_string = 0;
	while (*pointer != '\0') {
		if (idx == BUFFER_SIZE)
			break;
		if (*pointer == '"' || *pointer == '\'') {
			is_string ^= 1;
			pointer++;
			continue;
		}
		if ((*pointer == ' ' || *pointer == '\t') && !is_string) {
			if (strlen(ret)) {
				if (args[idx] == NULL) {
					args[idx] = (char *) malloc(sizeof(char) * BUFFER_SIZE);
					bzero(args[idx], BUFFER_SIZE);
				} else {
					bzero(args[idx], BUFFER_SIZE);
				}

				strncat(args[idx], ret, strlen(ret));
				strncat(args[idx], "\0", 1);
				bzero(ret, BUFFER_SIZE);
				idx++;
			}
		} else {
			strncat(ret, pointer, 1);
		}
		pointer++;
	}
	if (strlen(ret)) {
		args[idx] = (char *) malloc(sizeof(char) * BUFFER_SIZE);
		bzero(args[idx], BUFFER_SIZE);
		strncat(args[idx], ret, strlen(ret));
		strncat(args[idx], "\0", 1);
	}
}

int is_background() {
	int i;
	for (i = 0; args[i] != NULL; i++) {
		if (strcmp(args[i], "&") == 0 && args[i + 1] == NULL) {
			return i;
		}
	}
	return 0;
}

void call_execve(char *path) { // create new process to execute to command
	int child_state;
	pid_t pid;
	int background = is_background();
	if ((pid = fork()) == 0) {
		if (background) {
			args[background] = NULL;
		}
		execv(path, args);
		fprintf(stderr, "%s: command not found\n", args[0]);
		exit(0);
	} else if (pid > 0) {
		if (!background) {
			waitpid(pid, &child_state, 0);
			write_to_log(pid);
		}
	} else {
		perror("");
	}
}

void free_args() {
	int index;
	for (index = 0; args[index] != NULL; index++) {
		bzero(args[index], strlen(args[index]) + 1);
		args[index] = NULL;
		free(args[index]);
	}
}

int lens(char *line) {
	int i = 0;
	int len = 0;
	for (i = 0; line[i] != '\0'; i++) {

		len++;
	}
	return len;
}

int cd() { // cd special case
	if (args[1] == NULL) {
		fprintf(stderr, "expected argument to \"cd\"\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("");
		}
	}
	return 1;
}

void read_input(int mode, char *batch_path) {

	char c;
	int current_length = (BUFFER_SIZE + 1);
	line = (char *) malloc(sizeof(char) * (BUFFER_SIZE + 1));
	char *path = (char *) malloc(sizeof(char) * (BUFFER_SIZE + 1));
	fflush(stderr);
	printf("\n[SHELL ] ");
	fflush(stdout);
	int run = 1, len = 0;
	if (mode == BATCH) {
		if (open(batch_path, O_RDONLY) > 0) {
			batch_file = fopen(batch_path, "r");
		} else {
			fprintf(stderr, "cann't open or read the file\n");
			return;
		}
	}
	while (c != EOF && run) {

		if (mode == INTERACTIVE)
			c = getchar();
		else {
			c = fgetc(batch_file);
			if (feof(stdin)) {
				run = 0;
				break;
			}
		}
		switch (c) {
		case '\n':
			if (line[0] == '\0') {
				len = 0;
				free_args();
				bzero(line, current_length);
				fflush(stderr);
				printf("\n[SHELL ] ");
				fflush(stdout);
			} else {

				if (mode == BATCH) {
					printf("%s\n", line);
				}
				if (len >= BUFFER_SIZE) {
					fprintf(stderr, "buffer limit excited\n");
					bzero(line, current_length);
					len = 0;
					continue;
				}
				add_to_history(line);
				free_args();
				fill_argv(line);
				if (is_comment(line) == 0) {
					if (strchr(line, '=') != NULL
							&& strcmp(args[0], "echo") != 0) {
						assign_var(line);
					} else {
//						free_args();
//						fill_argv(line);
						if (strcmp(args[0], "echo") != 0) {
							int i;
							for (i = 0; args[i] != NULL; i++) {
								append_home_dir(args[i]);
								fix_echo_line(args[i]);
							}
						} else {
							int i;
							for (i = 1; args[i] != NULL; i++) {
								fix_echo_line(args[i]);
							}
						}

						if (strcmp("exit", line) == 0) {
							run = 0;
							break;
						} else if (strcmp("history", args[0]) == 0) {
							execute_history();
						} else if (strcmp("cd", args[0]) == 0)
							cd();
						else {
							strcpy(path, args[0]);
							if (strchr(path, '/') != NULL) {
								int fd;
								if ((fd = open(path, O_RDONLY)) > 0) {
									close(fd);
									call_execve(path);
								} else {
									fprintf(stderr,
											"%s: No such file or directory\n",
											args[0]);
								}
							} else {
								if (!is_command(path)) {
									fprintf(stderr, "%s: command not found\n",
											args[0]);
								} else {
									call_execve(path);
								}
							}

						}
					}
				}
				free_args();
				fflush(stderr);
				printf("\n[SHELL ] ");
				fflush(stdout);
				bzero(line, BUFFER_SIZE);
				len = 0;
			}
			bzero(line, BUFFER_SIZE);
			break;
		default:
			len++;
			if (len < BUFFER_SIZE) {
				strncat(line, &c, 1);
			}
			break;
		}
	}
	if (mode == BATCH)
		fclose(batch_file);
}

void fill_env_var(char ** envp) { // fill array with environment variables
	char** env;
	int idx = 0;
	char* thisEnv = malloc(sizeof(char) * BUFFER_SIZE);
	for (env = envp; *env != 0; env++) {
		bzero(thisEnv, BUFFER_SIZE);
		strcpy(thisEnv, *env);
		env_var[idx] = malloc(sizeof(char) * BUFFER_SIZE);
		thisEnv = strtok(thisEnv, "=");
		strcpy(env_var[idx++], thisEnv);
	}
}

int main(int argc, char **path, char ** envp) {
	struct stat st = { 0 };
	char *home = getenv("HOME");
	char *conf_path = (char *) malloc(sizeof(char) * (strlen(home)));
	strcpy(conf_path, home);
	strcat(conf_path, "/.shell");
	if (stat(conf_path, &st) == -1) {
		mkdir(conf_path, 0700);
	}
	history_path = (char *) malloc(sizeof(char) * (strlen(conf_path) + 7));
	log_path = (char *) malloc(sizeof(char) * (strlen(conf_path) + 4));
	strcpy(history_path, conf_path);
	strcat(history_path, "/history");
	strcpy(log_path, conf_path);
	strcat(log_path, "/log");
	signal(SIGCHLD, handle_signal);
	fill_env_var(envp);
	fill_bin_paths();
	if (argc == 1) {
		read_input(INTERACTIVE, "");
	} else if (argc == 2) {
		read_input(BATCH, path[1]);
	} else {
		fprintf(stderr, "too much arguments\n");
	}
	free_args();
	free(line);
	printf("\n");
	return 0;
}

