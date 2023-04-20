#include "OwnCmd.c" //comandos propios

// freopen()
int main()
{
  char inputString[MAXCOM], *parsedArgs[MAXLIST];
  char *parsedArgsPiped[MAXLIST];
  int execFlag = 0;
  char cmd[100];
  char *args[MAXLIST + 1]; // +1 para NULL al final
  int nargs = 0;
  char *token;
  init_shell();

  while (1)
  {
    char *args[MAXLIST + 1]; // +1 para NULL al final
    int nargs = 0;
    // print shell line
    printDir();
    // take inputnk
    if (takeInput(inputString))
      continue;
    // process
    execFlag = processString(inputString,
                             parsedArgs, parsedArgsPiped);
    // execflag returns zero if there is no command
    // or it is a builtin command,
    // 1 if it is a simple command
    // 2 if it is including a pipe.

    // execute
    if (execFlag == 1)
      execArgs(parsedArgs);

    if (execFlag == 2)
      execArgsPiped(parsedArgs, parsedArgsPiped);
  }
  return 0;
}

/*int main()
{
  printf("Hola mundon");
  return 0;
}*/
/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>

#include <readline/readline.h>
#include <readline/history.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported
*/
