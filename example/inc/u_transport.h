#ifndef U_TRANSPORT_H_
#define U_TRANSPORT_H_

#include <stdint.h>
#include "u_list.h"

// transport object used to hold information about a specific download job
typedef struct {
    uint8_t    *crid_p;     // identifies content
} sTransport;

// identifies a node in the network
typedef struct {
    // node stuff
} sNodeId;

typedef struct {
    uint16_t    sliceId;    // identifies the slice
    uint32_t    sliceSize;  // size of slice in bytes
} sSlice;

/**
 * @return  a list of regular nodes in the network having the specified slice.
 */
sList *transport_getNodeList(sTransport *transport_p, sSlice *slice_p);

/**
 * @return  a list of fallback server nodes, having  the specified slice, to 
 * be used when there is not enough regular nodes to download the slice in time 
 * to meet the deadline.
 */
sList *transport_getFallbackNodeList(sTransport *transport_p, sSlice *slice_p);

/**
 * store buf into the storage area reserved for the specified slice
 *
 * @param offset    offset in slice where buf_p starts
 * @param length    size of the slice-chunk pointed to by buf_p
 * @return          0 on success or negative error code on failure
 */
int32_t transport_storeSliceData(sTransport *transport_p, sSlice *slice_p, uint8_t *buf_p, uint32_t offset, uint32_t lenght);

/**
 * retrieve the content data stored in the storage area reserved for the specified 
 * slice and copies it into buf_out. buf_out must havethe capacity to stor sliceSize 
 * bytes.
 *
 * @return          0 on success or negative error code on failure
 */
int32_t transport_getSliceData(sTransport *transport_p, sSlice *slice_p, uint8_t *buf_out);

#endif
