/**
    ecCompile.c - Interface to the compiler

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"
#include    "ecCompiler.h"

/***************************** Forward Declarations ***************************/

static void compileError(EcCompiler *cp, cchar *fmt, ...);
static int compileInner(EcCompiler *cp, int argc, char **argv);
static EjsObj *loadScriptLiteral(Ejs *ejs, EjsString *script, cchar *cache);
static EjsObj *loadScriptFile(Ejs *ejs, cchar *path, cchar *cache);
static void manageCompiler(EcCompiler *cp, int flags);

/************************************ Code ************************************/

EcCompiler *ecCreateCompiler(Ejs *ejs, int flags)
{
    EcCompiler      *cp;

    if ((cp = mprAllocObj(ejs, EcCompiler, manageCompiler)) == 0) {
        return 0;
    }
    mprAddRoot(cp);
    cp->ejs = ejs;
    cp->strict = 0;
    cp->tabWidth = EC_TAB_WIDTH;
    cp->warnLevel = 1;
    cp->shbang = 1;
    cp->optimizeLevel = 9;
    cp->warnLevel = 1;

    if (flags & EC_FLAGS_DOC) {
        cp->doc = 1;
    }
    if (flags & EC_FLAGS_BIND) {
        cp->bind = 1;
    }
    if (flags & EC_FLAGS_DEBUG) {
        cp->debug = 1;
    }
    if (flags & EC_FLAGS_MERGE) {
        cp->merge = 1;
    }
    if (flags & EC_FLAGS_NO_OUT) {
        cp->noout = 1;
    }
    if (flags & EC_FLAGS_VISIBLE) {
        cp->visibleGlobals = 1;
    }
    if (ecResetModuleList(cp) < 0) {
        return 0;
    }
    ecInitLexer(cp);
    ecResetParser(cp);
    return cp;
}

        
static void manageCompiler(EcCompiler *cp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(cp->certFile);
        mprMark(cp->docToken);
        mprMark(cp->extraFiles);
        mprMark(cp->file);
        mprMark(cp->keywords);
        mprMark(cp->peekToken);
        mprMark(cp->putback);
        mprMark(cp->freeTokens);
        mprMark(cp->freeStates);
        mprMark(cp->state);
        mprMark(cp->stream);
        mprMark(cp->token);
        mprMark(cp->outputFile);
        mprMarkList(cp->fixups);
        mprMarkList(cp->require);
        mprMarkList(cp->modules);
        mprMark(cp->errorMsg);
    }
}


int ecCompile(EcCompiler *cp, int argc, char **argv)
{
    Ejs     *ejs;
    int     rc, saveCompiling;

    ejs = cp->ejs;
    saveCompiling = ejs->compiling;
    ejs->compiling = 1;
    rc = compileInner(cp, argc, argv);
    ejs->compiling = saveCompiling;
    return rc;
}


static int compileInner(EcCompiler *cp, int argc, char **argv)
{
    Ejs         *ejs;
    EjsModule   *mp;
    EcNode      **nodes;
    EjsBlock    *block;
    cchar       *ext;
    char        *msg;
    int         i, j, next, nextModule, lflags, rc;

    ejs = cp->ejs;
    nodes = (EcNode**) mprAlloc(cp->ejs, sizeof(EcNode*) * argc);
    if (nodes == 0) {
        return EJS_ERR;
    }
    /*
        Warn about source files mentioned multiple times.
     */
    for (i = 0; i < argc; i++) {
        for (j = 0; j < argc; j++) {
            if (i == j) {
                continue;
            }
            if (mprSamePath(cp, argv[i], argv[j])) {
                compileError(cp, "Loading source %s multiple times. Ignoring extra copies.", argv[i]);
                return EJS_ERR;
            }
        }
        if (cp->outputFile && mprSamePath(cp, cp->outputFile, argv[i])) {
            compileError(cp, "Output file is the same as input file: %s", argv[i]);
            return EJS_ERR;
        }
    }

    /*
        Compile source files and load any module files
     */
    for (i = 0; i < argc && !cp->fatalError; i++) {
        ext = mprGetPathExtension(cp, argv[i]);
        if (scasecmp(ext, EJS_MODULE_EXT) == 0 || scasecmp(ext, BLD_SHOBJ) == 0) {
            nextModule = ejsGetLength(ejs, ejs->modules);
            lflags = cp->strict ? EJS_LOADER_STRICT : 0;
            if ((rc = ejsLoadModule(cp->ejs, ejsCreateStringFromAsc(ejs, argv[i]), -1, -1, lflags)) < 0) {
                msg = mprAsprintf(cp, "Error initializing module %s\n%s", argv[i], ejsGetErrorMsg(cp->ejs, 1));
                EcLocation loc;
                loc.filename = sclone(cp, argv[i]);
                if (rc == MPR_ERR_CANT_INITIALIZE) {
                    ecError(cp, "Error", &loc, msg);
                } else {
                    ecError(cp, "Error", &loc, msg);
                }
                mprFree(msg);
                return EJS_ERR;
            }
            if (cp->merge) {
                /*
                    If merging, we must emit the loaded module into the output. So add to the compiled modules list.
                 */
                for (next = nextModule; (mp = mprGetNextItem(ejs->modules, &next)) != 0; ) {
                    if (mprLookupItem(cp->modules, mp) < 0 && mprAddItem(cp->modules, mp) < 0) {
                        compileError(cp, "Can't add module %s", mp->name);
                    }
                }
            }
            nodes[i] = 0;
        } else  {
            ecParseFile(cp, argv[i], &nodes[i]);
        }
    }

    /*
        Allocate the eval frame stack. This is used for property lookups. We have one dummy block at the top always.
        MOB -- why ?
     */
    block = ejsCreateBlock(ejs, 0);
    ejsSetName(block, MPR_NAME("Compiler"));
    ejsPushBlock(ejs, block);
    
    /*
        Process the internal representation and generate code
     */
    if (!cp->parseOnly && cp->errorCount == 0) {
        ecResetParser(cp);
        if (ecAstProcess(cp, argc, nodes) < 0) {
            ejsPopBlock(ejs);
            mprFree(nodes);
            return EJS_ERR;
        }
        if (cp->errorCount == 0) {
            ecResetParser(cp);
            if (ecCodeGen(cp, argc, nodes) < 0) {
                ejsPopBlock(ejs);
                mprFree(nodes);
                return EJS_ERR;
            }
        }
    }
    ejsPopBlock(ejs);
    mprFree(nodes);
    if (cp->errorCount > 0) {
        return EJS_ERR;
    }
    /*
        Add compiled modules to the interpreter
     */
    for (next = 0; ((mp = (EjsModule*) mprGetNextItem(cp->modules, &next)) != 0); ) {
        ejsAddModule(cp->ejs, mp);
    }
    return 0;
}


int ejsInitCompiler(EjsService *service)
{
    service->loadScriptLiteral = loadScriptLiteral;
    service->loadScriptFile = loadScriptFile;
    return 0;
}


//  MOB -- remove these
/*
    Load a script file. This indirect routine is used by the core VM to compile a file when required.
 */
static EjsObj *loadScriptFile(Ejs *ejs, cchar *path, cchar *cache)
{
    if (ejsLoadScriptFile(ejs, path, cache, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG | EC_FLAGS_THROW) < 0) {
        return 0;
    }
    return ejs->result;
}


//  MOB -- remove these
/*
    Function for ejs->loadScriptLiteral. This indirect routine is used by the core VM to compile a script when required.
 */
static EjsObj *loadScriptLiteral(Ejs *ejs, EjsString *script, cchar *cache)
{
    if (ejsLoadScriptLiteral(ejs, script, cache, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG | EC_FLAGS_THROW) < 0) {
        return 0;
    }
    return ejs->result;
}


/*
    Load and initialize a script file
 */
int ejsLoadScriptFile(Ejs *ejs, cchar *path, cchar *cache, int flags)
{
    EcCompiler      *ec;

    if ((ec = ecCreateCompiler(ejs, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (cache) {
        ec->noout = 0;
        ecSetOutputFile(ec, cache);
    } else {
        ec->noout = 1;
    }
    if (ecCompile(ec, 1, (char**) &path) < 0) {
        if (flags & EC_FLAGS_THROW) {
            ejsThrowSyntaxError(ejs, "%s", ec->errorMsg ? ec->errorMsg : "Can't parse script");
        }
        mprFree(ec);
        return EJS_ERR;
    }
    mprFree(ec);
    if (ejsRun(ejs) < 0) {
        return EJS_ERR;
    }
    return 0;
}


/*
    Load and initialize a script literal
 */
int ejsLoadScriptLiteral(Ejs *ejs, EjsString *script, cchar *cache, int flags)
{
    EcCompiler      *cp;
    cchar           *path;

    if ((cp = ecCreateCompiler(ejs, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (cache) {
        cp->noout = 0;
        ecSetOutputFile(cp, cache);
    } else {
        cp->noout = 1;
    }
    if (ecOpenMemoryStream(cp, ejsToMulti(ejs, script), script->length) < 0) {
        mprError(ejs, "Can't open memory stream");
        mprFree(cp);
        return EJS_ERR;
    }
    path = "__script__";
    if (ecCompile(cp, 1, (char**) &path) < 0) {
        if (flags & EC_FLAGS_THROW) {
            ejsThrowSyntaxError(ejs, "%s", cp->errorMsg ? cp->errorMsg : "Can't parse script");
        }
        mprFree(cp);
        return EJS_ERR;
    }
    ecCloseStream(cp);
    if (ejsRun(ejs) < 0) {
        mprFree(cp);
        return EJS_ERR;
    }
    mprFree(cp);
    return 0;
}


/*
    One-line embedding. Evaluate a file. This will compile and interpret the given Ejscript source file.
 */
int ejsEvalFile(cchar *path)
{
    EjsService      *service;   
    Ejs             *ejs;
    Mpr             *mpr;

    mpr = mprCreate(0, NULL, NULL);
    if ((service = ejsCreateService(mpr)) == 0) {
        mprFree(mpr);
        return MPR_ERR_MEMORY;
    }
    if ((ejs = ejsCreateVm(service, NULL, NULL, 0, NULL, 0)) == 0) {
        mprFree(mpr);
        return MPR_ERR_MEMORY;
    }
    if (ejsLoadScriptFile(ejs, path, NULL, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG) == 0) {
        ejsReportError(ejs, "Error in program");
        mprFree(mpr);
        return MPR_ERR;
    }
    mprFree(mpr);
    return 0;
}


/*
    One-line embedding. Evaluate a script. This will compile and interpret the given script.
 */
int ejsEvalScript(cchar *script)
{
    EjsService      *service;   
    Ejs             *ejs;
    Mpr             *mpr;

    mpr = mprCreate(0, NULL, NULL);
    if ((service = ejsCreateService(mpr)) == 0) {
        mprFree(mpr);
        return MPR_ERR_MEMORY;
    }
    if ((ejs = ejsCreateVm(service, NULL, NULL, 0, NULL, 0)) == 0) {
        mprFree(mpr);
        return MPR_ERR_MEMORY;
    }
    if (ejsLoadScriptLiteral(ejs, ejsCreateStringFromAsc(ejs, script), NULL, EC_FLAGS_NO_OUT | EC_FLAGS_DEBUG) == 0) {
        ejsReportError(ejs, "Error in program");
        mprFree(mpr);
        return MPR_ERR;
    }
    mprFree(mpr);
    return 0;
}


static void compileError(EcCompiler *cp, cchar *fmt, ...)
{
    va_list     args;


    cp->errorCount++;
    cp->error = 1;
    va_start(args, fmt);
    ecError(cp, "Error", NULL, fmt, args);
    va_end(args);
}


void ecError(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    ecErrorv(cp, severity, loc, fmt, args);
    va_end(args);
}


/*
    Create a line of spaces with an "^" pointer at the current parse error.
 */
static char *makeHighlight(EcCompiler *cp, MprChar *source, int col)
{
    MprChar     *up, *sp;
    char        *dest, *dp;
    int         tabCount, len, i;

    tabCount = 0;

    for (up = source, len = 0; *up; up++) {
        if (*up == '\t') {
            tabCount++;
        }
        len++;
    }
    len = (int) len + (tabCount * cp->tabWidth);
    len = max(len, col);

    /*
        Allow for "^" to be after the last char, plus one null.
     */
    if ((dest = mprAlloc(NULL, len + 2)) == NULL) {
        mprAssert(dest);
        return 0;
    }
    for (i = 0, dp = dest, sp = source; *sp; sp++, i++) {
        if (*sp== '\t') {
            *dp++ = *sp;
        } else {
            *dp++ = ' ';
        }
    }
    /*
        Cover the case where the ^ must go after the end of the input
     */
    if (col >= 0) {
        dest[col] = '^';
        if (dp == &dest[col]) {
            ++dp;
        }
        *dp = '\0';
    }
    return dest;
}


void ecErrorv(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, va_list args)
{
    cchar       *appName;
    char        *pointer, *errorMsg, *msg;
    int         errCode;

    errCode = 0;
    appName = mprGetAppName(cp);
    msg = mprAsprintfv(cp, fmt, args);

    if (loc) {
        if (loc->source) {
            pointer = makeHighlight(cp, loc->source, loc->column);
            errorMsg = mprAsprintf(cp, "%s: %s: %s: %d: %s\n  %@  \n  %s\n", appName, severity, loc->filename, 
                loc->lineNumber, msg, loc->source, pointer);
            mprFree(pointer);
        } else if (loc->lineNumber >= 0) {
            errorMsg = mprAsprintf(cp, "%s: %s: %s: %d: %s\n", appName, severity, loc->filename, loc->lineNumber, msg);
        } else {
            errorMsg = mprAsprintf(cp, "%s: %s: %s: 0: %s\n", appName, severity, loc->filename, msg);
        }
    } else {
        errorMsg = mprAsprintf(cp, "%s: %s: %s\n", appName, severity, msg);
    }
    cp->errorMsg = srejoin(cp, cp->errorMsg, "", errorMsg, NULL);
    mprBreakpoint();
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.

    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://www.embedthis.com/downloads/gplLicense.html

    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com

    @end
 */