
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "cJSON.h"
#include "cjson_helper.h"

// gcc -ggdb -std=c99 -o cjson_client cjson_client.c cJSON.c -lm

void get_operation_config(cJSON *parent) {
  const char id[] = "id";
  char *o_uuid = cJSON_GetObjectItem(parent, id)->valuestring;
  const char status[] = "status";
  char *o_status = cJSON_GetObjectItem(parent, status)->valueint;

  printf("id: %s, status: %d\n", o_uuid, o_status);
}

void build_config(cJSON* root) {
  const char operation[] = "operation";
  cJSON *operation_json = cJSON_GetObjectItem(root, operation);
  if (operation_json) {
    get_operation_config(operation_json);
  } else {
    printf("empty operation json\n");
  }
    
  char *out = cJSON_Print(root);
  printf("%s\n",out);
  free(out);
}

const char method[] = "method";
void build_single_sql_json(cJSON *root) {
  const char sql_type[] = "normalStatSQL";
  cJSON_AddStringToObject(root, method, sql_type);
}

void build_batch_sql_json(cJSON *root) {
  const char sql_type[] = "batchStatSQL";
  cJSON_AddStringToObject(root, method, sql_type);

  // todo: const char *strings[7]
  cJSON *sql_arr =
    cJSON_CreateStringArray((const char **)(stat->sql), stat->sql_cnt[0]);
  cJSON_AddItemToObject(root, sql, sql_arr);
}

void build_sql_stat_json(cJSON *root) {
  const char driverId[] = "driverid";
  const int driverid = 0;
  cJSON_AddNumberToObject(root, driverId, driverid); // add driverId

  const char operationId[] = "operationid";
  const char *uuid = "";
  cJSON_AddStringToObject(root, operationId, uuid);
}

int main() {
  char filename[] = "test.json";
  char msg[100] = "Hello!I have read this file.";
  int fd = open(filename, O_RDONLY);
  if (fd <= 0) {
    fprintf(stderr, "Could not open file: %s\n", filename);
    return;
  }

  char buf[1024];
  size_t len = read(fd, buf, sizeof(buf));
  if (len <= 0) {
    fprintf(stderr, "Could not open file: %s\n", filename);
    return;
  }

  printf("read from file: %s\n", buf);
  close(fd);

  return 0;
}

