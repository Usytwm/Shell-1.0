#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXCOM 1000 // Número máximo de letras que se admitirán
#define MAXLIST 100 // Número máximo de comandos que se admitirán
void init_shell();
void printDir();
int takeInput(char *str);
int processString(char *str, char **parsed, char **parsedpipe);
void execArgs(char **parsed);
void execArgsPiped(char **parsed, char **parsedpipe);

// Borrado del shell mediante secuencias de escape
#define clear() printf("\033[H\033[J")

// Shell durante el inicio
void init_shell()
{
    clear();
    char *username = getenv("USER");
    printf("USER is: @%s", username);
    printf("\n");
    sleep(1);
    clear();
}

// Función para tomar entrada
int takeInput(char *str)
{
    char *buf;
    buf = fgets(str, 1024, stdin);
    buf[strlen(buf) - 1] = '\0';
    if (strlen(buf) != 0)
    {
        // add_history(buf);
        strcpy(str, buf); // copiar una cadena de caracteres desde una ubicación de memoria a otra
        return 0;
    }
    else
    {
        return 1;
    }
}

// Función para imprimir el directorio actual.
void printDir()
{
    char cwd[1024];
    char *username = getenv("USER");
    getcwd(cwd, sizeof(cwd));
    printf("@%s-prompt:~%s$ ", username, cwd);
}

// Función donde se ejecuta el comando del sistema
void execArgs(char **parsed)
{
    // Forking a child
    pid_t pid = fork();

    if (pid == -1)
    {
        printf("Failed forking child..");
        return;
    }
    else if (pid == 0)
    {
        if (execvp(parsed[0], parsed) < 0)
        {
            printf("Could not execute command..");
        }
        exit(0);
    }
    else
    {
        // Esperando a que el proceso hijo termine
        wait(NULL);
        return;
    }
}

// Función donde se ejecutan los comandos del sistema canalizado
void execArgsPiped(char **parsed, char **parsedpipe)
{
    // 0 es final de lectura, 1 es final de escritura
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0)
    {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0)
    {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0)
    {
        // hijo 1 ejecutando..
        // Solo necesita escribir al final de la escritura
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0)
        {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    }
    else
    {
        // Ejecución principal
        p2 = fork();

        if (p2 < 0)
        {
            printf("\nCould not fork");
            return;
        }

        // Niño 2 ejecutando..
        // Solo necesita leer al final de lectura
        if (p2 == 0)
        {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0)
            {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        }
        else
        {
            // Padre ejecutando, esperando dos hijos
            wait(NULL);
            wait(NULL);
        }
    }
}

// Comando de ayuda integrado
void openHelp()
{
    puts("\n***WELCOME TO MY SHELL HELP***"
         "\nCopyright @ Suprotik Dey"
         "\n-Use the shell at your own risk..."
         "\nList of Commands supported:"
         "\n>cd"
         "\n>ls"
         "\n>exit"
         "\n>all other general commands available in UNIX shell"
         "\n>pipe handling"
         "\n>improper space handling");

    return;
}

// Función para ejecutar comandos incorporados
int ownCmdHandler(char **parsed)
{
    int NoOfOwnCmds = 4, i, switchOwnArg = 0; // Número de comandos propios
    char *ListOfOwnCmds[NoOfOwnCmds];
    char *username;

    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "hello";

    for (i = 0; i < NoOfOwnCmds; i++)
    {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0)
        {
            switchOwnArg = i + 1;
            break;
        }
    }

    switch (switchOwnArg)
    {
    case 1:
        printf("\nGoodbye\n");
        exit(0);
    case 2:
        chdir(parsed[1]);
        return 1;
    case 3:
        openHelp();
        return 1;
    case 4:
        username = getenv("USER");
        printf("\nHello %s.\nMind that this is "
               "not a place to play around."
               "\nUse help to know more..\n",
               username);
        return 1;
    default:
        break;
    }

    return 0;
}

// Función para encontrar tubería
int parsePipe(char *str, char **strpiped)
{
    int i;
    for (i = 0; i < 2; i++)
    {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }

    if (strpiped[1] == NULL)
        return 0; // Devuelve cero si no se encuentra ninguna tubería.
    else
    {
        return 1;
    }
}

// Función para analizar palabras de comando
void parseSpace(char *str, char **parsed)
{
    int i;

    for (i = 0; i < MAXLIST; i++)
    {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}

int processString(char *str, char **parsed, char **parsedpipe)
{

    char *strpiped[2];
    int piped = 0;

    piped = parsePipe(str, strpiped);

    if (piped)
    {
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedpipe);
    }
    else
    {
        parseSpace(str, parsed);
    }

    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}
