#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <boost/shared_ptr.hpp>
#include "Error_handle.hpp"

using BYTE = uint8_t;
#define Header_Size 12
// 8 Bytes for header
#define Protocol_Size 1
//1 Byte for protocol
#define Extra_Message_Size 1
// 1 Byte for extra message
#define Encoding_Size 1
// 1 Byte for content's encoding
#define Content_Type_Size 1
// 1 Byte for content type
#define Content_Length_Size 4
// 4 Byte for content length
#define Timestamp_Size 4
// 4 Byte for timestamp
#define CHUNK_SIZE 4096

#define Protocol_Off 0
#define Encoding_Off 1
#define Extra_Message_Off 2
#define Content_Type_Off 3
#define Content_Length_Off 4
#define Timestamp_Off 8
/*
*   The lay out of header is like following:
*   | protocol | extra message | encoding | content type | content length| time |
*   Some illustration:
*   1. protocol, specify that what the data is for.
*   2. extra message, specify whether extra message exsists. Note this solt
        do not store the extra message. If extra message exsists, the value
        of it is set to the length of extra messge.
*   3. encoding, specify the encoding the data uses
*   4. content type, specify the content's type, such as file, picture...
*   5. content length, specify the length of the content.
*   6. timestamp, which value represent the seconds from Jan.1, 1970 to now.
*/

class Message_Packet{

public:

    Message_Packet():Body(), complete(false), h_cursor(0), b_cursor(0){}

    state_code_t Process(BYTE* chunk, size_t&);
    state_code_t Clear();

    // Read from message
    BYTE protocol(){
        return Header[Protocol_Off];
    }
    BYTE content_type(){
        return Header[Content_Type_Off];
    }
    BYTE extra_message_size(){
        return Header[Extra_Message_Off];
    }
    BYTE encoding(){
        return Header[Encoding_Off];
    }
    uint32_t content_size(){
        return *reinterpret_cast<uint32_t*>(&Header[Content_Length_Off]);
    }
    uint32_t timestamp(){
        return *reinterpret_cast<uint32_t*>(&Header[Timestamp_Off]);
    }
    //both read and write
    BYTE* body(){
        return Body.get();
    }
    BYTE *header(){
        return Header;
    }
    //write to message
    void write_protocol(BYTE p){
        Header[Protocol_Off] = p;
    }
    void write_content_type(BYTE c){
        Header[Content_Type_Off] = c;
    }
    void write_extra_msg_size(BYTE e){
        Header[Extra_Message_Off] = e;
    }
    void write_encoding(BYTE e){
        Header[Encoding_Off] = e;
    }
    void write_content_size(uint32_t s){
        *reinterpret_cast<uint32_t*>(&Header[Content_Length_Off]) = s;
    }
    void write_timestamp(uint32_t t){
        *reinterpret_cast<uint32_t*>(&Header[Timestamp_Off]) = t;
    }

    void new_body(BYTE* chunk, size_t);

private:

    BYTE Header[Header_Size];
    boost::shared_ptr<BYTE> Body;
    // The length of body is dynamic
    bool complete;
    size_t h_cursor, b_cursor, content_length;

private:
    size_t write_header(BYTE *chunk, size_t&);
    size_t write_body(BYTE* chunk, size_t&);
};

#endif
