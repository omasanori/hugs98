/*
 * Primitives needed to implement the Haskell 98 Time & CPUTime modules.
 *
 * This file has to be included by builtin.c, and won't compile on its own.
 */
 

/* --------------------------------------------------------------------------
 * Time/CPUTime control:
 * ------------------------------------------------------------------------*/

static Void timeControl Args((Int));

static Void timeControl(what)
Int what; {
}

/* --------------------------------------------------------------------------
 * Time/CPUTime primitive table:
 * ------------------------------------------------------------------------*/

PROTO_PRIM(primClockTicks);
PROTO_PRIM(primGetCPUUsage);

PROTO_PRIM(primGetClockTime);
PROTO_PRIM(primGetCalTime);
PROTO_PRIM(primMkTime);

static struct primitive timePrimTable[] = {
  /* CPUTime primitives */
  {"clockTicks",           0, primClockTicks},
  {"getCPUUsage",          2, primGetCPUUsage},
  /* Time primitives */
  {"getClockTimePrim",     2, primGetClockTime},        
  {"toCalTimePrim",        4, primGetCalTime},        
  {"toClockTimePrim",     10, primMkTime},
  {0,			0, 0}
};

static struct primInfo timePrims = { timeControl, timePrimTable, 0 };

/* --------------------------------------------------------------------------
 * Time primitives:
 * ------------------------------------------------------------------------*/

primFun(primGetClockTime) { /* :: IO (Int,Int) */

#ifdef HAVE_GETTIMEOFDAY
  struct timeval tv;
  int rc;
  
  rc = gettimeofday(&tv,NULL);
  
  if (rc == -1) {
        IOFail(mkIOError(toIOError(errno),
			 "Time.getClockTime",
			 toIOErrorDescr(errno,TRUE),
			 nameNothing));
  }
  IOReturn(ap(ap(mkTuple(2), mkInt(tv.tv_sec)),mkInt(tv.tv_usec)));
#elif HAVE_FTIME
  struct timeb tb;
  int rc = 0;
  
# ifdef __MINGW32__
  ftime(&tb);
# else
  rc = ftime(&tb);
# endif
  
  if (rc == -1) {
        IOFail(mkIOError(toIOError(errno),
			 "Time.getClockTime",
			 toIOErrorDescr(errno,TRUE),
			 nameNothing));
  }
  
  IOReturn(ap(ap(mkTuple(2),mkInt(tb.time)),mkInt(tb.millitm * 1000)));
#elif HAVE_TIME
  time_t t = time(NULL);
  
  if (t == (time_t)-1) {
        IOFail(mkIOError(toIOError(errno),
			 "Time.getClockTime",
			 toIOErrorDescr(errno,TRUE),
			 nameNothing));
  }
  IOReturn(ap(ap(mkTuple(2),mkInt(t)),mkInt(0)));
#else
  IOFail(mkIOError(nameIllegal,
		   "Time.getClockTime",
		   "operation not supported",
		   nameNothing));
#endif
}

#if IS_WINDOWS
# define timezone _timezone
#endif


primFun(primGetCalTime) { /* Int   -> Int -> IO (.....) */
                          /* isUTC    secs */
                          /* isUTC => convert time to UTC, o/wise local time */

  Int isUTC;
#if defined(HAVE_LOCALTIME) && defined(HAVE_GMTIME)
  time_t secs;
  struct tm* tm;
  char* zoneNm = NULL;
  Int utcOff;
  Cell zoneStr = NIL;

  IntArg(isUTC,4);
  IntArg(secs,3);
  
  if (isUTC) {
    tm=gmtime(&secs);
  } else {
    tm=localtime(&secs);
  }
  
  /* Warning - ugliness. */
# ifdef HAVE_TM_ZONE
  zoneNm = (char*)tm->tm_zone;
# elif HAVE_TZNAME || IS_WINDOWS
  /* ToDo: fix autoconf macro AC_STRUCT_TIMEZONE so that it will recognise
   *       mingw's _tzname global. For now, force it.
   */
  zoneNm = (char*)(tm->tm_isdst ? tzname[1] : tzname[0]);
# else
  /* Don't know how to get at the timezone name, complain louder? */
  zoneNm = NULL;
# endif

# ifdef HAVE_TM_ZONE
  utcOff = tm->tm_gmtoff;
# elif HAVE_TIMEZONE
#  ifdef HAVE_ALTZONE
  utcOff = (tm->tm_isdst ? altzone : timezone);
#  else
  /* Assume DST adjustment is 1 hour */
  utcOff = (tm->tm_isdst ? (timezone - 3600) : timezone);
#  endif
# else
  /* Again, complain louder? */
  utcOff = 0;
# endif

  pushString(zoneNm);
  zoneStr = pop();

  IOReturn(ap(ap(ap(ap(ap(ap(ap(ap(ap(ap(ap(mkTuple(11),mkInt(tm->tm_sec)),
					 mkInt(tm->tm_min)),
				      mkInt(tm->tm_hour)),
				   mkInt(tm->tm_mday)),
				mkInt(tm->tm_mon)),
			     mkInt(tm->tm_year)),
			  mkInt(tm->tm_wday)),
		       mkInt(tm->tm_yday)),
		    mkInt(tm->tm_isdst)),
		 zoneStr),
	      mkInt(utcOff)));
#else

  IntArg(isUTC,4);

  IOFail(mkIOError(nameIllegal,
		   (isUTC ? "Time.toUTCTime" : "Time.toCalendarTime"),
		   "operation not supported",
		   nameNothing));
#endif

}

primFun(primMkTime) { /* Int{-year-}  -> Int{-month-} -> Int{-day-} ->
                         Int{-hour-}  -> Int{-mins-}  -> Int{-sec-} ->
			 Int{-tz offset-} -> Int{-isdst-} -> IO Int{-secs since Epoch-} */
#ifdef HAVE_MKTIME
  Int year, month, day;
  Int hour,min,sec;
  Int tz,isDst;

  struct tm tm;
  time_t t;
  
  IntArg(year,10);
  IntArg(month,9);
  IntArg(day,8);
  IntArg(hour,7);
  IntArg(min,6);
  IntArg(sec,5);
  IntArg(tz,4);
  IntArg(isDst,3);
  
  tm.tm_year = year;
  tm.tm_mon  = month;
  tm.tm_mday = day;
  tm.tm_hour = hour;
  tm.tm_min  = min;
  tm.tm_sec  = sec;
  /* The OpenGroup spec suggests that setting tm_isdst to a neg. value,
     makes mktime() try to figure this out on its own. */
  tm.tm_isdst = -1;
  
  t = mktime(&tm);
  
  if (t ==(time_t)-1) {
        IOFail(mkIOError(toIOError(errno),
			 "Time.toClockTime",
			 toIOErrorDescr(errno,TRUE),
			 nameNothing));
  }
  /* mktime() assumes that the given time was local, but we might have
     been passed an UTC cal. time, so we now have to add the UTC
     offset, that is, the difference between toClockTime's UTC offset and
     the UTC offset returned by mktime().
  */
# ifdef HAVE_TM_ZONE
  tz -= tm.tm_gmtoff;
# elif HAVE_TIMEZONE
#  ifdef HAVE_ALTZONE
  tz -= (tm.tm_isdst ? altzone : timezone);
#  else
  /* Assume DST adjustment is 1 hour */
  tz -= (tm.tm_isdst ? (timezone - 3600) : timezone);
#  endif
# else
  /* Unknown, assume nothing */
# endif
  
  IOReturn(mkInt(t+tz));
#else
  IOFail(mkIOError(nameIllegal,
		   "Time.toClockTime",
		   "operation not supported",
		   nameNothing));
#endif
}

/* --------------------------------------------------------------------------
 * CPUTime primitives:
 * ------------------------------------------------------------------------*/

#ifdef CLK_TCK
CAFInt(primClockTicks, CLK_TCK)
#elif defined(CLOCKS_PER_SEC)
CAFInt(primClockTicks, CLOCKS_PER_SEC)
#else
CAFInt(primClockTicks, sysconf(_SC_CLK_TCK))
#endif

/* 
 * The code for grabbing the process times has been lifted from GHC.
 * Don't feel too bad about that, since I wrote and maintained it. 
 */
primFun(primGetCPUUsage) { /* IO (Int,Int,Int,Int) */
  int userSec, userNSec;
  int sysSec,  sysNSec;
#if !IS_WINDOWS
#if defined(HAVE_GETRUSAGE) /* && ! irix_TARGET_OS && ! solaris2_TARGET_OS */
    struct rusage t;

    getrusage(RUSAGE_SELF, &t);
    userSec  = t.ru_utime.tv_sec;
    userNSec = 1000 * t.ru_utime.tv_usec;
    sysSec   = t.ru_stime.tv_sec;
    sysNSec  = 1000 * t.ru_stime.tv_usec;

#else
# if defined(HAVE_TIMES)
    struct tms t;
#  if defined(CLK_TCK)
#   define ticks CLK_TCK
#  else
    long ticks;
    ticks = sysconf(_SC_CLK_TCK);
#  endif

    times(&t);
    userSec  = t.tms_utime / ticks;
    userNSec = (t.tms_utime - userSec * ticks) * (1000000000 / ticks);
    sysSec   = t.tms_stime / ticks;
    sysNSec  = (t.tms_stime - sysSec * ticks) * (1000000000 / ticks);

# else
    IOFail(mkIOError(nameIllegal,
		     "CPUTime.getCPUTime",
		     "illegal operation",
		     nameNothing));
# endif
#endif
#else
/* Win32 version */

#ifdef _MSC_VER
#define NS_PER_SEC 10000000
#else
#define NS_PER_SEC 10000000LL
#endif
#define FT2usecs(ll,ft)    \
    (ll)=(ft).dwHighDateTime; \
    (ll) <<= 32;              \
    (ll) |= (ft).dwLowDateTime;

    FILETIME creationTime, exitTime, kernelTime, userTime;
#ifdef _MSC_VER
    unsigned __int64 uT, kT;
#else
    unsigned long long uT, kT;
#endif
 
    /* Notice that the 'process time' includes the time used
       by all the threads of a process, all of which may not
       be kept busy running the Hugs interpreter...
    */
    if (!GetProcessTimes (GetCurrentProcess(), &creationTime,
		          &exitTime, &kernelTime, &userTime)) {
	/* Probably on a Win95 box..*/
        userSec  = 0;
        userNSec = 0;
        sysSec   = 0;
        sysNSec  = 0;
    } else {

      FT2usecs(uT, userTime);
      FT2usecs(kT, kernelTime);

      userSec  = (unsigned int)(uT / NS_PER_SEC);
      userNSec = (unsigned int)((uT - userSec * NS_PER_SEC) * 100);
      sysSec   = (unsigned int)(kT / NS_PER_SEC);
      sysNSec  = (unsigned int)((kT - sysSec * NS_PER_SEC) * 100);
    }
#endif
    IOReturn(ap(ap(ap(ap( mkTuple(4), mkInt(userSec)),
		      mkInt(userNSec)),
		   mkInt(sysSec)),
		mkInt(sysNSec)));
}

