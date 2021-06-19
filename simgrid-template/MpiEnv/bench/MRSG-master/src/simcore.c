/* Copyright (c) 2012. MRSG Team. All rights reserved. */

/* This file is part of MRSG.

MRSG is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MRSG is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MRSG.  If not, see <http://www.gnu.org/licenses/>. */

#include <msg/msg.h>
#include <xbt/sysdep.h>
#include <xbt/log.h>
#include <xbt/asserts.h>
#include "common.h"
#include "worker.h"
#include "dfs.h"
#include "mrsg.h"

XBT_LOG_NEW_DEFAULT_CATEGORY (msg_test, "MRSG");

#define MAX_LINE_SIZE 256

int master (int argc, char *argv[]);
int worker (int argc, char *argv[]);

static void check_config (void);
static msg_error_t run_simulation (const char* platform_file, const char* deploy_file, const char* mr_config_file);
static void init_mr_config (const char* mr_config_file);
static void read_mr_config_file (const char* file_name);
static void init_config (void);
static void init_job (void);
static void init_stats (void);
static void free_global_mem (void);

int MRSG_main (const char* plat, const char* depl, const char* conf)
{
    int argc = 8;
    char* argv[] = {
	"mrsg",
	"--cfg=tracing:1",
	"--cfg=tracing/buffer:1",
	"--cfg=tracing/filename:tracefile.trace",
	"--cfg=tracing/categorized:1",
	"--cfg=tracing/uncategorized:1",
	"--cfg=viva/categorized:cat.plist",
	"--cfg=viva/uncategorized:uncat.plist"
    };

    msg_error_t  res = MSG_OK;

    config.initialized = 0;

    check_config ();

    MSG_init (&argc, argv);
    res = run_simulation (plat, depl, conf);

    if (res == MSG_OK)
	return 0;
    else
	return 1;
}

/**
 * @brief Check if the user configuration is sound.
 */
static void check_config (void)
{
    xbt_assert (user.task_cost_f != NULL, "Task cost function not specified.");
    xbt_assert (user.map_output_f != NULL, "Map output function not specified.");
}

/**
 * @param  platform_file   The path/name of the platform file.
 * @param  deploy_file     The path/name of the deploy file.
 * @param  mr_config_file  The path/name of the configuration file.
 */
static msg_error_t run_simulation (const char* platform_file, const char* deploy_file, const char* mr_config_file)
{
    msg_error_t  res = MSG_OK;

    read_mr_config_file (mr_config_file);

    MSG_create_environment (platform_file);

    // for tracing purposes..
    TRACE_category_with_color ("MAP", "1 0 0");
    TRACE_category_with_color ("REDUCE", "0 0 1");

    MSG_function_register ("master", master);
    MSG_function_register ("worker", worker);
    MSG_launch_application (deploy_file);

    init_mr_config (mr_config_file);

    res = MSG_main ();

    free_global_mem ();

    return res;
}

/**
 * @brief  Initialize the MapReduce configuration.
 * @param  mr_config_file  The path/name of the configuration file.
 */
static void init_mr_config (const char* mr_config_file)
{
    srand (12345);
    init_config ();
    init_stats ();
    init_job ();
    distribute_data ();
}

/**
 * @brief  Read the MapReduce configuration file.
 * @param  file_name  The path/name of the configuration file.
 */
static void read_mr_config_file (const char* file_name)
{
    char    property[256];
    FILE*   file;

    /* Set the default configuration. */
    config.chunk_size = 67108864;
    config.chunk_count = 0;
    config.chunk_replicas = 3;
    config.slots[MAP] = 2;
    config.amount_of_tasks[REDUCE] = 1;
    config.slots[REDUCE] = 2;

    /* Read the user configuration file. */

    file = fopen (file_name, "r");

    xbt_assert (file != NULL, "Error reading cofiguration file: %s", file_name);

    while ( fscanf (file, "%256s", property) != EOF )
    {
	if ( strcmp (property, "chunk_size") == 0 )
	{
	    fscanf (file, "%lg", &config.chunk_size);
	    config.chunk_size *= 1024 * 1024; /* MB -> bytes */
	}
	else if ( strcmp (property, "input_chunks") == 0 )
	{
	    fscanf (file, "%d", &config.chunk_count);
	}
	else if ( strcmp (property, "dfs_replicas") == 0 )
	{
	    fscanf (file, "%d", &config.chunk_replicas);
	}
	else if ( strcmp (property, "map_slots") == 0 )
	{
	    fscanf (file, "%d", &config.slots[MAP]);
	}
	else if ( strcmp (property, "reduces") == 0 )
	{
	    fscanf (file, "%d", &config.amount_of_tasks[REDUCE]);
	}
	else if ( strcmp (property, "reduce_slots") == 0 )
	{
	    fscanf (file, "%d", &config.slots[REDUCE]);
	}
	else
	{
	    printf ("Error: Property %s is not valid. (in %s)", property, file_name);
	    exit (1);
	}
    }

    fclose (file);

    /* Assert the configuration values. */

    xbt_assert (config.chunk_size > 0, "Chunk size must be greater than zero");
    xbt_assert (config.chunk_count > 0, "The amount of input chunks must be greater than zero");
    xbt_assert (config.chunk_replicas > 0, "The amount of chunk replicas must be greater than zero");
    xbt_assert (config.slots[MAP] > 0, "Map slots must be greater than zero");
    xbt_assert (config.amount_of_tasks[REDUCE] >= 0, "The number of reduce tasks can't be negative");
    xbt_assert (config.slots[REDUCE] > 0, "Reduce slots must be greater than zero");
}

/**
 * @brief  Initialize the config structure.
 */
static void init_config (void)
{
    const char*    process_name = NULL;
    msg_host_t     host;
    msg_process_t  process;
    size_t         wid;
    unsigned int   cursor;
    w_info_t       wi;
    xbt_dynar_t    process_list;

    /* Initialize hosts information. */

    config.number_of_workers = 0;

    process_list = MSG_processes_as_dynar ();
    xbt_dynar_foreach (process_list, cursor, process)
    {
	process_name = MSG_process_get_name (process);
	if ( strcmp (process_name, "worker") == 0 )
	    config.number_of_workers++;
    }

    config.workers = xbt_new (msg_host_t, config.number_of_workers);

    wid = 0;
    config.grid_cpu_power = 0.0;
    xbt_dynar_foreach (process_list, cursor, process)
    {
	process_name = MSG_process_get_name (process);
	host = MSG_process_get_host (process);
	if ( strcmp (process_name, "worker") == 0 )
	{
	    config.workers[wid] = host;
	    /* Set the worker ID as its data. */
	    wi = xbt_new (struct w_info_s, 1);
	    wi->wid = wid;
	    MSG_host_set_data (host, (void*)wi);
	    /* Add the worker's cpu power to the grid total. */
	    config.grid_cpu_power += MSG_get_host_speed (host);
	    wid++;
	}
    }
    config.grid_average_speed = config.grid_cpu_power / config.number_of_workers;
    config.heartbeat_interval = maxval (HEARTBEAT_MIN_INTERVAL, config.number_of_workers / 100);
    config.amount_of_tasks[MAP] = config.chunk_count;
    config.initialized = 1;
}

/**
 * @brief  Initialize the job structure.
 */
static void init_job (void)
{
    int     i;
    size_t  wid;

    xbt_assert (config.initialized, "init_config has to be called before init_job");

    job.finished = 0;
    job.heartbeats = xbt_new (struct heartbeat_s, config.number_of_workers);
    for (wid = 0; wid < config.number_of_workers; wid++)
    {
	job.heartbeats[wid].slots_av[MAP] = config.slots[MAP];
	job.heartbeats[wid].slots_av[REDUCE] = config.slots[REDUCE];
    }

    /* Initialize map information. */
    job.tasks_pending[MAP] = config.amount_of_tasks[MAP];
    job.task_status[MAP] = xbt_new0 (int, config.amount_of_tasks[MAP]);
    job.task_instances[MAP] = xbt_new0 (int, config.amount_of_tasks[MAP]);
    job.task_list[MAP] = xbt_new0 (msg_task_t*, config.amount_of_tasks[MAP]);
    for (i = 0; i < config.amount_of_tasks[MAP]; i++)
	job.task_list[MAP][i] = xbt_new0 (msg_task_t, MAX_SPECULATIVE_COPIES);

    job.map_output = xbt_new (size_t*, config.number_of_workers);
    for (i = 0; i < config.number_of_workers; i++)
	job.map_output[i] = xbt_new0 (size_t, config.amount_of_tasks[REDUCE]);

    /* Initialize reduce information. */
    job.tasks_pending[REDUCE] = config.amount_of_tasks[REDUCE];
    job.task_status[REDUCE] = xbt_new0 (int, config.amount_of_tasks[REDUCE]);
    job.task_instances[REDUCE] = xbt_new0 (int, config.amount_of_tasks[REDUCE]);
    job.task_list[REDUCE] = xbt_new0 (msg_task_t*, config.amount_of_tasks[REDUCE]);
    for (i = 0; i < config.amount_of_tasks[REDUCE]; i++)
	job.task_list[REDUCE][i] = xbt_new0 (msg_task_t, MAX_SPECULATIVE_COPIES);
}

/**
 * @brief  Initialize the stats structure.
 */
static void init_stats (void)
{
    xbt_assert (config.initialized, "init_config has to be called before init_stats");

    stats.map_local = 0;
    stats.map_remote = 0;
    stats.map_spec_l = 0;
    stats.map_spec_r = 0;
    stats.reduce_normal = 0;
    stats.reduce_spec = 0;
}

/**
 * @brief  Free allocated memory for global variables.
 */
static void free_global_mem (void)
{
    size_t  i;

    for (i = 0; i < config.chunk_count; i++)
	xbt_free_ref (&chunk_owner[i]);
    xbt_free_ref (&chunk_owner);

    xbt_free_ref (&config.workers);
    xbt_free_ref (&job.task_status[MAP]);
    xbt_free_ref (&job.task_instances[MAP]);
    xbt_free_ref (&job.task_status[REDUCE]);
    xbt_free_ref (&job.task_instances[REDUCE]);
    xbt_free_ref (&job.heartbeats);
    for (i = 0; i < config.amount_of_tasks[MAP]; i++)
	xbt_free_ref (&job.task_list[MAP][i]);
    xbt_free_ref (&job.task_list[MAP]);
    for (i = 0; i < config.amount_of_tasks[REDUCE]; i++)
	xbt_free_ref (&job.task_list[REDUCE][i]);
    xbt_free_ref (&job.task_list[REDUCE]);
}

// vim: set ts=8 sw=4:
