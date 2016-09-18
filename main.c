/**
 * Contains the main and program logic for the mash shell.
 *
 * @author Ted Mader
 * @date 2016-09-14
 */

#include "commands.h"
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * Reads and returns an entire line from stream.
 *
 * @return the line
 */
char* get_input() {
  char* line = NULL;
  unsigned long bufsize = 0;
  getline(&line, &bufsize, stdin);
  return line;
}

/**
* Modifies a string by breaking into tokens.
*
* @param line a string to be split
* @return the tokens
*/
char** split(char* line) {
  int bufsize = 64;
  char** tokens = malloc(bufsize * sizeof(char*));
  char* delims = " \t\r\n\a";
  char* token = strtok(line, delims);
  int i = 0;
  while (token != NULL) {
    tokens[i] = token;
    i++;
    token = strtok(NULL, delims);
  }
  tokens[i] = NULL;
  return tokens;
}

void redirect(char** argv) {
  int i = 0;
  int in = 0;
  int out = 0;
  char* in_argv = NULL;
  char* out_argv = NULL;
  while (argv[i] != NULL) {
    if (strcmp(argv[i], "<") == 0) {
      in = 1;
      out = 0;
    } else if (strcmp(argv[i], "<") == 0) {
      in = 0;
      out = 1;
    }
    if (in) {
      strcat(in_argv, " ");
      strcat(in_argv, argv[i]);
    } else if (out) {
      strcat(out_argv, " ");
      strcat(out_argv, argv[i]);
    }
    i++;
  }
  if (in_argv != NULL) {
    in = (open(in_argv, O_RDONLY), 0);
  }
  if (out_argv != NULL) {
    out = (open(out_argv, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR), 1);
  }
  close(in);
  close(out);
}

/**
* Forks and executes a process.
*
* @param argv the vector of arguments
* @return the status
*/
int run(char** argv) {
  pid_t pid;
  int status;
  pid = fork();
  if (pid == 0) {
    redirect(argv);
    execvp(*argv, argv);
    printf("mash: command not found: %s\n", *argv);
    exit(1);
  } else if (pid < 0) {
    perror("mash");
    exit(1);
  } else {
    while (wait(&status) != pid);
  }
  return 1;
}

/**
* Executes a command if recognized, otherwise enters run().
*
* @param argv the vector of arguments
* @return the status returned by run()
*/
int execute(char** argv) {
  if (*argv == NULL) {
    return 1;
  }
  int i = 0;
  while (command_labels[i] != NULL) {
    if (strcmp(argv[0], command_labels[i]) == 0) {
      return (*command_functions[i])(argv);
    }
    i++;
  }
  return run(argv);
}

/**
 * Handles commands during the shell loop
 *
 * @return the status
 */
int loop() {
  char* line;
  char** argv;
  int status;
  do {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char* delims = "/";
    char* temp = strtok(cwd, delims);
    char* dir;
    while (temp != NULL) {
      dir = temp;
      temp = strtok(NULL, delims);
    }
    printf("-> %s ", dir);
    line = get_input();
    argv = split(line);
    status = execute(argv);
    free(line);
    free(argv);
  } while (status);
  return status;
}

/**
 * Called at program startup.
 *
 * @param argc the number of arguments
 * @param argv the vector of arguments
 */
int main(int argc, char** argv) {
  return loop();
}
