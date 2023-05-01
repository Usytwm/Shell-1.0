#include "built_in.c"

#define MAX_NUM_ARGUMENTS 50
#define MAX_HISTORY_LENGTH 10
#define MAX_COMMAND_LENGTH 100

int pipes_func(char *(*args)[MAX_NUM_ARGUMENTS], int num_cmds, int background)
{
    int pipes[MAX_COMMAND_LENGTH - 1][2];

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

            if (built_in(args[i], num_cmds, background) == 0)
            {
                exit(EXIT_SUCCESS);
            }
            /*if (execvp(args[i][0], args[i]) == -1)
            {
                return EXIT_FAILURE;
            }*/
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
    int status;
    pid_t pidt = getpid();
    if (!background)
    {
        // Establecer el proceso hijo en primer plano
        set_foreground(pidt);
        waitpid(pidt, &status, WUNTRACED);
        set_background(pidt);
    }
    else
    {
        // Establecer el proceso hijo en segundo plano
        bg_pids[num_bg_pids] = pidt;
        proces[num_bg_pids] = "Pipes";
        num_bg_pids++;
        set_background(pidt);
    }

    return EXIT_SUCCESS;
}

void ended_processes()
{
    while ((pid_gen = waitpid(-1, NULL, WNOHANG)) > 0 || num_bg_pids != 0) // terminar el proceso
    {
        int l = Process(pid_gen);
        if (l == -1)
        {
            l = num_bg_pids - 1;
        }
        printf("[%d] Done -> %s\n", il++, proces[l]);
        fg(l);
        for (int j = num_bg_pids - 1; j > 0; j--)
        {
            bg_pids[j - 1] = bg_pids[j];
            if (j - 2 >= 0)
            {
                proces[j - 2] = proces[j - 1];
            }
        }
    }
}

/**
 * @brief El metodo se encarga de procesar cada comando simple o estandarizado.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param last_null Puntero al comienzo del comando.
 * @param num_arguments Numero de argumentos tokenizados.
 * @param auxiliar1 Esacio donde sera almacenado el comando
 * @param len Indice de auxiliar1
 * @return No devuelve nada
 */
void auxiliar_process(char **arguments, int *last_null, int num_arguments, char **auxiliar1, int *len)
{
    int i = *last_null + 1;     // creo un indice q empieza en el token siguiente al ultimo null
    *last_null = num_arguments; // actualizo el ultimo null como el operador actual
    while (i < num_arguments)   // tomo todo lo que hay entre operador y operador
    {
        auxiliar1[*len] = arguments[i]; // lo agrego a auxiliar
        i++;                            // aumento los indices
        (*len)++;
    }
    auxiliar1[*len] = NULL; // hago null la ultima posicion
}

/**
 * @brief El metodo se encarga de procesar cada comando simple o estandarizado.
 * @param arguments Matriz con la entrada tokenizada del usuario.
 * @param last_null Puntero al comienzo del comando.
 * @param num_arguments Numero de argumentos tokenizados.
 * @return Devuelve 1 si ocurrio algun error y 0 e.o.c.
 */
int tokenized_util(char **arguments, int *last_null, int num_arguments, int background)
{
    char *auxiliar1[MAX_NUM_ARGUMENTS]; // construyo el comando simple que voy a procesar
    int len = 0;                        // creo un indice para ir construyendo auxiliar1

    auxiliar_process(arguments, last_null, num_arguments, auxiliar1, &len);

    int std_status = std_method(auxiliar1, len, background);
    if (std_status == 1)
        std_status = built_in(auxiliar1, len, background);

    return std_status;
}

/**
 * @brief El metodo se encarga de parsear cada comando y ejecutarlo.
 * @param parsed_arguments Matriz con la entrada tokenizada del usuario.
 * @return No devuelve nada.
 */
void tokenized(char *parsed_arguments, int background)
{
    char *operators[] = {"then", "else", "end"
                                         "||",
                         "&&", NULL};
    char *token;
    int piped_len = 0;
    char *piped_args[MAX_NUM_ARGUMENTS][MAX_NUM_ARGUMENTS];
    char *arguments[MAX_NUM_ARGUMENTS]; // Arreglo de punteros a los argumentos del comando
    int num_arguments = 0;              // Inicializar el número de argumentos
    int last_null = -1;
    int or_status = 0;
    int if_status = 0;
    int else_status = 0;
    int else_before_end = 0;
    int end_status = 0;
    int not_else = 0;
    int first_true = 0;
    int piped = 0;
    int piped_out = 0;

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
            int end_pipe = 0;
            int if_flag = 0;
            for (int i = 0; i < 4; i++)
            {
                if (strcmp(token, "if") == 0)
                {
                    last_null++;
                    num_arguments++;           // Incrementar el número de argumentos
                    token = strtok(NULL, " "); // Obtener el siguiente token del comando
                    if_flag = 1;
                }
                else if (strcmp(token, operators[i]) == 0)
                {
                    end_pipe = 1;
                    break;
                }
            }

            if (if_flag == 1)
                continue;
            else if (end_pipe == 1)
            {
                char *args1[MAX_NUM_ARGUMENTS]; // construyo el comando simple que voy a procesar
                int len1 = 0;                   // creo un indice para ir construyendo auxiliar1
                auxiliar_process(arguments, &last_null, num_arguments, args1, &len1);
                for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
                {
                    piped_args[piped_len][i] = args1[i];
                }
                if (pipes_func(piped_args, piped_len, background) == EXIT_SUCCESS)
                {
                    piped_out = 1;
                    piped = 0;
                }
            }
        }

        if (strcmp(token, "|") == 0)
        {
            piped = num_arguments;
            char *args1[MAX_NUM_ARGUMENTS]; // construyo el comando simple que voy a procesar
            int len1 = 0;                   // creo un indice para ir construyendo auxiliar1
            auxiliar_process(arguments, &last_null, piped, args1, &len1);
            for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
            {
                piped_args[piped_len][i] = args1[i];
            }
            piped_len++;
        }
        else if (strcmp(token, "if") == 0)
            last_null++;
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
            if (strcmp(arguments[last_null], "if") == 0)
            {
                if_status += 1; // esto me da cuantos if tengo actualmente
                int std_status;
                if (piped == 0 && piped_out == 1)
                    std_status = 0;
                else
                    std_status = tokenized_util(arguments, &last_null, num_arguments, background);
                else_status = 0;     // actualizo los estados de else
                else_before_end = 0; // para el nuevo bloque de if
                if (std_status == 1) // si hubo un error en la condicion
                {
                    else_status = 1; // actualizo el else para que se ejecute
                }
                else
                {
                    end_status = 1; // actualizo el end para que se ejecute si no hay else
                    first_true = 1;
                }
            }
            else
                printf("then: \"if\" command is missing\n");
        }
        else if (strcmp(token, "else") == 0 && first_true != 2)
        {
            else_before_end = 1;
            if (else_status == 1) // si hubo error en la condicion
            {
                last_null = num_arguments; // last_null ahora es el else
            }
            else // si no hubo error
            {
                int std_status = tokenized_util(arguments, &last_null, num_arguments, background); // ejecuto el comando entre then y else
                not_else = 1;                                                                      // actualizo el estado de no ejecutar lo que hay en el else
                first_true = 2;
            }
        }
        else if (strcmp(token, "end") == 0)
        {
            if (first_true == 2 || (not_else == 1 && if_status > 0)) // si no hay que ejecutar el contenido del else y hubo un if al menos
            {
                last_null = num_arguments; // last_null es el end actual
                if_status -= 1;            // quito un if de los que haya sin finalizar
            }
            else if (else_status == 1 && else_before_end == 1) // si hay que ejecutar el contenido del else
            {
                int std_status = tokenized_util(arguments, &last_null, num_arguments, background); // ejecuto el comando
                else_status = 0;                                                                   // actualizo el else como ejecutado
                if_status -= 1;                                                                    // quito un if de los que haya sin finalizar
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

            if (if_status == 0) // si no quedan if por finalizar
                not_else = 0;   // actualizo el estado de los else

            else_before_end = 0;
        }

        num_arguments++;           // Incrementar el número de argumentos
        token = strtok(NULL, " "); // Obtener el siguiente token del comando
    }
    arguments[num_arguments] = NULL; // Agregar un puntero nulo al final del arreglo de argumentos

    ended_processes();

    if (strcmp(arguments[num_arguments - 1], "&") == 0)
        background = 1;
    else
        background = 0;

    if (background)
    {
        arguments[num_arguments - 1] = '\0';
        num_arguments--;
    }

    if (piped != 0)
    {
        char *args2[MAX_NUM_ARGUMENTS]; // construyo el comando simple que voy a procesar
        int len2 = 0;                   // creo un indice para ir construyendo auxiliar1
        int aux1 = piped;

        auxiliar_process(arguments, &aux1, num_arguments, args2, &len2);

        for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
        {
            piped_args[piped_len][i] = args2[i];
        }

        piped_len++;

        if (pipes_func(piped_args, piped_len, background) == EXIT_SUCCESS)
        {
            // fg(bg_pids[num_bg_pids - 1], 1);
            return;
        }
    }

    if (if_status > 0)
    {
        printf("Fatal error: \"end\" command is missing\n");
        exit(1);
    }

    if (num_arguments > 0 && num_arguments - 1 != last_null) // Ejecuta el ultimo comando si aun no se ha hecho
    {
        tokenized_util(arguments, &last_null, num_arguments, background);
    }
}

/**
 * @brief El metodo se encarga de parsear cada comando y ejecutarlo.
 * @param command Entrada del usuario sin procesar
 * @return No devuelve nada.
 */
void parse_command(char *command)
{
    char *parsed_arguments[MAX_NUM_ARGUMENTS];
    int num_tok = 0;
    int background = 0;

    char *token = strtok(command, ";"); // Obtener el primer token del comando

    while (token != NULL && num_tok < MAX_NUM_ARGUMENTS - 1) // Mientras haya tokens y no se haya alcanzado el número máximo de argumentos
    {
        parsed_arguments[num_tok] = token; // Agregar el token al arreglo de argumentos
        num_tok++;                         // Incrementar el número de argumentos
        token = strtok(NULL, ";");         // Obtener el siguiente token del comando
    }
    parsed_arguments[num_tok] = NULL; // Agregar un puntero nulo al final del arreglo de argumentos

    int index = 0;

    while (parsed_arguments[index] != NULL && index < MAX_NUM_ARGUMENTS - 1)
    {
        tokenized(parsed_arguments[index], background);
        index++;
    }
}