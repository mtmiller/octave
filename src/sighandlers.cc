/*

Copyright (C) 1996, 1997 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstdlib>

#include <iostream>
#include <new>

#ifdef HAVE_UNISTD_H
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <unistd.h>
#endif

#include "cmd-edit.h"
#include "quit.h"

#include "defun.h"
#include "error.h"
#include "load-save.h"
#include "oct-map.h"
#include "pager.h"
#include "pt-bp.h"
#include "sighandlers.h"
#include "syswait.h"
#include "toplev.h"
#include "utils.h"

// Nonzero means we have already printed a message for this series of
// SIGPIPES.  We assume that the writer will eventually give up.
int pipe_handler_error_count = 0;

// TRUE means we can be interrupted.
bool can_interrupt = false;

// TRUE means we should try to enter the debugger on SIGINT.
static bool Vdebug_on_interrupt = false;

#if RETSIGTYPE == void
#define SIGHANDLER_RETURN(status) return
#else
#define SIGHANDLER_RETURN(status) return status
#endif

#if defined (MUST_REINSTALL_SIGHANDLERS)
#define MAYBE_REINSTALL_SIGHANDLER(sig, handler) \
  octave_set_signal_handler (sig, handler)
#define REINSTALL_USES_SIG 1
#else
#define MAYBE_REINSTALL_SIGHANDLER(sig, handler) \
  do { } while (0)
#endif

#if defined (__EMX__)
#define MAYBE_ACK_SIGNAL(sig) \
  octave_set_signal_handler (sig, SIG_ACK)
#define ACK_USES_SIG 1
#else
#define MAYBE_ACK_SIGNAL(sig) \
  do { } while (0)
#endif

static void
my_friendly_exit (const char *sig_name, int sig_number)
{
  static bool been_there_done_that = false;

  if (been_there_done_that)
    {
#if defined (SIGABRT)
      octave_set_signal_handler (SIGABRT, SIG_DFL);
#endif

      std::cerr << "panic: attempted clean up apparently failed -- aborting...\n";
      abort ();
    }
  else
    {
      been_there_done_that = true;

      std::cerr << "panic: " << sig_name << " -- stopping myself...\n";

      save_user_variables ();

      if (sig_number < 0)
	exit (1);
      else
	{
	  octave_set_signal_handler (sig_number, SIG_DFL);

#if defined (HAVE_RAISE)
	  raise (sig_number);
#elif defined (HAVE_KILL)
	  kill (getpid (), sig_number);
#else
	  exit (1);
#endif
	}

    }
}

sig_handler *
octave_set_signal_handler (int sig, sig_handler *handler)
{
#if defined (HAVE_POSIX_SIGNALS)
  struct sigaction act, oact;
  act.sa_handler = handler;
  act.sa_flags = 0;
  sigemptyset (&act.sa_mask);
  sigemptyset (&oact.sa_mask);
  sigaction (sig, &act, &oact);
  return oact.sa_handler;
#else
  return signal (sig, handler);
#endif
}

static RETSIGTYPE
generic_sig_handler (int sig)
{
  my_friendly_exit (sys_siglist[sig], sig);

  SIGHANDLER_RETURN (0);
}

// Handle SIGCHLD.

#ifdef SIGCHLD
static RETSIGTYPE
sigchld_handler (int /* sig */)
{
  volatile octave_interrupt_handler saved_interrupt_handler
     = octave_ignore_interrupts ();

  // I wonder if this is really right, or if SIGCHLD should just be
  // blocked on OS/2 systems the same as for systems with POSIX signal
  // functions.

#if defined (__EMX__)
  volatile sig_handler *saved_sigchld_handler
    = octave_set_signal_handler (SIGCHLD, SIG_IGN);
#endif

  sigset_t set, oset;

  BLOCK_CHILD (set, oset);

  int n = octave_child_list::length ();

  for (int i = 0; i < n; i++)
    {
      octave_child& elt = octave_child_list::elem (i);

      pid_t pid = elt.pid;

      if (pid > 0)
	{
	  int status;

	  if (waitpid (pid, &status, WNOHANG) > 0)
	    {
	      elt.pid = -1;

	      octave_child::dead_child_handler f = elt.handler;

	      if (f)
		f (pid, status);

	      break;
	    }
	}
    }

  octave_set_interrupt_handler (saved_interrupt_handler);

  UNBLOCK_CHILD (oset);

#ifdef __EMX__
  octave_set_signal_handler (SIGCHLD, saved_sigchld_handler);
#endif

  MAYBE_ACK_SIGNAL (SIGCHLD);

  MAYBE_REINSTALL_SIGHANDLER (SIGCHLD, sigchld_handler);

  SIGHANDLER_RETURN (0);
}
#endif /* defined(SIGCHLD) */

#ifdef SIGFPE
#if defined (__alpha__)
static RETSIGTYPE
sigfpe_handler (int /* sig */)
{
  MAYBE_ACK_SIGNAL (SIGFPE);

  MAYBE_REINSTALL_SIGHANDLER (SIGFPE, sigfpe_handler);

  std::cerr << "error: floating point exception -- trying to return to prompt\n";

  // XXX FIXME XXX -- will setting octave_interrupt_state really help
  // here?

  if (can_interrupt)
    octave_interrupt_state = 1;

  SIGHANDLER_RETURN (0);
}
#endif /* defined(__alpha__) */
#endif /* defined(SIGFPE) */

#if 0
#if defined (SIGWINCH)
static RETSIGTYPE
sigwinch_handler (int /* sig */)
{
  MAYBE_ACK_SIGNAL (SIGWINCH);

  MAYBE_REINSTALL_SIGHANDLER (SIGWINCH, sigwinch_handler);

  command_editor::resize_terminal ();

  SIGHANDLER_RETURN (0);
}
#endif
#endif

// Handle SIGINT by restarting the parser (see octave.cc).
//
// This also has to work for SIGBREAK (on systems that have it), so we
// use the value of sig, instead of just assuming that it is called
// for SIGINT only.

static RETSIGTYPE
#if defined (ACK_USES_SIG) || defined (REINSTALL_USES_SIG)
sigint_handler (int sig)
#else
sigint_handler (int)
#endif
{
  MAYBE_ACK_SIGNAL (sig);

  MAYBE_REINSTALL_SIGHANDLER (sig, sigint_handler);

  if (! octave_initialized)
    exit (1);

  if (can_interrupt)
    {
      if (Vdebug_on_interrupt)
	{
	  if (! octave_debug_on_interrupt_state)
	    {
	      octave_debug_on_interrupt_state = true;

	      SIGHANDLER_RETURN (0);
	    }
	  else
	    // Clear the flag and do normal interrupt stuff.
	    octave_debug_on_interrupt_state = false;
	}

      if (octave_interrupt_immediately)
	octave_jump_to_enclosing_context ();
      else
	octave_interrupt_state = 1;
    }

  SIGHANDLER_RETURN (0);
}

#ifdef SIGPIPE
static RETSIGTYPE
sigpipe_handler (int /* sig */)
{
  MAYBE_ACK_SIGNAL (SIGPIPE);

  MAYBE_REINSTALL_SIGHANDLER (SIGPIPE, sigpipe_handler);

  if (pipe_handler_error_count++ == 0)
    std::cerr << "warning: broken pipe\n";

  // Don't loop forever on account of this.

  // XXX FIXME XXX -- will setting octave_interrupt_state really help
  // here?

  if (pipe_handler_error_count  > 100)
    octave_interrupt_state = 1;

  SIGHANDLER_RETURN (0);
}
#endif /* defined(SIGPIPE) */

octave_interrupt_handler
octave_catch_interrupts (void)
{
  octave_interrupt_handler retval;

#ifdef SIGINT
  retval.int_handler = octave_set_signal_handler (SIGINT, sigint_handler);
#endif

#ifdef SIGBREAK
  retval.brk_handler = octave_set_signal_handler (SIGBREAK, sigint_handler);
#endif

  return retval;
}

octave_interrupt_handler
octave_ignore_interrupts (void)
{
  octave_interrupt_handler retval;

#ifdef SIGINT
  retval.int_handler = octave_set_signal_handler (SIGINT, SIG_IGN);
#endif

#ifdef SIGBREAK
  retval.brk_handler = octave_set_signal_handler (SIGBREAK, SIG_IGN);
#endif

  return retval;
}

octave_interrupt_handler
octave_set_interrupt_handler (const volatile octave_interrupt_handler& h)
{
  octave_interrupt_handler retval;

#ifdef SIGINT
  retval.int_handler = octave_set_signal_handler (SIGINT, h.int_handler);
#endif

#ifdef SIGBREAK
  retval.brk_handler = octave_set_signal_handler (SIGBREAK, h.brk_handler);
#endif

  return retval;
}

// Install all the handlers for the signals we might care about.

void
install_signal_handlers (void)
{
  octave_catch_interrupts ();

#ifdef SIGABRT
  octave_set_signal_handler (SIGABRT, generic_sig_handler);
#endif

#ifdef SIGALRM
  octave_set_signal_handler (SIGALRM, generic_sig_handler);
#endif

#ifdef SIGBUS
  octave_set_signal_handler (SIGBUS, generic_sig_handler);
#endif

#ifdef SIGCHLD
  octave_set_signal_handler (SIGCHLD, sigchld_handler);
#endif

  // SIGCLD
  // SIGCONT

#ifdef SIGEMT
  octave_set_signal_handler (SIGEMT, generic_sig_handler);
#endif

#ifdef SIGFPE
#if defined (__alpha__)
  octave_set_signal_handler (SIGFPE, sigfpe_handler);
#else
  octave_set_signal_handler (SIGFPE, generic_sig_handler);
#endif
#endif

#ifdef SIGHUP
  octave_set_signal_handler (SIGHUP, generic_sig_handler);
#endif

#ifdef SIGILL
  octave_set_signal_handler (SIGILL, generic_sig_handler);
#endif

  // SIGINFO
  // SIGINT

#ifdef SIGIOT
  octave_set_signal_handler (SIGIOT, generic_sig_handler);
#endif

#ifdef SIGLOST
  octave_set_signal_handler (SIGLOST, generic_sig_handler);
#endif

#ifdef SIGPIPE
  octave_set_signal_handler (SIGPIPE, sigpipe_handler);
#endif

#ifdef SIGPOLL
  octave_set_signal_handler (SIGPOLL, SIG_IGN);
#endif

#ifdef SIGPROF
  octave_set_signal_handler (SIGPROF, generic_sig_handler);
#endif

  // SIGPWR

#ifdef SIGQUIT
  octave_set_signal_handler (SIGQUIT, generic_sig_handler);
#endif

#ifdef SIGSEGV
  octave_set_signal_handler (SIGSEGV, generic_sig_handler);
#endif

  // SIGSTOP

#ifdef SIGSYS
  octave_set_signal_handler (SIGSYS, generic_sig_handler);
#endif

#ifdef SIGTERM
  octave_set_signal_handler (SIGTERM, generic_sig_handler);
#endif

#ifdef SIGTRAP
  octave_set_signal_handler (SIGTRAP, generic_sig_handler);
#endif

  // SIGTSTP
  // SIGTTIN
  // SIGTTOU
  // SIGURG

#ifdef SIGUSR1
  octave_set_signal_handler (SIGUSR1, generic_sig_handler);
#endif

#ifdef SIGUSR2
  octave_set_signal_handler (SIGUSR2, generic_sig_handler);
#endif

#ifdef SIGVTALRM
  octave_set_signal_handler (SIGVTALRM, generic_sig_handler);
#endif

#ifdef SIGIO
  octave_set_signal_handler (SIGIO, SIG_IGN);
#endif

#if 0
#ifdef SIGWINCH
  octave_set_signal_handler (SIGWINCH, sigwinch_handler);
#endif
#endif

#ifdef SIGXCPU
  octave_set_signal_handler (SIGXCPU, generic_sig_handler);
#endif

#ifdef SIGXFSZ
  octave_set_signal_handler (SIGXFSZ, generic_sig_handler);
#endif
}

static Octave_map
make_sig_struct (void)
{
  Octave_map m;

#ifdef SIGABRT
  m ["ABRT"](0) = SIGABRT;
#endif

#ifdef SIGALRM
  m ["ALRM"](0) = SIGALRM;
#endif

#ifdef SIGBUS
  m ["BUS"](0) = SIGBUS;
#endif

#ifdef SIGCHLD
  m ["CHLD"](0) = SIGCHLD;
#endif

#ifdef SIGCLD
  m ["CLD"](0) = SIGCLD;
#endif

#ifdef SIGCONT
  m ["CONT"](0) = SIGCONT;
#endif

#ifdef SIGEMT
  m ["EMT"](0) = SIGEMT;
#endif

#ifdef SIGFPE
  m ["FPE"](0) = SIGFPE;
#endif

#ifdef SIGHUP
  m ["HUP"](0) = SIGHUP;
#endif

#ifdef SIGILL
  m ["ILL"](0) = SIGILL;
#endif

#ifdef SIGINFO
  m ["INFO"](0) = SIGINFO;
#endif

#ifdef SIGINT
  m ["INT"](0) = SIGINT;
#endif

#ifdef SIGIOT
  m ["IOT"](0) = SIGIOT;
#endif

#ifdef SIGLOST
  m ["LOST"](0) = SIGLOST;
#endif

#ifdef SIGPIPE
  m ["PIPE"](0) = SIGPIPE;
#endif

#ifdef SIGPOLL
  m ["POLL"](0) = SIGPOLL;
#endif

#ifdef SIGPROF
  m ["PROF"](0) = SIGPROF;
#endif

#ifdef SIGPWR
  m ["PWR"](0) = SIGPWR;
#endif

#ifdef SIGQUIT
  m ["QUIT"](0) = SIGQUIT;
#endif

#ifdef SIGSEGV
  m ["SEGV"](0) = SIGSEGV;
#endif

#ifdef SIGSTOP
  m ["STOP"](0) = SIGSTOP;
#endif

#ifdef SIGSYS
  m ["SYS"](0) = SIGSYS;
#endif

#ifdef SIGTERM
  m ["TERM"](0) = SIGTERM;
#endif

#ifdef SIGTRAP
  m ["TRAP"](0) = SIGTRAP;
#endif

#ifdef SIGTSTP
  m ["TSTP"](0) = SIGTSTP;
#endif

#ifdef SIGTTIN
  m ["TTIN"](0) = SIGTTIN;
#endif

#ifdef SIGTTOU
  m ["TTOU"](0) = SIGTTOU;
#endif

#ifdef SIGURG
  m ["URG"](0) = SIGURG;
#endif

#ifdef SIGUSR1
  m ["USR1"](0) = SIGUSR1;
#endif

#ifdef SIGUSR2
  m ["USR2"](0) = SIGUSR2;
#endif

#ifdef SIGVTALRM
  m ["VTALRM"](0) = SIGVTALRM;
#endif

#ifdef SIGIO
  m ["IO"](0) = SIGIO;
#endif

#ifdef SIGWINCH
  m ["WINCH"](0) = SIGWINCH;
#endif

#ifdef SIGXCPU
  m ["XCPU"](0) = SIGXCPU;
#endif

#ifdef SIGXFSZ
  m ["XFSZ"](0) = SIGXFSZ;
#endif

  return m;
}

octave_child_list *octave_child_list::instance = 0;

bool
octave_child_list::instance_ok (void)
{
  bool retval = true;

  if (! instance)
    instance = new octave_child_list ();

  if (! instance)
    {
      ::error ("unable to create child list object!");

      retval = false;
    }

  return retval;
}

void
octave_child_list::insert (pid_t pid, octave_child::dead_child_handler f)
{
  if (instance_ok ())
    instance->do_insert (pid, f);
}

void
octave_child_list::remove (pid_t pid)
{
  if (instance_ok ())
    instance->do_remove (pid);
}

int
octave_child_list::length (void)
{
  return (instance_ok ()) ? instance->do_length () : 0;
}

octave_child&
octave_child_list::elem (int i)
{
  static octave_child foo;

  return (instance_ok ()) ? instance->do_elem (i) : foo;
}

void
octave_child_list::do_insert (pid_t pid, octave_child::dead_child_handler f)
{
  // Insert item in first open slot, increasing size of list if
  // necessary.

  bool enlarge = true;

  for (int i = 0; i < curr_len; i++)
    {
      octave_child& tmp = list (i);

      if (tmp.pid < 0)
	{
	  list (i) = octave_child (pid, f);
	  enlarge = false;
	  break;
	}
    }

  if (enlarge)
    {
      int total_len = list.length ();

      if (curr_len == total_len)
	{
	  if (total_len == 0)
	    list.resize (16);
	  else
	    list.resize (total_len * 2);
	}

      list (curr_len) = octave_child (pid, f);
      curr_len++;
    }
}

void
octave_child_list::do_remove (pid_t pid)
{
  // Mark the record for PID invalid.

  for (int i = 0; i < curr_len; i++)
    {
      octave_child& tmp = list (i);

      if (tmp.pid == pid)
	{
	  tmp.pid = -1;
	  break;
	}
    }
}

int
octave_child_list::do_length (void) const
{
  return curr_len;
}

octave_child&
octave_child_list::do_elem (int i)
{
  static octave_child foo;

  int n = do_length ();

  if (i >= 0 && i < n)
    return list (i);
  else
    return foo;
}

static int
debug_on_interrupt (void)
{
  Vdebug_on_interrupt = check_preference ("debug_on_interrupt");

  return 0;
}

void
symbols_of_sighandlers (void)
{
  DEFVAR (debug_on_interrupt, false, debug_on_interrupt,
    "-*- texinfo -*-\n\
@defvr {Built-in Variable} debug_on_interrupt\n\
If @code{debug_on_interrupt} is nonzero, Octave will try to enter\n\
debugging mode when it receives an interrupt signal (typically\n\
generated with @kbd{C-c}).  If a second interrupt signal is received\n\
before reaching the debugging mode, a normal interrupt will occur.\n\
The default value is 0.\n\
@end defvr");

  DEFCONST (SIG, make_sig_struct (),
    "-*- texinfo -*-\n\
@defvr {Built-in Variable} SIG\n\
Structure of Unix signal names and their defined values.\n\
@end defvr");
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
