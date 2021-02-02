#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

/* The mode if not use built-in command */
#define NOMAL_EXECU 0 
#define R_REDIRECT 1
#define L_REDIRECT 2
#define PIPE_EXECU 3

/* pipe end index */
#define W_END 1
#define R_END 0

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

extern void init_shell();

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);


/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd, "pwd", "prints the current working directory"},
  {cmd_cd, "cd", "takes one argument, a directory path, and changes the current working directory to that directory"}
};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens *tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens *tokens) {
  exit(0);
}

/* prints the current working directory to standard output */
int cmd_pwd(unused struct tokens *tokens) {
  int buffer_size = 1000;
  char *pwd = (char*)malloc(buffer_size * sizeof(char));
  if(getcwd(pwd, buffer_size)){
    printf("%s\n", pwd);
    free(pwd);
  } else {
    printf("errno:%d getcwd doesn't work. \n", errno);
  }
  return 1;
}

/* takes one argument, a directory path, and changes the current working directory to that directory */
int cmd_cd(struct tokens *tokens){
  size_t tokens_length = tokens_get_length(tokens);
  char *new_path;
  if (tokens_length > 1)
  {
    new_path = tokens_get_token(tokens, 1);
    if (chdir(new_path))
    {
      printf("error: %d chdir doesn't work.\n", errno);
    }
  } else
  {
    printf("please enter a path.\n");
  }

  return 1;
}

/* Execute the process with given name even without full path, if failed return -1 */
int exec_with_resolution(char *proc_name, char *proc_argv[]) {
  char *__paths = getenv("PATH"); /* do not modify this */
  char *paths;
  char *path;
  char *current_path;
  size_t proc_name_lenght = strlen(proc_name);

  if (execv(proc_name, proc_argv) == -1)
  {
    if (errno == ENOENT)
    {   
        /* use environment path */
        paths = (char *)malloc(strlen(__paths) + 1);
        strcpy(paths, __paths);
        while ((path = strsep(&paths, ":")))
        {
          current_path = (char *)malloc(strlen(path) + 1 + proc_name_lenght + 1);/* path/process'\0' */
          strcpy(current_path, path);
          strcat(current_path, "/");
          strcat(current_path, proc_name);
          proc_argv[0] = current_path;

          if (execv(proc_argv[0], proc_argv) == -1 && errno != ENOENT)
          {
            break;
          }              
        }
    }        
    perror("Fail to execute process");
  }
  return -1;
}

/* analyze the mode */
void analyze_mode(int *mode, struct tokens *tokens) {
  size_t tokens_length = tokens_get_length(tokens);
  char *token;

  for (size_t i = 0; i < tokens_length; i++)
  {
    token = tokens_get_token(tokens, i);
    if (strcmp("<", token) == 0)
    {
      *mode = L_REDIRECT;
      return;
    } else if (strcmp(">", token) == 0)
    {
      *mode = R_REDIRECT;
      return;
    } else if (strcmp("|", token) == 0)
    {
      *mode = PIPE_EXECU;
      return;
    }
       
  }
  *mode = NOMAL_EXECU;
  return;
}

/* mode NOMAL_EXECU */
void mode_nomal(struct tokens *tokens){
  pid_t cpid;
  int wstatus;
  size_t tokens_length = tokens_get_length(tokens);
  int start_pipe[2];
  
  /* create pipe for initialization purpose */
  pipe(start_pipe);

  cpid = fork();
  if (cpid) /* parent process */
  {
    /* -------------------- handle foreground problem ----------------------- */
    /* ignore the SIGTTOU */
    struct sigaction ign_ttou;
    struct sigaction dfl_ttou;
    dfl_ttou.sa_handler = SIG_DFL;
    ign_ttou.sa_handler = SIG_IGN;
    sigaction(SIGTTOU, &ign_ttou, NULL);

    /* close read end */
    close(start_pipe[R_END]);

    /* set child to its own proc group */
    if (setpgid(cpid, cpid) < 0)
    {
      perror("setpgid failed");
    }

    /* put child process to foreground */
    if (tcsetpgrp(STDIN_FILENO, cpid) < 0)
    {
      perror("tcsetpgrp failed");
    }
    
    /* send message */
    write(start_pipe[W_END], "1", 1);

    /* close pipe */
    close(start_pipe[W_END]); 
    /* -------------------- handle foreground problem ----------------------- */

    if (wait(&wstatus) == -1)
    {
      perror("wait failed");
    }

    /* put parent process to foreground */
    if (tcsetpgrp(STDIN_FILENO, getpgid(getpid())) < 0)
    {
      perror("tcsetpgrp failed");
    }

    /* reset SIGTTOU */
    sigaction(SIGTTOU, &dfl_ttou, NULL);

    /* put cursor to next line */
    printf("\n");
    
  } else  /* child process */
  {
    char *process_argv[tokens_length + 1];
    cpid = getpid();
    char buff;

    /* close write end */
    close(start_pipe[W_END]);

    /* read message */
    read(start_pipe[R_END], &buff, 1);

    /* close pipe */
    close(start_pipe[R_END]);
        
    /* load arguments*/
    for (size_t i = 0; i < tokens_length; i++)
    {
      process_argv[i] = tokens_get_token(tokens, i);
    }
    process_argv[tokens_length] = (char*) NULL;

    /* execute */
    if(exec_with_resolution(process_argv[0], process_argv) == -1) {
      printf("exec failed.\n");
    }
    
    exit(3);
  }
}

/* mode R_REDIRECT & L_REDIRECT, direction should be either R_REDIRECT or L_REDIRECT.*/
void mode_redirect(struct tokens *tokens, int direction) {
  size_t tokens_length = tokens_get_length(tokens);
  char **argv = (char**) malloc(tokens_length * sizeof(char *));
  const char* direction_str;
  char *file_path;
  int pipe_fd[2];
  int start_pipe[2];
  pid_t cpid;
  size_t i;

  /* create pipe for initialization purpose */
  pipe(start_pipe);

  if (direction == R_REDIRECT)
  {
    direction_str = ">";
  } else if (direction == L_REDIRECT)
  {
    direction_str = "<";
  } else
  {
    printf("direction is not correct.\n");
    return;
  }
  
  /* load argument */
  for (i = 0; i < tokens_length; i++)
  {
    argv[i] = tokens_get_token(tokens, i);
    if (strcmp(argv[i], direction_str) == 0)
    {
      argv[i] = (char *)NULL;
      break;
    } 
  }
  
  /* get file path */
  file_path = tokens_get_token(tokens, i + 1);

  /* create pipe */
  pipe(pipe_fd);

  /* fork */
  cpid = fork();
  if (cpid == 0) /* child process */
  {
    char buff;
    /* close write end */
    close(start_pipe[W_END]);

    /* read message */
    read(start_pipe[R_END], &buff, 1);

    /* close pipe */
    close(start_pipe[R_END]);

    /* process i/o redirect */
    if (direction == R_REDIRECT)
    {
      dup2(pipe_fd[1], STDOUT_FILENO);
      close(pipe_fd[1]);
    } else
    {
      dup2(pipe_fd[0], STDIN_FILENO);
      close(pipe_fd[0]);
    }
    
    /* execute process */
    exec_with_resolution(argv[0], argv);

    exit(-1);
  } else  /* parent process */
  {
    int wstatus;
    int fd;
    void* buffer;
    size_t buffer_size = 1000;
    size_t buffer_read;
    /* -------------------- handle foreground problem ----------------------- */
    /* ignore the SIGTTOU */
    struct sigaction ign_ttou;
    struct sigaction dfl_ttou;
    dfl_ttou.sa_handler = SIG_DFL;
    ign_ttou.sa_handler = SIG_IGN;
    sigaction(SIGTTOU, &ign_ttou, NULL);

    /* close read end */
    close(start_pipe[R_END]);

    /* set child to its own proc group */
    if (setpgid(cpid, cpid) < 0)
    {
      perror("setpgid failed");
    }

    /* put child process to foreground */
    if (tcsetpgrp(STDIN_FILENO, cpid) < 0)
    {
      perror("tcsetpgrp failed");
    }
    
    /* send message */
    write(start_pipe[W_END], "1", 1);

    /* close pipe */
    close(start_pipe[W_END]); 
    /* -------------------- handle foreground problem ----------------------- */
    /* initialize buffer */
    buffer = malloc(buffer_size);

    /* connect the file and the process */
    if (direction == R_REDIRECT)  /* output to file */
    {
      /* open file */
      if ((fd = open(file_path, O_WRONLY)) == -1) {
        perror("failed to open file");
      };
      /* close needless pipe end */
      close(pipe_fd[1]);
      /* read to buffer and write to file*/
      while ((buffer_read = read(pipe_fd[0], buffer, buffer_size)))
      {
        if (write(fd, buffer, buffer_read) == -1)
        {
          perror("function write failed");
          break;
        }
      }
      fsync(fd);
      close(fd);
    } else  /* file as input */
    {
      /* open file */
      if ((fd = open(file_path, O_RDONLY)) == -1) {
        perror("failed to open file");
      };
      /* close needless pipe end */
      close(pipe_fd[0]);
      /* read to buffer and write to pipe */
      while ((buffer_read = read(fd, buffer, buffer_size)))
      {
        if (write(pipe_fd[1], buffer, buffer_read) == -1)
        {
          perror("function write failed");
          break;
        }
      }
      close(fd);
    }
    
    if (wait(&wstatus) == -1)
    {
      perror("wait failed");
    }

    /* put parent process to foreground */
    if (tcsetpgrp(STDIN_FILENO, getpgid(getpid())))
    {
      perror("tcsetpgrp failed");
    }

    /* reset SIGTTOU */
    sigaction(SIGTTOU, &dfl_ttou, NULL);

    /* put cursor to next line */
    printf("\n");

    free(buffer);
    return;
  }

}

/* mode PIPE_EXECU */
void mode_pipe(struct tokens *tokens) {
  size_t tokens_length = tokens_get_length(tokens);
  int current_proc_i = 0;
  int current_proc_argv_i = 0;
  int pipe_num = 0; /* the number of '|' */
  int argv_num[tokens_length];/* count the num of argv of each process */
  int **pipes;
  int **start_pipes;
  pid_t cpid;
  pid_t cpgid;
  char ***argv_group;
  int wstatus;
  /* SIGTTOU related */
  struct sigaction ign_ttou;
  struct sigaction dfl_ttou;
  dfl_ttou.sa_handler = SIG_DFL;
  ign_ttou.sa_handler = SIG_IGN;

  /* analyze the tokens */
  argv_num[pipe_num] = 0;
  for (size_t i = 0; i < tokens_length; i++)
  {
    char *token = tokens_get_token(tokens, i);
    if (strcmp(token, "|") == 0)
    {
      pipe_num ++;
      argv_num[pipe_num] = 0;
    } else
    {
      argv_num[pipe_num]++;
    }  
  }

  /* initialize argument */ 
  argv_group = (char***) malloc((pipe_num + 1) * sizeof(char**));
  for (size_t i = 0; i < (pipe_num + 1); i++)
  {
    argv_group[i] = (char**)malloc(argv_num[i] * sizeof(char*) + 1); /* have NULL pointer at the end */
  }
  current_proc_i = 0;
  current_proc_argv_i = 0;
  for (size_t i = 0; i < tokens_length; i++)
  {
    char *token = tokens_get_token(tokens, i);
    if (strcmp(token, "|") == 0)
    {
      argv_group[current_proc_i][current_proc_argv_i] = (char*) NULL;
      current_proc_i ++;
      current_proc_argv_i = 0;
    } else
    {
      argv_group[current_proc_i][current_proc_argv_i] = token;
      current_proc_argv_i ++;
    }   
  } 

  /* print argv_group */
  // for (size_t i = 0; i < (pipe_num + 1);  i++)
  // {
  //   for (size_t j = 0; j < argv_num[i]; j++)
  //   {
  //     printf("process %ld, argument %ld: %s\n", i, j, argv_group[i][j]);
  //   }   
  // }
  
  /* create pipe */
  pipes = (int**) malloc(pipe_num * sizeof(int*)); 
  for (size_t i = 0; i < pipe_num; i++)
  {
    pipes[i] = (int*) malloc(2 * sizeof(int));
    pipe(pipes[i]);
  }

  /* create start pipes */
  start_pipes = (int**) malloc((pipe_num + 1) * sizeof(int*));
  for (size_t i = 0; i < (pipe_num + 1); i++)
  {
    start_pipes[i] = (int*) malloc(2 * sizeof(int));
    pipe(start_pipes[i]);
  }
  
  /* fork */
  cpid = fork(); /* handle head process */
  if (cpid == 0)
  {
    dup2(pipes[0][W_END], STDOUT_FILENO);
    close(pipes[0][W_END]);

    /* wait message to start */
    char buf;
    close(start_pipes[0][W_END]);
    read(start_pipes[0][R_END], &buf, 1);
    close(start_pipes[0][R_END]);

    exec_with_resolution(argv_group[0][0], argv_group[0]);
    exit(-1);
  } else  /* parent */
  {
    /* set child process to its own pgid */
    if (setpgid(cpid, cpid) < 0) {perror("setpgid failed");} 
    cpgid = cpid;

    /* close useless pipe end */
    close(start_pipes[0][R_END]);
  }
  
  for (size_t i = 1; i < pipe_num; i++)
  {
    cpid = fork();
    if (cpid == 0)  /* child process */
    {
      dup2(pipes[i - 1][R_END], STDIN_FILENO);
      dup2(pipes[i][W_END], STDOUT_FILENO);
      close(pipes[i - 1][R_END]);
      close(pipes[i][W_END]);

      /* wait message to start */
      char buf;
      close(start_pipes[i][W_END]);
      read(start_pipes[i][R_END], &buf, 1);
      close(start_pipes[i][R_END]);

      exec_with_resolution(argv_group[i][0], argv_group[i]);
      exit(-1);
    } else
    {
      /* put child process to cpgid */
      if (setpgid(cpid, cpgid) < 0) {perror("setpgid failed");} 

      /* close useless pipe end */
      close(start_pipes[i][R_END]);
    }
    
  }
  cpid = fork(); /* handle tail process */
  if (cpid == 0)
  {
    dup2(pipes[pipe_num - 1][R_END], STDIN_FILENO);
    close(pipes[pipe_num - 1][R_END]);

    /* wait message to start */
    char buf;
    close(start_pipes[pipe_num][W_END]);
    read(start_pipes[pipe_num][R_END], &buf, 1);
    close(start_pipes[pipe_num][R_END]);

    exec_with_resolution(argv_group[pipe_num][0], argv_group[pipe_num]);
    exit(-1);
  } else
  {
    /* put child process to cpgid */
    if (setpgid(cpid, cpgid) < 0) {perror("setpgid failed");} 

    /* close useless pipe end */
    close(start_pipes[pipe_num][R_END]);
  }
  /* ingore STTOU */
  sigaction(SIGTTOU, &ign_ttou, NULL);

  /* put child process to foreground */
  if (tcsetpgrp(STDIN_FILENO, cpgid) < 0) {perror("tcsetpgrp failed");}
  
  /* send message to child to start */
  for (size_t i = 0; i < (pipe_num + 1); i++)
  {
    write(start_pipes[i][W_END], "1", 1);
  }

  /* wait all child to finish */
  while ((cpid = wait(&wstatus)) > 0);


  /* put parent process to foreground */
  if (tcsetpgrp(STDIN_FILENO, getpgid(getpid())) < 0)
  {
    perror("tcsetpgrp failed");
  }

  /* reset SIGTTOU */
  sigaction(SIGTTOU, &dfl_ttou, NULL);

  /* put cursor to next line */
  printf("\n");

  return;

}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

int main(unused int argc, unused char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
      /* REPLACE this to run commands as programs. */
      int mode;

      /* find is if doing redirection of piping */
      analyze_mode(&mode, tokens);

      if (mode == NOMAL_EXECU)
      {
        mode_nomal(tokens);
      } else if (mode == R_REDIRECT)
      {
        mode_redirect(tokens, R_REDIRECT);
      } else if (mode == L_REDIRECT)
      {
        mode_redirect(tokens, L_REDIRECT);
      } else if (mode == PIPE_EXECU)
      {
        mode_pipe(tokens);
      }
        
    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}
