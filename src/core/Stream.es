/*
    Stream.es -- Stream interface.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Stream objects represent streams of data that pass data elements between an endpoint known as a source or sink 
        and a consumer or producer. Streams are full-duplex and may be stacked where intermediate streams are be used 
        as filters or data mutators. Example endpoints are the File, Socket, and Http classes. The TextStream is an 
        example of a filter stream. The data elements passed by streams may be any series of objects including: bytes, 
        lines of text, numbers or objects. Streams may buffer the incoming data or not. Streams may issue events to 
        registered listeners for topics of interest. Streams may offer synchronous and asynchronous APIs. 
        @spec ejs
        @spec evolving
     */
    interface Stream {

        use default namespace public

        /** 
            Add a listener to the stream. 
            @param name Name of the event to listen for. The name may be an array of events.
            @param listener Callback listening function. The function is called with the following signature:
                function listener(event: String, ...args): Void
            @event readable Issued when the stream becomes readable. 
            @event writable Issued when the stream becomes writable.
            @event close Issued when stream is being closed.
         */
        function addListener(name, listener: Function): Void

        /** 
            The current async mode. Set to true if the stream is in async mode.
         */
        function get async(): Boolean

        /** 
            Set the current sync/async mode. The async mode affects the blocking APIs: close, read and write.
            If in async mode, all Stream calls will not block. If listeners have been registered, they can be used to
            respond to events to interface with the stream.
            @param enable If true, set the stream into async mode
         */
        function set async(enable: Boolean): Void

        /** 
            Close the stream. 
            @event close Issued before closing the stream.
         */
        function close(): Void

        /**
            Flush the stream and underlying streams. The act of flushing requests a stream with buffered data to 
            attempt to write all buffered data downstream. Flushing a stream with read data will cause the
            stream to discard the read data. If the stream is in sync mode, this call will block until all data is
            written. If the stream is in async mode, it will attempt to write all data but will not wait for completion.
         */
        function flush(): Void 

        /** 
            Read a data from the stream. 
            If data is available, the call will return immediately. 
            If no data is available and the stream is in sync mode, the call will block until data is available.
            If no data is available and the stream is in async mode, the call will not block and will return immediately.
            In this case a "readable" event will be issued when data is available for reading.
            @param buffer Destination byte array for read data.
            @param offset Offset in the byte array to place the data. If the offset is -1, then data is
                appended to the buffer write $position which is then updated. 
            @param count Read up to this number of bytes. If -1, read as much as the buffer will hold up. If the 
                stream is of fixed and known length (such as a file) and the buffer is of sufficient size or is growable, 
                read the entire stream. 
            @returns a count of the bytes actually read. Returns null on eof.
            @event readable Issued when there is new read data available.
            @event writable Issued when the stream becomes empty.
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number 

        /** 
            Remove a listener from the stream. 
            @param name Event name previously used with addListener. The name may be an array of events.
            @param listener Listener function previously used with addListener.
         */
        function removeListener(name, listener: Function): Void

        /** 
            Write data to the stream. 
            If the stream can accept all the write data, the call returns immediately with the number of elements written. 
            If writing more data than the stream can absorb in sync mode, the call will block until the data is written.
            If writing more data than the stream can absorb in async mode, the call will not block and will return 
            immediately. A "writable" event will be issued when all the data has been written.
            @param data Data to write. 
            @returns a count of the bytes actually written.
            @throws IOError if there is an I/O error.
            @event readable Issued when data is written and a consumer can read without blocking.
            @event writable Issued when the stream becomes empty and it is ready to be written to.
         */
        function write(...data): Number
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
