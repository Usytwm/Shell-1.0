#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "methods.c"

#define MAX_COMMAND_LENGTH 100 // Longitud máxima del comando
#define MAX_NUM_ARGUMENTS 10   // Número máximo de argumentos del comando
#define MAX_HISTORY_LENGTH 10

int main()
{
    int hist_count = 0;
    char history[MAX_HISTORY_LENGTH][MAX_COMMAND_LENGTH];

    for (int i = 0; i < MAX_HISTORY_LENGTH; i++)
    {
        for (int j = 0; j < MAX_COMMAND_LENGTH; j++)
        {
            history[i][j] = '\0';
        }
    }

    char *command;                      // Puntero al comando ingresado por el usuario
    char *arguments[MAX_NUM_ARGUMENTS]; // Arreglo de punteros a los argumentos del comando
    char *token;                        // Puntero al token actual
    char *working_directory;            // Puntero al directorio de trabajo actual
    char *background_indicator;         // Puntero al indicador de ejecución en segundo plano
    int num_arguments;                  // Número de argumentos del comando
    int background;                     // Indicador de ejecución en segundo plano
    pid_t pid;                          // Identificador del proceso hijo

    while (1) // Bucle infinito para leer comandos del usuario
    {
        printf("my-prompt $ "); // Imprimir el prompt del shell
        fflush(stdout);         // Limpiar el buffer de salida

        command = calloc(MAX_COMMAND_LENGTH, sizeof(char)); // Asignar memoria para el comando
        fgets(command, MAX_COMMAND_LENGTH, stdin);          // Leer el comando ingresado por el usuario

        command[strlen(command) - 1] = '\0'; // Eliminar el salto de línea final

        if (command[0] != ' ')
        {
            char *substring = "again";
            if (strncmp(command, substring, strlen(substring)) == 0)
            {
                int k1 = command[strlen(substring) + 1] - '0';
                int k = hist_count - k1;
                for (int j = hist_count; j > 0; j--)
                {
                    strcpy(history[j], history[j - 1]);
                }
                strcpy(command, history[k + 1]);
                // command = history[k + 1];
                strcpy(history[0], command);
                if (hist_count < MAX_HISTORY_LENGTH)
                {
                    hist_count++;
                }
            }
            else
            {
                for (int j = hist_count; j > 0; j--)
                {
                    strcpy(history[j], history[j - 1]);
                }
                strcpy(history[0], command);

                if (hist_count < MAX_HISTORY_LENGTH)
                {
                    hist_count++;
                }
            }
        }

        num_arguments = 0; // Inicializar el número de argumentos
        background = 0;    // Inicializar el indicador de ejecución en segundo plano

        token = strtok(command, " "); // Obtener el primer token del comando

        while (token != NULL && num_arguments < MAX_NUM_ARGUMENTS - 1) // Mientras haya tokens y no se haya alcanzado el número máximo de argumentos
        {
            if (strcmp(token, "&") == 0) // Si el token es el indicador de ejecución en segundo plano
            {
                background = 1; // Establecer el indicador de ejecución en segundo plano
            }
            else if (token[0] == '#')
            {
                break;
            }
            else // Si el token es un argumento del comando
            {
                arguments[num_arguments] = token; // Agregar el token al arreglo de argumentos
                num_arguments++;                  // Incrementar el número de argumentos
            }

            token = strtok(NULL, " "); // Obtener el siguiente token del comando
        }
        arguments[num_arguments] = NULL; // Agregar un puntero nulo al final del arreglo de argumentos

        if (num_arguments > 0) // Si se ingresó un comando
        {
            int std_status = std_method(arguments, num_arguments, history, background);
            if (std_status == 1)
                built_in(arguments, num_arguments, history, background);
        }

        free(command); // Liberar la memoria del comando
    }

    return 0; // Salir del programa con un estado de finalización de 0
}