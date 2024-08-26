#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int 
main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <n>\n", argv[0]);
        exit(1);
    }

    int n = atoi(argv[1]);
    if (n < 2) {
        fprintf(stderr, "Error: <n> debe ser un número mayor o igual a 2.\n");
        exit(1);
    }

    int pipe_fd[2];
    pipe(pipe_fd);

    pid_t pid = fork();

    if (pid == 0) { // Primer proceso hijo
        close(pipe_fd[1]);  // Cerrar escritura en el hijo

        int p;
        while (1) {
            if (read(pipe_fd[0], &p, sizeof(p)) <= 0) {
                close(pipe_fd[0]);
                exit(0);
            }

            printf("primo %d\n", p);

            int pipe_fd_next[2];
            pipe(pipe_fd_next);

            pid_t pid_next = fork();

            if (pid_next == 0) { // Proceso hijo siguiente
                close(pipe_fd_next[1]);  // Cerrar escritura en el hijo
                pipe_fd[0] = pipe_fd_next[0];  // Continuar con el nuevo pipe
            } else { // Proceso padre
                close(pipe_fd_next[0]);  // Cerrar lectura en el padre
                int num;

                // Leer el resto de los números y filtrar los múltiplos de p
                while (read(pipe_fd[0], &num, sizeof(num)) > 0) {
                    if (num % p != 0) {
                        write(pipe_fd_next[1], &num, sizeof(num));
                    }
                }

                close(pipe_fd_next[1]);  // Cerrar escritura cuando termine
                close(pipe_fd[0]);  // Cerrar el pipe izquierdo
                wait(NULL);  // Esperar al proceso hijo
                break;  // Salir del bucle en el proceso padre
            }
        }
    } else { // Proceso padre principal
        close(pipe_fd[0]);  // Cerrar lectura en el padre

        // Generar la secuencia de números de 2 a n
        for (int i = 2; i <= n; i++) {
            write(pipe_fd[1], &i, sizeof(i));
        }

        close(pipe_fd[1]);  // Cerrar escritura cuando termine
        wait(NULL);  // Esperar al proceso hijo
    }

    return 0;
}