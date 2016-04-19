#ifndef ASSIGNMENT_H_
#define ASSIGNMENT_H_

/*

voddler net (Vnet) is a p2p network compriced of end user nodes, capable to act 
both as clients, to retrieve content data, and servers, deliver content data to 
other nodes in the network. Each such content, or movie, is divided into slices
about 4Mb large, each such slice is stored on a number of nodes, but no node 
have all the slices making up a content. When downloading slices from the other 
nodes in the network, the downloading node use the Vnet protocol and the Vnet
connection framework to send requests and receive responses from those nodes. 
Such a request or response have a limited size and cannot hold a complete slice,
so downloading a slice is done by requesting chunks of a slice from a bunch of 
nodes assembling the complete slice as the responses arrive. The connection 
framework requires that no more then one request is sent to the same connection
at a time. As Vnet is used by a movie service, it is important that the download
meet the requested deadlines, or the movie playback will halt. At the same time
we do not want to overuse our fallback-nodes as one of the main features of Vnet 
is to reduce the server-side load.

Your assignment is to implement the assignment_downloadSlice() method according 
to the definition in this file using the functionality of the provided header 
files and to fill the eAssignmentDownloadResult enum  with appropriate result 
codes. Of cource you could write helper functions and data structures as you wish.

We do not expect your solution to compile, after all we dont provide you with any 
lib, but are more intrested to see how you interprete the interfaces and the 
overall architecture of your solution.

Good luck!

*/

#include "u_transport.h"

typedef enum {

} eAssignmentDownloadResult;

/**
 * callback to be called when a slice data download is compleated, or have failed.
 */
typedef int32_t (*assignment_downloadComplete)(eAssignmentDownloadResult result, sTransport *transport_p, sSlice *slice_p);

/**
 * called to download data of a specific slice of a specific content. When the the 
 * download is compleate (or for any reason, should fail), the result is transmitted 
 * to the original caller by the provided callback function.
 *
 * @param transport_p       holds information about the overall download
 * @param slice_p           the slice to download
 * @param done_cb           callback to be called when download compleats, 
 *                          successfully or not.
 * @param relativeDeadline  number of ms until the slice should be completly downloaded
 * @return                  0 on success or appropriate negative value on failure
 */
int32_t assignment_downloadSlice(sTransport *transport_p, sSlice *slice_p, assignment_downloadComplete done_cb, int32_t relativeDeadline);

#endif
