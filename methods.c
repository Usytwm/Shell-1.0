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
#define MAX_NUM_ARGUMENTS 50
#define MAX_COMMAND_LENGTH 100

/* Arreglo de punteros a cadenas de caracteres para las variables */
char *vars[MAX_NUM_ARGUMENTS];

/* Arreglo de punteros a cadenas de caracteres para los valores de las variables */
char *values[MAX_NUM_ARGUMENTS];

/* Número de variables definidas */
int num_vars = 0;

/* Función para definir una nueva variable */
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
        FILE *archivo;
        char linea[100];

        archivo = fopen("help", "r");

        if (archivo == NULL) {
            printf("Error: no se pudo abrir el archivon");
            return 1;
        }

        while (fgets(linea, 100, archivo) != NULL) {
            printf("%s", linea);
        }

        fclose(archivo);

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
 * @brief El metodo se encarga de procesar cada comando simple o estandarizado.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param last_null Puntero al comienzo del comando.
 * @param num_arguments Numero de argumentos tokenizados.
 * @param background Estado de segundo plano del proceso actual.
 * @return Devuelve 1 si ocurrio algun error y 0 e.o.c.
*/
int tokenized_util(char **arguments, int *last_null, int num_arguments, int background)
{
    char *auxiliar1[MAX_NUM_ARGUMENTS]; //construyo el comando simple que voy a procesar
    int i = *last_null + 1; //creo un indice q empieza en el token siguiente al ultimo null
    int len = 0; //creo un indice para ir construyendo auxiliar1
    *last_null = num_arguments; //actualizo el ultimo null como el operador actual
    while (i < num_arguments) //tomo todo lo que hay entre operador y operador
    {
        auxiliar1[len] = arguments[i]; //lo agrego a auxiliar 
        i++; //aumento los indices
        len++;
    }
    auxiliar1[len] = NULL; //hago null la ultima posicion

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
    int fd[2];
    pid_t pid;
    char *operators[] = {"|", "||", "&&", NULL};
    char *token;
    int status = 0;
    char *arguments[MAX_NUM_ARGUMENTS]; // Arreglo de punteros a los argumentos del comando
    int num_arguments = 0; // Inicializar el número de argumentos
    int last_null = -1;
    int or_status = 0;
    int if_status = 0;
    int else_status = 0;
    int else_before_end = 0;
    int end_status = 0;
    int not_else = 0;
    int first_true = 0;
    int piped = 0;

    token = strtok(parsed_arguments, " "); // Obtener el primer token del comando

    while (token != NULL && num_arguments < MAX_NUM_ARGUMENTS - 1) // Mientras haya tokens y no se haya alcanzado el número máximo de argumentos
    {
        if (or_status == 1)
        {
            return;
        }
        
        arguments[num_arguments] = token; // Agregar el token al arreglo de argumentos
        
        if (piped != 0)
        {
            int init = 0;
            int i = 0;
            while (operators[i] != NULL)
            {
                if (strcmp(token, operators[i]) == 0)
                {
                    init = 1;
                    break;
                }
                i++;
            }
            
            if (init == 1)
            {
                if(pipe(fd) == -1)
                {
                    fprintf(stderr, "Error creating pipe\n");
                    return;
                }
                pid = fork();
                if (pid < 0)
                {
                    fprintf(stderr, "Error forking process\n");
                    return;
                }
                else if (pid == 0)
                {
                    int aux = piped;
                    // Child process (command 2)
                    close(fd[1]); // Close unused write end
                    dup2(fd[0], STDIN_FILENO); // Redirect stdin to read end of pipe
                    close(fd[0]); // Close read end of pipe 
                    int std_status = tokenized_util(arguments, &aux, num_arguments, background);
                    fprintf(stderr, "Error executing command 2\n");
                }
                else
                {
                    // Parent process (command 1)
                    close(fd[0]); // Close unused read end
                    dup2(fd[1], STDOUT_FILENO); // Redirect stdout to write end of pipe
                    close(fd[1]); // Close write end of pipe
                    int std_status = tokenized_util(arguments, &last_null, piped, background);
                    fprintf(stderr, "Error executing command 1\n");
                }
                piped = 0;
            }
        }

        if (strcmp(token, "|") == 0)
        {
            piped = num_arguments;
        }
        else if (strcmp(token, "&&") == 0)
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
        else if (strcmp(token, "then") == 0 && first_true == 0)
        {
            if (strcmp(arguments[last_null + 1], "if") == 0)
            {
                if_status += 1; //esto me da cuantos if tengo actualmente
                last_null = last_null + 1; //actualizo el ultimo null
                int std_status = tokenized_util(arguments, &last_null, num_arguments, background);
                else_status = 0; //actualizo los estados de else 
                else_before_end = 0; //para el nuevo bloque de if
                if (std_status == 1) //si hubo un error en la condicion
                {
                    else_status = 1; //actualizo el else para que se ejecute
                }
                else
                {
                    end_status = 1; //actualizo el end para que se ejecute si no hay else
                    first_true = 1;
                }
            }
            else
                printf("then: \"if\" command is missing\n");
        }
        else if (strcmp(token, "else") == 0 && first_true != 2)
        {
            else_before_end = 1;
            if (else_status == 1) //si hubo error en la condicion
            {
                last_null = num_arguments; //last_null ahora es el else
            }
            else //si no hubo error
            {
                int std_status = tokenized_util(arguments, &last_null, num_arguments, background); //ejecuto el comando entre then y else
                not_else = 1; //actualizo el estado de no ejecutar lo que hay en el else
                first_true = 2;
            }
        }
        else if (strcmp(token, "end") == 0)
        {
            if (first_true == 2 || (not_else == 1 && if_status > 0)) //si no hay que ejecutar el contenido del else y hubo un if al menos
            {
                last_null = num_arguments; //last_null es el end actual
                if_status -= 1; //quito un if de los que haya sin finalizar
            }
            else if (else_status == 1 && else_before_end == 1) //si hay que ejecutar el contenido del else
            {
                int std_status = tokenized_util(arguments, &last_null, num_arguments, background); //ejecuto el comando
                else_status = 0; //actualizo el else como ejecutado
                if_status -= 1; //quito un if de los que haya sin finalizar
            }
            else if (end_status == 1 && else_before_end == 0 && strcmp(arguments[last_null], "end") != 0)
            {
                int std_status = tokenized_util(arguments, &last_null, num_arguments, background);
                if_status -= 1;
            }
            else
            {
                last_null = num_arguments;
                if_status -= 1;
            }

            if (if_status == 0) //si no quedan if por finalizar
                not_else = 0; //actualizo el estado de los else
            
            else_before_end = 0;
        }

        num_arguments++; // Incrementar el número de argumentos
        token = strtok(NULL, " "); // Obtener el siguiente token del comando
    }
    arguments[num_arguments] = NULL; // Agregar un puntero nulo al final del arreglo de argumentos

    if (piped != 0)
    {
        if (pipe(fd) == -1)
        {
            fprintf(stderr, "Error creating pipe\n");
            return;
        }
        pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Error forking process\n");
            return;
        }
        else if (pid == 0)
        {
            int aux = piped;
            // Child process (command 2)
            close(fd[1]); // Close unused write end
            dup2(fd[0], STDIN_FILENO); // Redirect stdin to read end of pipe
            close(fd[0]); // Close read end of pipe 
            int std_status = tokenized_util(arguments, &aux, num_arguments, background);
            fprintf(stderr, "Error executing command 2\n");
        }
        else
        {
            // Parent process (command 1)
            close(fd[0]); // Close unused read end
            dup2(fd[1], STDOUT_FILENO); // Redirect stdout to write end of pipe
            close(fd[1]); // Close write end of pipe
            int std_status = tokenized_util(arguments, &last_null, piped, background);
            fprintf(stderr, "Error executing command 1\n");
        }
        return;
    }

    if (if_status > 0)
    {
        printf("Fatal error: \"end\" command is missing\n");
        exit(1);
    }

    if (num_arguments > 0 && num_arguments - 1 != last_null) // Si se ingresó un comando
    {
        tokenized_util(arguments, &last_null, num_arguments, background);
    }
}

void parse_command(char *command)
{
    char *parsed_arguments[MAX_NUM_ARGUMENTS];
    int background = 0; // Inicializar el indicador de ejecución en segundo plano
    int num_tok = 0;
    
    char *token = strtok(command, ";"); // Obtener el primer token del comando

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
}