//==========================================================================
//
//      sys/kern/kern_subr.c
//
//     
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


/*	$OpenBSD: kern_subr.c,v 1.10 1999/11/07 17:39:14 provos Exp $	*/
/*	$NetBSD: kern_subr.c,v 1.15 1996/04/09 17:21:56 ragge Exp $	*/

/*
 * Copyright (c) 1982, 1986, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)kern_subr.c	8.3 (Berkeley) 1/21/94
 */

#include <sys/param.h>
#ifndef __ECOS
#include <sys/systm.h>
#include <sys/proc.h>
#endif // __ECOS
#include <sys/malloc.h>
#include <sys/queue.h>

int
uiomove(cp, n, uio)
	register caddr_t cp;
	register int n;
	register struct uio *uio;
{
	register struct iovec *iov;
	u_int cnt;
	int error = 0;

#ifdef DIAGNOSTIC
	if (uio->uio_rw != UIO_READ && uio->uio_rw != UIO_WRITE)
		panic("uiomove: mode");
	if (uio->uio_segflg == UIO_USERSPACE && uio->uio_procp != curproc)
		panic("uiomove proc");
#endif
	while (n > 0 && uio->uio_resid) {
		iov = uio->uio_iov;
		cnt = iov->iov_len;
		if (cnt == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			continue;
		}
		if (cnt > n)
			cnt = n;
		switch (uio->uio_segflg) {

		case UIO_USERSPACE:
			if (uio->uio_rw == UIO_READ)
				error = copyout(cp, iov->iov_base, cnt);
			else
				error = copyin(iov->iov_base, cp, cnt);
			if (error)
				return (error);
			break;

		case UIO_SYSSPACE:
#if defined(UVM)
			if (uio->uio_rw == UIO_READ)
				error = kcopy(cp, iov->iov_base, cnt);
			else
				error = kcopy(iov->iov_base, cp, cnt);
			if (error)
				return(error);
#else
			if (uio->uio_rw == UIO_READ)
				bcopy((caddr_t)cp, iov->iov_base, cnt);
			else
				bcopy(iov->iov_base, (caddr_t)cp, cnt);
			break;
#endif
		}
		iov->iov_base = (char *)(iov->iov_base) + cnt;
		iov->iov_len -= cnt;
		uio->uio_resid -= cnt;
		uio->uio_offset += cnt;
		cp += cnt;
		n -= cnt;
	}
	return (error);
}

#ifndef __ECOS
/*
 * Give next character to user as result of read.
 */
int
ureadc(c, uio)
	register int c;
	register struct uio *uio;
{
	register struct iovec *iov;

	if (uio->uio_resid == 0)
#ifdef DIAGNOSTIC
		panic("ureadc: zero resid");
#else
		return (EINVAL);
#endif
again:
	if (uio->uio_iovcnt <= 0)
#ifdef DIAGNOSTIC
		panic("ureadc: non-positive iovcnt");
#else
		return (EINVAL);
#endif
	iov = uio->uio_iov;
	if (iov->iov_len <= 0) {
		uio->uio_iovcnt--;
		uio->uio_iov++;
		goto again;
	}
	switch (uio->uio_segflg) {

	case UIO_USERSPACE:
		if (subyte(iov->iov_base, c) < 0)
			return (EFAULT);
		break;

	case UIO_SYSSPACE:
		*(char *)iov->iov_base = c;
		break;
	}
	iov->iov_base++;
	iov->iov_len--;
	uio->uio_resid--;
	uio->uio_offset++;
	return (0);
}
#endif // __ECOS

/*
 * General routine to allocate a hash table.
 */
#ifdef __ECOS
void *
hashinit(int elements, int type, int flags, u_long *hashmask)
#else
void *
hashinit(elements, type, flags, hashmask)
	int elements, type, flags;
	u_long *hashmask;
#endif
{
	long hashsize;
	LIST_HEAD(generic, generic) *hashtbl;
	int i;

	if (elements <= 0)
		panic("hashinit: bad cnt");
	for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
		continue;
	hashsize >>= 1;
	hashtbl = malloc((u_long)hashsize * sizeof(*hashtbl), type, flags);
	for (i = 0; i < hashsize; i++)
		LIST_INIT(&hashtbl[i]);
	*hashmask = hashsize - 1;
	return (hashtbl);
}

#ifndef __ECOS
/*
 * "Shutdown hook" types, functions, and variables.
 */

struct shutdownhook_desc {
	LIST_ENTRY(shutdownhook_desc) sfd_list;
	void	(*sfd_fn) __P((void *));
	void	*sfd_arg;
};

LIST_HEAD(, shutdownhook_desc) shutdownhook_list;

int shutdownhooks_done;

void *
shutdownhook_establish(fn, arg)
	void (*fn) __P((void *));
	void *arg;
{
	struct shutdownhook_desc *ndp;

	ndp = (struct shutdownhook_desc *)
	    malloc(sizeof (*ndp), M_DEVBUF, M_NOWAIT);
	if (ndp == NULL)
		return NULL;

	ndp->sfd_fn = fn;
	ndp->sfd_arg = arg;
	LIST_INSERT_HEAD(&shutdownhook_list, ndp, sfd_list);

	return (ndp);
}

void
shutdownhook_disestablish(vhook)
	void *vhook;
{
#ifdef DIAGNOSTIC
	struct shutdownhook_desc *dp;

	for (dp = shutdownhook_list.lh_first; dp != NULL;
	    dp = dp->sfd_list.le_next)
                if (dp == vhook)
			break;
	if (dp == NULL)
		panic("shutdownhook_disestablish: hook not established");
#endif

	LIST_REMOVE((struct shutdownhook_desc *)vhook, sfd_list);
}

/*
 * Run shutdown hooks.  Should be invoked immediately before the
 * system is halted or rebooted, i.e. after file systems unmounted,
 * after crash dump done, etc.
 */
void
doshutdownhooks()
{
	struct shutdownhook_desc *dp;

	if (shutdownhooks_done)
		return;

	for (dp = shutdownhook_list.lh_first; dp != NULL; dp =
	    dp->sfd_list.le_next)
		(*dp->sfd_fn)(dp->sfd_arg);
}

/*
 * "Power hook" types, functions, and variables.
 */

struct powerhook_desc {
	LIST_ENTRY(powerhook_desc) sfd_list;
	void	(*sfd_fn) __P((int, void *));
	void	*sfd_arg;
};

LIST_HEAD(, powerhook_desc) powerhook_list;

void *
powerhook_establish(fn, arg)
	void (*fn) __P((int, void *));
	void *arg;
{
	struct powerhook_desc *ndp;

	ndp = (struct powerhook_desc *)
	    malloc(sizeof(*ndp), M_DEVBUF, M_NOWAIT);
	if (ndp == NULL)
		return NULL;

	ndp->sfd_fn = fn;
	ndp->sfd_arg = arg;
	LIST_INSERT_HEAD(&powerhook_list, ndp, sfd_list);

	return (ndp);
}

void
powerhook_disestablish(vhook)
	void *vhook;
{
#ifdef DIAGNOSTIC
	struct powerhook_desc *dp;

	for (dp = powerhook_list.lh_first; dp != NULL;
	    dp = dp->sfd_list.le_next)
                if (dp == vhook)
			break;
	if (dp == NULL)
		panic("powerhook_disestablish: hook not established");
#endif

	LIST_REMOVE((struct powerhook_desc *)vhook, sfd_list);
	free(vhook, M_DEVBUF);
}

/*
 * Run power hooks.
 */
void
dopowerhooks(why)
	int why;
{
	struct powerhook_desc *dp;

	for (dp = LIST_FIRST(&powerhook_list); 
	     dp != NULL; 
	     dp = LIST_NEXT(dp, sfd_list)) {
		(*dp->sfd_fn)(why, dp->sfd_arg);
	}
}
#endif // __ECOS
