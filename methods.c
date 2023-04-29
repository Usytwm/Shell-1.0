#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_HISTORY_LENGTH 10
#define MAX_NUM_ARGUMENTS 10
#define MAX_COMMAND_LENGTH 100
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 80 /* Longitud máxima de la línea de comando */
#define MAX_ARGS 10 /* Número máximo de argumentos */
#define MAX_VARS 10 /* Número máximo de variables */

/* Arreglo de punteros a cadenas de caracteres para las variables */
char *vars[MAX_VARS];

/* Arreglo de punteros a cadenas de caracteres para los valores de las variables */
char *values[MAX_VARS];

/* Número de variables definidas */
int num_vars = 0;

/* Función para definir una nueva variable */
void set(char *name, char *value)
{
    if (num_vars < MAX_VARS)
    {
        vars[num_vars] = strdup(name);
        values[num_vars] = strdup(value);
        num_vars++;
    }
    else
    {
        printf("Error: demasiadas variables definidas\n");
    }
}

/* Función para obtener el valor de una variable */
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

/* Función para eliminar una variable */
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

/*int main(void)
{
    char *args[MAX_ARGS]; /* Arreglo de punteros a cadenas de caracteres
    char line[MAX_LINE];  /* Línea de comando ingresada por el usuario
    int should_run = 1;   /* Bandera para indicar si el shell debe seguir ejecutándose

    while (should_run)
    {
        printf("shell> "); /* Imprime el prompt del shell
        fflush(stdout);    /* Limpia el buffer de salida

        /* Lee la línea de comando ingresada por el usuario
        fgets(line, MAX_LINE, stdin);

        /* Elimina el salto de línea al final de la línea de comando *
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }

        /* Divide la línea de comando en argumentos
        char *token = strtok(line, " ");
        int i = 0;
        while (token != NULL && i < MAX_ARGS - 1)
        {
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        args[i] = NULL; /* Agrega un puntero nulo al final del arreglo de argumentos

        /* Si el comando es "set", define una nueva variable
        if (strcmp(args[0], "set") == 0)
        {
            set(args[1], args[2]);
        }
        else if (strcmp(args[0], "get") == 0)
        { /* Si el comando es "get", obtiene el valor de una variable
            char *value = get(args[1]);
            if (value != NULL)
            {
                printf("%s\n", value);
            }
            else
            {
                printf("Error: variable no definida\n");
            }
        }
        else if (strcmp(args[0], "unset") == 0)
        { /* Si el comando es "unset", elimina una variable
            unset(args[1]);
        }
        else
        {                       /* Si el comando no es "set", "get" o "unset", ejecuta el comando ingresado por el usuario
            pid_t pid = fork(); /* Crea un nuevo proceso hijo
            if (pid == 0)
            {                          /* Proceso hijo
                execvp(args[0], args); /* Ejecuta el comando *
                exit(0);               /* Termina el proceso hijo *
            }
            else if (pid > 0)
            {               /* Proceso padre *
                wait(NULL); /* Espera a que el proceso hijo termine *
            }
            else
            { /* Error al crear el proceso hijo *
                printf("Error al crear el proceso hijo\n");
            }
        }
    }

    return 0;
}
*/
/*void set(char *name, char *value)
{
    if (setenv(name, value, 1) != 0)
    {
        printf("Error al establecer la variable de entorno\n");
    }

    //char *value = getenv(name);
    // environ[]=var
}

char *get(char *name)
{
    char *value = getenv(name);
    if (value == NULL)
    {
        printf("La variable de entorno no existe\n");
    }
    return value;
}

void unset(char *name)
{
    if (unsetenv(name) != 0)
    {
        printf("Error al eliminar la variable de entorno\n");
    }
}
*/
/**
 * @brief El metodo se encarga de revisar si la entrada del usuario corresponde a algun comando.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param num_arguments Numero de argumentos tokenizados.
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
                // p1 = values;
                while ((p1 = strchr(p1, '`')) != NULL)
                {
                    p2 = strchr(p1 + 1, '`');
                    if (p2 == NULL)
                        break;
                    len = p2 - p1 - 1;
                    token = strtok(p1 + 1, "`");
                    count++;
                    printf("%s \n", token);
                    // printf("%s \n", p1 + 1);
                    //p1 = p2 + 1;
                }
                if (count > 0)
                {
                    // char command[] = "ls -l";
                    FILE *fp = popen(token, "r");
                    char *var = malloc(strlen(values) + 1); // asigno memoria para var
                    char *val = malloc(1024);
                    memcpy(val, "", 1024);
                    memcpy(var, values, strlen(values) + 1);
                    while (fgets(var, 100, fp) != NULL)
                    {
                        var[strlen(var)-1]='\0';
                        strcat(val, var);
                        strcat(val, " ");
                    }

                    set(arguments[1], val);
                    
                }
                else
                {
                    set(arguments[1], values);
                }
            }
        }
        else
        {
            for (int i = 0; i < MAX_VARS; i++)
            {
                if (vars[i] == NULL)
                {
                    break;
                }
                printf("%s = %s\n", vars[i], values[i]);
                /* code */
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
    /*else if (strcmp(arguments[0], "ls") == 0)
    {
        DIR *dir;
        struct dirent *ent;

        if ((dir = opendir(".")) != NULL)
        {
            // leer todos los archivos en el directorio actual
            while ((ent = readdir(dir)) != NULL) {
                printf("%s\n", ent->d_name);
            }
            closedir(dir);
        }
        else
        {
            // si no se puede abrir el directorio, imprimir un mensaje de error
            perror("ls");
            return 1;
        }
        return 0;
    }
    else if (strcmp(arguments[0], "pwd") == 0)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("getcwd() error");
            return 1;
        }
        return 0;
    }*/
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
            if (!background)              // Si el comando no se está ejecutando en segundo plano
                waitpid(pid, &status, 0); // Esperar a que el proceso hijo termine
            else
                wait(NULL);
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

/**
 * @brief El metodo se encarga de procesar los pipes en la entrada.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param num_arguments Numero de argumentos tokenizados.
 * @param background Estado de segundo plano del proceso actual.
 * @param input_fd File descriptor de la entrada.
 * @param output_fd File descriptor de la salida.
 * @return
 */
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

/**
 * @brief El metodo se encarga de procesar cada comando simple o estandarizado.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param last_null Puntero al comienzo del comando.
 * @param num_arguments Numero de argumentos tokenizados.
 * @param background Estado de segundo plano del proceso actual.
 * @return Devuelve 1 si ocurrio algun error y 0 e.o.c.
 */
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

/**
 * @brief El metodo se encarga de parsear cada comando y ejecutarlo.
 * @param parsed_arguments Matriz con la entrada tokenizada del usuario.
 * @param background Estado de segundo plano del proceso actual.
 * @return No devuelve nada.
 */
void tokenized(char *parsed_arguments, int background)
{
    char *token;
    int status = 0;
    char *arguments[MAX_NUM_ARGUMENTS]; // Arreglo de punteros a los argumentos del comando
    int num_arguments = 0;              // Inicializar el número de argumentos
    int last_null = -1;
    int or_status = 0;
    int if_status = 0;
    int else_status = 0;
    int end_status = 0;
    int not_else = 0;

    token = strtok(parsed_arguments, " "); // Obtener el primer token del comando

    while (token != NULL && num_arguments < MAX_NUM_ARGUMENTS - 1) // Mientras haya tokens y no se haya alcanzado el número máximo de argumentos
    {
        if (or_status == 1)
        {
            return;
        }

        arguments[num_arguments] = token; // Agregar el token al arreglo de argumentos

        if (strcmp(token, "&&") == 0)
        {
            int std_status = tokenized_util(arguments, &last_null, num_arguments, background);
            if (std_status == 1)
                return;
        }
        else if (strcmp(token, "||") == 0)
        {
            int std_status = tokenized_util(arguments, &last_null, num_arguments, background);
            if (std_status == 0)
            {
                or_status = 1;
            }
        }
        else if (strcmp(token, "then") == 0)
        {
            if (strcmp(arguments[last_null + 1], "if") == 0)
            {
                if_status += 1;
                arguments[last_null + 1] = NULL;
                last_null = last_null + 1;
                int std_status = tokenized_util(arguments, &last_null, num_arguments, background);
                arguments[last_null] = NULL;
                if (std_status == 1)
                {
                    else_status = 1;
                }
            }
            else
                printf("then: \"if\" command is missing\n");
        }
        else if (strcmp(token, "else") == 0)
        {
            if (else_status == 1)
            {
                last_null = num_arguments;
                arguments[last_null] = NULL;
            }
            else
            {
                int std_status = tokenized_util(arguments, &last_null, num_arguments, background);
                arguments[last_null] = NULL;
                not_else = 1;
            }
        }
        else if (strcmp(token, "end") == 0)
        {
            if (not_else == 1 && if_status > 0)
            {
                last_null = num_arguments;
                arguments[last_null] = NULL;
                if_status -= 1;
            }
            else if (else_status == 1)
            {
                int std_status = tokenized_util(arguments, &last_null, num_arguments, background);
                arguments[last_null] = NULL;
                else_status = 0;
                if_status -= 1;
            }
            if (if_status == 0)
                not_else = 0;
        }

        num_arguments++;           // Incrementar el número de argumentos
        token = strtok(NULL, " "); // Obtener el siguiente token del comando
    }
    arguments[num_arguments] = NULL; // Agregar un puntero nulo al final del arreglo de argumentos

    if (if_status > 0)
    {
        printf("Fatal error: \"end\" command is missing\n");
        exit(1);
    }

    if (num_arguments > 0 && arguments[num_arguments - 1] != NULL) // Si se ingresó un comando
    {
        tokenized_util(arguments, &last_null, num_arguments, background);
    }
}