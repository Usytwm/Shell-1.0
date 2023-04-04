#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include <stdbool.h>
#include <io.h>
#define MAX_LINE_LENGTH 1024
#include <Windows.h>
#include <tchar.h>
#include <direct.h>

#include <sys/types.h>

#define MAX_CMD_LENGTH 1024

int main()
{
    char cmd[MAX_CMD_LENGTH];       // cadena de comando
    char *args[MAX_CMD_LENGTH / 2]; // argumentos de la cadena de comando
    int i, j, num_args;
    int status;

    while (1)
    {
        printf("my-prompt $ ");

        // leer la cadena de comando
        fgets(cmd, MAX_CMD_LENGTH, stdin);

        // eliminar el salto de línea final
        cmd[strlen(cmd) - 1] = '\0';

        // partir la cadena de comando en argumentos
        args[0] = strtok(cmd, " ");
        num_args = 0;

        if (args[0] != NULL)
        {
            if (strcmp(args[0], "cd") == 0)
            {
                while (args[num_args] != NULL)
                {
                    num_args++;
                    args[num_args] = strtok(NULL, "");
                }
                if (num_args == 1)
                {
                    // si no se proporciona un directorio, cambiar al directorio de inicio del usuario
                    chdir(getenv("HOME"));
                }
                else
                {
                    // cambiar al directorio especificado
                    chdir(args[1]);
                }
                continue;
            }
            else if (strcmp(args[0], "pwd") == 0)
            {
                // obtener el directorio actual
                char cwd[MAX_CMD_LENGTH];
                getcwd(cwd, sizeof(cwd));

                // imprimir el directorio actual
                printf("%s\n", cwd);
            }
            else if (strcmp(args[0], "ls") == 0)
            {
                // obtener el directorio actual
                char cwd[MAX_CMD_LENGTH];
                getcwd(cwd, sizeof(cwd));

                // abrir el directorio actual
                WIN32_FIND_DATA ffd;
                HANDLE hFind = FindFirstFile("*", &ffd);

                if (INVALID_HANDLE_VALUE == hFind)
                {
                    fprintf(stderr, "Error: no se pudo abrir el directorio actual\n");
                    continue;
                }

                // imprimir el contenido del directorio actual
                do
                {
                    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        printf("%s/\n", ffd.cFileName);
                    }
                    else
                    {
                        printf("%s\n", ffd.cFileName);
                    }
                } while (FindNextFile(hFind, &ffd) != 0);

                // cerrar el directorio actual
                FindClose(hFind);
            }
            else if (strcmp(args[0], "exit") == 0)
            {
                exit(0);
            }
            else
            {
                while (args[num_args] != NULL)
                {
                    num_args++;
                    args[num_args] = strtok(NULL, " ");
                }
                // crear proceso hijo para ejecutar el comando
                PROCESS_INFORMATION pi;
                STARTUPINFO si;

                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));

                // construir la cadena de argumentos para CreateProcess()
                char cmd_args[MAX_CMD_LENGTH];
                sprintf(cmd_args, "%s", args[0]);
                for (i = 1; i < num_args; i++)
                {
                    sprintf(cmd_args + strlen(cmd_args), " %s", args[i]);
                }

                // crear el proceso hijo
                if (!CreateProcess(NULL, cmd_args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
                {
                    fprintf(stderr, "Error: no se pudo crear el proceso hijo\n");
                    continue;
                }

                // esperar a que el proceso hijo termine
                WaitForSingleObject(pi.hProcess, INFINITE);

                // obtener el estado de salida del proceso hijo
                GetExitCodeProcess(pi.hProcess, (LPDWORD)&status);

                // cerrar los identificadores de proceso y de hilo del proceso hijo
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                // imprimir el estado de salida del proceso hijo
                printf("El proceso hijo con PID %d terminó con estado %d\n", pi.dwProcessId, status);
            }
        }
    }
    return 0;
}
