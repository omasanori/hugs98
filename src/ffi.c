/* --------------------------------------------------------------------------
 * This is the Hugs foreign function interface
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2002, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: ffi.c,v $
 * $Revision: 1.32 $
 * $Date: 2003/07/24 13:39:41 $
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "output.h"
#include "strutil.h"

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static Void local foreignType    Args((Int,Type));
#ifdef DOTNET
static Cell local foreignTypeTag Args((Int,Type));
#endif
static Void local foreignGet     Args((Int,Type,String,Int));
static Void local foreignPut     Args((Int,Type,String,Int));
static Void local ffiInclude     Args((Text));
static Void local ffiDeclare     Args((Int,Type,String,Int));
static Void local ffiDeclareList Args((Int,List,String));
static Void local foreignType    Args((Int,Type));
static Void local ffiGetList     Args((Int,List,String));
static Void local ffiPutList     Args((Int,List,String));
static Void local ffiCallFun     Args((Int,Text,List,List));
static Void local ffiDeclareFun  Args((Int,Text,Bool,Bool,List,Type));
static Void local ffiFunTypeCast Args((Int,List,Type));
static Void local ffiPrimProto   Args((Text,Int));
static Void local ffiPrimHeader  Args((Text,Int));
static Void local ffiReturn      Args((Type,String,Int));

static FILE* out = NIL;                /* file we're generating code into */
static List  includes = NIL;           /* files already #included         */

Void ffi(what)
Int what; {
    switch (what) {
        case RESET   : if (out) {
                           fclose(out);
                           out = NIL;
                       }
                       includes = NIL;
		       break;
    }
}

static String ffiFlags = 0; /* extra flags for compilation command line */

Void ffiSetFlags(s) 
String s; {
    if (s == 0) return;
    if (ffiFlags) {
        Int l = strlen(ffiFlags);
        ffiFlags=(char *)realloc(ffiFlags,l+strlen(s)+2);
	if (ffiFlags==0) {
	    ERRMSG(0) "String storage space exhausted"
	    EEND;
	}
	ffiFlags[l] = ' ';
	strcpy(ffiFlags+l+1,s);
    } else {
        ffiFlags = strCopy(s);
    }
}

Bool foreignNeedStubs(imps,exps)
List imps;
List exps; {
#ifndef DOTNET
 return (nonNull(imps) || nonNull(exps)); 
#else 
 if (isNull(exps)) {
   List xs;
   for (xs = imps; nonNull(xs); xs=tl(xs)) {
     if (isName(hd(xs)) && 
	 ((name(hd(xs)).foreignFlags & FFI_CCONV_DOTNET) == 0) ) {
       return TRUE;
     }
   }
   return FALSE;
 } else {
   return TRUE;
 }
#endif
}

Void foreignHeader(fn) 
String fn; {
    String fnm = mkFFIFilename(fn);
    FILE* f = fopen(fnm,"w");
    if (f == NULL) {
        ERRMSG(0) "Unable to create file '%s'", fnm
        EEND;
    }
    out = f;
    fprintf(out,"/* Machine generated file, do not modify */\n");
    fprintf(out,"#include \"HsFFI.h\"\n");
    fprintf(out,"static HugsAPI5 *hugs = 0;\n");
}

Void foreignFooter(fn,mn,is,es)
String fn;
Text mn;
List is;
List es; {
    List xs   = NIL;
    fprintf(out,"\n");

    /* Table of all primitives generated by foreign imports */
    fprintf(out,"static struct hugs_primitive hugs_primTable[] = {\n");
    for(xs=is; nonNull(xs); xs=tl(xs)) {
        Name n       = hd(xs);
#ifdef DOTNET
	if (name(n).foreignFlags & FFI_CCONV_DOTNET != 0) continue;
#endif
        fprintf(out,"    {\"%s\", ",textToStr(name(n).text));
        fprintf(out,"%d, ",name(n).arity);
        fprintf(out,"hugsprim_%s_%d},\n",textToStr(name(n).extFun),name(n).foreignId);
    }
    for(xs=es; nonNull(xs); xs=tl(xs)) {
        Name n       = hd(xs);
        Text ext     = name(n).extFun;
        Bool dynamic = inventedText(ext);
        if (dynamic) {
            fprintf(out,"    {\"%s\", 3, ",textToStr(name(n).text));
            fprintf(out,"hugsprim_%s},\n",textToStr(name(n).extFun));
        }
    }
    fprintf(out,"};\n");
    fprintf(out,"\n");

    /* The control function: rebuilds stable ptr table on RESET */
    fprintf(out,
            "static void hugs_primControl(int);\n"
            "static void hugs_primControl(what)\n"
            "int what; {\n");

    if (nonNull(es)) {
      fprintf(out,
	      "    switch (what) {\n"
              "        case %d:\n", RESET
	      );
    }

    for(xs=es; nonNull(xs); xs=tl(xs)) {
        Name n       = hd(xs);
        Text ext     = name(n).extFun;
        Bool dynamic = inventedText(ext);
        if (!dynamic) {
            fprintf(out, "        hugs_stable_for_%s = ", textToStr(ext));
            fprintf(out, "hugs->lookupName(");
            fprintf(out, "\"%s\"", textToStr(module(name(n).mod).text));
            fprintf(out, ", \"%s\"", textToStr(name(n).text));
            fprintf(out, ");\n");
        }
    }
    if (nonNull(es)) {
      fprintf(out,"    }\n");
    }
    fprintf(out, "}\n");

    /* For use as a plugin, rename the initialization function with a name  */
    /* derived from the module name, but abbreviated for limited linkers.   */
    /* example: Foreign.Marshal.Alloc yields initFMAlloc()                  */
    fprintf(out, "\n");
    fprintf(out, "#ifdef STATIC_LINKAGE\n");
    fprintf(out, "#define initModule init");
    {
	String s = textToStr(mn);
	String next;
	while ((next = strchr(s, '.')) != NULL) {
	    fprintf(out, "%c", s[0]);
	    s = next+1;
	}
	fprintf(out, "%s\n", s);
    }
    fprintf(out, "#endif\n");
    fprintf(out, "\n");

    /* Boilerplate initialization function */
    fprintf(out,
           "static struct hugs_primInfo hugs_prims = { hugs_primControl, hugs_primTable, 0 };\n"
           "\n"
           "#ifdef __cplusplus\n"
           "extern \"C\" {\n"
           "#endif\n"
           "#ifndef __cplusplus\n"
	   "DLLEXPORT(int)  HugsAPIVersion(void);\n"
           "#endif\n"
	   "DLLEXPORT(int)  HugsAPIVersion() {return (%d);}\n"
           "DLLEXPORT(void) initModule(HugsAPI5 *);\n"
           "DLLEXPORT(void) initModule(HugsAPI5 *hugsAPI) {\n"
           "    hugs = hugsAPI;\n"
           "    hugs->registerPrims(&hugs_prims);\n"
           ,HUGS_API_VERSION);
    fprintf(out,
           "}\n"
           "#ifdef __cplusplus\n"
           "}\n"
           "#endif\n"
           "\n");

    fclose(out);
    out = NIL;

    compileAndLink(fn, ffiFlags);
}

#ifdef DOTNET
static Cell foreignTypeTag(l,t)
Int    l;
Type   t; {
         if (t == typeUnit)   return mkInt(FFI_TYPE_UNIT);
    else if (t == typeChar)   return mkInt(FFI_TYPE_CHAR);
    else if (t == typeInt)    return mkInt(FFI_TYPE_INT);
    else if (t == typeInt8)   return mkInt(FFI_TYPE_INT8);
    else if (t == typeInt16)  return mkInt(FFI_TYPE_INT16);
    else if (t == typeInt32)  return mkInt(FFI_TYPE_INT32);
    else if (t == typeInt64)  return mkInt(FFI_TYPE_INT64);
    else if (t == typeWord8)  return mkInt(FFI_TYPE_WORD8);
    else if (t == typeWord16) return mkInt(FFI_TYPE_WORD16);
    else if (t == typeWord32) return mkInt(FFI_TYPE_WORD32);
    else if (t == typeWord64) return mkInt(FFI_TYPE_WORD64);
    else if (t == typeFloat)  return mkInt(FFI_TYPE_FLOAT);
    else if (t == typeDouble) return mkInt(FFI_TYPE_DOUBLE);
    else if (t == typeBool)   return mkInt(FFI_TYPE_BOOL);
    else if (t == typeAddr)   return mkInt(FFI_TYPE_ADDR);
    else if (getHead(t) == typePtr)     return mkInt(FFI_TYPE_PTR);
    else if (getHead(t) == typeFunPtr)  return mkInt(FFI_TYPE_FUNPTR);
    else if (getHead(t) == typeForeign) return mkInt(FFI_TYPE_FOREIGN);
    else if (getHead(t) == typeStable)  return mkInt(FFI_TYPE_STABLE);
    else if (getHead(t) == typeObject)  return mkInt(FFI_TYPE_OBJECT);
	 else if (getHead(t) == typeList && 
		  nthArg(1,t) == typeChar) 
		                        return mkInt(FFI_TYPE_STRING);
    else {
        ERRMSG(l) "Illegal foreign type" ETHEN
        ERRTEXT " \"" ETHEN ERRTYPE(t);
        ERRTEXT "\""
        EEND;
   }
   return 0;

} 
#endif


static Void local foreignType(l,t)
Int    l;
Type   t; {
    if      (t == typeChar)   fprintf(out,"HsChar");
    else if (t == typeInt)    fprintf(out,"HsInt");
    else if (t == typeInt8)   fprintf(out,"HsInt8");
    else if (t == typeInt16)  fprintf(out,"HsInt16");
    else if (t == typeInt32)  fprintf(out,"HsInt32");
    else if (t == typeInt64)  fprintf(out,"HsInt64");
    else if (t == typeWord8)  fprintf(out,"HsWord8");
    else if (t == typeWord16) fprintf(out,"HsWord16");
    else if (t == typeWord32) fprintf(out,"HsWord32");
    else if (t == typeWord64) fprintf(out,"HsWord64");
    else if (t == typeFloat)  fprintf(out,"HsFloat");
    else if (t == typeDouble) fprintf(out,"HsDouble");
    else if (t == typeBool)   fprintf(out,"HsBool");
    else if (t == typeAddr)   fprintf(out,"HsAddr");
    else if (getHead(t) == typePtr)    fprintf(out,"HsPtr");
    else if (getHead(t) == typeFunPtr) fprintf(out,"HsFunPtr");
    else if (getHead(t) == typeForeign)fprintf(out,"HugsForeign");
    else if (getHead(t) == typeStable) fprintf(out,"HsStablePtr");
#ifdef DOTNET
    else if (getHead(t) == typeObject) fprintf(out,"HsPtr");
#endif
    else {
        ERRMSG(l) "Illegal foreign type" ETHEN
        ERRTEXT " \"" ETHEN ERRTYPE(t);
        ERRTEXT "\""
        EEND;
   }
}

static Void local foreignGet(l,t,nm,num)
Int    l;
Type   t; 
String nm; 
Int    num; {
    if (t == typeUnit)        fprintf(out,"hugs->getUnit();\n");
    else if (t == typeChar)   fprintf(out,"%s%d = hugs->getChar();\n",       nm, num);
    else if (t == typeInt)    fprintf(out,"%s%d = hugs->getInt();\n",        nm, num);
    else if (t == typeInt8)   fprintf(out,"%s%d = hugs->getInt8();\n",       nm, num);
    else if (t == typeInt16)  fprintf(out,"%s%d = hugs->getInt16();\n",      nm, num);
    else if (t == typeInt32)  fprintf(out,"%s%d = hugs->getInt32();\n",      nm, num);
    else if (t == typeInt64)  fprintf(out,"%s%d = hugs->getInt64();\n",      nm, num);
    else if (t == typeWord8)  fprintf(out,"%s%d = hugs->getWord8();\n",      nm, num);
    else if (t == typeWord16) fprintf(out,"%s%d = hugs->getWord16();\n",     nm, num);
    else if (t == typeWord32) fprintf(out,"%s%d = hugs->getWord32();\n",     nm, num);
    else if (t == typeWord64) fprintf(out,"%s%d = hugs->getWord64();\n",     nm, num);
    else if (t == typeFloat)  fprintf(out,"%s%d = hugs->getFloat();\n",      nm, num);
    else if (t == typeDouble) fprintf(out,"%s%d = hugs->getDouble();\n",     nm, num);
    else if (t == typeBool)   fprintf(out,"%s%d = hugs->getBool();\n",       nm, num);
    else if (t == typeAddr)   fprintf(out,"%s%d = hugs->getAddr();\n",       nm, num);
    else if (getHead(t) == typePtr)    fprintf(out,"%s%d = hugs->getPtr();\n",        nm, num);
    else if (getHead(t) == typeFunPtr) fprintf(out,"%s%d = hugs->getFunPtr();\n",     nm, num);
    else if (getHead(t) == typeForeign)fprintf(out,"%s%d = hugs->getForeign();\n", nm, num);
    else if (getHead(t) == typeStable) fprintf(out,"%s%d = hugs->getStablePtr4();\n",  nm, num);
#ifdef DOTNET
    else if (getHead(t) == typeObject)    fprintf(out,"%s%d = hugs->getPtr();\n",        nm, num);
#endif
    else {
        ERRMSG(l) "Illegal outbound (away from Haskell) type" ETHEN
        ERRTEXT " \"" ETHEN ERRTYPE(t);
        ERRTEXT "\""
        EEND;
   }
}

static Void local foreignPut(l,t,nm,num)
Int    l;
Type   t; 
String nm; 
Int    num; {
    if      (t == typeUnit)   fprintf(out,"\n");
    else if (t == typeChar)   fprintf(out,"hugs->putChar(%s%d);\n",       nm, num);
    else if (t == typeInt)    fprintf(out,"hugs->putInt(%s%d);\n",        nm, num);
    else if (t == typeInt8)   fprintf(out,"hugs->putInt8(%s%d);\n",       nm, num);
    else if (t == typeInt16)  fprintf(out,"hugs->putInt16(%s%d);\n",      nm, num);
    else if (t == typeInt32)  fprintf(out,"hugs->putInt32(%s%d);\n",      nm, num);
    else if (t == typeInt64)  fprintf(out,"hugs->putInt64(%s%d);\n",      nm, num);
    else if (t == typeWord8)  fprintf(out,"hugs->putWord8(%s%d);\n",      nm, num);
    else if (t == typeWord16) fprintf(out,"hugs->putWord16(%s%d);\n",     nm, num);
    else if (t == typeWord32) fprintf(out,"hugs->putWord32(%s%d);\n",     nm, num);
    else if (t == typeWord64) fprintf(out,"hugs->putWord64(%s%d);\n",     nm, num);
    else if (t == typeFloat)  fprintf(out,"hugs->putFloat(%s%d);\n",      nm, num);
    else if (t == typeDouble) fprintf(out,"hugs->putDouble(%s%d);\n",     nm, num);
    else if (t == typeBool)   fprintf(out,"hugs->putBool(%s%d);\n",       nm, num);
    else if (t == typeAddr)   fprintf(out,"hugs->putAddr(%s%d);\n",       nm, num);
    else if (getHead(t) == typePtr)    fprintf(out,"hugs->putPtr(%s%d);\n",        nm, num);
    else if (getHead(t) == typeFunPtr) fprintf(out,"hugs->putFunPtr(%s%d);\n",     nm, num);
    else if (getHead(t) == typeForeign)fprintf(out,"hugs->putForeign(%s%d);\n", nm, num);
    else if (getHead(t) == typeStable) fprintf(out,"hugs->putStablePtr4(%s%d);\n", nm, num);
#ifdef DOTNET
    else if (getHead(t) == typeObject) fprintf(out,"hugs->putPtr(%s%d);\n",        nm, num);
#endif
    else {
        ERRMSG(l) "Illegal inbound (coming into Haskell) type" ETHEN
        ERRTEXT " \"" ETHEN ERRTYPE(t);
        ERRTEXT "\""
        EEND;
   }
}

static Void local ffiInclude(fn)                     /* Add #include  */
Text fn; {
    if (fn != -1 && !varIsMember(fn,includes)) {
        fprintf(out, "#include \"%s\"\n", textToStr(fn));
        includes = cons(mkVar(fn),includes);
    }
}

static Void local ffiDeclare(line,ty,prefix,i)       /* Declare variable  */
Int    line;
Type   ty;
String prefix; 
Int    i; {
    if (ty != typeUnit) {
        fprintf(out,"    ");
        foreignType(line,ty);
        fprintf(out," %s%d;\n",prefix,i);
    }
}

static Void local ffiReturn(ty,prefix,i)             /* Return variable  */
Type   ty;
String prefix; 
Int    i; {
    if (ty != typeUnit) {
        fprintf(out,"    return %s%d;\n",prefix,i);
    } else {
        fprintf(out,"    return;\n");
    }
}

static Void local ffiDeclareList(line,tys,prefix)   /* Declare variables */
Int    line;
List   tys; 
String prefix; {
    Int  i;
    for(i=1; nonNull(tys); tys=tl(tys),++i) {
        ffiDeclare(line,hd(tys),prefix,i);
    }
}

static Void local ffiGetList(line,tys,prefix)   /* Get values from Haskell */
Int    line;
List   tys; 
String prefix; {
    Int  i;
    for(i=1; nonNull(tys); tys=tl(tys),++i) {
        fprintf(out,"    ");
        foreignGet(line,hd(tys),prefix,i);
    }
}

static Void local ffiPutList(line,tys,prefix)    /* Put values to Haskell */
Int    line;
List   tys; 
String prefix; {
    Int  i;
    for(i=1; nonNull(tys); tys=tl(tys),++i) {
        fprintf(out,"    ");
        foreignPut(line,hd(tys),prefix,i);
    }
}

static Void local ffiDeclareFun(line,n,indirect,extraArg,argTys,resultTy)
Int  line;
Text n;
Bool indirect;
Bool extraArg; /* Add a StablePtr argument? */
List argTys;
List resultTy; {
    Int  i;
    if (resultTy == typeUnit) {
        fprintf(out,"void");
    } else {
        foreignType(line,resultTy);
    }
    if (indirect) {
        fprintf(out," (*%s)", textToStr(n));
    } else {
        fprintf(out," %s", textToStr(n));
    }
    fprintf(out,"(");
    if (extraArg) {
#if defined(__sparc__) || defined(__i386__) || defined(_X86_)
        /* On SPARC we need an additional dummy argument due to stack alignment
           restrictions, see the comment in mkThunk in builtin.c. On x86
           platforms we need it, too, but for a different reason: The "real"
           return address is still visible on the stack as an additional
           argument, but we return to a small stub which pops the stable pointer
           before the "real" return. */
        fprintf(out,"HugsStablePtr fun1, void* unusedArg");
#else
        fprintf(out,"HugsStablePtr fun1");
#endif
        if (nonNull(argTys)) {
            fprintf(out,", ");
        }
    }
    for(i=1; nonNull(argTys); argTys=tl(argTys),++i) {
        foreignType(line,hd(argTys));
        fprintf(out," arg%d",i);
        if (nonNull(tl(argTys))) {
            fprintf(out,", ");
        }
    }
    fprintf(out,")");
}

static Void local ffiFunTypeCast(line,argTys,resultTy)
Int line;
List argTys;
List resultTy; {
    Int  i;
    fprintf(out,"(");
    if (resultTy == typeUnit) {
        fprintf(out,"void");
    } else {
        foreignType(line,resultTy);
    }
    fprintf(out," (*)(");
    for(i=1; nonNull(argTys); argTys=tl(argTys),++i) {
        foreignType(line,hd(argTys));
        if (nonNull(tl(argTys))) {
            fprintf(out,", ");
        }
    }
    fprintf(out,"))");
}

static Void local ffiCallFun(line,e,argTys,resultTy)
Int  line;
Text e;
List argTys;
Type resultTy; {
    Int  i;
    fprintf(out,"    ");
    if (resultTy != typeUnit) {
        fprintf(out,"res1 = ");
    }
    fprintf(out,"%s(", textToStr(e));
    for(i=1; nonNull(argTys); argTys=tl(argTys),++i) {
        fprintf(out,"arg%d",i);
        if (nonNull(tl(argTys))) {
            fprintf(out,", ");
        }
    }
    fprintf(out,");\n");
}


/* Generate a Hugs Prim prototype.
 * name should match the C function we're calling because we know
 * that name is a valid C identifier whereas the Haskell name may
 * not be.
 */
static Void local ffiPrimProto(name,id)
Text name; 
Int  id; {
    fprintf(out,"\nstatic void hugsprim_%s_%d(HugsStackPtr);\n",textToStr(name),id);
}

/* Generate a Hugs Prim Header.
 * name should match the C function we're calling because we know
 * that name is a valid C identifier whereas the Haskell name may
 * not be.
 */
static Void local ffiPrimHeader(name,id)
Text name; 
Int  id; {
    fprintf(out,"static void hugsprim_%s_%d(HugsStackPtr hugs_root)\n", textToStr(name),id);
}

/* Generate C code for calling C functions from Haskell.
 * The code has to be compiled with a C compiler and dynamically
 * loaded.
 *
 * For example:
 * 
 *     foreign import "static fn cid" name :: Int -> Float -> IO Char
 * ==>
 *     
 *     static void hugsprim_extnm(HugsStackPtr);
 *     static void hugsprim_extnm(HugsStackPtr hugs_root)
 *     {
 *         int   arg1 = hugs->getInt();                             
 *         float arg2 = hugs->getFloat();
 *         char  res1 = ext_nm(arg1,arg2);
 *         hugs->putChar(res1);
 *         hugs->returnIO(hugs_root,1);
 *     }
 * 
 */
Void implementForeignImport(line,n,id,fn,cid,isStatic,libName,argTys,isIO,resultTy)
Int  line;
Name n;
Int  id;
Text fn;   /* Include file */
Text cid;  /* Function name */
Bool isStatic;
Text libName;
List argTys;
Bool isIO;
Type resultTy; {

#ifdef DOTNET
    if ( name(n).foreignFlags & FFI_CCONV_DOTNET ) {
      /* .NET methods are bound when invoked, just record
       * the method name + the types we're calling it at.
       * 
       */
      List params = dupList(argTys);
      Int  flags  = (Int)fn;
      map1Over(foreignTypeTag,line,params);
      
      /* Qualifying the method name with the class & namespace
       * prefix is redundant, but as a nicety we support being
       * verbose -- symmetric with static methods 
       * verbosity.
       */
      if ( ((flags & FFI_DOTNET_METHOD) != 0) &&
	   ((flags & FFI_DOTNET_STATIC) == 0) ) {
	char* meth = strrchr(textToStr(cid),'.');
	if ( (meth && *(meth+1) != '\0') ) {
	  /* Dotted name (with non-empty last component), use
	   * last component.
	   */
	  cid = findText(meth+1);
	}
      }

      name(n).number = EXECNAME;
      name(n).foreignInfo = 
	pair (cid,
	      pair(libName,
		   pair(mkInt(flags),
			pair(mkInt(isIO),
			     pair(foreignTypeTag(line,resultTy),
				  params)))));
      return;
    } else {
#endif

    ffiInclude(fn);

    ffiPrimProto(cid,id);
    ffiPrimHeader(cid,id);
    fprintf(out,"{\n");

#if 0
    /* Prototype for function we're going to call */
    fprintf(out,"    extern ");
    ffiDeclareFun(line,cid,FALSE,FALSE,argTys,resultTy);
    fprintf(out,";\n");
#endif

    ffiDeclareList(line,argTys,"arg");
    ffiDeclare(line,resultTy,"res",1);
    ffiGetList(line,argTys,"arg");
    ffiCallFun(line,cid,argTys,resultTy);
    fprintf(out,"    ");
    foreignPut(line,resultTy,"res",1);
    if (isIO || nonNull(argTys)) {
      fprintf(out,"    hugs->return%s(hugs_root,%d);\n", 
              isIO?"IO":"Id",
              resultTy==typeUnit ? 0 : 1);
    }
    fprintf(out,"}\n");
#ifdef DOTNET
    }
#endif
}

Void implementForeignImportDynamic(line,id,e,argTys,isIO,resultTy)
Int  line;
Int  id;
Text e;
List argTys;
Bool isIO;
Type resultTy; {
    ffiPrimProto(e,id);
    ffiPrimHeader(e,id);
    fprintf(out,"{\n");

    /* Declare arguments and result */
    fprintf(out,"    ");
    ffiDeclareFun(line,e,TRUE,FALSE,argTys,resultTy);
    fprintf(out,";\n");

    ffiDeclareList(line,argTys,"arg");
    ffiDeclare(line,resultTy,"res",1);
    fprintf(out,"    %s = ", textToStr(e));
    ffiFunTypeCast(line,argTys,resultTy);
    fprintf(out,"hugs->getFunPtr();\n");
    ffiGetList(line,argTys,"arg");
    ffiCallFun(line,e,argTys,resultTy);
    fprintf(out,"    ");
    foreignPut(line,resultTy,"res",1);
    if (isIO || nonNull(argTys)) {
      fprintf(out,"    hugs->return%s(hugs_root,%d);\n", 
              isIO?"IO":"Id",
              resultTy==typeUnit ? 0 : 1);
    }
    fprintf(out,"}\n");
}

/*
 * For wrappers, we generate:
 *
 * For example:
 * 
 *     foreign import "wrapper" name ::            (Int -> Float -> Char) 
 *                                   -> IO (FunPtr (Int -> Float -> Char))
 * ==>
 *     
 *     static HsChar wrapper(HugsStablePtr arg1, HsInt arg2, HsFloat arg3);
 *     static HsChar wrapper(HugsStablePtr arg1, HsInt arg2, HsFloat arg3);
 *     {
 *         HsChar res1;
 *         hugs->derefStablePtr4(arg1);                             
 *         hugs->putInt(arg2);                             
 *         hugs->putFloat(arg3);
 *         if (hugs->runIO(2)) {
 *             exit(hugs->getInt());
 *         }
 *         res1 = hugs->getChar();
 *         return res1;
 *     }
 * 
 *     static void hugsprim_name(HugsStackPtr hugs_root);
 *     static void hugsprim_name(HugsStackPtr hugs_root)
 *     {
 *         HugsStablePtr arg1 = hugs->makeStablePtr4();    
 *         void* thunk = hugs->mkThunk(&wrapper,arg1);
 *         hugs->putAddr(thunk);
 *         hugs->returnIO(hugs_root,1);
 *     }
 */
Void implementForeignImportWrapper(line,id,e,argTys,isIO,resultTy)
Int  line;
Int  id;
Text e;
List argTys;
Bool isIO;
Type resultTy; {
    /* Prototype for function we're generating */
    fprintf(out,"\nstatic ");
    ffiDeclareFun(line,e,FALSE,TRUE,argTys,resultTy);
    fprintf(out,";\n");

    /* The function wrapper */
    fprintf(out,"static ");
    ffiDeclareFun(line,e,FALSE,TRUE,argTys,resultTy);
    fprintf(out,"\n{\n");
    ffiDeclare(line,resultTy,"res",1);

    /* Push function pointer and arguments */
    fprintf(out,"    hugs->derefStablePtr4(fun1);\n");
    ffiPutList(line,argTys,"arg");

    /* Make the call and check for uncaught exception */
    /* ToDo: I'm not sure that exiting from the Hugs session is the right 
     * response to the Haskell function calling System.exit.
     */
    fprintf(out,"    if (hugs->run%s(%d)) {\n", isIO?"IO":"Id", length(argTys));
    fprintf(out,
            "        exit(hugs->getInt());\n"
            "    }\n"
            );
    fprintf(out,"    ");
    foreignGet(line,resultTy,"res",1);
    ffiReturn(resultTy,"res",1);

    /* Return result */
    fprintf(out,"}\n");

    ffiPrimProto(e,id);
    ffiPrimHeader(e,id);
    fprintf(out,
            "{\n"
            "    HugsStablePtr arg1 = hugs->makeStablePtr4();\n"
            "    void* thunk = hugs->mkThunk((HsFunPtr)%s,arg1);\n",
            textToStr(e)
            );
    fprintf(out,
            "    hugs->putAddr(thunk);\n"
            "    hugs->returnIO(hugs_root,1);\n"
            "}\n");
}

/* 
 * Generate C code for calling C functions from Haskell.
 * The code has to be compiled with a C compiler and dynamically
 * loaded.
 *
 * For example:
 * 
 *     foreign export "extnm" name :: Int -> Float -> IO Char
 * ==>
 *     
 *     HugsStablePtr hugs_stable_for_extnm = -1;
 *     char extnm(int arg1, float arg2);
 *     char extnm(int arg1, float arg2)
 *     {
 *         char  res1;
 *         hugs->putStablePtr(hugs_stable_for_extnm);
 *         hugs->putInt(arg1);                             
 *         hugs->putFloat(arg2);                             
 *         if (hugs->runIO(2)) {
 *             exit(hugs->getInt());
 *         }
 *         res1 = hugs->getChar();
 *         return res1;
 *     }
 *
 */
Void implementForeignExport(line,id,e,argTys,isIO,resultTy)
Int  line;
Int  id;
Text e;
List argTys;
Bool isIO;
Type resultTy; {

    /* Prototype for function we're generating */
    fprintf(out,"\nextern ");
    ffiDeclareFun(line,e,FALSE,FALSE,argTys,resultTy);
    fprintf(out,";\n");

    fprintf(out,"static HugsStablePtr hugs_stable_for_%s = -1;\n", textToStr(e));

    /* The function wrapper */
    ffiDeclareFun(line,e,FALSE,FALSE,argTys,resultTy);
    fprintf(out,"\n{\n");
    ffiDeclare(line,resultTy,"res",1);

    /* Push function pointer and arguments */
    fprintf(out,"    hugs->putStablePtr(hugs_stable_for_%s);\n", textToStr(e));
    ffiPutList(line,argTys,"arg");

    /* Make the call and check for uncaught exception */
    if (isIO) {
        /* ToDo: I'm not sure that exiting from the Hugs session is the right 
         * response to the Haskell function calling System.exit.
         */
        fprintf(out,"    if (hugs->runIO(%d)) {\n", length(argTys));
        fprintf(out,
                "        exit(hugs->getInt());\n"
                "    }\n"
                );
    } else {
        fprintf(out,"    hugs->ap(%d);\n", length(argTys));
    } 
    fprintf(out,"    ");
    foreignGet(line,resultTy,"res",1);
    ffiReturn(resultTy,"res",1);
    fprintf(out,"}\n");
}

/* 
 * Generate primitive for address of a C symbol.
 *
 * For example:
 * 
 *     foreign import "static & cid" name :: Addr
 * ==>
 *     
 *     static void hugsprim_name(HugsStackPtr);
 *     static void hugsprim_name(HugsStackPtr hugs_root)
 *     {
 *         extern int cid; // probably the wrong type but it doesn't matter 
 *         hugs->putAddr(&cid);
 *         hugs_returnId(1);
 *     }
 */
Void implementForeignImportLabel(line, id, fn, cid, n, ty)
Int  line;
Int  id;
Text fn;   /* Include file */
Text cid;  /* Function name */
Text n;    /* Haskell name */
Type ty; {
    ffiInclude(fn);
    ffiPrimProto(cid,id);
    ffiPrimHeader(cid,id);
    fprintf(out,"{\n");
    if (getHead(ty) == typeFunPtr)
	fprintf(out,"    hugs->putFunPtr((HsFunPtr)&%s);\n", textToStr(cid));
    else
	fprintf(out,"    hugs->putPtr(&%s);\n", textToStr(cid));
    fprintf(out,"}\n");
}

/* ToDo: 
 * chain all foreign exports together and free at end of run?
 * copy GreenCard.h into Test.c? 
 */

/*-------------------------------------------------------------------------*/
