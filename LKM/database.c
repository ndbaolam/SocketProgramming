#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
  MYSQL *conn;
  MYSQL_RES *res;
  MYSQL_ROW row;

  const char *server = "127.0.0.1";       // MySQL server running in Docker
  const char *user = "root";              // MySQL root user
  const char *password = "root_password"; // MySQL root password set in Docker
  const char *database = "testdb";        // Database name created in Docker

  // Initialize MySQL connection
  conn = mysql_init(NULL);

  // Connect to the database
  if (conn == NULL || !mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
  {
    fprintf(stderr, "MySQL connection error: %s\n", mysql_error(conn));
    return 1;
  }

  // Execute a query
  if (mysql_query(conn, "SELECT DATABASE()"))
  {
    fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
    return 1;
  }

  // Store the result of the query
  res = mysql_store_result(conn);

  // Print the result
  while ((row = mysql_fetch_row(res)) != NULL)
  {
    printf("Connected to database: %s\n", row[0]);
  }

  // Free the result and close the connection
  mysql_free_result(res);
  mysql_close(conn);

  return 0;
}
