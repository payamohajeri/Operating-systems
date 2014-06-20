/*
 *  netstat.c
 *  NetStat
 *
 *  Created by Rene Hexel on 17/09/10.
 *  Copyright 2010 Rene Hexel. All rights reserved.
 *
 *  A simple function that returns network statistics.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "netstat.h"

int
get_net_statistics(struct net_stat *stats)
{
	if (!stats)
	{
		errno = EINVAL;
		return -1;
	}

	char line[160];
	FILE *npipe = popen("netstat -i | grep '^e[a-z][a-z]*0[ \t]' | head -n1",
			    "r");
	if (!npipe) return -1;

	stats->time_stamp = time(NULL);

	/*
	 * record the time stamp and get a line of response from the
	 * command sequence pipe
	 */
	if (!fgets(line, sizeof(line), npipe))
	{
		int error = errno;
		pclose(npipe);
		errno = error;
		return -1;
	}
	pclose(npipe);

	/*
	 * try parsing a Linux style response first
	 */
	int n = sscanf(line,
		       "%32s\t%d\t%*d\t%d\t%d\t%d\t%d",
		       stats->name, &stats->mtu,
		       &stats->in_packets, &stats->in_errors,
		       &stats->out_packets, &stats->out_errors);
	/*
	 * if unsuccessful, try parsing a BSD style response
	 */
	if (n != 6)
	{
		n = sscanf(line,
			   "%32s %d %*s %*s %d %d %d %d",
			   stats->name, &stats->mtu,
			   &stats->in_packets, &stats->in_errors,
			   &stats->out_packets, &stats->out_errors);
		if (n != 6)
		{
			errno = ENODEV;
			return -1;
		}
	}

	return 0;
}
