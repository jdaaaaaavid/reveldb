/*
 * =============================================================================
 *
 *       Filename:  xconfig.c
 *
 *    Description:  reveldb configuration parser.
 *
 *        Created:  12/12/2012 10:30:58 PM
 *
 *         Author:  Fu Haiping (forhappy), haipingf@gmail.com
 *        Company:  ICT ( Institute Of Computing Technology, CAS )
 *
 * =============================================================================
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xconfig.h"
#include "cJSON.h"
#include "log.h"

static char *
_load_config_body(const char *filename)
{
    assert(filename != NULL);
    int err = -1;
    long file_len = -1;
    FILE *fp = NULL;
    char *json_buf = NULL;
    
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        LOG_ERROR(("failed to open configuration file: %s", filename));
        return NULL;
    }

    err = fseek(fp , 0, SEEK_END);
    if (err != 0) {
        LOG_ERROR(("failed to seek: %s", strerror(errno)));
        return NULL;
    }

    file_len = ftell(fp);
    
    err = fseek(fp , 0, SEEK_SET);
    if (err != 0) {
        LOG_ERROR(("failed to seek: %s", strerror(errno)));
        return NULL;
    }

    json_buf = (char *)malloc(sizeof(char) * (file_len + 1));
    memset(json_buf, 0, (file_len + 1));
    err = fread(json_buf, 1, file_len, fp);
    if (err != file_len) {
        fclose(fp);
        free(json_buf);
        LOG_ERROR(("failed to read file: %s", filename));
        return NULL;
    }

    fclose(fp);
    return json_buf;
}

static reveldb_config_t *
_init_internal_config(const char *config)
{
    assert(config != NULL);

    cJSON *root = NULL;
	cJSON *server = NULL;
	cJSON *db = NULL;
	cJSON *log = NULL;
	cJSON *iter = NULL;
    reveldb_config_t *reveldb_config = NULL;
    /* config value length */
    size_t config_vlen = -1;

    reveldb_config = (reveldb_config_t *)malloc(sizeof(reveldb_config_t));
    if (reveldb_config == NULL) {
        LOG_ERROR(("failed to make room for reveldb_config_t."));
        return NULL;
    }
    

	root = cJSON_Parse(config);
	if (!root) {
		LOG_ERROR(("parsing json error before: [%s]\n",
                    cJSON_GetErrorPtr()));
        free(reveldb_config);
        return NULL;
	} else {
		server = cJSON_GetObjectItem(root, "server");
        reveldb_server_config_t *server_config =
            (reveldb_server_config_t *)malloc(sizeof(reveldb_server_config_t));
        if (server_config == NULL) {
            LOG_ERROR(("failed to make room for reveldb_server_config_t."));
            free(reveldb_config);
            cJSON_Delete(root);
            return NULL;
        }
        reveldb_db_config_t *db_config =
            (reveldb_db_config_t *)malloc(sizeof(reveldb_db_config_t));
        if (db_config == NULL) {
            LOG_ERROR(("failed to make room for reveldb_db_config_t."));
            free(reveldb_config);
            free(server_config);
            cJSON_Delete(root);
            return NULL;
        }
        reveldb_log_config_t *log_config =
            (reveldb_log_config_t *)malloc(sizeof(reveldb_log_config_t));
        if (log_config == NULL) {
            LOG_ERROR(("failed to make room for reveldb_log_config_t."));
            free(reveldb_config);
            free(server_config);
            free(log_config);
            cJSON_Delete(root);
            return NULL;
        }

		iter = cJSON_GetObjectItem(server, "host");
        config_vlen = strlen(iter->valuestring);
        server_config->host = (char *)malloc(sizeof(char) * (config_vlen + 1));
        memset(server_config->host, 0, (config_vlen + 1));
        strncpy(server_config->host, iter->valuestring, config_vlen);
        config_vlen = -1;

		iter = cJSON_GetObjectItem(server, "ports"); 
        config_vlen = strlen(iter->valuestring);
        server_config->ports = (char *)malloc(sizeof(char) * (config_vlen + 1));
        memset(server_config->ports, 0, (config_vlen + 1));
        strncpy(server_config->ports, iter->valuestring, config_vlen);
        config_vlen = -1;

		iter = cJSON_GetObjectItem(server, "username");
        config_vlen = strlen(iter->valuestring);
        server_config->username = (char *)malloc(sizeof(char) * (config_vlen + 1));
        memset(server_config->username, 0, (config_vlen + 1));
        strncpy(server_config->username, iter->valuestring, config_vlen);
        config_vlen = -1;


		iter = cJSON_GetObjectItem(server, "password");
        config_vlen = strlen(iter->valuestring);
        server_config->password = (char *)malloc(sizeof(char) * (config_vlen + 1));
        memset(server_config->password, 0, (config_vlen + 1));
        strncpy(server_config->password, iter->valuestring, config_vlen);
        config_vlen = -1;

		iter = cJSON_GetObjectItem(server, "datadir");
        config_vlen = strlen(iter->valuestring);
        server_config->datadir = (char *)malloc(sizeof(char) * (config_vlen + 1));
        memset(server_config->datadir, 0, (config_vlen + 1));
        strncpy(server_config->datadir, iter->valuestring, config_vlen);
        config_vlen = -1;

        iter = cJSON_GetObjectItem(server, "pidfile");
        config_vlen = strlen(iter->valuestring);
        server_config->pidfile = (char *)malloc(sizeof(char) * (config_vlen + 1));
        memset(server_config->pidfile, 0, (config_vlen + 1));
        strncpy(server_config->pidfile, iter->valuestring, config_vlen);
        config_vlen = -1;

		db = cJSON_GetObjectItem(root, "db");

        iter = cJSON_GetObjectItem(db, "dbname");
        config_vlen = strlen(iter->valuestring);
        db_config->dbname = (char *)malloc(sizeof(char) * (config_vlen + 1));
        memset(db_config->dbname, 0, (config_vlen + 1));
        strncpy(db_config->dbname, iter->valuestring, config_vlen);
        config_vlen = -1;

        iter = cJSON_GetObjectItem(db, "lru_cache_size");
        db_config->lru_cache_size = iter->valueint;

        iter = cJSON_GetObjectItem(db, "create_if_missing");
        db_config->create_if_missing = (iter->valueint == 1) ? true : false;

        iter = cJSON_GetObjectItem(db, "error_if_exist");
        db_config->error_if_exist = (iter->valueint == 1) ? true : false;

        iter = cJSON_GetObjectItem(db, "write_buffer_size");
        db_config->write_buffer_size = iter->valueint;

        iter = cJSON_GetObjectItem(db, "paranoid_checks");
        db_config->paranoid_checks = (iter->valueint == 1) ? true : false;

        iter = cJSON_GetObjectItem(db, "max_open_files");
        db_config->max_open_files = iter->valueint;

        iter = cJSON_GetObjectItem(db, "block_size");
        db_config->block_size = iter->valueint;

        iter = cJSON_GetObjectItem(db, "block_restart_interval");
        db_config->block_restart_interval = iter->valueint;

        iter = cJSON_GetObjectItem(db, "compression");
        db_config->compression = (iter->valueint == 1) ? true : false;

        iter = cJSON_GetObjectItem(db, "verify_checksums");
        db_config->verify_checksums = (iter->valueint == 1) ? true : false;

        iter = cJSON_GetObjectItem(db, "fill_cache");
        db_config->fill_cache = (iter->valueint == 1) ? true : false;

        iter = cJSON_GetObjectItem(db, "sync");
        db_config->sync = (iter->valueint == 1) ? true : false;

		log = cJSON_GetObjectItem(root, "log");

        iter = cJSON_GetObjectItem(log, "level");
        config_vlen = strlen(iter->valuestring);
        log_config->level = (char *)malloc(sizeof(char) * (config_vlen + 1));
        memset(log_config->level, 0, (config_vlen + 1));
        strncpy(log_config->level, iter->valuestring, config_vlen);
        config_vlen = -1;

        iter = cJSON_GetObjectItem(log, "stream");
        config_vlen = strlen(iter->valuestring);
        log_config->stream = (char *)malloc(sizeof(char) * (config_vlen + 1));
        memset(log_config->stream, 0, (config_vlen + 1));
        strncpy(log_config->stream, iter->valuestring, config_vlen);
        
        reveldb_config->server_config = server_config;
        reveldb_config->db_config = db_config;
        reveldb_config->log_config = log_config;

		cJSON_Delete(root);
	}


    return reveldb_config;
}

reveldb_config_t *
reveldb_config_init(const char *file)
{
    assert(file != NULL);

    reveldb_config_t * reveldb_config = NULL;
    char *json_buf = _load_config_body(file);

    reveldb_config = _init_internal_config(json_buf);

    free(json_buf);
    return reveldb_config;
}

void
reveldb_config_fini(reveldb_config_t *config)
{
    // TODO: free it.
}
