#ifndef PROTOCOL_H_
#define PROTOCOL_H_

/*

FILE_FEED_REQUEST:   <crid><slice_id><offset><chunk_size>

FILE_FEED_RESPONSE:  <crid><slice_id><offset><chunk_size>*<chunk_data>*<extended_info>
# PULL_PROT_EXTENDED_INFO_NODE_BUSY
# id:        1
# data size: 4
#
# FILE_FEED_EXTENDED_INFO_NO_SLICE_AVAILABLE
# id:        128
# data size: 0


<crid>:                     136<ascii>
<slice_id>:                 <netshort>
<offset>:                   <netlong>
<chunk_data>:               binary content data
<extended_info>:            <extended_info_id><extended_info_data_size>*<extended_info_data>
<ascii>:                    '0'-'9','a'-'z'
<extended_info_id>:         <uint8>
<extended_info_data_size>:  <netlong> 
<extended_info_data>:       binary data
<netshort>:                 unsigned 16 bit integer in network byte order
<netlong>:                  unsigned 32 bit integer in network byte order

*/

#define MAX_FILE_FEED_CHUNK_SIZE    51200
#define FILE_FEED_REQUEST           (uint16_t) 0x4036
#define FILE_FEED_RESPONSE          (uint16_t) 0x3938

#endif /*PROTOCOL_H_*/
