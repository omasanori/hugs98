/* acconfig.h

   Descriptive text for the C preprocessor macros that
   the Hugs configuration script can define.
   The current version may not use all of them; autoheader copies the ones
   your configure.in uses into your configuration header file templates.

   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  Although this order
   can split up related entries, it makes it easier to check whether
   a given entry is in the file.

   Leave the following blank line there!!  Autoheader needs it.  */


/* The following symbols are defined in options.h:
 * 
 *   BYTECODE_PRIMS
 *   CHECK_TAGS
 *   DEBUG_CODE
 *   DEBUG_PRINTER
 *   DONT_PANIC
 *   GIMME_STACK_DUMPS
 *   HUGSDIR
 *   HUGSPATH
 *   HUGSSUFFIXES
 *   HUGS_FOR_WINDOWS
 *   HUGS_VERSION
 *   IGNORE_MODULES
 *   INTERNAL_PRIMS
 *   LARGE_HUGS
 *   PATH_CANONICALIZATION
 *   PROFILING
 *   REGULAR_HUGS
 *   SMALL_BANNER
 *   SMALL_HUGS
 *   USE_PREPROCESSOR
 *   USE_READLINE
 *   WANT_TIMER
 *   HASKELL_98_ONLY
 */

/* Define to alignment constraint on chars */
#undef ALIGNMENT_CHAR

/* Define to alignment constraint on doubles */
#undef ALIGNMENT_DOUBLE

/* Define to alignment constraint on floats */
#undef ALIGNMENT_FLOAT

/* Define to alignment constraint on ints */
#undef ALIGNMENT_INT

/* Define to alignment constraint on longs */
#undef ALIGNMENT_LONG

/* Define to alignment constraint on long longs */
#undef ALIGNMENT_LONG_LONG

/* Define to alignment constraint on shorts */
#undef ALIGNMENT_SHORT

/* Define to alignment constraint on unsigned chars */
#undef ALIGNMENT_UNSIGNED_CHAR

/* Define to alignment constraint on unsigned ints */
#undef ALIGNMENT_UNSIGNED_INT

/* Define to alignment constraint on unsigned longs */
#undef ALIGNMENT_UNSIGNED_LONG

/* Define to alignment constraint on unsigned long longs */
#undef ALIGNMENT_UNSIGNED_LONG_LONG

/* Define to alignment constraint on unsigned shorts */
#undef ALIGNMENT_UNSIGNED_SHORT

/* Define to alignment constraint on void pointers */
#undef ALIGNMENT_VOID_P

/* C compiler invocation use to build a dynamically loadable library.
 * Typical value: "gcc -shared"
 * Must evaluate to a literal C string.
 */
#define MKDLL_CMD ""

/* Define if you have malloc.h and it defines _alloca - eg for Visual C++. */
#define HAVE__ALLOCA 0

/* Define if you have /bin/sh */
#define HAVE_BIN_SH 0

/* Define if you have the GetModuleFileName function.  */
#define HAVE_GETMODULEFILENAME 0

/* Define if heap profiler can (and should) automatically invoke hp2ps
 * to convert heap profile (in "profile.hp") to postscript.
 */
#define HAVE_HP2PS 0

/* Define if compiler supports gcc's "labels as values" (aka computed goto)
 * feature (which is used to speed up instruction dispatch in the interpreter).
 * Here's what typical code looks like:
 *
 * void *label[] = { &&l1, &&l2 };
 * ...
 * goto *label[i];
 * l1: ...
 * l2: ...
 * ...
 */
#define HAVE_LABELS_AS_VALUES 0

/* Define if C compiler supports long long types */
#undef HAVE_LONG_LONG

/* Define if compiler supports prototypes. */
#define HAVE_PROTOTYPES 0

/* Define if you have the WinExec function.  */
#define HAVE_WINEXEC 0

/* Define if jmpbufs can be treated like arrays.
 * That is, if the following code compiles ok:
 *
 * #include <setjmp.h>
 * 
 * int test1() {
 *     jmp_buf jb[1];
 *     jmp_buf *jbp = jb;
 *     return (setjmp(jb[0]) == 0);
 * }
 */
#define JMPBUF_ARRAY   0

/* Define if your C compiler inserts underscores before symbol names */
#undef LEADING_UNDERSCORE

/* Define if signal handlers have type void (*)(int)
 * (Otherwise, they're assumed to have type int (*)(void).)
 */
#define VOID_INT_SIGNALS 0

/* Define if time.h or sys/time.h define the altzone variable.  */
#undef HAVE_ALTZONE

/* Define if time.h or sys/time.h define the timezone variable.  */
#undef HAVE_TIMEZONE
 

/* Leave that blank line there!!  Autoheader needs it.
   If you're adding to this file, keep in mind:
   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  */


/* autoheader doesn't grok AC_CHECK_LIB_NOWARN so we have to add them
   manually.  */

@BOTTOM@

/* Define if netinet/in.h defines the in_addr type.  */
#undef HAVE_IN_ADDR_T

/* Define if you have the dl library (-ldl).  */
#undef HAVE_LIBDL

/* Define if you have the dld library (-ldld).  */
#undef HAVE_LIBDLD

/* Define if you have the m library (-lm).  */
#undef HAVE_LIBM

/* Define if you have the editline library (-leditline).  */
#undef HAVE_LIBREADLINE

/* Define if struct msghdr contains msg_accrights field */
#undef HAVE_MSGHDR_MSG_ACCRIGHTS
 
/* Define if struct msghdr contains msg_control field */
#undef HAVE_MSGHDR_MSG_CONTROL

/* Define to Haskell type for cc_t */
#undef HTYPE_CC_T

/* Define to Haskell type for char */
#undef HTYPE_CHAR

/* Define to Haskell type for clock_t */
#undef HTYPE_CLOCK_T

/* Define to Haskell type for dev_t */
#undef HTYPE_DEV_T

/* Define to Haskell type for signed double */
#undef HTYPE_DOUBLE

/* Define to Haskell type for float */
#undef HTYPE_FLOAT

/* Define to Haskell type for gid_t */
#undef HTYPE_GID_T

/* Define to Haskell type for GLbitfield */
#undef HTYPE_GLBITFIELD

/* Define to Haskell type for GLboolean */
#undef HTYPE_GLBOOLEAN

/* Define to Haskell type for GLbyte */
#undef HTYPE_GLBYTE

/* Define to Haskell type for GLclampd */
#undef HTYPE_GLCLAMPD
 
/* Define to Haskell type for GLclampf */
#undef HTYPE_GLCLAMPF

/* Define to Haskell type for GLdouble */
#undef HTYPE_GLDOUBLE

/* Define to Haskell type for GLenum */
#undef HTYPE_GLENUM

/* Define to Haskell type for GLfloat */
#undef HTYPE_GLFLOAT

/* Define to Haskell type for GLint */
#undef HTYPE_GLINT

/* Define to Haskell type for GLshort */
#undef HTYPE_GLSHORT

/* Define to Haskell type for GLsizei */
#undef HTYPE_GLSIZEI

/* Define to Haskell type for GLubyte */
#undef HTYPE_GLUBYTE

/* Define to Haskell type for GLuint */
#undef HTYPE_GLUINT

/* Define to Haskell type for GLushort */
#undef HTYPE_GLUSHORT

/* Define to Haskell type for int */
#undef HTYPE_INT

/* Define to Haskell type for ino_t */
#undef HTYPE_INO_T

/* Define to Haskell type for long */
#undef HTYPE_LONG

/* Define to Haskell type for long long */
#undef HTYPE_LONG_LONG

/* Define to Haskell type for mode_t */
#undef HTYPE_MODE_T

/* Define to Haskell type for nlink_t */
#undef HTYPE_NLINK_T

/* Define to Haskell type for off_t */
#undef HTYPE_OFF_T

/* Define to Haskell type for pid_t */
#undef HTYPE_PID_T

/* Define to Haskell type for ptrdiff_t */
#undef HTYPE_PTRDIFF_T

/* Define to Haskell type for short */
#undef HTYPE_SHORT

/* Define to Haskell type for sig_atomic_t */
#undef HTYPE_SIG_ATOMIC_T

/* Define to Haskell type for signed char */
#undef HTYPE_SIGNED_CHAR

/* Define to Haskell type for size_t */
#undef HTYPE_SIZE_T

/* Define to Haskell type for speed_t */
#undef HTYPE_SPEED_T

/* Define to Haskell type for ssize_t */
#undef HTYPE_SSIZE_T

/* Define to Haskell type for time_t */
#undef HTYPE_TIME_T

/* Define to Haskell type for tcflag_t */
#undef HTYPE_TCFLAG_T

/* Define to Haskell type for uid_t */
#undef HTYPE_UID_T

/* Define to Haskell type for unsigned char */
#undef HTYPE_UNSIGNED_CHAR

/* Define to Haskell type for unsigned int */
#undef HTYPE_UNSIGNED_INT

/* Define to Haskell type for unsigned long */
#undef HTYPE_UNSIGNED_LONG

/* Define to Haskell type for unsigned long long */
#undef HTYPE_UNSIGNED_LONG_LONG

/* Define to Haskell type for unsigned short */
#undef HTYPE_UNSIGNED_SHORT

/* Define to Haskell type for wchar_t */
#undef HTYPE_WCHAR_T

/* The value of E2BIG.  */
#undef CCONST_E2BIG

/* The value of EACCES.  */
#undef CCONST_EACCES

/* The value of EADDRINUSE.  */
#undef CCONST_EADDRINUSE

/* The value of EADDRNOTAVAIL.  */
#undef CCONST_EADDRNOTAVAIL

/* The value of EADV.  */
#undef CCONST_EADV

/* The value of EAFNOSUPPORT.  */
#undef CCONST_EAFNOSUPPORT

/* The value of EAGAIN.  */
#undef CCONST_EAGAIN

/* The value of EALREADY.  */
#undef CCONST_EALREADY

/* The value of EBADF.  */
#undef CCONST_EBADF

/* The value of EBADMSG.  */
#undef CCONST_EBADMSG

/* The value of EBADRPC.  */
#undef CCONST_EBADRPC

/* The value of EBUSY.  */
#undef CCONST_EBUSY

/* The value of ECHILD.  */
#undef CCONST_ECHILD

/* The value of ECOMM.  */
#undef CCONST_ECOMM

/* The value of ECONNABORTED.  */
#undef CCONST_ECONNABORTED

/* The value of ECONNREFUSED.  */
#undef CCONST_ECONNREFUSED

/* The value of ECONNRESET.  */
#undef CCONST_ECONNRESET

/* The value of EDEADLK.  */
#undef CCONST_EDEADLK

/* The value of EDESTADDRREQ.  */
#undef CCONST_EDESTADDRREQ

/* The value of EDIRTY.  */
#undef CCONST_EDIRTY

/* The value of EDOM.  */
#undef CCONST_EDOM

/* The value of EDQUOT.  */
#undef CCONST_EDQUOT

/* The value of EEXIST.  */
#undef CCONST_EEXIST

/* The value of EFAULT.  */
#undef CCONST_EFAULT

/* The value of EFBIG.  */
#undef CCONST_EFBIG

/* The value of EFTYPE.  */
#undef CCONST_EFTYPE

/* The value of EHOSTDOWN.  */
#undef CCONST_EHOSTDOWN

/* The value of EHOSTUNREACH.  */
#undef CCONST_EHOSTUNREACH

/* The value of EIDRM.  */
#undef CCONST_EIDRM

/* The value of EILSEQ.  */
#undef CCONST_EILSEQ

/* The value of EINPROGRESS.  */
#undef CCONST_EINPROGRESS

/* The value of EINTR.  */
#undef CCONST_EINTR

/* The value of EINVAL.  */
#undef CCONST_EINVAL

/* The value of EIO.  */
#undef CCONST_EIO

/* The value of EISCONN.  */
#undef CCONST_EISCONN

/* The value of EISDIR.  */
#undef CCONST_EISDIR

/* The value of ELOOP.  */
#undef CCONST_ELOOP

/* The value of EMFILE.  */
#undef CCONST_EMFILE

/* The value of EMLINK.  */
#undef CCONST_EMLINK

/* The value of EMSGSIZE.  */
#undef CCONST_EMSGSIZE

/* The value of EMULTIHOP.  */
#undef CCONST_EMULTIHOP

/* The value of ENAMETOOLONG.  */
#undef CCONST_ENAMETOOLONG

/* The value of ENETDOWN.  */
#undef CCONST_ENETDOWN

/* The value of ENETRESET.  */
#undef CCONST_ENETRESET

/* The value of ENETUNREACH.  */
#undef CCONST_ENETUNREACH

/* The value of ENFILE.  */
#undef CCONST_ENFILE

/* The value of ENOBUFS.  */
#undef CCONST_ENOBUFS

/* The value of ENODATA.  */
#undef CCONST_ENODATA

/* The value of ENODEV.  */
#undef CCONST_ENODEV

/* The value of ENOENT.  */
#undef CCONST_ENOENT

/* The value of ENOEXEC.  */
#undef CCONST_ENOEXEC

/* The value of ENOLCK.  */
#undef CCONST_ENOLCK

/* The value of ENOLINK.  */
#undef CCONST_ENOLINK

/* The value of ENOMEM.  */
#undef CCONST_ENOMEM

/* The value of ENOMSG.  */
#undef CCONST_ENOMSG

/* The value of ENONET.  */
#undef CCONST_ENONET

/* The value of ENOPROTOOPT.  */
#undef CCONST_ENOPROTOOPT

/* The value of ENOSPC.  */
#undef CCONST_ENOSPC

/* The value of ENOSR.  */
#undef CCONST_ENOSR

/* The value of ENOSTR.  */
#undef CCONST_ENOSTR

/* The value of ENOSYS.  */
#undef CCONST_ENOSYS

/* The value of ENOTBLK.  */
#undef CCONST_ENOTBLK

/* The value of ENOTCONN.  */
#undef CCONST_ENOTCONN

/* The value of ENOTDIR.  */
#undef CCONST_ENOTDIR

/* The value of ENOTEMPTY.  */
#undef CCONST_ENOTEMPTY

/* The value of ENOTSOCK.  */
#undef CCONST_ENOTSOCK

/* The value of ENOTTY.  */
#undef CCONST_ENOTTY

/* The value of ENXIO.  */
#undef CCONST_ENXIO

/* The value of EOPNOTSUPP.  */
#undef CCONST_EOPNOTSUPP

/* The value of EPERM.  */
#undef CCONST_EPERM

/* The value of EPFNOSUPPORT.  */
#undef CCONST_EPFNOSUPPORT

/* The value of EPIPE.  */
#undef CCONST_EPIPE

/* The value of EPROCLIM.  */
#undef CCONST_EPROCLIM

/* The value of EPROCUNAVAIL.  */
#undef CCONST_EPROCUNAVAIL

/* The value of EPROGMISMATCH.  */
#undef CCONST_EPROGMISMATCH

/* The value of EPROGUNAVAIL.  */
#undef CCONST_EPROGUNAVAIL

/* The value of EPROTO.  */
#undef CCONST_EPROTO

/* The value of EPROTONOSUPPORT.  */
#undef CCONST_EPROTONOSUPPORT

/* The value of EPROTOTYPE.  */
#undef CCONST_EPROTOTYPE

/* The value of ERANGE.  */
#undef CCONST_ERANGE

/* The value of EREMCHG.  */
#undef CCONST_EREMCHG

/* The value of EREMOTE.  */
#undef CCONST_EREMOTE

/* The value of EROFS.  */
#undef CCONST_EROFS

/* The value of ERPCMISMATCH.  */
#undef CCONST_ERPCMISMATCH

/* The value of ERREMOTE.  */
#undef CCONST_ERREMOTE

/* The value of ESHUTDOWN.  */
#undef CCONST_ESHUTDOWN

/* The value of ESOCKTNOSUPPORT.  */
#undef CCONST_ESOCKTNOSUPPORT

/* The value of ESPIPE.  */
#undef CCONST_ESPIPE

/* The value of ESRCH.  */
#undef CCONST_ESRCH

/* The value of ESRMNT.  */
#undef CCONST_ESRMNT

/* The value of ESTALE.  */
#undef CCONST_ESTALE

/* The value of ETIME.  */
#undef CCONST_ETIME

/* The value of ETIMEDOUT.  */
#undef CCONST_ETIMEDOUT

/* The value of ETOOMANYREFS.  */
#undef CCONST_ETOOMANYREFS

/* The value of ETXTBSY.  */
#undef CCONST_ETXTBSY

/* The value of EUSERS.  */
#undef CCONST_EUSERS

/* The value of EWOULDBLOCK.  */
#undef CCONST_EWOULDBLOCK

/* The value of EXDEV.  */
#undef CCONST_EXDEV

