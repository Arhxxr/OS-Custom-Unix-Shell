// The MIT License (MIT)
//
// Copyright (c) 2024 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation fistrcspnles (the "Software"),
// to deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define WHITESPACE " \t\n"
#define MAX_COMMAND_SIZE 255
#define MAX_ARGS 32

void execute_command(char* args[]) {
  
  pid_t pid = fork();
  
  if (pid < 0) {
    perror("fork");
    exit(0);
  } else if (pid == 0) {
    int i;
    int out_file = -1;
    for (i = 0; args[i] != NULL; i++) {
      if (strcmp(args[i], ">") == 0) {
        if (args[i + 1] == NULL) {
          char error_message[30] = "An error has occurred\n";
          write(STDERR_FILENO, error_message, strlen(error_message));
          exit(0);
        }
        out_file = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (out_file == -1) {
          char error_message[30] = "An error has occurred\n";
          write(STDERR_FILENO, error_message, strlen(error_message));
        }
        dup2(out_file, STDOUT_FILENO);
        dup2(out_file, STDERR_FILENO);
        close(out_file);
        args[i] = NULL;
        args[i + 1] = NULL;
        break;
      }
    }
    if (execvp(args[0], args) == -1) {
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
      exit(0);
    }
  } else {
    int status, status2;
    waitpid(pid, &status, 0);
    waitpid(-1, &status2, 0);
  }
}

void interactive_mode() {
  
  char input[MAX_COMMAND_SIZE];
  char* args[MAX_ARGS];

  while (1) {
    printf("msh> ");
    fflush(stdout);

    if (fgets(input, MAX_COMMAND_SIZE, stdin) == NULL ||
        strcmp(input, "exit\n") == 0) {
      break;
    }

    if (strcmp(input, "\n") == 0) {
      continue;
    }

    input[strcspn(input, "\n")] = '\0';

    char* token = strtok(input, WHITESPACE);
    int i = 0;
    
    while (token != NULL && i < MAX_ARGS - 1) {
      args[i++] = token;
      token = strtok(NULL, WHITESPACE);
    }

    args[i] = NULL;

    if (i == 0) {
      continue;
    }

    int redirect = -1;
    
    for (i = 0; args[i] != NULL; i++) {
      if (strcmp(args[i], ">") == 0) {
        redirect = i;
        break;
      }
    }

    if (redirect != -1) {
      if (redirect == 0 || args[redirect - 1] == NULL) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
      }
    }

    if (redirect != -1) {
      if (args[redirect + 1] != NULL && args[redirect + 2] != NULL) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        continue;
      }
    }

    if (strcmp(args[0], "cd") == 0) {
      if (i == 2) {
        if (chdir(args[1]) != 0) {
          perror("chdir");
        }
      } else {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
      }
    } else {
      execute_command(args);
    }
  }
}

void batch_mode(char* filename) {
  
  FILE* file = fopen(filename, "r");

  if (file == NULL) {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }

  char input[MAX_COMMAND_SIZE];
  char* args[MAX_ARGS];

  while (fgets(input, MAX_COMMAND_SIZE, file)) {
    if (strcmp(input, "\n") == 0) {
      continue;
    }

    if (strcmp(input, "\n") == 0 || strcmp(input, ">\n") == 0) {
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
      continue;
    }

    input[strcspn(input, "\n")] = '\0';

    char* token = strtok(input, WHITESPACE);
    int i = 0;
    
    while (token != NULL && i < MAX_ARGS - 1) {
      args[i++] = token;
      token = strtok(NULL, WHITESPACE);
    }

    char* trm = strdup(input);
    char* trm_end = &trm[strlen(trm) - 1];

    while (trm_end > trm && isspace(*trm_end)) {
      *trm_end-- = '\0';
    }

    while (*trm && isspace(*trm)) {
      ++trm;
    }

    if (strcmp(trm, "") == 0) {
      continue;
    }

    args[i] = NULL;

    int redirect = -1;
    
    for (i = 0; args[i] != NULL; i++) {
      if (strcmp(args[i], ">") == 0) {
        redirect = i;
        break;
      }
    }

    if (redirect != -1) {
      if (redirect == 0 || args[redirect - 1] == NULL) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
      }
    }

    if (redirect != -1) {
      if (args[redirect + 1] != NULL && args[redirect + 2] != NULL) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        continue;
      }
    }

    if (strcmp(args[0], "cd") == 0) {
      if (i == 2) {
        if (chdir(args[1]) != 0) {
          perror("chdir");
        }
      } else {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
      }
    } else if (strcmp(args[0], "exit") == 0) {
      if (args[1] != NULL) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
      } else {
        fclose(file);
        exit(0);
      }
    } else {
      execute_command(args);
    }
  }
  fclose(file);
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    interactive_mode();

  } else if (argc == 2 && strcmp(argv[1], "exit") == 0) {
    if (argv[2] != NULL) {
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
    } else {
      exit(0);
    }

  } else if (argc == 2 && strcmp(argv[1], "cd") == 0) {
    if (chdir("/") != 0) {
      perror("chdir");
    }

  } else if (argc == 2) {
    batch_mode(argv[1]);

  } else if (argc > 2) {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);

  } else {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(0);
  }
  
  return 0;
}