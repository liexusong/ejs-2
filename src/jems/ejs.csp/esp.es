#!/usr/bin/env ejs
/*
    ejspage.es -- Ejscript web templating engine command 
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

require ejs.web.template

class EjsPage {
    private var verbose: Number = 1

    private var cmdOptions = [
        [ "layout", String ],
        [ "overwrite", null, false ],
        [ "quiet", null, false ],
        [ "v", null, true ],
        [ "verbose", null, true ],
    ]

    function usage(e = null): Void {
        if (e) {
            error(e)
        }
        error("\nUsage: ejspage [options] [commands] ...\n" +
            "  Options:\n" + 
            "    --layout layoutPage\n" + 
            "    --overwrite\n" + 
            "    --quiet\n" + 
            "    --verbose\n")
        App.exit(1)
    }

    function main() {
        let cmd
        try {
            cmd = CmdArgs(cmdOptions)
            processOptions(cmd)
            if (cmd.args.length == 0) {
                usage()
            }
        } catch (e) {
            usage(e)
        }
        try {
            let options = cmd.options
            let parser = new TemplateParser
            for each (f in cmd.args) {
                f = Path(f)
                options.dir = f.dirname
                result = parser.build(f.readString(), options)
                let script = f.replaceExt(".es");
                if (script.exists && !options.overwrite) {
                    throw "Script " + script + " already exists. Use --overwrite."
                }
                script.write(result)
            }
        } catch (e) {
            throw e
            error("ejspage: Error: " + ((e is String) ? e : e.message) + "\n")
            App.exit(2)
        }
    }

    function processOptions(cmd) {
        let options = cmd.options
        if (options.version) {
            print(Config.version)
            App.exit(0)
        }
        if (options.v) {
            options.verbose = true
        }
        if (options.quiet) {
            options.verbose = false
        }
    }
}

var page: EjsPage = new EjsPage
page.main()

/*
 *  @copy   default
 *
 *  Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
 *  Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
 *
 *  This software is distributed under commercial and open source licenses.
 *  You may use the GPL open source license described below or you may acquire
 *  a commercial license from Embedthis Software. You agree to be fully bound
 *  by the terms of either license. Consult the LICENSE.TXT distributed with
 *  this software for full details.
 *
 *  This software is open source; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version. See the GNU General Public License for more
 *  details at: http://www.embedthis.com/downloads/gplLicense.html
 *
 *  This program is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  This GPL license does NOT permit incorporating this software into
 *  proprietary programs. If you are unable to comply with the GPL, you must
 *  acquire a commercial license to use this software. Commercial licenses
 *  for this software and support services are available from Embedthis
 *  Software at http://www.embedthis.com
 *
 *  @end
 */
