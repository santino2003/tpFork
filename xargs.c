#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#ifndef NARGS
#define NARGS 4
#endif

void execute_command(char *cmd, char **args, int count) {
    pid_t pid = fork();
    if (pid == 0) { // Proceso hijo
        // Crear el array de argumentos para execvp
        char *exec_args[NARGS + 2]; // +2: uno para el comando y uno para NULL
        exec_args[0] = cmd;
        for (int i = 0; i < count; i++) {
            exec_args[i + 1] = args[i];
        }
        exec_args[count + 1] = NULL;
        
        execvp(cmd, exec_args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // Proceso padre
        // Esperar a que el hijo termine
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "El comando terminó con error\n");
            exit(EXIT_FAILURE);
        }
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <comando>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *command = argv[1];
    char *buffer = NULL;
    size_t bufsize = 0;
    ssize_t len;
    char *args[NARGS];
    int arg_count = 0;

    while ((len = getline(&buffer, &bufsize, stdin)) != -1) {
        // Eliminar el salto de línea al final del buffer
        if (buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Agregar el argumento al array de argumentos
        args[arg_count++] = strdup(buffer);
        if (arg_count == NARGS) {
            // Ejecutar el comando con el grupo de argumentos
            execute_command(command, args, NARGS);

            // Liberar la memoria de los argumentos
            for (int i = 0; i < NARGS; i++) {
                free(args[i]);
            }

            // Reiniciar el contador de argumentos
            arg_count = 0;
        }
    }

    // Ejecutar los argumentos restantes
    if (arg_count > 0) {
        execute_command(command, args, arg_count);

        // Liberar la memoria de los argumentos restantes
        for (int i = 0; i < arg_count; i++) {
            free(args[i]);
        }
    }

    free(buffer);
    return EXIT_SUCCESS;
}
