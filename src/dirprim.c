/*
 * Primitives needed to implement the Haskell 98 Directory module.
 *
 * This file has to be included by builtin.c, and won't compile on its own.
 */
 

/* --------------------------------------------------------------------------
 * Directory control:
 * ------------------------------------------------------------------------*/

static Void dirControl Args((Int));

static Void dirControl(what)
Int what; {
}

/* --------------------------------------------------------------------------
 * Directory primitive table:
 * ------------------------------------------------------------------------*/

PROTO_PRIM(primCreateDirectory);
PROTO_PRIM(primRemoveDirectory);
PROTO_PRIM(primRemoveFile);
PROTO_PRIM(primRenameDirectory);
PROTO_PRIM(primRenameFile);
PROTO_PRIM(primGetDirectory);
PROTO_PRIM(primSetDirectory);
PROTO_PRIM(primFileExist);
PROTO_PRIM(primDirExist);
PROTO_PRIM(primGetPermissions);
PROTO_PRIM(primSetPermissions);
PROTO_PRIM(primGetDirContents);
PROTO_PRIM(primGetModTime);

static struct primitive dirPrimTable[] = {
  {"createDirectory",      3, primCreateDirectory},
  {"removeDirectory",      3, primRemoveDirectory},
  {"removeFile",           3, primRemoveFile},
  {"renameDirectory",      4, primRenameDirectory},
  {"renameFile",           4, primRenameFile},
  {"getCurrentDirectory",  2, primGetDirectory},
  {"setCurrentDirectory",  3, primSetDirectory},
  {"doesFileExist",        3, primFileExist},
  {"doesDirectoryExist",   3, primDirExist},
  {"getPerms",             3, primGetPermissions},
  {"setPerms",             7, primSetPermissions},
  {"getDirContents",       3, primGetDirContents},
  {"getModTime",           3, primGetModTime},

  {0,			0, 0}
};

static struct primInfo dirPrims = { dirControl, dirPrimTable, 0 };

/* --------------------------------------------------------------------------
 * Directory primitives:
 * ------------------------------------------------------------------------*/

primFun(primCreateDirectory) { /* create a directory, :: String -> IO ()   */
  int rc;
  String s = evalName(IOArg(1));
  
  if (!s) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.createDirectory",
		     "illegal directory name",
		     IOArg(1)));
  }
  
#if defined(_MSC_VER) || defined(__MINGW__)
   rc = mkdir(s);
#else
   rc = mkdir(s,0777);
#endif
   if (rc != 0) {
      IOFail(mkIOError(toIOError(errno),
		       "Directory.createDirectory",
		       toIOErrorDescr(errno,FALSE),
		       IOArg(1)));
   }
  IOReturn(nameUnit);
}

primFun(primRemoveDirectory) { /* remove a directory	   */
  int rc;
  String s = evalName(IOArg(1));
  
  if (!s) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.removeDirectory",
		     "illegal directory name",
		     IOArg(1)));
  }
  
   rc = rmdir(s);

   if (rc != 0) {
     IOFail(mkIOError(toIOError(errno),
		      "Directory.removeDirectory",
		      toIOErrorDescr(errno,FALSE),
		      IOArg(1)));
   }
  IOReturn(nameUnit);
}

primFun(primRemoveFile) { /* remove a file	   */
  int rc;
  String s = evalName(IOArg(1));
  
  if (!s) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.removeFile",
		     "illegal file name",
		     IOArg(1)));
  }
  
   rc = unlink(s);

   if (rc != 0) {
     IOFail(mkIOError(toIOError(errno),
		      "Directory.removeFile",
		      toIOErrorDescr(errno,TRUE),
		      IOArg(1)));
   }
  IOReturn(nameUnit);
}

/* Pair of macros for creating temporary strings */
#ifdef HAVE_ALLOCA
# define ALLOC_STRING(x) (String)alloca(sizeof(char)*(x + 1))
# define FREE_STRING(x)
#elif HAVE__ALLOCA
# define ALLOC_STRING(x) (String)_alloca(sizeof(char)*(x + 1))
# define FREE_STRING(x)
#else
# define ALLOC_STRING(x) (String)malloc(sizeof(char)*(x + 1))
# define FREE_STRING(x)  free(x)
#endif

primFun(primRenameDirectory) { /* rename a directory	   */
  int rc;
  String tmpStr;
  String to;
  String from;

  tmpStr = evalName(IOArg(1));
  to = ALLOC_STRING(strlen(tmpStr));
  strcpy(to, tmpStr);

  from = evalName(IOArg(2));

  if (!from) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.renameDirectory",
		     "illegal directory name",
		     IOArg(2)));
  }

  if (!to) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.renameDirectory",
		     "illegal directory name",
		     IOArg(1)));
  }
  
  rc = rename(from,to);

  FREE_STRING(to);

  if (rc != 0) {
     IOFail(mkIOError(toIOError(errno),
		      "Directory.renameDirectory",
		      toIOErrorDescr(errno,FALSE),
		      IOArg(1)));
  }
  IOReturn(nameUnit);
}

primFun(primRenameFile) { /* rename a file	   */
  int rc;
  String tmpStr;
  String to;
  String from;

  tmpStr = evalName(IOArg(1));
  to = ALLOC_STRING(strlen(tmpStr));
  strcpy(to, tmpStr);

  from = evalName(IOArg(2));
  
  if (!from) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.renameFile",
		     "illegal directory name",
		     IOArg(1)));
  }
  
  if (!to) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.renameFile",
		     "illegal directory name",
		     IOArg(2)));
  }
  
  rc = rename(from,to);
  
  FREE_STRING(to);

  if (rc != 0) {
    IOFail(mkIOError(toIOError(errno),
		     "Directory.renameFile",
		     toIOErrorDescr(errno,TRUE),
		     IOArg(1)));
  }
  IOReturn(nameUnit);
}


primFun(primGetDirectory) { /* IO String - get current directory. */
  int rc;
  char buffer[FILENAME_MAX+1];
  if ((char*)(getcwd(buffer,FILENAME_MAX)) != (char*)NULL) {    
    pushString(buffer);
    IOReturn(pop());
  } else {
    IOFail(mkIOError(toIOError(errno),
		     "Directory.getCurrentDirectory",
		     toIOErrorDescr(errno,FALSE),
		     IOArg(1)));
  }
}

primFun(primSetDirectory) { /* String -> IO () - set current directory. */
  int rc;
  String s = evalName(IOArg(1));

  if (!s) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.setCurrentDirectory",
		     "illegal directory name",
		     IOArg(1)));
  }
  
   rc = chdir(s);

   if (rc != 0) {
     IOFail(mkIOError(toIOError(errno),
		      "Directory.setCurrentDirectory",
		      toIOErrorDescr(errno,FALSE),
		      IOArg(1)));
   }
   IOReturn(nameUnit);
}

primFun(primFileExist) { /* FilePath -> IO Bool - check to see if file exists. */
  int rc;
  String s = evalName(IOArg(1));
  struct stat st;

  if (!s) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.doesFileExist",
		     "illegal file name",
		     IOArg(1)));
  }
  
  rc = stat(s, &st);
  
  if (rc < 0) {
    IOReturn(nameFalse);
  } else {
    IOBoolResult(((st.st_mode & S_IFDIR) != S_IFDIR));
  }
}

primFun(primDirExist) { /* FilePath -> IO Bool - check to see if directory exists. */
  int rc;
  String s = evalName(IOArg(1));
  struct stat st;

  if (!s) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.doesDirectoryExist",
		     "illegal directory name",
		     IOArg(1)));
  }
  
  rc = stat(s, &st);
  
  if (rc < 0) {
    IOReturn(nameFalse);
  } else {
    IOBoolResult(((st.st_mode & S_IFDIR) == S_IFDIR));
  }
}

#define IS_FLAG_SET(x,flg) ((x.st_mode & flg) == flg)
#define ToBool(v) ( (v) ? nameTrue : nameFalse)

primFun(primGetPermissions) { /* FilePath -> IO (Bool,Bool,Bool,Bool) */
  int rc;
  String s = evalName(IOArg(1));
  struct stat st;

  if (!s) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.getPermissions",
		     "illegal file name",
		     IOArg(1)));
  }
  
  rc = stat(s, &st);
  
  if (rc != 0) {
    IOFail(mkIOError(toIOError(errno),
		     "Directory.getPermissions",
		     toIOErrorDescr(errno,TRUE),
		     IOArg(1)));
  } else {
    IOReturn(ap(ap(ap(ap( mkTuple(4),
			  ToBool(IS_FLAG_SET(st, S_IREAD))),
		      ToBool(IS_FLAG_SET(st, S_IWRITE))),
		   ToBool(IS_FLAG_SET(st, S_IEXEC) && !IS_FLAG_SET(st,S_IFDIR))),
		ToBool(IS_FLAG_SET(st,S_IEXEC) && IS_FLAG_SET(st,S_IFDIR))));
  }
}

#define EVAL_BOOL(x,y) \
   eval(y);\
   if (whnfHead==nameTrue) { \
      x = TRUE; \
   } else if (whnfHead==nameFalse) { \
      x = FALSE; \
   } else { \
      IOFail(mkIOError(nameIllegal, \
	     "Directory.setPermissions", \
	     "illegal flag", \
	     nameNothing)); \
   }

#ifdef _MSC_VER
#define READ_FLAG   S_IREAD
#define WRITE_FLAG  S_IWRITE
#define EXEC_FLAG   S_IEXEC
#else
#define READ_FLAG   S_IRUSR
#define WRITE_FLAG  S_IWUSR
#define EXEC_FLAG   S_IXUSR
#endif

#define SET_CHMOD_FLAG(x,y)  (x ? y : 0)

primFun(primSetPermissions) { /* FilePath -> Bool -> Bool -> Bool -> Bool -> IO () */
  int rc;
  String str;

  Bool   r;
  Bool   w;
  Bool   e;
  Bool   s;
  
  EVAL_BOOL(s, IOArg(1));
  EVAL_BOOL(e, IOArg(2));
  EVAL_BOOL(w, IOArg(3));
  EVAL_BOOL(r, IOArg(4));
  
  str = evalName(IOArg(5));
  
  if (!str) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.setPermissions",
		     "illegal file name",
		     IOArg(5)));
  }

  rc = chmod(str,
	     SET_CHMOD_FLAG(r, READ_FLAG)  |
	     SET_CHMOD_FLAG(w, WRITE_FLAG) |
	     SET_CHMOD_FLAG(e||s, EXEC_FLAG));
	     
  if (rc != 0) {
    IOFail(mkIOError(toIOError(errno),
		     "Directory.setPermissions",
		     toIOErrorDescr(errno,TRUE),
		     IOArg(5)));
  } else {
    IOReturn(nameUnit);
  }
  
}

/* Pedantically remove these local defs. */
#undef READ_FLAG
#undef WRITE_FLAG
#undef EXEC_FLAG
#undef SET_CHMOD_FLAG

primFun(primGetDirContents) { /* FilePath -> IO [FilePath] */
  int rc;
#ifdef _MSC_VER
  /* The MS CRT doesn't provide opendir()/readdir(), but uses
     the 'std' MS find first/next/close group of functions for
     iterating over the contents of a directory. */
  long dirHandle;
  struct _finddata_t fData;
  char buffer[FILENAME_MAX+20];
  struct stat st;
  Cell ls;
  String fName = evalName(IOArg(1));
  
  if (!fName) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.getDirectoryContents",
		     "illegal directory name",
		     IOArg(1)));
  }
  
  if (strlen(fName) > FILENAME_MAX) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.getDirectoryContents",
		     "file name too long",
		     IOArg(1)));
  }
  
  /* First, check whether the directory exists... */
  if ( (stat(fName, &st) < 0) || ((st.st_mode & S_IFDIR) != S_IFDIR) ) {
    IOFail(mkIOError(toIOError(errno),
		     "Directory.getDirectoryContents",
		     toIOErrorDescr(errno,FALSE),
		     IOArg(1)));
  }
  
  strcpy(buffer, fName);
  strcat(buffer, "\\*.*");

  dirHandle = _findfirst(buffer, &fData);
  rc = dirHandle;
  
  ls = nameNil;
  
  while (rc >= 0) {
    pushString(fData.name);
    ls = ap2(nameCons, pop(), ls);
    rc = _findnext(dirHandle, &fData);
  }
  if (errno != ENOENT) {
    IOFail(mkIOError(toIOError(errno),
		     "Directory.getDirectoryContents",
		     toIOErrorDescr(errno,FALSE),
		     IOArg(1)));
  }

  /* Close and release resources */
  rc = _findclose(dirHandle);
  if (rc == -1 && errno != ENOENT) {
    IOFail(mkIOError(toIOError(errno),
		     "Directory.getDirectoryContents",
		     toIOErrorDescr(errno,FALSE),
		     IOArg(1)));
  }
  IOReturn(ls);
#elif defined(HAVE_DIRENT_H)
  /* opendir() / readdir() implementation. */
  DIR* dir;
  struct dirent* pDir;
  Cell ls;
  String fName = evalName(IOArg(1));
  
  if (!fName) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.getDirectoryContents",
		     "illegal file name",
		     IOArg(1)));
  }
  
  dir = opendir(fName);
  
  if (dir == NULL) {
    IOFail(mkIOError(toIOError(errno),
		     "Directory.getDirectoryContents",
		     toIOErrorDescr(errno,FALSE),
		     IOArg(1)));
  }
  
  ls = nameNil;
  
  /* To ensure that the test below doesn't
     succeed just because the impl of readdir()
     'forgot' to reset 'errno', do it ourselves. */
  errno = 0;

  while (pDir = readdir(dir)) {
    pushString(pDir->d_name);
    ls = ap2(nameCons, pop(), ls);
  }

  if (errno != 0) {
    int rc = errno;
    closedir(dir);
    IOFail(mkIOError(toIOError(rc),
		     "Directory.getDirectoryContents",
		     toIOErrorDescr(rc,FALSE),
		     IOArg(1)));
  }

  closedir(dir);
  IOReturn(ls);

#else
  /* Sorry, don't know how to access a directory on your platform */
    IOFail(mkIOError(nameIllegal,
		     "Directory.getDirectoryContents",
		     "operation not supported",
		     IOArg(1)));
#endif
}

primFun(primGetModTime) { /* FilePath -> IO Int{-time_t-} - get the mod. time of the file/directory. */
  int rc;
  String s = evalName(IOArg(1));
  struct stat st;

  if (!s) {
    IOFail(mkIOError(nameIllegal,
		     "Directory.getModificationTime",
		     "illegal file name",
		     IOArg(1)));
  }
  
  rc = stat(s, &st);
  
  if (rc < 0) {
    IOFail(mkIOError(toIOError(errno),
		     "Directory.getModificationTime",
		     toIOErrorDescr(errno,TRUE),
		     IOArg(1)));
  } else {
    IOReturn(mkInt(st.st_mtime));
  }
}