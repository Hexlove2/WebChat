#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define SMALL_BUF 1024

void handle_request(int client_sock);
void error_handling(char *message);
void send_response(int clnt_sock, const char *message);

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if(argc!=2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
        error_handling("bind() error");

    if(listen(serv_sock, 5)==-1)
        error_handling("listen() error");

    while(1) {
        clnt_adr_sz=sizeof(clnt_adr);
        clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        if(clnt_sock==-1)
            error_handling("accept() error");

        handle_request(clnt_sock);
        close(clnt_sock);
    }

    close(serv_sock);
    return 0;
}

void handle_request(int clnt_sock) {
    char req_line[SMALL_BUF];
    char method[10];
    char content_length[SMALL_BUF];
    char content_type[SMALL_BUF];
    char header[SMALL_BUF];
    int str_len, length = 0;
    
    // Reading the request line
    int idx = 0;
    while (1) {
        char c;
        if (read(clnt_sock, &c, 1) <= 0) return;
        req_line[idx++] = c;
        if (c == '\n') break;
    }
    req_line[idx] = 0;

    // Parsing the HTTP method from the request line
    sscanf(req_line, "%s", method);
    printf("ssssssss \n");
    printf("%s\n",req_line);
    printf("ssssssss \n");
    // Handling the OPTIONS request (preflight request for CORS)
    if (strcmp(method, "OPTIONS") == 0) {
        char response[] =
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "\r\n";
        write(clnt_sock, response, sizeof(response) - 1);
        printf("CORS SUCCESS \n");
        return;
    }
    
    // Handling only POST requests
    if (strcmp(method, "POST") != 0) {
        send_response(clnt_sock, "HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        return;
    }

    // 逐行读取请求头
    while (1) {
        idx = 0;
        while (1) {
            char c;
            if (read(clnt_sock, &c, 1) <= 0) return;
            header[idx++] = c;
            if (c == '\n') break;
        }
        header[idx] = 0;
        if (strcmp(header, "\r\n") == 0) break;

        printf("Header: %s", header);

        if (strstr(header, "Content-Length") != NULL)
            sscanf(header, "Content-Length: %d", &length);
    }

    if (length == 0) {
        send_response(clnt_sock, "HTTP/1.1 411 Length Required\r\n\r\n");
        return;
    }

    // Reading the body based on Content-Length
    char *body = malloc(length + 1);
    if (body == NULL) {
        send_response(clnt_sock, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
        return;
    }
    int total_read = 0, n;
    while (total_read < length) {
        n = read(clnt_sock, body + total_read, length - total_read);
        if (n <= 0) break;
        total_read += n;
    }
    body[total_read] = '\0';
    printf("Received Body: %s\n", body);

    // Process the received data (e.g., parse JSON, handle login logic, etc.)
    // Placeholder for JSON parsing and login logic

    // Sending the response
    char response[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n"
        "{\"status\":\"success\"}";
    send_response(clnt_sock, response);
    // Free the allocated memory for the body
    free(body);
}

void send_response(int clnt_sock, const char *message) {
    write(clnt_sock, message, strlen(message));
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}