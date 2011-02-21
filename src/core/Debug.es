/*
    Debug.es -- Debug class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Debug configuration class. Singleton class containing the application's debug configuration.
        @spec ejs
        @stability prototype
     */
    class Debug {

        use default namespace public

        /**
            Break to the debugger. Suspend execution and break to the debugger.
         */ 
        native static function breakpoint(): void

        /**
            The current debug mode. This property is read-write. Setting mode to true will put the application in 
            debug mode. When debug mode is enabled, the runtime will typically suspend timeouts and will take other 
            actions to make debugging easier.
         */
        native static function get mode(): Boolean

        /**
            @duplicate Debug.mode
            @param value True to turn debug mode on or off.
         */
        native static function set mode(value: Boolean): void
    }

    /** 
        Convenient way to trap to the debugger
     */
    native function breakpoint(): Void
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
