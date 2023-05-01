#include <signal.h>
#include <readline/readline.h>
#include "parse_command.c"

////////////////////////
int sigint_count = 0; // variable global para contar las veces que se ha recibido la señal SIGINT

/*int Pipes(char input[1024], char *cmds, int num_cmds, int pipes[MAX_CMDS - 1][2])
{
    char *cmd = strtok(input, "|");
    while (cmd != NULL && num_cmds < MAX_CMDS)
    {
        cmds[num_cmds++] = cmd;
        cmd = strtok(NULL, "|");
    }

    // Crear los pipes
    for (int i = 0; i < num_cmds - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Crear los procesos hijos y ejecutar los comandos
    for (int i = 0; i < num_cmds; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Proceso hijo
            if (i > 0)
            {
                // Conectar la entrada estándar con el pipe anterior
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < num_cmds - 1)
            {
                // Conectar la salida estándar con el siguiente pipe
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            // Cerrar los descriptores de archivo de los pipes
            for (int j = 0; j < num_cmds - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            // Ejecutar el comando
            char cmd_copy[MAX_COMMAND_LENGTH];
            strncpy(cmd_copy, cmds[i], sizeof(cmd_copy));
            char *args[MAX_COMMAND_LENGTH];
            int num_args = 0;
            char *arg = strtok(cmd_copy, " ");
            while (arg != NULL && num_args < MAX_COMMAND_LENGTH)
            {
                args[num_args++] = arg;
                arg = strtok(NULL, " ");
            }
            args[num_args] = NULL;
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // Cerrar los descriptores de archivo de los pipes en el proceso padre
    for (int i = 0; i < num_cmds - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Esperar a que todos los procesos hijos terminen
    for (int i = 0; i < num_cmds; i++)
    {
        wait(NULL);
    }

    return EXIT_SUCCESS;
}*/

void sigint_handler(int sig_num)
{
    if (sigint_count == 0)
    {
        sigint_count++;
        printf("\nPresione Ctrl-C nuevamente para salir o Enter para continuar.");
        fflush(stdout);
    }
    else
    {
        printf("\nTerminando el programa.\n");
        exit(EXIT_SUCCESS);
    }
}

int main()
{
    signal(SIGINT, sigint_handler);

    HISTORY_STATE *my_history;
    read_history(".myshell_history"); // Inicializa el historial de comandos
    my_history = history_get_history_state();
    using_history(); // Establece el límite máximo de comandos en el historial
    stifle_history(MAX_HISTORY_LENGTH);

    char *command; // Puntero al comando ingresado por el usuario

    while (1) // Bucle infinito para leer comandos del usuario
    {
        command = calloc(MAX_COMMAND_LENGTH, sizeof(char)); // Asignar memoria para el comando
        command = readline("my-prompt $ ");

        if (command[0] != ' ')
        {
            char *substring = "again";
            if (strncmp(command, substring, strlen(substring)) == 0)
            { ///////////////////////////

                FILE *fp;
                char *line = malloc(strlen(command) + 1);
                fp = fopen(".myshell_history", "r");
                if (fp == NULL)
                {
                    printf("Error al abrir el archivo de historial.\n");
                    return 1;
                }
                int pos = 1;
                while (fgets(line, MAX_COMMAND_LENGTH, fp))
                {
                    if (command[strlen(substring) + 1] - '0' == pos)
                    {
                        memcpy(command, line, strlen(line) + 1);
                        command[strlen(command) - 1] = '\0';
                        break;
                    }
                    pos++;
                }
                fclose(fp);
                ///////////////////////////////
                // HIST_ENTRY *comm = history_get(command[strlen(substring) + 1] - '0');
                // command = strdup(comm->line);
                add_history(command);
                write_history(".myshell_history");
            }
            else
            {
                if (strcmp(command, "") != 0)
                {
                    add_history(command);
                    write_history(".myshell_history");
                }
            }
        }
        for (int i = 0; i < strlen(command); i++)
        {
            if (command[i] == '#')
            {
                for (int j = i; j < strlen(command); j++)
                {
                    command[j] = '\0';
                }
                break;
            }
        }

        parse_command(command);

        free(command); // Liberar la memoria del comando
    }

    return 0; // Salir del programa con un estado de finalización de 0
}