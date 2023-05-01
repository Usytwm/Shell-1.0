#include <signal.h>
#include <readline/readline.h>
#include "parse_command.c"

////////////////////////
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