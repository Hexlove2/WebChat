#!/bin/bash

# 获取 MySQL 配置
CFLAGS=$(mysql_config --cflags)
LIBS=$(mysql_config --libs)

# 编译代码
gcc httpserver.c -o hser $CFLAGS $LIBS