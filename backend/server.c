#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <stdbool.h>
#include <iconv.h>

#define BUF_SIZE 2048
#define MAX_CLNT 256
#define SMALL_BUF 20

void *handle_clnt(void *arg);  // 处理客户端连接的线程函数
void error_handling(char *message);  // 错误处理函数
void send_message(char *msg, int len, int sock);  // 发送消息给所有客户端
void perform_handshake(int sock, const char *req);  // 执行 WebSocket 握手
char *base64_encode(const unsigned char *input, int length);  // Base64 编码函数
int encode_frame(const char *msg, int msg_len, char *encoded_msg, int encoded_msg_len);  // 编码 WebSocket 帧
int decode_frame(char *encoded_msg, int encoded_msg_len, char *msg, int msg_len);  // 解码 WebSocket 帧
int read_full_frame(int sock, char *buffer, int buffer_size);
int convert_encoding(const char *from_encoding, const char *to_encoding, char *input, size_t input_size, char *output, size_t output_size);

int clnt_cnt = 0;  // 当前连接的客户端数量
int clnt_socks[MAX_CLNT];  // 客户端套接字数组
pthread_mutex_t mutx;  // 互斥锁

// 编译命令: gcc -g server.c -o cser -lssl -lcrypto -lpthread -liconv
int main(int argc, char const *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    pthread_t t_id;
    
    pthread_mutex_init(&mutx, NULL);  // 初始化互斥锁

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);  // 创建服务器套接字
    
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(7777);

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind error");  // 绑定地址和端口
    
    if (listen(serv_sock, 5) == -1)
        error_handling("listen error");  // 监听端口
    
    while (1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);  // 接受客户端连接

        // 将套接字添加到客户端套接字数组中
        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);

        // 创建线程处理客户端连接
        pthread_create(&t_id, NULL, handle_clnt, (void *)&clnt_sock);
        pthread_detach(t_id);
        printf("connected client: %d \n", clnt_sock);
    }

    close(serv_sock);  // 关闭服务器套接字
    return 0;
}

void *handle_clnt(void *arg) {
    int sock = *((int *)arg);
    int str_len = 0, i;
    int zero_cnt=0;
    char msg[BUF_SIZE];
    bool handshake_done = false;  // 标志位，标识是否已经完成握手

    while ((str_len = read(sock, msg, sizeof(msg) - 1), MSG_WAITALL) != 0) {
        if(str_len==0)
          zero_cnt++;
        else
          zero_cnt=0;
        if(zero_cnt>5)
          break;
        printf("msg大小为:%lu,str_len大小为:%d \n", sizeof(msg),str_len);
        msg[str_len] = '\0';  // 确保字符串以 null 结尾
        if(!handshake_done)
            printf("收到的原消息为: %s\n", msg);
        if (!handshake_done && strstr(msg, "Upgrade: websocket")) {
            // 检查是否为 WebSocket 握手请求
            printf("开始握手\n");
            perform_handshake(sock, msg);  // 执行握手
            handshake_done = true;
        } else {
            // 解码 WebSocket 帧
            char decoded_msg[BUF_SIZE] = {0};  // 初始化并清空解码消息缓冲区
            int decoded_len = decode_frame(msg, str_len, decoded_msg, sizeof(decoded_msg));

            if (decoded_len > 0) {
                printf("解码后的消息为: %s\n", decoded_msg);
                send_message(decoded_msg, decoded_len, sock);  // 发送消息
            }
        }
        memset(msg, 0, sizeof(msg));  // 清空消息缓冲区
    }
    // 客户端断开连接
    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++) {
        if (clnt_socks[i] == sock) {
            for (int j = i; j < clnt_cnt - 1; j++)
                clnt_socks[j] = clnt_socks[j + 1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(sock);  // 关闭客户端套接字
    return NULL;
}

// 发送消息给所有客户端，除了发送者
void send_message(char *msg, int len, int sock) {
    int i;
    char encoded_msg[BUF_SIZE];

    // 包装消息为JSON格式

    int encoded_len = encode_frame(msg, len, encoded_msg, sizeof(encoded_msg));  // 编码 WebSocket 帧

    pthread_mutex_lock(&mutx);
    for (i = 0; i < clnt_cnt; i++)
        if (clnt_socks[i] != sock) {
            write(clnt_socks[i], encoded_msg, encoded_len);  // 发送消息
        }
    pthread_mutex_unlock(&mutx);      
}

// 错误处理函数
void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

// 执行 WebSocket 握手
void perform_handshake(int sock, const char *req) {
    const char *magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char response[BUF_SIZE];
    char key[256];
    char *key_start = strstr(req, "Sec-WebSocket-Key: ");
    if (key_start) {
        key_start += strlen("Sec-WebSocket-Key: ");
        char *key_end = strstr(key_start, "\r\n");
        if (key_end) {
            strncpy(key, key_start, key_end - key_start);
            key[key_end - key_start] = '\0';

            // 拼接客户端密钥和魔法字符串
            char accept_key[512];
            snprintf(accept_key, sizeof(accept_key), "%s%s", key, magic);
            printf("拼接完成的str为:%s \n",accept_key);

            // 计算 SHA1 哈希
            unsigned char hash[SHA_DIGEST_LENGTH];
            SHA1((unsigned char *)accept_key, strlen(accept_key), hash);

            // Base64 编码
            char *encoded_hash = base64_encode(hash, sizeof(hash));

            snprintf(response, sizeof(response),
                     "HTTP/1.1 101 Switching Protocols\r\n"
                     "Upgrade: websocket\r\n"
                     "Connection: Upgrade\r\n"
                     "Sec-WebSocket-Accept: %s\r\n\r\n",
                     encoded_hash);

            write(sock, response, strlen(response));  // 发送握手响应
            free(encoded_hash);
        }
    }
}

// Base64 编码函数
char *base64_encode(const unsigned char *input, int length) {
    BIO *bmem, *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);

    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    char *buff = (char *)malloc(bptr->length);
    memcpy(buff, bptr->data, bptr->length - 1);
    buff[bptr->length - 1] = 0;

    BIO_free_all(b64);

    return buff;
}

// 编码 WebSocket 帧
int encode_frame(const char *msg, int msg_len, char *encoded_msg, int encoded_msg_len) {
    int idx = 0;
    encoded_msg[idx++] = 0x81;  // FIN + 文本帧

    if (msg_len <= 125) {
        encoded_msg[idx++] = msg_len;
    } else if (msg_len <= 65535) {
        encoded_msg[idx++] = 126;
        encoded_msg[idx++] = (msg_len >> 8) & 0xFF;
        encoded_msg[idx++] = msg_len & 0xFF;
    } else {
        encoded_msg[idx++] = 127;
        for (int i = 7; i >= 0; i--) {
            encoded_msg[idx++] = (msg_len >> (i * 8)) & 0xFF;
        }
    }

    memcpy(encoded_msg + idx, msg, msg_len);
    return idx + msg_len;
}

// 解码 WebSocket 帧
int decode_frame(char *encoded_msg, int encoded_msg_len, char *msg, int msg_len) {
    if (encoded_msg_len < 2) {
        return -1;
    }

    int idx = 0;
    unsigned char byte = encoded_msg[idx++];
    if ((byte & 0x80) != 0x80) {  // 检查 FIN 位，确保这是最后一帧
        return -1;
    }

    byte = encoded_msg[idx++];
    int payload_len = byte & 0x7F;  // 获取负载长度

    if (payload_len == 126) {  // 如果负载长度是 126，那么接下来的两个字节表示实际长度
        if (encoded_msg_len < 4) {
            return -1;
        }
        int high_byte = encoded_msg[idx++];
        int low_byte = encoded_msg[idx++];
        payload_len = (high_byte << 8) | low_byte;
    } else if (payload_len == 127) {  // 如果负载长度是 127，那么接下来的八个字节表示实际长度
        if (encoded_msg_len < 10) {
            return -1;
        }
        payload_len = 0;
        for (int i = 0; i < 8; i++) {
            payload_len = (payload_len << 8) | encoded_msg[idx++];
        }
    }

    if (encoded_msg_len < idx + payload_len) {  // 检查数据长度是否匹配
        return -1;
    }

    bool masked = (byte & 0x80) != 0;  // 检查 MASK 位
    unsigned char masking_key[4] = {0};

    if (masked) {
        for (int i = 0; i < 4; i++) {
            masking_key[i] = encoded_msg[idx++];
        }
    }

    for (int i = 0; i < payload_len; i++) {
        msg[i] = encoded_msg[idx + i] ^ (masked ? masking_key[i % 4] : 0);  // 解码负载数据
    }
    
    msg[payload_len] = '\0';  // 确保消息以空字符结尾
    printf("解码后的消息: %s\n", msg);
    return payload_len;  // 返回负载长度
}

int convert_encoding(const char *from_encoding, const char *to_encoding, char *input, size_t input_size, char *output, size_t output_size) {
    iconv_t cd = iconv_open(to_encoding, from_encoding);
    if (cd == (iconv_t)-1) {
        perror("iconv_open");
        return -1;
    }

    char **pin = &input;
    char **pout = &output;
    size_t inbytesleft = input_size;
    size_t outbytesleft = output_size;

    if (iconv(cd, pin, &inbytesleft, pout, &outbytesleft) == (size_t)-1) {
        perror("iconv");
        iconv_close(cd);
        return -1;
    }

    iconv_close(cd);
    return 0;
}


