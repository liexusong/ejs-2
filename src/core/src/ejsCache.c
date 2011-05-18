/**
    ejsCache.c - API for the Cache class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/************************************* Code ***********************************/

EjsVoid *ejsCacheExpire(Ejs *ejs, EjsObj *cache, EjsString *key, EjsDate *when)
{
    EjsAny  *argv[3];

    ejsFreeze(ejs, 1);
    argv[0] = key;
    argv[1] = when;
    return ejsRunFunctionBySlot(ejs, cache, ES_Cache_expire, 2, argv);
}


EjsAny *ejsCacheRead(Ejs *ejs, EjsObj *cache, EjsString *key, EjsObj *options)
{
    EjsAny  *argv[3];

    ejsFreeze(ejs, 1);
    argv[0] = key;
    argv[1] = (options) ? options : S(null);
    return ejsRunFunctionBySlot(ejs, cache, ES_Cache_read, 2, argv);
}


EjsAny *ejsCacheReadObj(Ejs *ejs, EjsObj *cache, EjsString *key, EjsObj *options)
{
    EjsAny  *value, *argv[3];

    ejsFreeze(ejs, 1);
    argv[0] = key;
    argv[1] = (options) ? options : S(null);
    if ((value = ejsRunFunctionBySlot(ejs, cache, ES_Cache_read, 2, argv)) == 0 || value == S(null)) {
        return 0;
    }
    return ejsDeserialize(ejs, value);
}


EjsBoolean *ejsCacheRemove(Ejs *ejs, EjsObj *cache, EjsString *key)
{
    EjsAny  *argv[3];

    ejsFreeze(ejs, 1);
    argv[0] = key;
    return ejsRunFunctionBySlot(ejs, cache, ES_Cache_remove, 1, argv);
}


EjsVoid *ejsCacheSetLimits(Ejs *ejs, EjsObj *cache, EjsObj *limits)
{
    EjsAny  *argv[2];

    ejsFreeze(ejs, 1);
    argv[0] = limits;
    return ejsRunFunctionBySlot(ejs, cache, ES_Cache_setLimits, 2, argv);
}


EjsNumber *ejsCacheWrite(Ejs *ejs, EjsObj *cache, EjsString *key, EjsString *value, EjsObj *options)
{
    EjsAny  *argv[3];

    ejsFreeze(ejs, 1);
    argv[0] = key;
    argv[1] = value;
    argv[2] = (options) ? options : S(null);
    return ejsRunFunctionBySlot(ejs, cache, ES_Cache_write, 3, argv);
}


EjsNumber *ejsCacheWriteObj(Ejs *ejs, EjsObj *cache, EjsString *key, EjsAny *value, EjsObj *options)
{
    EjsAny  *argv[3];

    ejsFreeze(ejs, 1);
    argv[0] = key;
    argv[1] = ejsSerialize(ejs, value, 0);
    argv[2] = (options) ? options : S(null);
    return ejsRunFunctionBySlot(ejs, cache, ES_Cache_write, 3, argv);
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