#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "methods.c"

int main()
{
    HISTORY_STATE *my_history;
    read_history(".myshell_history");
    // Inicializa el historial de comandos
    my_history = history_get_history_state();
    using_history();
    // Establece el límite máximo de comandos en el historial
    stifle_history(MAX_HISTORY_LENGTH);

    char *command;                      // Puntero al comando ingresado por el usuario
    char *parsed_arguments[MAX_NUM_ARGUMENTS];
    char *token;                        // Puntero al token actual
    int num_tok;
    int background;                     // Indicador de ejecución en segundo plano
    pid_t pid;                          // Identificador del proceso hijo

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
        

        background = 0; // Inicializar el indicador de ejecución en segundo plano
        num_tok = 0;
        
        token = strtok(command, ";"); // Obtener el primer token del comando

        while (token != NULL && num_tok < MAX_NUM_ARGUMENTS - 1) // Mientras haya tokens y no se haya alcanzado el número máximo de argumentos
        {
            parsed_arguments[num_tok] = token; // Agregar el token al arreglo de argumentos
            num_tok++; // Incrementar el número de argumentos
            token = strtok(NULL, ";"); // Obtener el siguiente token del comando
        }
        parsed_arguments[num_tok] = NULL; // Agregar un puntero nulo al final del arreglo de argumentos

        int index = 0;

        while(parsed_arguments[index] != NULL && index < MAX_NUM_ARGUMENTS - 1)
        {
            tokenized(parsed_arguments[index], background);
            index++;
        }
        free(command); // Liberar la memoria del comando
    }

    return 0; // Salir del programa con un estado de finalización de 0
}