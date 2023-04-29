#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "methods.c"

int sigint_count = 0; // variable global para contar las veces que se ha recibido la señal SIGINT

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
    read_history(".myshell_history");
    // Inicializa el historial de comandos
    my_history = history_get_history_state();
    using_history();
    // Establece el límite máximo de comandos en el historial
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
            {
                HIST_ENTRY *comm = history_get(command[strlen(substring) + 1] - '0');
                command = strdup(comm->line);
                add_history(command);
            }
            else
            {
                add_history(command);
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