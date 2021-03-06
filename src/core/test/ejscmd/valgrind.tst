/*
    valgrind.tst - Test compiler
 */

module test.api.c {

    let ejsFlags = [
        "",
        "--optimize 1",
        "--optimize 9",
        "--nodebug",
        "--nobind",
    ]
    let valgrind = "/usr/bin/env valgrind -q --tool=memcheck --suppressions=ejs.supp " 

    if (test.os == "LINUX" && test.depth >= 2) {
        for (i = 0; i < test.depth && i < ejsFlags.length; i++) {
            let flags = ejsFlags[i]
            let command = "ejs " + flags

            //  MOB - not scripts exist yet
            for each (f in Path("../scripts").glob("*.es")) {
                testCmd(valgrind + command + f)
            }
        }
    } else {
        test.skip("Run on Linux")
    }

}
