/**
 * $Id: interface.c,v 1.7 2008/08/19 09:44:37 mr-russ Exp $
 *
 * public interface to libc
 *
 * Copyright (c) 2001 by Joerg Wendland, Bret Mogilefsky
 * Copyright (c) 2004,2005 by Russell Smith
 *
 * see included file COPYING for details
 *
 */

#define _GNU_SOURCE
#include "nss-pgsql.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static pthread_mutex_t lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

/*
 * passwd functions
 */
enum nss_status
_nss_pgsql_setpwent(void)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;

	pthread_mutex_lock(&lock);
	if(backend_open(CONNECTION_USERGROUP)) {
		getent_prepare("allusers");
		retval = NSS_STATUS_SUCCESS;
	}
	pthread_mutex_unlock(&lock);

	return retval;
}

enum nss_status
_nss_pgsql_endpwent(void)
{
	pthread_mutex_lock(&lock);
	getent_close(CONNECTION_USERGROUP);
	backend_close(CONNECTION_USERGROUP);
	pthread_mutex_unlock(&lock);

	return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_pgsql_getpwent_r(struct passwd *result, char *buffer,
                      size_t buflen, int *errnop)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;
	int lerrno = 0;

	pthread_mutex_lock(&lock);

	// Make sure the database is opened in case no one has called setpwent()
	if(!backend_isopen(CONNECTION_USERGROUP)) {
		retval = _nss_pgsql_setpwent();
	}

	if(backend_open(CONNECTION_USERGROUP)) {
		retval = backend_getpwent(result, buffer, buflen, &lerrno);
	}
	*errnop = lerrno;
	pthread_mutex_unlock(&lock);

	return retval;
}

enum nss_status
_nss_pgsql_getpwnam_r(const char *pwnam, struct passwd *result,
                      char *buffer, size_t buflen, int *errnop)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;
	int lerrno = 0;

	pthread_mutex_lock(&lock);
	if(backend_open(CONNECTION_USERGROUP)) {
		retval = backend_getpwnam(pwnam, result, buffer, buflen, &lerrno);
	}
	backend_close(CONNECTION_USERGROUP);
	*errnop = lerrno;
	pthread_mutex_unlock(&lock);

	return retval;
}

enum nss_status
_nss_pgsql_getpwuid_r(uid_t uid, struct passwd *result, char *buffer,
                      size_t buflen, int *errnop)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;
	int lerrno = 0;

	pthread_mutex_lock(&lock);
	if(backend_open(CONNECTION_USERGROUP)) {
		retval = backend_getpwuid(uid, result, buffer, buflen, &lerrno);
	}
	*errnop = lerrno;
	backend_close(CONNECTION_USERGROUP);
	pthread_mutex_unlock(&lock);

	return retval;
}

/*
 * group functions
 */
enum nss_status
_nss_pgsql_setgrent(void)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;

	pthread_mutex_lock(&lock);
	if(backend_open(CONNECTION_USERGROUP)) {
		getent_prepare("allgroups");
		retval = NSS_STATUS_SUCCESS;
	} 
	pthread_mutex_unlock(&lock);

	return retval;
}

enum nss_status
_nss_pgsql_endgrent(void)
{
	pthread_mutex_lock(&lock);
	getent_close(CONNECTION_USERGROUP);
	backend_close(CONNECTION_USERGROUP);
	pthread_mutex_unlock(&lock);

	return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_pgsql_getgrent_r(struct group *result, char *buffer,
                      size_t buflen, int *errnop)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;
	int lerrno = 0;

	pthread_mutex_lock(&lock);

	// Make sure the database is opened in case no one has called setpwent()
	if(!backend_isopen(CONNECTION_USERGROUP)) {
		retval = _nss_pgsql_setgrent();
	}

	if(backend_open(CONNECTION_USERGROUP)) {
		retval = backend_getgrent(result, buffer, buflen, &lerrno);
	}
	*errnop = lerrno;
	pthread_mutex_unlock(&lock);

	return retval;
}

enum nss_status
_nss_pgsql_getgrnam_r(const char *grnam, struct group *result,
                      char *buffer, size_t buflen, int *errnop)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;
	int lerrno = 0;

	pthread_mutex_lock(&lock);
	if(backend_open(CONNECTION_USERGROUP)) {
		retval = backend_getgrnam(grnam, result, buffer, buflen, &lerrno);
	} 
	backend_close(CONNECTION_USERGROUP);
	*errnop = lerrno;
	pthread_mutex_unlock(&lock);

	return retval;
}

enum nss_status
_nss_pgsql_getgrgid_r(uid_t gid, struct group *result,
                      char *buffer, size_t buflen, int *errnop)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;
	int lerrno = 0;

	pthread_mutex_lock(&lock);
	if(backend_open(CONNECTION_USERGROUP)) {
		retval = backend_getgrgid(gid, result, buffer, buflen, &lerrno);
	}
	backend_close(CONNECTION_USERGROUP);
	*errnop = lerrno;
	pthread_mutex_unlock(&lock);

	return retval;
}

enum nss_status
_nss_pgsql_initgroups_dyn(const char *user, gid_t group, long int *start,
                          long int *size, gid_t **groupsp, long int limit,
                          int *errnop)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;
	int lerrno = 0;
	size_t numgroups;

	pthread_mutex_lock(&lock);
	if(backend_open(CONNECTION_USERGROUP)) {
		numgroups = backend_initgroups_dyn(user, group, start, size, groupsp,
		                                   limit, &lerrno);
		retval = (numgroups > 0) ? NSS_STATUS_SUCCESS : NSS_STATUS_NOTFOUND;
	}
	backend_close(CONNECTION_USERGROUP);
	*errnop = lerrno;
	pthread_mutex_unlock(&lock);

	return retval;
}

/*
 * shadow passwd functions
 */
enum nss_status
_nss_pgsql_setspent(void)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;

	pthread_mutex_lock(&lock);
	if(backend_open(CONNECTION_SHADOW)) {
		getent_prepare("shadow");
		retval = NSS_STATUS_SUCCESS;
	}
	pthread_mutex_unlock(&lock);

	return retval;
}

enum nss_status
_nss_pgsql_endspent(void)
{
	pthread_mutex_lock(&lock);
	getent_close(CONNECTION_SHADOW);
	backend_close(CONNECTION_SHADOW);
	pthread_mutex_unlock(&lock);

	return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_pgsql_getspent_r(struct spwd *result, char *buffer,
                      size_t buflen, int *errnop)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;
	int lerrno = 0;
	
	pthread_mutex_lock(&lock);

	// Make sure the database is opened in case no one has called setspent()
	if(!backend_isopen(CONNECTION_SHADOW)) {
		retval = _nss_pgsql_setspent();
	}

	if(backend_open(CONNECTION_SHADOW)) {
		retval = backend_getspent(result, buffer, buflen, &lerrno);
	}

	*errnop = lerrno;
	pthread_mutex_unlock(&lock);

	return retval;
}

enum nss_status
_nss_pgsql_getspnam_r(const char *spnam, struct spwd *result,
                      char *buffer, size_t buflen, int *errnop)
{
	enum nss_status retval = NSS_STATUS_UNAVAIL;
	int lerrno = 0;
	
	pthread_mutex_lock(&lock);
	if(backend_open(CONNECTION_SHADOW)) {
		retval = backend_getspnam(spnam, result, buffer, buflen, &lerrno);
	}
	backend_close(CONNECTION_SHADOW);
	*errnop = lerrno;
	pthread_mutex_unlock(&lock);

	return retval;
}
