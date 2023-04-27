#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_HISTORY_LENGTH 10
#define MAX_NUM_ARGUMENTS 10
#define MAX_COMMAND_LENGTH 100

/**
 * @brief El metodo se encarga de revisar si la entrada del usuario corresponde a algun comando.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param num_arguments Numero de argumentos tokenizados.
 * @param history Historial de comandos introducidos.
 * @param background Estado de segundo plano del proceso actual.
 * @return Devuelve 1 si hubo algun error y 0 e.o.c.
 */
int built_in(char **arguments, int num_arguments, int background)
{
    int status; // Estado de finalización del proceso hijo

    if (strcmp(arguments[0], "exit") == 0) // Si el comando es "exit"
    {
        write_history(".myshell_history");
        exit(0); // Salir del shell
    }
    else if (strcmp(arguments[0], "cd") == 0) // Si el comando es "cd"
    {
        if (num_arguments > 1) // Si se especificó un directorio
        {
            if (chdir(arguments[1]) != 0) // Cambiar al directorio especificado
            {
                printf("cd: %s: No such file or directory\n", arguments[1]); // Imprimir un mensaje de error si no se pudo cambiar al directorio
                return 1;
            }
        }
        else // Si no se especificó un directorio
        {
            printf("cd: The function requires an argument\n");
            return 1;
        }
    }
    else if (strcmp(arguments[0], "history") == 0)
    {
        HIST_ENTRY **history_lis = history_list();
        if (history_list != NULL)
        {
            for (int i = 0; history_lis[i] != NULL; i++)
            {
                printf("%d %s\n", i + 1, history_lis[i]->line);
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
                return 1;                                         // Salir del proceso hijo con un estado de finalización de 1
            }
        }
        else if (pid > 0) // Si se está ejecutando en el proceso padre
        {
            if (!background) // Si el comando no se está ejecutando en segundo plano
                waitpid(pid, &status, 0); // Esperar a que el proceso hijo termine
            else
                wait(NULL);
        }
        else // Si no se pudo crear el proceso hijo
        {
            printf("fork: Unable to create child process\n"); // Imprimir un mensaje de error
            return 1;
        }
    }
    return 0;
}

/**
 * @brief El metodo se encarga de revisar si la entrada del usuario contiene >, <, >>.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param num_arguments Numero de argumentos tokenizados.
 * @param history Historial de comandos introducidos.
 * @param background Estado de segundo plano del proceso actual.
 * @return Devuelve 1 si no encontro coincidencias y 0 e.o.c.
 */
int std_method(char **arguments, int num_arguments, int background)
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
            built_in(arguments, num_arguments, background);

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
            built_in(arguments, num_arguments, background);

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

int pipes_util(char **arguments, int num_arguments,
               int background, int input_fd, int output_fd)
{
    int status;
    pid_t pid = fork();

    if (pid == 0)
    {
        // child process
        if (input_fd != STDIN_FILENO)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != STDOUT_FILENO)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        if (num_arguments > 0) // Si se ingresó un comando
        {
            int std_status = std_method(arguments, num_arguments, background);
            if (std_status == 1)
                built_in(arguments, num_arguments, background);
        }

        perror("execvp");
        exit(1);
    }
    else if (pid > 0)
    {
        // parent process
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
    else
    {
        perror("fork failed");
        return -1;
    }
}

int tokenized_util(char **arguments, int *last_null, int num_arguments, int background)
{
    char *auxiliar1[MAX_NUM_ARGUMENTS];
    int i = *last_null + 1;
    int len = 0;
    *last_null = num_arguments;
    while (i < num_arguments)
    {
        auxiliar1[len] = arguments[i];
        i++;
        len++;
    }
    auxiliar1[len] = NULL;

    int std_status = std_method(auxiliar1, len, background);
    if (std_status == 1)
        std_status = built_in(auxiliar1, len, background);
    
    return std_status;
}

void tokenized(char *token, char *parsed_arguments, int background)
{
    int status = 0;
    char *arguments[MAX_NUM_ARGUMENTS]; // Arreglo de punteros a los argumentos del comando
    int num_arguments = 0; // Inicializar el número de argumentos
    int last_null = -1;

    token = strtok(parsed_arguments, " "); // Obtener el primer token del comando

    while (token != NULL && num_arguments < MAX_NUM_ARGUMENTS - 1) // Mientras haya tokens y no se haya alcanzado el número máximo de argumentos
    {
        arguments[num_arguments] = token; // Agregar el token al arreglo de argumentos
        if (strcmp(token, "&&") == 0)
        {
            int std_status = tokenized_util(arguments, &last_null, num_arguments, background);
            if (std_status == 1)
                return;
        }
        if (strcmp(token, "||") == 0)
        {
            int std_status = tokenized_util(arguments, &last_null, num_arguments, background);
            if (std_status == 1)
            {
                
            }
        }
        num_arguments++; // Incrementar el número de argumentos
        token = strtok(NULL, " "); // Obtener el siguiente token del comando
    }
    arguments[num_arguments] = NULL; // Agregar un puntero nulo al final del arreglo de argumentos

    if (num_arguments > 0) // Si se ingresó un comando
    {
        tokenized_util(arguments, &last_null, num_arguments, background);
    }
}