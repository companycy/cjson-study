
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

// todo: memory issue
// todo: build parameters in sql_stat

void get_operation_config(cJSON *parent, struct operation_config *config) {
  if (parent == NULL || config == NULL) {
    printf("json root is null or config is null in get_operation_config\n");
    return;
  }

  const char id[] = "id";
  config->uuid = cJSON_GetObjectItem(parent, id)->valuestring;
  const char status[] = "status";
  config->status = cJSON_GetObjectItem(parent, status)->valueint;

  printf("id: %s, status: %d\n", config->uuid, config->status);
}

void get_server_config(cJSON *parent, struct server_config *config) {
  if (parent == NULL || config == NULL) {
    printf("json root is null or config is null in get_server_config\n");
    return;
  }

  const char url[] = "url";
  config->url = cJSON_GetObjectItem(parent, url)->valuestring;
  const char driver[] = "driver";
  config->driver = cJSON_GetObjectItem(parent, driver)->valuestring;
  const char user[] = "user";
  config->user = cJSON_GetObjectItem(parent, user)->valuestring;
  const char passwd[] = "password";
  config->passwd = cJSON_GetObjectItem(parent, passwd)->valuestring;

  printf("url: %s, user: %s, driver: %s, passwd: %s\n",
	 config->url, config->user, config->driver, config->passwd);
}

void get_queue_config(cJSON *parent, struct queue_config *config) {
  if (parent == NULL || config == NULL) {
    printf("json root is null or config is null in get_queue_config\n");
    return;
  }

  const char host[] = "host";
  config->host = cJSON_GetObjectItem(parent, host)->valuestring;

  // unused so far
  // char *p = strchr(config->host, ':');
  // if (p) {
  //   config->port = atoi(p);
  // }

  const char topic[] = "topic";
  config->topic = cJSON_GetObjectItem(parent, topic)->valuestring;
  const char key[] = "key";
  config->key = cJSON_GetObjectItem(parent, key)->valuestring;

  printf("host: %s, port: %d, topic: %s, key: %s\n",
	 config->host, config->port, config->topic, config->key);
}

void build_config(cJSON* root, struct operation_config *o_config,
		  struct server_config *master_config, struct server_config *slave_config,
		  struct queue_config *q_config) {
  const char operation[] = "operation";
  cJSON *operation_json = cJSON_GetObjectItem(root, operation);
  if (operation_json) {
    // printf("parse operation json finish\n");
    // struct operation_config o_config = {0};
    get_operation_config(operation_json, o_config);
    // printf("get_operation_config finish\n");
  } else {
    printf("empty operation json\n");
  }

  const char master[] = "master";
  cJSON *master_json = cJSON_GetObjectItem(root, master);
  if (master_json) {
    // printf("parse master json finish\n");
    // struct server_config master_config = {0};
    get_server_config(master_json, master_config);
    // printf("get_master_server_config finish\n");
  } else {
    printf("empty master json\n");
    // return;
  }

  const char slave[] = "slave";
  cJSON *slave_json = cJSON_GetObjectItem(root, slave);
  if (slave_json) {
    // printf("parse slave json finish\n");
    // struct server_config slave_config = {0};
    get_server_config(slave_json, slave_config);
    // printf("get_slave_server_config finish\n");
  } else {
    printf("empty slave json\n");
    // return;
  }

  const char queue[] = "queue";
  cJSON *queue_json = cJSON_GetObjectItem(root, queue);
  if (queue_json) {
    // printf("parse queue json finish\n");
    // struct queue_config q_config = {0};
    get_queue_config(queue_json, q_config);
    // printf("get_queue_config finish\n");
  } else {
    printf("empty queue json\n");
    // return;
  }
    
  char *out = cJSON_Print(root);
  printf("%s\n",out);
  free(out);
}

void build_single_sql_json(const struct sql_stat *stat, cJSON *root) {
  const char sql_type[] = "normalStatSQL";
  cJSON_AddStringToObject(root, method, sql_type);

  cJSON_AddStringToObject(root, sql, (char*)(stat->sql[0]));
}

void build_batch_sql_json(const struct sql_stat *stat, cJSON *root) {
  const char sql_type[] = "batchStatSQL";
  cJSON_AddStringToObject(root, method, sql_type);

  // todo: const char *strings[7]
  cJSON *sql_arr =
    cJSON_CreateStringArray((const char **)(stat->sql), stat->sql_cnt[0]);
  cJSON_AddItemToObject(root, sql, sql_arr);
}

void build_single_prepare_sql_json(const struct sql_stat *stat, cJSON *root) {
  const char sql_type[] = "normalPrepareSQL";
  cJSON_AddStringToObject(root, method, sql_type);

  cJSON_AddStringToObject(root, sql, (char*)(stat->sql[0]));

  const char *str[512] = {0};	// prepare to call cJSON_CreateStringArray
  for (int i = 0; i < stat->parameters_cnt[0]; ++i) {
    *(str+i) = stat->parameters[i];
  }

  cJSON *paramter_arr =
    cJSON_CreateStringArray(str, stat->parameters_cnt[0]);
    // cJSON_CreateStringArray((const char **)(stat->parameters), stat->parameters_cnt[0]);
  cJSON_AddItemToObject(root, paramter, paramter_arr);
}

void build_batch_prepare_sql_json(const struct sql_stat *stat, cJSON *root) {
  const char sql_type[] = "batchPrepareSQL";
  cJSON_AddStringToObject(root, method, sql_type);

  cJSON *sql_arr = cJSON_CreateStringArray((const char **)(stat->sql), stat->sql_cnt[0]);
  cJSON_AddItemToObject(root, sql, sql_arr);

  // todo:
  // for (int i = 0; i < stat->parameter_cnt; ++i) {
  //   cJSON *paramter_arr = cJSON_CreateStringArray((const char **)stat->paramter, stat->parameter_cnt[i]);
  //   cJSON_AddItemToObject(stat, paramter, paramter_arr);
  // }
}

void build_sql_stat_json(const struct sql_stat *stat,
			 const struct installer_config *i_config,
			 const struct operation_config *o_config,
			 cJSON *root) {

  // char *sql_type;
  // switch (stat->type) {
  // case NORMAL_STAT_SQL: sql_type = "normalStatSQL"; break;
  // case BATCH_STAT_SQL: sql_type = "batchStatSQL"; break;
  // case NORMAL_PREPARE_SQL: sql_type = "normalPrepareSQL"; break;
  // case BATCH_PREPARE_SQL: sql_type = "batchPrepareSQL"; break;
  // default: break;
  // }

  const char driverId[] = "driverid";
  cJSON_AddNumberToObject(root, driverId, i_config->driverid); // add driverId

  const char operationId[] = "operationid";
  cJSON_AddStringToObject(root, operationId, o_config->uuid);

  switch (stat->type) {
  case NORMAL_STAT_SQL: build_single_sql_json(stat, root); break;
  case BATCH_STAT_SQL: build_batch_sql_json(stat, root); break;
  case NORMAL_PREPARE_SQL: build_single_prepare_sql_json(stat, root); break;
  case BATCH_PREPARE_SQL: build_batch_prepare_sql_json(stat, root); break;
  default: break;
  }

  const char sqlid[] = "sqlid";
  cJSON_AddStringToObject(root, sqlid, stat->sqlid);

  const char mtime[] = "mtime";
  cJSON_AddNumberToObject(root, mtime, stat->mtime);

  const char stime[] = "stime";
  cJSON_AddNumberToObject(root, stime, stat->stime);

  if (stat->is_iud) {
    const char m_update_cnt[] = "mupdatecnt";
    cJSON_AddNumberToObject(root, m_update_cnt, stat->m_update_cnt);

    // const char s_update_cnt[] = "supdatecnt";
    // cJSON_AddNumberToObject(root, m_update_cnt, stat->m_update_cnt);
  }

  const char time[] = "time";
  cJSON_AddNumberToObject(root, time, stat->timestamp);
}

void test() {
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
  // build_config(buf);
  close(fd);
}

