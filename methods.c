#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_HISTORY_LENGTH 10
#define MAX_COMMAND_LENGTH 100

/**
 * @brief El metodo se encarga de revisar si la entrada del usuario corresponde a algun comando.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param num_arguments Numero de argumentos tokenizados.
 * @param history Historial de comandos introducidos.
 * @param background Estado de segundo plano del proceso actual.
 * @return No devuelve nada.
*/
void built_in(char **arguments, int num_arguments, char history[][MAX_COMMAND_LENGTH], int background)
{
    int status; // Estado de finalización del proceso hijo

    if (strcmp(arguments[0], "exit") == 0) // Si el comando es "exit"
    {
        exit(0); // Salir del shell
    }
    else if (strcmp(arguments[0], "cd") == 0) // Si el comando es "cd"
    {
        if (num_arguments > 1) // Si se especificó un directorio
        {
            if (chdir(arguments[1]) != 0) // Cambiar al directorio especificado
            {
                printf("cd: %s: No such file or directory\n", arguments[1]); // Imprimir un mensaje de error si no se pudo cambiar al directorio
            }
        }
        else // Si no se especificó un directorio
        {
            printf("cd: The function requires an argument\n");
        }
    }
    else if (strcmp(arguments[0], "history") == 0)
    {
        int counter = 1;
        printf("COMMANDS HISTORY:\n");
        for (int x = MAX_HISTORY_LENGTH; x >= 0; x--)
        {
            if (strlen(history[x]) > 0)
            {
                printf("%d: %s\n", counter, history[x]);
                counter++;
            }
        }
    }
    else // Si el comando no es "exit" ni "cd"
    {
        pid_t pid = fork(); // Crear un nuevo proceso hijo

        if (pid == 0) // Si se está ejecutando en el proceso hijo
        {
            if (execvp(arguments[0], arguments) == -1) // Ejecutar el comando con los argumentos especificados
            {
                printf("%s: command not found\n", arguments[0]); // Imprimir un mensaje de error si no se pudo ejecutar el comando
                exit(1); // Salir del proceso hijo con un estado de finalización de 1
            }
        }
        else if (pid > 0) // Si se está ejecutando en el proceso padre
        {
            if (!background) // Si el comando no se está ejecutando en segundo plano
            {
                waitpid(pid, &status, 0); // Esperar a que el proceso hijo termine
            }
        }
        else // Si no se pudo crear el proceso hijo
        {
            printf("fork: Unable to create child process\n"); // Imprimir un mensaje de error
        }
    }
}

/**
 * @brief El metodo se encarga de revisar si la entrada del usuario contiene >, <, >>.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param num_arguments Numero de argumentos tokenizados.
 * @param history Historial de comandos introducidos.
 * @param background Estado de segundo plano del proceso actual.
 * @return Devuelve 1 si no encontro coincidencias y 0 e.o.c.
*/
int std_method(char **arguments, int num_arguments, char history[][MAX_COMMAND_LENGTH], int background)
{
    char *in_file, *out_file;
    int append = 0, in_fd, out_fd, status;
    pid_t pid;

    in_file = out_file = NULL;
    for (int i = 0; i < num_arguments; i++)
    {
        if (strcmp(arguments[i], "<") == 0)
        {
            in_file = arguments[i + 1];
            arguments[i] = NULL;
        }
        else if (strcmp(arguments[i], ">") == 0)
        {
            out_file = arguments[i + 1];
            arguments[i] = NULL;
        }
        else if (strcmp(arguments[i], ">>") == 0)
        {
            out_file = arguments[i + 1];
            append = 1;
            arguments[i] = NULL;
        }
    }
    
    if (in_file != NULL)
    {
        in_fd = open(in_file, O_RDONLY);
        if (in_fd == -1)
        {
            perror("open");
            return 1;
        }
        // Crear un proceso hijo
        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(1);
        }
        else if (pid == 0)
        {
            // El código del proceso hijo

            // Redirigir la salida estándar al archivo
            dup2(in_fd, STDIN_FILENO);

            // Ejecutar el comando deseado
            built_in(arguments, num_arguments, history, background);

            // Si el comando falla, imprimir un mensaje de error
            perror("execvp");
            exit(1);
        }
        else
        {
            // El código del proceso padre

            // Esperar a que el proceso hijo termine
            wait(NULL);

            // Cerrar el archivo de salida
            close(in_fd);
        }
    }
    // Abrir el archivo de salida, si es necesario
    else if (out_file != NULL)
    {
        out_fd = open(out_file, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (out_fd == -1)
        {
            perror("open");
            return 1;
        }
        // Crear un proceso hijo
        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(1);
        }
        else if (pid == 0)
        {
            // El código del proceso hijo

            // Redirigir la salida estándar al archivo
            dup2(out_fd, STDOUT_FILENO);

            // Ejecutar el comando deseado
            built_in(arguments, num_arguments, history, background);

            // Si el comando falla, imprimir un mensaje de error
            perror("execvp");
            exit(1);
        }
        else
        {
            // El código del proceso padre

            // Esperar a que el proceso hijo termine
            wait(NULL);

            // Cerrar el archivo de salida
            close(out_fd);
        }
    }
    else
        return 1;
    return 0;
}