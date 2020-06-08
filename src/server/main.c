#include "uchat.h"
#include "server.h"

void *mx_communicate(void *data) {
    t_comm *connection = (t_comm *)data;
    char buff[MX_MAX];
    int socket_fd = connection->socket_fd;
    int connection_fd = connection->connection_fd;
    char *status = connection->status;

    socket_fd = 0;
    free(connection);
    while (1) {
        bzero(buff, MX_MAX);
        read(connection_fd, buff, sizeof(buff));
        if (mx_strcmp(buff, "exit\n") == 0) {
            close(connection_fd);
            *status = 0;
            printf("Connection closed\n");
            pthread_exit(NULL);
        }
        mx_printstr_endl(buff);
        // write(socket_fd, "got it\n", sizeof(char) * mx_strlen("got it\n"));
    }
}

void accept_clients(int socket_fd) {
    int connection_fd;
    unsigned int len;
    struct sockaddr_in cli;
    pthread_t *threads = malloc(sizeof(pthread_t) * MX_MAX_THREADS);
    char *status = malloc(sizeof(char) * MX_MAX_THREADS);;

    mx_memset(status, 0, MX_MAX_THREADS);
    while (1) {
        printf("Ready for new client\n");
        len = sizeof(cli);
        connection_fd = accept(socket_fd, (MX_SA*)&cli, &len);
        if (connection_fd < 0) {
            mx_print_error_endl("Server acccept failed!");
            exit(1);
        }
        else
            mx_thread_manager(&threads, &status, socket_fd, connection_fd);
    }
}

int mx_start_server(int port) {
    // TODO Refactor
    int socket_fd;
    struct sockaddr_in server_address;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        mx_print_error_endl("Socket creation failed!");
        exit(1);
    }
    else
        mx_printstr_endl("Socket successfully created!");

    bzero(&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    if ((bind(socket_fd, (MX_SA*)&server_address, sizeof(server_address))) != 0) {
        mx_print_error_endl("Socket bind failed!");
        exit(1);
    }
    else
        mx_printstr_endl("Socket successfully binded!");

    if ((listen(socket_fd, 5)) != 0) {
        mx_print_error_endl("Listen failed!");
        exit(1);
    }
    else
        mx_printstr_endl("Server listening!");

    accept_clients(socket_fd);

    close(socket_fd);

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        mx_print_error_endl("uchat_server: must take parameter - port to run");
        mx_print_error_endl("usage: ./uchat_server PORT");
        exit(1);
    }
    else {
        if (!mx_check_port(argv[1])) {
            mx_print_error_endl("uchat_server: not valid port");
            exit(1);
        }

        mx_start_server(mx_atoi(argv[1]));
    }

    pthread_exit(NULL);
}