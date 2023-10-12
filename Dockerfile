FROM mysql:latest

ENV MYSQL_ROOT_PASSWORD=password
ENV MYSQL_DATABASE=musicDB
ENV MYSQL_USER=username
ENV MYSQL_PASSWORD=password

COPY ./init.sql /docker-entrypoint-initdb.d/init.sql
