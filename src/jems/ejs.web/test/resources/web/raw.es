require ejs.web

exports.app = function(r: Request) {
    /*
        Raw access to the request API. "this" set to request
     */
    write("Hello World\r\n")
    finalize()
}
