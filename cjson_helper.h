#ifndef _CJSON_HELPER_H_
#define _CJSON_HELPER_H_

#include <stdio.h>
#include "cJSON.h"
#include "config_helper.h"

struct operation_config {
  char *uuid;
  int status;
};

struct server_config {
  char *url;
  char *driver;
  char *user;
  char *passwd;
};

struct queue_config {
  char *host;
  int port;
  char *topic;
  char *key;
};

enum sql_type {
  NORMAL_STAT_SQL = 0,
  BATCH_STAT_SQL = 1,
  NORMAL_PREPARE_SQL = 2,
  BATCH_PREPARE_SQL = 3
};

struct sql_stat {
  int type;			  // sql type
  char sqlid[64];		  // unique id
  unsigned char sql[16][512];	  // sql statements
  int sql_cnt[16];		  // count of sql statements
  char parameters[16][512];	  // todo:
  int parameters_cnt[16];	  // count of parameter
  int mtime;			  // time cost for exec in master db
  int stime;			  // time cost for exec in slave db
  int is_iud;			  // indicate if it's insert/update/delete
  long m_update_cnt;		  // row count affected by sql on master
  long s_update_cnt;		  // row count affected by sql on slave
  unsigned long timestamp;        // sql timestamp
};

static const char sql[] = "sql";
static const char method[] = "method";
static const char paramter[] = "para";

void build_config(cJSON* root, struct operation_config *o_config,
		  struct server_config *master_config, struct server_config *slave_config,
		  struct queue_config *q_config);
void build_sql_stat_json(const struct sql_stat *stat,
			 const struct installer_config *i_config,
			 const struct operation_config *o_config,
			 cJSON *root);

#endif // _CJSON_HELPER_H_
