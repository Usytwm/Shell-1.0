#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <readline/history.h>

#define MAX_NUM_ARGUMENTS 50

char *functions[] = {"basic", "multi-pipes", "background", "spaces",
                     "history", "ctrl+c", "chain", "if", "multi-if", "variables"};

/* Arreglo de punteros a cadenas de caracteres para las variables */
char *vars[MAX_NUM_ARGUMENTS];

/* Arreglo de punteros a cadenas de caracteres para los valores de las variables */
char *values[MAX_NUM_ARGUMENTS];

/* Número de variables definidas */
int num_vars = 0;

/* pid de los procesos*/
pid_t pid_gen;

/*para enumear los procesos en background a la hora de imprimirlos*/
int il = 1;

char *proces[MAX_NUM_ARGUMENTS];

/*procesos en segundo plano*/
pid_t bg_pids[MAX_NUM_ARGUMENTS];

/*cantidad de procesos en egundo plano*/
int num_bg_pids = 0;

/**
 * @brief Función para definir una nueva variable
 * @param name Nombre de la variable a definir
 * @param value Valor asignado a la variable
 */
void set(char *name, char *value)
{
    if (num_vars < MAX_NUM_ARGUMENTS)
    {
        vars[num_vars] = strdup(name);
        values[num_vars] = strdup(value);
        num_vars++;
    }
    else
    {
        printf("Error: too many variables\n");
    }
}

/**
 * @brief Función para obtener el valor de una variable
 * @param name Nombre de la variable a obtener
 */
char *get(char *name)
{
    int i;
    for (i = 0; i < num_vars; i++)
    {
        if (strcmp(name, vars[i]) == 0)
        {
            return values[i];
        }
    }
    return "0";
}

/**
 * @brief Función para eliminar una variable
 * @param name Nombre de la variable a eliminar
 */
void unset(char *name)
{
    int i;
    for (i = 0; i < num_vars; i++)
    {
        if (strcmp(name, vars[i]) == 0)
        {
            free(vars[i]);
            vars[i] = NULL;
            free(values[i]);
            values[i] = NULL;
            num_vars--;
            for (; i < num_vars; i++)
            {
                vars[i] = vars[i + 1];
                values[i] = values[i + 1];
            }
            break;
        }
    }
}

/**
 * @brief Establece el proceso con el PID especificado como proceso en primer plano.
 * @param pid El PID del proceso que se moverá al primer plano.
 */
void set_foreground(pid_t pid)
{
    tcsetpgrp(STDIN_FILENO, getpgid(pid));
}

/**
 * @brief Establece el proceso actual como proceso en segundo plano.
 * @param pid El PID del proceso que se moverá al segundo plano.
 */
void set_background(pid_t pid)
{
    tcsetpgrp(STDIN_FILENO, getpgid(getpid()));
}

/**
 * @brief Mueve el proceso con el PID especificado al primer plano o el último proceso en segundo plano al primer plano si pid es 0.
 * @param pid El PID del proceso que se moverá al primer plano o 0 para mover el último proceso en segundo plano al primer plano.
 */
void fg(int pid)
{
    int i;
    pid_t fg_pid;
    char *pro;
    if (pid == 0)
    {
        // Mover el último proceso en segundo plano al primer plano
        if (num_bg_pids > 0)
        {
            fg_pid = bg_pids[num_bg_pids - 1];
            pro = proces[num_bg_pids - 1];
            num_bg_pids--;
            set_foreground(fg_pid);
            waitpid(fg_pid, NULL, WUNTRACED);
            set_background(getpid());
        }
        else
        {
            printf("No hay procesos en segundo plano\n");
        }
    }
    else
    {
        // Mover el proceso con el PID especificado al primer plano
        for (i = 0; i < num_bg_pids; i++)
        {
            if (bg_pids[i] == pid)
            {
                fg_pid = bg_pids[i];
                set_foreground(fg_pid);
                waitpid(fg_pid, NULL, WUNTRACED);
                set_background(getpid());
                bg_pids[num_bg_pids - 1] = 0;
                proces[num_bg_pids] = NULL;
                num_bg_pids--;

                return;
            }
        }
        // printf("No se encontró el proceso con el PID especificado\n");
    }
}

/**
 * Muestra una lista de todos los procesos en segundo plano.
 */
void jobs()
{
    int i;
    for (i = 0; i < num_bg_pids; i++)
    {
        printf("[%d] %s -> %d\n", i, proces[i], bg_pids[i]);
    }
}

int Process(pid_t pid)
{
    for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
    {
        if (bg_pids[i] == pid)
        {
            return i;
        }
    }
    return -1;
}

/**
 * @brief El metodo se encarga de revisar si la entrada del usuario corresponde a algun comando.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param num_arguments Numero de argumentos tokenizados.
 * @return Devuelve 1 si hubo algun error y 0 e.o.c.
 */
int built_in(char **arguments, int num_arguments, int background)
{
    int status; // Estado de finalización del proceso hijo

    if (strcmp(arguments[0], "exit") == 0) // Si el comando es "exit"
    {
        exit(0); // Salir del shell
    }
    else if (strcmp(arguments[0], "true") == 0)
        return 0;
    else if (strcmp(arguments[0], "false") == 0)
        return 1;
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
        return 0;
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
        return 0;
    }
    else if (strcmp(arguments[0], "help") == 0)
    {
        FILE *file;
        char linea[100];
        char route[100];

        if (arguments[1] == NULL)
        {
            sprintf(route, ".help/%s", "help");
            file = fopen(route, "r");
            if (file == NULL)
            {
                printf("Error: file is missing\n");
                return 1;
            }
            while (fgets(linea, 100, file) != NULL)
            {
                printf("%s", linea);
            }
            fclose(file);
            return 0;
        }
        else
        {
            for (int i = 0; i < 10; i++)
            {
                if (strcmp(functions[i], arguments[1]) == 0)
                {
                    sprintf(route, ".help/%s", functions[i]);
                    file = fopen(route, "r");
                    if (file == NULL)
                    {
                        printf("Error: file is missing\n");
                        return 1;
                    }
                    while (fgets(linea, 100, file) != NULL)
                    {
                        printf("%s", linea);
                    }
                    fclose(file);
                    return 0;
                }
            }
        }
        printf("Error: file is missing\n");
        return 1;
    }
    else if (strcmp(arguments[0], "set") == 0)
    {
        if (num_arguments > 1)
        {
            if (num_arguments > 2)
            {
                char *token;
                int i = 3;
                char *values = malloc(strlen(arguments[2]) + 1); // asigno memoria para values
                memcpy(values, arguments[2], strlen(arguments[2]) + 1);
                while (arguments[i] != NULL)
                {
                    char *newc = arguments[i];
                    strcat(values, " ");
                    strcat(values, newc);
                    i++;
                }
                char *p1 = malloc(strlen(values) + 1); // asigno memoria para p1
                char *p2;
                int len;
                int count = 0;
                memcpy(p1, values, strlen(values) + 1);
                while ((p1 = strchr(p1, '`')) != NULL)
                {
                    p2 = strchr(p1 + 1, '`');
                    if (p2 == NULL)
                        break;
                    len = p2 - p1 - 1;
                    token = strtok(p1 + 1, "`");
                    count++;
                }
                if (count > 0)
                {
                    FILE *fp = popen(token, "r");
                    char *var = malloc(strlen(values) + 1); // asigno memoria para var
                    char *val = malloc(1024);
                    memcpy(val, "", 1024);
                    memcpy(var, values, strlen(values) + 1);
                    while (fgets(var, 100, fp) != NULL)
                    {
                        var[strlen(var) - 1] = '\0';
                        strcat(val, var);
                        strcat(val, " ");
                    }

                    set(arguments[1], val);
                    fclose(fp);
                }
                else
                {
                    set(arguments[1], values);
                }
            }
        }
        else
        {
            for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
            {
                if (vars[i] == NULL)
                {
                    break;
                }
                printf("%s = %s\n", vars[i], values[i]);
            }
        }
        return 0;
    }
    else if (strcmp(arguments[0], "get") == 0)
    {
        if (num_arguments > 1)
        {
            char *value = get(arguments[1]);
            if (strcmp(value, "0") == 0)
            {
                printf("%s is missing\n", arguments[1]);
                return 0;
            }
            printf("%s\n", value);
        }
        return 0;
    }
    else if (strcmp(arguments[0], "unset") == 0)
    {
        if (num_arguments > 1)
        {
            unset(arguments[1]);
        }
        return 0;
    }
    else if (strcmp(arguments[0], "jobs") == 0)
    {
        jobs();
    }
    else if (strcmp(arguments[0], "fg") == 0)
    {
        // Mover un proceso en segundo plano al primer plano
        if (num_arguments == 1)
            fg(0);
        else
            fg(atoi(arguments[1]));
    }
    else // Si el comando no es "exit" ni "cd"
    {
        pid_t pid = fork(); // Crear un nuevo proceso hijo

        if (pid == 0) // Si se está ejecutando en el proceso hijo
        {
            if (execvp(arguments[0], arguments) == -1) // Ejecutar el comando con los argumentos especificados
            {
                printf("%s: command not found\n", arguments[0]); // Imprimir un mensaje de error si no se pudo ejecutar el comando
                return 1;                                        // Salir del proceso hijo con un estado de finalización de 1
            }
        }
        else if (pid > 0) // Si se está ejecutando en el proceso padre
        {
            if (!background)
            {
                // Establecer el proceso hijo en primer plano
                set_foreground(pid);
                waitpid(pid, &status, WUNTRACED);
                set_background(pid);
            }
            else
            {
                int i = 1;
                char *process_auxiliar = malloc(100); // asigno memoria para process auxiliar
                memcpy(process_auxiliar, arguments[0], 100);
                while (arguments[i] != NULL)
                {
                    char *newc = arguments[i];
                    strcat(process_auxiliar, " ");
                    strcat(process_auxiliar, newc);
                    i++;
                }
                // Establecer el proceso hijo en segundo plano
                bg_pids[num_bg_pids] = pid;
                proces[num_bg_pids] = process_auxiliar;
                num_bg_pids++;
                set_background(pid);
            }
        }
        else // Si no se pudo crear el proceso hijo
        {
            printf("fork: Unable to create child process\n"); // Imprimir un mensaje de error
            return 1;
        }
        return 0;
    }
    return 1;
}

/**
 * @brief El metodo se encarga de revisar si la entrada del usuario contiene >, <, >>.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param num_arguments Numero de argumentos tokenizados.
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

            // Redirigir la salida estándar al file
            dup2(in_fd, STDIN_FILENO);

            // Ejecutar el comando deseado
            if (built_in(arguments, num_arguments, background) == 1)
            {
                perror("execvp");
            }
            exit(1);
        }
        else
        {
            if (!background)
            {
                set_foreground(pid);
                waitpid(pid, &status, WUNTRACED);
                set_background(pid);
            }
            else
            {

                // Establecer el proceso hijo en segundo plano
                bg_pids[num_bg_pids] = pid;
                proces[num_bg_pids] = "Redirigir salida";
                num_bg_pids++;
                set_background(pid);
            }
        }
    }
    // Abrir el file de salida, si es necesario
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

            // Redirigir la salida estándar al file
            dup2(out_fd, STDOUT_FILENO);

            // Ejecutar el comando deseado
            if (built_in(arguments, num_arguments, background) == 1)
            {
                perror("execvp");
            }
            exit(1);
            // Si el comando falla, imprimir un mensaje de error
        }
        else
        {
            // El código del proceso padre
            if (!background)
            {
                set_foreground(pid);
                waitpid(pid, &status, WUNTRACED);
                set_background(pid);
            }
            else
            {
                // Establecer el proceso hijo en segundo plano
                bg_pids[num_bg_pids] = pid;
                proces[num_bg_pids] = "Redireccion de entrada";
                num_bg_pids++;
                set_background(pid);
            } // Esperar a que el proceso hijo termine
        }
    }
    else
        return 1;
    return 0;
}