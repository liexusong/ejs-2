/*
    ejsApp.c -- App class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Methods **********************************/
/*  
    Get the application command line arguments
    static function get args(): String
 */
static EjsObj *app_args(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    EjsArray    *args;
    int         i;

    args = ejsCreateArray(ejs, ejs->argc);
    for (i = 0; i < ejs->argc; i++) {
        ejsSetProperty(ejs, args, i, ejsCreateStringFromAsc(ejs, ejs->argv[i]));
    }
    return (EjsObj*) args;
}


/*  
    Get the current working directory
    function get dir(): Path
 */
static EjsObj *app_dir(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetCurrentPath(ejs));
}


/*  
    Set the current working directory
    function chdir(value: String|Path): void
 */
static EjsObj *app_chdir(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar   *path;

    mprAssert(argc == 1);

    if (ejsIs(ejs, argv[0], Path)) {
        path = ((EjsPath*) argv[0])->value;

    } else if (ejsIs(ejs, argv[0], String)) {
        path = ejsToMulti(ejs, argv[0]);

    } else {
        ejsThrowIOError(ejs, "Bad path");
        return NULL;
    }
    if (chdir((char*) path) < 0) {
        ejsThrowIOError(ejs, "Can't change the current directory");
    }
    return 0;
}

/*  
    Get the directory containing the application's executable file.
    static function get exeDir(): Path
 */
static EjsObj *app_exeDir(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetAppDir(ejs));
}


/*  
    Get the application's executable filename.
    static function get exePath(): Path
 */
static EjsObj *app_exePath(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return (EjsObj*) ejsCreatePathFromAsc(ejs, mprGetAppPath(ejs));
}


/*  
    Exit the application
    static function exit(status: Number, how: String = "default"): void
 */
static EjsObj *app_exit(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    cchar   *how;
    int     status, mode;

    if (ejs->dontExit) {
        ejsThrowStateError(ejs, "App.exit has been disabled");
        return 0;
    }
    status = argc >= 1 ? ejsGetInt(ejs, argv[0]) : 0;
    how = ejsToMulti(ejs, argc >= 2 ? ejsToString(ejs, argv[1]): S(empty));

    if (scmp(how, "default") == 0) {
        mode = MPR_EXIT_DEFAULT;
    } else if (scmp(how, "immediate") == 0) {
        mode = MPR_EXIT_IMMEDIATE;
    } else if (scmp(how, "graceful") == 0) {
        mode = MPR_EXIT_GRACEFUL;
    } else {
        mode = MPR_EXIT_NORMAL;
    }
    mprTerminate(mode);
    ejsAttention(ejs);
    return 0;
}


#if ES_App_env
/*  
    Get all environment vars
    function get env(): Object
 */
static EjsAny *app_env(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
#if VXWORKS
    return S(null);
#else
    EjsPot  *result;
    char        **ep, *pair, *key, *value;

    result = ejsCreatePot(ejs, S(Object), 0);
    for (ep = environ; ep && *ep; ep++) {
        pair = sclone(*ep);
        key = stok(pair, "=", &value);
        ejsSetPropertyByName(ejs, result, EN(key), ejsCreateStringFromAsc(ejs, value));
    }
    return result;
#endif
}
#endif


/*  
    Get an environment var
    function getenv(key: String): String
 */
static EjsAny *app_getenv(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    cchar   *value;

    value = getenv(ejsToMulti(ejs, argv[0]));
    if (value == 0) {
        return S(null);
    }
    return ejsCreateStringFromAsc(ejs, value);
}


/*  
    Put an environment var
    function putenv(key: String, value: String): void
 */
static EjsObj *app_putenv(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
#if !WINCE
#if BLD_UNIX_LIKE
    char    *key, *value;

    key = sclone(ejsToMulti(ejs, argv[0]));
    value = sclone(ejsToMulti(ejs, argv[1]));
    setenv(key, value, 1);
#else
    //  TODO OPT
    char *cmd = sjoin(ejsToMulti(ejs, argv[0]), "=", ejsToMulti(ejs, argv[1]), NULL);
    putenv(cmd);
#endif
#endif
    return 0;
}


/*  
    Get the ejs module search path. Does not actually read the environment.
    function get search(): Array
 */
static EjsObj *app_search(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    return (EjsObj*) ejs->search;
}


/*  
    Set the ejs module search path. Does not actually update the environment.
    function set search(path: Array): Void
 */
static EjsObj *app_set_search(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    ejsSetSearchPath(ejs, (EjsArray*) argv[0]);
    return 0;
}


/*  
    Get a default search path. NOTE: this does not modify ejs->search.
    function get createSearch(searchPaths: String): Void
 */
static EjsObj *app_createSearch(Ejs *ejs, EjsObj *app, int argc, EjsObj **argv)
{
    cchar   *searchPath;

    searchPath = (argc == 0) ? NULL : ejsToMulti(ejs, argv[0]);
    return (EjsObj*) ejsCreateSearchPath(ejs, searchPath);
}


/*  
    static function get pid (): void
 */
static EjsNumber *app_pid(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    return ejsCreateNumber(ejs, getpid());
}


/*  
    static function run(timeout: Number = -1, oneEvent: Boolean = false): Boolean
 */
static EjsObj *app_run(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprTime     mark, remaining;
    int         rc, oneEvent, timeout;

    timeout = (argc > 0) ? ejsGetInt(ejs, argv[0]) : MAXINT;
    oneEvent = (argc > 1) ? ejsGetInt(ejs, argv[1]) : 0;

    if (timeout < 0) {
        timeout = MAXINT;
    }
    mark = mprGetTime();
    remaining = timeout;
    do {
        rc = mprWaitForEvent(ejs->dispatcher, remaining); 
        remaining = mprGetRemainingTime(mark, timeout);
    } while (!oneEvent && !ejs->exiting && remaining > 0 && !mprIsStopping());
    return (rc == 0) ? S(true) : S(false);
}


/*  
    Pause the application. This services events while asleep.
    static function sleep(delay: Number = -1): void
 */
static EjsObj *app_sleep(Ejs *ejs, EjsObj *unused, int argc, EjsObj **argv)
{
    MprTime     mark, remaining;
    int         timeout;

    timeout = (argc > 0) ? ejsGetInt(ejs, argv[0]) : MAXINT;
    if (timeout < 0) {
        timeout = MAXINT;
    }
    mark = mprGetTime();
    remaining = timeout;
    do {
        mprWaitForEvent(ejs->dispatcher, (int) remaining); 
        remaining = mprGetRemainingTime(mark, timeout);
    } while (!ejs->exiting && remaining > 0 && !mprIsStopping());
    return 0;
}


/*********************************** Factory **********************************/

void ejsConfigureAppType(Ejs *ejs)
{
    EjsType     *type;

    type = ejsGetTypeByName(ejs, N("ejs", "App"));
    mprAssert(type);
    ejsSetSpecial(ejs, S_App, type);

    ejsSetProperty(ejs, type, ES_App__inputStream, ejsCreateFileFromFd(ejs, 0, "stdin", O_RDONLY));
    ejsSetProperty(ejs, type, ES_App__outputStream, ejsCreateFileFromFd(ejs, 1, "stdout", O_WRONLY));
    ejsSetProperty(ejs, type, ES_App__errorStream, ejsCreateFileFromFd(ejs, 2, "stderr", O_WRONLY));

    ejsBindMethod(ejs, type, ES_App_args, (EjsProc) app_args);
    ejsBindMethod(ejs, type, ES_App_createSearch, (EjsProc) app_createSearch);
    ejsBindMethod(ejs, type, ES_App_dir, (EjsProc) app_dir);
    ejsBindMethod(ejs, type, ES_App_chdir, (EjsProc) app_chdir);
    ejsBindMethod(ejs, type, ES_App_exeDir, (EjsProc) app_exeDir);
    ejsBindMethod(ejs, type, ES_App_exePath, (EjsProc) app_exePath);
    ejsBindMethod(ejs, type, ES_App_env, (EjsProc) app_env);
    ejsBindMethod(ejs, type, ES_App_exit, (EjsProc) app_exit);
    ejsBindMethod(ejs, type, ES_App_getenv, (EjsProc) app_getenv);
    ejsBindMethod(ejs, type, ES_App_putenv, (EjsProc) app_putenv);
    ejsBindMethod(ejs, type, ES_App_pid, (EjsProc) app_pid);
    ejsBindMethod(ejs, type, ES_App_run, (EjsProc) app_run);
    ejsBindAccess(ejs, type, ES_App_search, (EjsProc) app_search, (EjsProc) app_set_search);
    ejsBindMethod(ejs, type, ES_App_sleep, (EjsProc) app_sleep);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2011. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2011. All Rights Reserved.

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

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
