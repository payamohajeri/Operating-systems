/* $Id: glibtop.h,v 1.15 1998/09/09 15:19:23 martin Exp $ */

/* Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the Gnome Top Library.
   Contributed by Martin Baulig <martin@home-of-linux.org>, April 1998.

   The Gnome Top Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Top Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef __GLIBTOP_H__
#define __GLIBTOP_H__

#include <glibtop/global.h>

#include <glibtop_server.h>
//#include <glibtop/types.h>

#ifdef HAVE_GLIBTOP_MACHINE_H
#include <glibtop_machine.h>
#endif

#ifndef GLIBTOP_MOUNTENTRY_LEN
#define GLIBTOP_MOUNTENTRY_LEN	79
#endif

typedef struct _glibtop		glibtop;

#include <glibtop/sysdeps.h>

struct _glibtop
{
	unsigned flags;
	unsigned method;		/* Server Method */
	unsigned error_method;		/* Error Method */
#ifdef HAVE_GLIBTOP_MACHINE_H
	glibtop_machine machine;	/* Machine dependent data */
#endif
	int input [2];			/* Pipe client <- server */
	int output [2];			/* Pipe client -> server */
	int socket;			/* Accepted connection of a socket */
	int ncpu;			/* Number of CPUs, zero if single-processor */
	unsigned long os_version_code;	/* Version code of the operating system */
	const char *name;		/* Program name for error messages */
	const char *server_command;	/* Command used to invoke server */
	const char *server_host;	/* Host the server should run on */
	const char *server_user;	/* Name of the user on the target host */
	const char *server_rsh;		/* Command used to connect to the target host */
	unsigned long features;		/* Server is required for this features */
	unsigned long server_port;	/* Port on which daemon is listening */
	glibtop_sysdeps sysdeps;	/* Detailed feature list */
	glibtop_sysdeps required;	/* Required feature list */
	pid_t pid;			/* PID of the server */
};

extern glibtop *glibtop_global_server;

extern const unsigned long glibtop_server_features;

#define glibtop_init()	glibtop_init_r(&glibtop_global_server, 0, 0);

#define glibtop_close()	glibtop_close_r(glibtop_global_server);

extern glibtop *glibtop_init_r __P((glibtop **, unsigned long, unsigned));
extern glibtop *glibtop_init_s __P((glibtop **, unsigned long, unsigned));

#ifdef GLIBTOP_GUILE

/* You need to link with -lgtop_guile to get this stuff here. */

extern void glibtop_boot_guile __P((void));

#endif

#ifdef GLIBTOP_GUILE_NAMES

/* You need to link with -lgtop_guile_names to get this stuff here. */

extern void glibtop_boot_guile_names __P((void));

#ifndef GLIBTOP_NAMES
#define GLIBTOP_NAMES
#endif

#endif

#endif
