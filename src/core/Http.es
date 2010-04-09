/**
    Http.es -- HTTP client side communications
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    //  MOB -- do we really want this to be dynamic?
    /** 
        The Http object represents a Hypertext Transfer Protocol version 1/1 client connection. It is used to issue 
        HTTP requests and capture responses. It supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
        @spec ejs
        @stability evolving
     */
    dynamic class Http implements Stream {

        use default namespace public

        /** HTTP status code */     static const Continue           : Number    = 100
        /** HTTP status code */     static const Ok                 : Number    = 200
        /** HTTP status code */     static const Created            : Number    = 201
        /** HTTP status code */     static const Accepted           : Number    = 202
        /** HTTP status code */     static const NotAuthoritative   : Number    = 203
        /** HTTP status code */     static const NoContent          : Number    = 204
        /** HTTP status code */     static const Reset              : Number    = 205
        /** HTTP status code */     static const Partial            : Number    = 206
        /** HTTP status code */     static const MultipleChoice     : Number    = 300
        /** HTTP status code */     static const MovedPermanently   : Number    = 301
        /** HTTP status code */     static const MovedTemporarily   : Number    = 302
        /** HTTP status code */     static const SeeOther           : Number    = 303
        /** HTTP status code */     static const NotModified        : Number    = 304
        /** HTTP status code */     static const UseProxy           : Number    = 305
        /** HTTP status code */     static const BadRequest         : Number    = 400
        /** HTTP status code */     static const Unauthorized       : Number    = 401
        /** HTTP status code */     static const PaymentRequired    : Number    = 402
        /** HTTP status code */     static const Forbidden          : Number    = 403
        /** HTTP status code */     static const NotFound           : Number    = 404
        /** HTTP status code */     static const BadMethod          : Number    = 405
        /** HTTP status code */     static const NotAccepted        : Number    = 406
        /** HTTP status code */     static const ProxyAuth          : Number    = 407
        /** HTTP status code */     static const ClientTimeout      : Number    = 408
        /** HTTP status code */     static const Conflict           : Number    = 409
        /** HTTP status code */     static const Gone               : Number    = 410
        /** HTTP status code */     static const LengthRequired     : Number    = 411
        /** HTTP status code */     static const PrecondFailed      : Number    = 412
        /** HTTP status code */     static const EntityTooLarge     : Number    = 413
        /** HTTP status code */     static const ReqTooLong         : Number    = 414
        /** HTTP status code */     static const UnsupportedType    : Number    = 415
        /** HTTP status code */     static const ServerError        : Number    = 500
        /** HTTP status code */     static const NotImplemented     : Number    = 501
        /** HTTP status code */     static const BadGateway         : Number    = 502
        /** HTTP status code */     static const Unavailable        : Number    = 503
        /** HTTP status code */     static const GatewayTimeout     : Number    = 504
        /** HTTP status code */     static const Version            : Number    = 505

        /* Cached response data */
        private var _response: String

        /** 
            Create an Http object. The object is initialized with the Uri
            @param uri The (optional) Uri to initialize with.
            @throws IOError if the Uri is malformed.
         */
        native function Http(uri: Uri? = null)

        /** 
            @duplicate Stream.addListener
            @event headers Issued when the response headers have been fully received.
            @event readable Issued when some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
            @event complete Issued when the request completes
            @event error Issued if the request does not complete successfully.
            All events are called with the signature:
            function (event: String, http: Http): Void
         */
        native function addListener(name, listener: Function): Void

        /** 
            @duplicate Stream.async
         */
        native function get async(): Boolean

        /** 
            @duplicate Stream.async
         */
        native function set async(enable: Boolean): Void

        /** 
            The preferred chunk size used if transfer chunk encoding will be used. Chunked encoding is used when 
            an explicit request content length is unknown at the time the request headers must be emitted. Chunked 
            encoding is automatically enabled if $post, $put or $upload is called and a contentLength has not been 
            previously set. The chunk size is normally automatically determined by can be explicitly specified via
            set $chunkSize. Set to zero if a size has not yet been defined.
         */
        native function get chunkSize(): Boolean

        /** 
            Set the preferred chunk size to use for transfer chunk encoding. Chunked encoding is used when an explicit 
            request content length is unknown at the time the request headers must be emitted.  Chunked encoding is 
            automatically enabled if $post, $put or $upload is called and a contentLength has not been previously set.
            @param value Number of bytes in chunks. This is advisory and the platform is free to determine a different
            chunk size if required.
            @hide
         */
        native function set chunkSize(value: Boolean): Void

        /** 
            @duplicate Stream.close 
            This closes any open network connection and resets the http object to be ready for another connection. 
            Connections should be explicitly closed rather than relying on the garbage collector to dispose of the 
            Http object and automatically close the connection.
         */
        native function close(): Void 

        /** 
            Commence a HTTP request for the current method and uri. The HTTP method should be defined via the $method 
            property and Uri via the $uri property. This routine is typically not used. Rather it is invoked via one 
            of the Http methods get(), head(), post() instead. This call, and the Http method calls  may not immediately
            initiate the connection. The Http class will delay connections until finalize() is called explicitly or 
            implicitly reading $status or response content. This enables the request content length to be determined 
            automatically for smaller requests where the request body data can be buffered and measured before sending 
            the request headers.  
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server.
            @event connect Issued a "connect" event when the connection is complete.
         */
        native function connect(uri: Uri? = null, ...data): Void

        /** 
            Filename of the certificate file for this HTTP object. The certificate is only used if secure
            communications are in use. Currently not implemented.
            @hide
         */
        native function get certificate(): Path

        /** 
            @duplicate Http.certificate
            @param certFile The name of the certificate file.
            @throws IOError if the file does not exist.
         */
        native function set certificate(certFile: Path): Void

        /** 
            Response content body length. Set to the length of the response body in bytes or -1 if no body or not known.
         */
        native function get contentLength(): Number

        /** 
            Response content type derrived from the response Http Content-Type header.
         */
        native function get contentType(): String

        /** 
            When the response was generated. Response date derrived from the response Http Date header.
         */
        native function get date(): Date

        /** 
            Commence a DELETE request for the current uri. See connect() for connection details.
            @param uri The uri to delete. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function del(uri: Uri? = null, ...data): Void

        /** 
            Encoding scheme for serializing strings. The default encoding is UTF-8. Not yet implemented.
            @hide
         */
        function get encoding(): String
            "utf-8"

        /** 
            @duplicate Http.encoding
            @param enc String representing the encoding scheme
            @hide
         */
        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** 
            When the response content expires. This is derrived from the response Http Expires header.
         */
        native function get expires(): Date

        /** 
            Signals the end of write data. If using chunked writes (no content length specified), finalize() must
            be called to properly signify the end of write data. This causes the chunk filter to write a chunk trailer.
            It is good practice to call finalize() at the end of writing regardless of whether chunked transfer is used 
            or not.
         */
        native function finalize(): Void 

        /** 
            @duplicate Stream.flush
            @hide 
         */
        function flush(): Void {}

        /** 
            Get whether redirects should be automatically followed by this Http object.
            @return True if redirects are automatically followed.
         */
        native function get followRedirects(): Boolean

        /** 
            Eanble or disable following redirects from the connection remove server. Default is true.
            @param flag Set to true to follow redirects.
         */
        native function set followRedirects(flag: Boolean): Void

        /** 
            Commence a POST request with form data the current uri. See connect() for connection details.
            @param uri Optional request uri. If non-null, this overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param postData Optional object hash of key value pairs to use as the post data. These are www-url-encoded and
                the content mime type is set to "application/x-www-form-urlencoded".
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function form(uri: Uri, postData: Object): Void

        /** 
            Commence a GET request for the current uri. See connect() for connection details.
            This call initiates a GET request. It does not wait for the request to complete. 
            Once initiated, one of the $read or response routines  may be used to receive the 
            response data.
            @param uri The uri to get. This overrides any previously defined uri for the Http object. If null, use
                a previously defined uri.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function get(uri: Uri? = null, ...data): Void

        /** 
            Get the (proposed) request headers to send with the request
            @return The set of request headers that will be used when the request is sent.
         */
        native function getRequestHeaders(): Object

        /** 
            Commence a HEAD request for the current uri. See connect() for connection details.
            @param uri The request uri. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function head(uri: Uri? = null): Void

        /** 
            Get the value of a response header. This is a higher performance API than using response.headers["key"].
            @return The header field value as a string or null if not known.
         */
        native function header(key: String): String

        /** 
            Response headers. Use header() to retrieve a single header value.
            Set to an object filled with all the response headers. If multiple headers of the same key value are
                defined, their contents will be catenated with a ", " separator as per the HTTP/1.1 specification.
         */
        native function get headers(): Object

        /** 
            Is the connection is utilizing SSL.
            @return True if the connection is using SSL.
         */
        native function get isSecure(): Boolean

        /** 
            Private key file for this Https object. NOT currently supported.
            @return The file name.
            @hide
         */
        native function get key(): Path

        /** 
            @duplicate Http.key
            @param keyFile The name of the key file.
            @throws IOError if the file does not exist.
            @hide
         */
        native function set key(keyFile: Path): Void

        /** 
            When the response content was last modified. Set to the the value of the response Http Last-Modified header.
            Set to null if not known.
         */
        native function get lastModified(): Date

        /** 
            Http request method for this Http object.
         */
        native function get method(): String

        /** 
            Set or reset the Http object's request method. Default method is GET.
            @param name The method name as a string.
            @throws IOError if the request is not GET, POST, HEAD, OPTIONS, PUT, DELETE or TRACE.
         */
        native function set method(name: String)

        /** 
            Commence an OPTIONS request for the current uri. See connect() for connection details.
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function options(uri: Uri? = null): Void

        /** 
            Initiate a POST request for the current uri. This call initiates a POST request. It does not wait for the 
            request to complete. Posted data is NOT URL encoded. If you want to post data to a form, consider using 
            the $form method instead which automatically URL encodes the data. Post data may be supplied may alternatively 
            be supplied via $write.  If a contentLength has not been previously defined for this request, chunked transfer 
            encoding will be enabled.
            @param uri Optional request uri. If non-null, this overrides any previously defined uri for the Http object. 
                If null, use a previously defined uri.
            @param data Data objects to send with the post request. Data is written raw and is not encoded or converted. 
                However, this routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function post(uri: Uri, ...data): Void

        /** 
            Commence a PUT request for the current uri. See connect() for connection details.
            If a contentLength has not been previously defined for this request, chunked transfer encoding will be enabled.
            @param uri The uri to put. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param data Optional data objects to write to the request stream. Data is written raw and is not encoded 
                or converted.  However, put intelligently handles arrays such that, each element of the array will be 
                written. If encoding of put data is required, use the BinaryStream filter. If no putData is supplied,
                and the $contentLength is non-zero you must call $write to supply the body data.
            @param data Optional object hash of key value pairs to use as the post data.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function put(uri: Uri, ...data): Void

        /** 
            @duplicate Stream.read
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number

        /** 
            Read the response as a string. This call will block and should not be used in async mode.
            @param count of bytes to read. Returns the entire response contents if count is -1.
            @returns a string of $count characters beginning at the start of the response data.
            @throws IOError if an I/O error occurs.
         */
        native function readString(count: Number = -1): String

        /** 
            Read the request response as an array of lines. This call will block and should not be used in async mode.
            @param count of linese to read. Returns the entire response contents if count is -1.
            @returns an array of strings
            @throws IOError if an I/O error occurs.
         */
        function readLines(count: Number = -1): Array {
            let stream: TextStream = TextStream(this)
            result = stream.readLines()
            return result
        }

        /** 
            Read the request response as an XML document. This call will block and should not be used in async mode.
            @returns the response content as an XML object 
            @throws IOError if an I/O error occurs.
         */
        function readXml(): XML
            XML(response)

        /** 
            Remove a listener to the stream. If there are no listeners on the stream, the stream is put back into sync mode.
            @param name Event name previously used with addListener. The name may be an array of events.
            @param listener Listener function previously used with addListener.
         */
        native function removeListener(name, listener: Function): Void

        /** 
            Response body content. The first time this property is read, the response content will be read and buffered.
            Don't use this property in async mode as it will block. Set to the response as a string of characters. If 
            the response has no body content, the empty string will be returned.
            @throws IOError if an I/O error occurs.
         */
        native function get response(): String

        //  TODO - implement
        /** 
            The maximum number of retries for a request. Retries are essential as the HTTP protocol permits a 
            server or network to be unreliable. The default retries is 2.
            @hide
         */
        native function get retries(): Number

        /** 
            Define the number of retries of a request. Retries are essential as the HTTP protocol permits a server or
            network to be unreliable. The default retries is 2.
            @param count Number of retries. A retry count of 1 will retry a failed request once.
            @hide
         */
        native function set retries(count: Number): Void

        /** 
            Set the user credentials to use if the request requires authentication.
         */
        native function setCredentials(username: String, password: String): Void

        /** 
            Set a request header. Use setHeaders() to set all the headers. Use getRequestHeaders() to retrieve and examine
            the request header set.
            @param key The header keyword for the request, e.g. "accept".
            @param value The value to associate with the header, e.g. "yes"
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        native function setHeader(key: String, value: String, overwrite: Boolean = true): Void

        /** 
            Set request headers. Use setHeader() to set a single header. Use getRequestHeaders() to retrieve and examine 
            the request headers set.
            @param headers Object hash of headers to set.
            @param overwrite If true, the new set of headers completely replaces the existing set of request headers.
                If overwrite is false, a new header value will be catenated to the old value after a ", " separator.
         */
        native function setHeaders(headers: Object, overwrite: Boolean = true): Void

        /** 
            Http response status code from the Http response status line, e.g. 200. Set to null if unknown.
         */
        native function get status(): Number

        /** 
            Descriptive status message for the Http response. This message may come from either the HTTP response status
                line or from a local error message if the response fails to parse.
         */
        native function get statusMessage(): String

        /**
            @hide
         */
        function get success(): Boolean
            200 <= status && status < 300

        /** 
            Request timeout in milliseconds. This is the idle timeout value. If the request has no I/O activity for 
            this time period, it will be retried or aborted.
         */
        native function get timeout(): Number

        /** 
            Set the request timeout.
            @param timeout Number of milliseconds to complete while attempting requests. -1 means no timeout.
         */
        native function set timeout(timeout: Number): Void

        /** 
            Commence a TRACE request for the current uri. See connect() for connection details.
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function trace(uri: Uri? = null): Void

        /** 
            Upload files using multipart/mime. This routine initiates a POST request and sends the specified files
            and form fields using multipart mime encoding. This call is synchronous (blocks) until complete.
            @param uri The uri to upload to. This overrides any previously defined uri for the Http object.
            @param files Object hash of files to upload
            @param fields Object hash of of form fields to send
            @example
                fields = { name: "John Smith", address: "700 Park Avenue" }
                files = { file1: "a.txt, file2: "b.txt" }
                http.upload(URL, files, fields)
         */
        function upload(uri: String, files: Object, fields: Object? = null): Void {
            let boundary = "<<Upload Boundary>>"
            let buf = new ByteArray(4096)
            let http = this
            buf.addListener("readable", function (event, buf) {
                http.write(buf)
                buf.flush()
            })
            setHeader("Content-Type", "multipart/form-data; boundary=" + boundary)
            post(uri)
            if (fields) {
                for (key in fields) {
                    buf.write('--' + boundary + "\r\n")
                    buf.write('Content-Disposition: form-data; name=' + Uri.encode(key) + "\r\n")
                    buf.write('Content-Type: application/x-www-form-urlencoded\r\n\r\n')
                    buf.write(Uri.encode(fields[key]) + "\r\n")
                }
            }
            for (key in files) {
                file = files[key]
                buf.write('--' + boundary + "\r\n")
                buf.write('Content-Disposition: form-data; name=' + key + '; filename=' + Path(file).basename + "\r\n")
                buf.write('Content-Type: ' + Uri(file).mimeType + "\r\n\r\n")

                f = File(file).open()
                data = new ByteArray
                while (f.read(data)) {
                    buf.write(data)
                }
                f.close()
                buf.write("\r\n")
            }
            buf.write('--' + boundary + "--\r\n\r\n")
            http.finalize()
            http.wait()
        }

        /** 
            The current Uri for this Http object. The Uri is used for the request URL when making a $connect call.
         */
        native function get uri(): Uri

        /** 
            Set the Http object's Uri. The Uri is used for the request URL when making a $connect call.
            @param newUri The new Uri as a Uri object. If a string is supplied it will automatically be cast to a Uri
                object before making the call.
            @throws IOError if the Uri is malformed.
         */
        native function set uri(newUri: Uri): Void

        /** 
            Wait for a request to complete.
            @param timeout Time in seconds to wait for the request to complete
            @return True if the request successfully completes.
         */
        native function wait(timeout: Number = -1): Boolean

        /** 
            @duplicate Stream.write
            The Http.contentLength should normally be set prior to writing any data to ensure that the request 
            "Content-length" header is properly defined. If the body length has not been defined, the data will be 
            transferred using chunked transfers. In this case, you should call close() with no data to signify the 
            end of the write stream. Failure to define the Content-Length header will cause the remote server to have 
            to close the underling HTTP connection at the completion of the request. This will erode performance by 
            preventing  HTTP keep-alive for subsequent requests.
            @throws StateError if the Http method is not set to POST or if more post data is written than specified via 
                the contentLength property.
         */
        native function write(...data): Void

//  TODO - Cleanup and remove
        //  LEGACY 11/23/2010 1.0.0-B1
        /** @hide */
        function addHeader(key: String, value: String, overwrite: Boolean = true): Void
            setHeader(key, value, overwrite)

        //  DEPRECATED
        /** @hide */
        function get bodyLength(): Void
            contentLength

        //  DEPRECATED
        /** @hide */
        function set bodyLength(value: Number): Void {
            setHeader("content-length", value)
        }

        //  DEPRECATED
        /** @hide */
        function get code(): Number
            status

        //  DEPRECATED
        /** @hide */
        function get codeString(): String
            statusMessage

        // DEPRECATED
        /** 
            The number of response data bytes that are currently available for reading.
            @returns The number of available bytes.
            @hide
         */
        native function get available(): Number 

        //  DEPRECATED
        /**
            Get the value of the content encoding of the response.
            @return A string with the content type or null if not known.
            @hide
         */
        function get contentEncoding(): String
            header("content-encoding")

        //  DEPRECATED
        /** @hide */
        static function mimeType(ext: String): String
            Uri(ext)..mimeType
    }
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
