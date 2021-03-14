//INCLUDES
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//GLOBAL VARIABLES
char name[25];


//PROTOTYPES

int shell_cd(char **args);
int shell_help(char **args);
int shell_cls(char **args);
int shell_exit(char **args);


//DATA

char *builtin_str[] = {
  "cd",
  "help",
  "cls",
  "cat",
  "exit"
};




int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_cls,
  &shell_exit
};



//FUNCTIONS
int shell_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}




int shell_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "shell: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("shell");
    }
  }
  return 1;
}




int shell_cls(char **args){
  printf("\e[1;1H\e[2J");

  return 1;
}





int shell_help(char **args)
{
  int i;
  printf("Stephen Brennan's shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < shell_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}




int shell_exit(char **args)
{
  return 0;
}





int shell_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("shell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("shell");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}





int shell_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < shell_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return shell_launch(args);
}





#define shell_RL_BUFSIZE 1024



char *shell_read_line(void)
{
  int bufsize = shell_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {

    c = getchar();

    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }

    position++;

    if (position >= bufsize) {
      bufsize += shell_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}




#define shell_TOK_BUFSIZE 64
#define shell_TOK_DELIM " \t\r\n\a"




char **shell_split_line(char *line)
{
  int bufsize = shell_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, shell_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += shell_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, shell_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}





void shell_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("\033[0;32marinCLI\033[0m@\033[0;34m%s\033[0;32m$ ", name);
    printf("\033[0m");
    line = shell_read_line();
    args = shell_split_line(line);
    status = shell_execute(args);

    free(line);
    free(args);
  } while (status);
}






int main(int argc, char **argv)
{

  //loads some files
    
  //GETS THE NAME OF THE USER TO BE USED IN THE MAIN LOOP
  FILE *fptr = fopen("name.txt", "r");

  //ERROR CHECKING
  if(!fptr){
      printf("\n\n\nERR: Couldn't open file\n\n\n");
      strcpy(name, "root");
   }

    //If the file can be opened it does this
  else{
      fseek(fptr, SEEK_END, 0);
      ftell(fptr);
      rewind(fptr);
      //reads the name inside the file into a global variable
      fread(name, 25, 1, fptr);
  }

  // Run command loop.
  shell_loop();


  return EXIT_SUCCESS;
}