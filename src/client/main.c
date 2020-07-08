#include "uchat.h"

char *mx_json_string_s_file(int id, int num, char *buff, int buf_size) {
    json_object *jobj = json_object_new_object();

    json_object_object_add(jobj, "id", json_object_new_int(id));
    json_object_object_add(jobj, "num", json_object_new_int(num));
    json_object_object_add(jobj, "data", json_object_new_string_len(buff, buf_size));
    json_object_object_add(jobj, "is_last", json_object_new_boolean(buf_size < 1024 ? true : false));

    return (char *)json_object_to_json_string(jobj);
}

// from cient
void mx_send_file_test(int socket_fd, char *path) {
    FILE *file;
    char buffer[1024];
    int b = 1;
    char *json_str;
    int pack_num = 1;

    if (!(file = fopen(path, "r")))
        mx_terminate("open");

    while ((b = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0) {
        // for (unsigned long i = 0; i < sizeof(buffer); i++) {
        //     printf("bytes[%lu] = %d\n", i, (int)buffer[i]);
        // }
        json_str = mx_json_string_s_file(1, pack_num++, buffer, b);
        send(socket_fd, json_str, strlen(json_str), 0);
        mx_strdel(&json_str);
        usleep(1000);
        bzero(buffer, sizeof(buffer));
    }

    if (fclose(file) < 0)
        mx_terminate("close");
}
// {"type": "REG", "name": "BogdanUeban", "passw": "qwerty"}

// {"type": "S_IN", "name": "BogdanUeban", "passw": "qwerty"}


// {"type": "N_GRP", "name": "TEST", "id": [1]}

// {"type": "S_MES", "gid": 1, "did": -1, "uid": 1, "uid2": -1, "msg": "", "time": 3819524, "file": "Makefile"}
// {"type": "S_MES", "gid": 1, "did": -1, "uid": 1, "uid2": -1, "msg": "", "time": 3819524, "file": "photo-1573935146153-f6322e84d1e4.jpeg"}

static void communicate(SSL *ssl, int socket_fd) {
    int bytes_read = 0;
    char buff[MX_MAX];
    int n = 0;

    while(1) {
        mx_show_server_certs(ssl);
        mx_printstr("Enter the string: ");
        while ((buff[n++] = getchar()) != '\n');
        n = 0;

        if (buff[0] == '1') {
            while (1) {
                bytes_read = read(socket_fd, buff, sizeof(buff));
                buff[bytes_read] = '\0';
                mx_printstr_endl(buff);
                char f = getchar();
                if (f == '2')
                    break;
            }
        }

        SSL_write(ssl, buff, strlen(buff));
        // FILE TRANSFER TEST
        json_object *jobj = json_tokener_parse(buff);
        json_object *j_type = NULL;
        json_object_object_get_ex(jobj, "file", &j_type);
        if (j_type) {
            const char *path = json_object_get_string(j_type);
            if (!strcmp(path, "photo-1573935146153-f6322e84d1e4.jpeg"))
                mx_send_file_test(socket_fd, "photo-1573935146153-f6322e84d1e4.jpeg");
        }
        // FILE TRANSFER TEST
        bzero(buff, sizeof(buff));
        // bytes_read = SSL_read(ssl, buff, sizeof(buff));
        // buff[bytes_read] = '\0';
        // mx_printstr_endl(buff);
        // if ((strncmp(buff, "exit", 4)) == 0) {
        //     mx_printstr_endl("Client Exit...");
        //     break;
        // }
    }
}

static int open_connection(char *ip, int port) {
    struct sockaddr_in server_address;
    struct hostent *host = NULL;
    int socket_fd = -1;

    if ((host = gethostbyname(ip)) == NULL) {
        perror(ip);
        exit(MX_FAILURE);
    }
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        mx_print_error_endl("Socket creation failed!");
        exit(MX_FAILURE);
    }
    else
        mx_printstr_endl("Socket successfully created!");
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);
    if (connect(socket_fd, (MX_SA*)&server_address, sizeof(server_address))) {
        close(socket_fd);
        mx_print_error_endl("Connection with the server failed!");
        exit(MX_FAILURE);
    }
    else
        mx_printstr_endl("Connected to the server!");
    return socket_fd;
}

int mx_start_client(char *ip, int port) {
    int socket_fd = -1;
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;

    SSL_library_init();
    ctx = mx_init_client_ctx();
    socket_fd = open_connection(ip, port);
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socket_fd);

    if (SSL_connect(ssl) == MX_SSL_FAILURE)
        ERR_print_errors_fp(stderr);
    else {
        communicate(ssl, socket_fd);
        SSL_free(ssl);
    }
    close(socket_fd);
    SSL_CTX_free(ctx);

    return 0;
}

int main(int argc, char **argv) {
    int status = MX_SUCCESS;

    if (argc < 3) {
        mx_print_error("uchat: must take two parameters -");
        mx_print_error_endl("server IP address and port");
        mx_print_error_endl("usage: ./uchat IP PORT");
        exit(MX_FAILURE);
    }
    else {
        if (!mx_check_ip(strdup(argv[1]))) {
            mx_print_error_endl("uchat: not valid IP address");
            exit(MX_FAILURE);
        }
        if (!mx_check_port(argv[2])) {
            mx_print_error_endl("uchat: not valid port");
            exit(MX_FAILURE);
        }
        // status = mx_init_client(argc, argv);
        mx_start_client(argv[1], mx_atoi(argv[2]));
    }
    return status;
}


// {"type": "REG", "name": "kali", "passw": "qwerty"}
// {"type": "REG", "name": "geralt", "passw": "qwerty"}
// {"type": "REG", "name": "BogdanUeban", "passw": "qwerty"}
// {"type": "REG", "name": "dendi", "passw": "qwerty"}
// {"type": "S_IN", "name": "kali", "passw": "qwerty"}
// {"type": "S_IN", "name": "geralt", "passw": "qwerty"}
// {"type": "S_IN", "name": "BogdanUeban", "passw": "qwerty"}
// {"type": "S_IN", "name": "dendi", "passw": "qwerty"}
// {"type": "S_MES", "gid": -1, "did": -2, "uid": 2, "uid2": 1, "msg": "Hello I'm Geralt", "time": 3819524, "file": ""}
// {"type": "S_OUT", "id": 1}
// {"type": "S_OUT", "id": 2}

// {"type": "N_GRP", "name": "TEST", "id": [1]}
// {"type": "INV", "gid": 1, "uid": [2,3,4]}
// // {"type": "S_MES", "gid": 1, "did": -1, "uid": 1, "uid2": -1, "msg": "Bogdash milash", "time": 3819524, "file": ""}
