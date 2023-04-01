#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LINE 1024 // The maximum length command

int should_run = 1;  // flag to determine when to exit program
int should_wait = 1; // flag to determine if process should run in the background

/**
 * Redirects stdin from a file.
 *
 * @param fileName the file to redirect from
 */
void redirectIn(char *fileName)
{
    int in = open(fileName, O_RDONLY);
    dup2(in, 0);
    close(in);
}

/**
 * Redirects stdout to a file.
 *
 * @param fileName the file to redirect to
 */
void redirectOut(char *fileName)
{
    int out = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    dup2(out, 1);
    close(out);
}

/**
 * Runs a command.
 *
 * @param *args[] the args to run
 */
void run(char *args[])
{
    pid_t pid;
    if (strcmp(args[0], "exit") != 0)
    {
        pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Fork Failed");
        }
        else if (pid == 0)
        { /* child process */
            execvp(args[0], args);
        }
        else
        { /* parent process */
            if (should_wait)
            {
                waitpid(pid, NULL, 0);
            }
            else
            {
                should_wait = 0;
            }
        }
        redirectIn("/dev/tty");
        redirectOut("/dev/tty");
    }
    else
    {
        should_run = 0;
    }
}

/**
 * Creates a pipe.
 *
 * @param args [description]
 */
void createPipe(char *args[])
{
    int fd[2];
    pipe(fd);

    dup2(fd[1], 1);
    close(fd[1]);

    printf("args = %s\n", *args);

    run(args);

    dup2(fd[0], 0);
    close(fd[0]);
}

/**
 * Creates a tokenized form of the input with spaces to separate words.
 *
 * @param  *input the input string
 * @return tokenized the tokenized stirng
 */
char *tokenize(char *input)
{
    int i;
    int j = 0;
    char *tokenized = (char *)malloc((MAX_LINE * 2) * sizeof(char));

    // add spaces around special characters
    for (i = 0; i < strlen(input); i++)
    {
        if (input[i] != '>' && input[i] != '<' && input[i] != '|')
        {
            tokenized[j++] = input[i];
        }
        else
        {
            tokenized[j++] = ' ';
            tokenized[j++] = input[i];
            tokenized[j++] = ' ';
        }
    }
    tokenized[j++] = '\0';

    // add null to the end
    char *end;
    end = tokenized + strlen(tokenized) - 1;
    end--;
    *(end + 1) = '\0';

    return tokenized;
}

/**
 * Runs a basic shell.
 *
 * @return 0 upon completion
 */
int main(void)
{
    char *args[MAX_LINE]; // command line arguments

    while (should_run)
    {
        printf("MillerShell$ ");
        fflush(stdout);

        char input[MAX_LINE];
        fgets(input, MAX_LINE, stdin);

        char *tokens;
        tokens = tokenize(input);

        if (tokens[strlen(tokens) - 1] == '&')
        {
            should_wait = 0;
            tokens[strlen(tokens) - 1] = '\0';
        }

        char *arg = strtok(tokens, " ");
        int i = 0;
        while (arg)
        {
            if (*arg == '<')
            {
                redirectIn(strtok(NULL, " "));
            }
            else if (*arg == '>')
            {
                redirectOut(strtok(NULL, " "));
            }
            else if (*arg == '|')
            {
                args[i] = NULL;
                createPipe(args);
                i = 0;
            }
            else
            {
                args[i] = arg;
                i++;
            }
            arg = strtok(NULL, " ");
        }
        args[i] = NULL;

        run(args);
    }
    return 0;
}