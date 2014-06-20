/*
 *  netstat.h
 *  NetStat
 *
 *  Created by Rene Hexel on 17/09/10.
 *  Copyright 2010 Rene Hexel. All rights reserved.
 *
 *  
 */
#ifndef _NET_STAT_H_
#define _NET_STAT_H_	1

#include <time.h>

#define	NS_INTEFRACE_LEN	32	// maximum interface name length

struct net_stat
{
	char name[NS_INTEFRACE_LEN];	// interface name
	int in_packets;			// count of incoming packets
	int out_packets;		// count of outgoing packets
	int mtu;			// maximum transfer unit
	int in_errors;			// incoming errors encountered
	int out_errors;			// outgoing errors encountered
	time_t time_stamp;		// time stamp of data measured
};


/*
 * Get the current network statistics are reported by netstat(1) or netstat(8)
 * Parameters: stats -- a pointer to a struct net_stat to be filled in
 * Return value: 0 if okay, -1 in case of an error
 */
int get_net_statistics(struct net_stat *stats);

#endif // _NET_STAT_H_
