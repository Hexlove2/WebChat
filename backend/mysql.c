#include <mysql/mysql.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool user_search(const char *username)
{   
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    char query[256];
    snprintf(query, sizeof(query), "SELECT username FROM clients WHERE username='%s'", username);

    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return false;
    }

    if (mysql_real_connect(conn, "localhost", "chat", "123456", "ChatClient", 3306, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        return false;
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "SELECT error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "mysql_store_result() failed\n");
        mysql_close(conn);
        return false;
    }    
    return (mysql_num_rows(res) > 0);
}
bool password_check(const char *password)
{

    return true;
}
