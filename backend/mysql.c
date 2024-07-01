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
bool password_check(const char *username, const char *password)
{
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    bool checkresult;

    char query[256];
    snprintf(query,sizeof(query), "SELECT password FROM clients WHERE username='%s'",username);
    conn=mysql_init(NULL);
    if(conn==NULL)
    {
        fprintf(stderr,"mysql_init error\n");
        return false;
    }

    if(mysql_real_connect(conn,"localhost","chat","123456","ChatClient",3306, NULL, 0)==NULL)
    {
        fprintf(stderr,"mysql_real_connect error\n");
        mysql_close(conn);
        return false;
    }

    if(mysql_query(conn,query))
    {
        fprintf(stderr, "Select Error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }

    res=mysql_store_result(conn);
    if(res==NULL)
    {
        fprintf(stderr, "mysql_store_result erro\n");
        mysql_close(conn);
        return false;
    }

    row=mysql_fetch_row(res);
    printf("password from Web:%s\n",password);
    printf("password from Mysql:%s\n",row[0]);
    
    if(!strcmp(row[0],password))
        checkresult = true;
    else
        checkresult = false;
    mysql_free_result(res);
    mysql_close(conn);
    if(checkresult)
      printf("check successed\n");
    else
      printf("check failed\n");
    return checkresult;
}

bool register_mysql(char *username, char *password)
{
    MYSQL *conn;
    char query[256];

    conn=mysql_init(NULL);
    if(conn == NULL)
    {
        fprintf(stderr,"mysql_init error");
        return false;
    }

    if(mysql_real_connect(conn,"localhost", "chat", "123456", "ChatClient",3306, NULL, 0)==NULL)
    {
        fprintf(stderr, "mysql_real_connect error");
        mysql_close(conn);
        return false;
    }

    snprintf(query,sizeof(query),"INSERT INTO clients (username, password) VALUES ('%s', '%s')",username,password);

    if(mysql_query(conn,query))
    {
        fprintf(stderr,"mysql_query error");
        mysql_close(conn);
        return false;
    }
    return true;
}