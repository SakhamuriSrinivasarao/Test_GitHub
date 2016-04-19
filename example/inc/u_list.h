/*-----------------------------------------------------------------------
 * List data type implemented as a doubly linked list which is thread-safe
 * (pthreads). Note that only pointers should be stored in the list.
 *
 * Example - Create a list of char*, iterate through it, and destroy it
 * ====================================================================
 *   // create list. note the use of free() as a function pointer argument to
 *   // clean up the elements at destruction.
 *   sList* list = list_create(free);
 *   if (!list) {
 *       // report error
 *   }
 *   ...
 *   // fill list with strings
 *   list_pushBack(list, strdup("string 1"));
 *   list_pushBack(list, strdup("string 2"));
 *   ...
 *   // loop through list
 *   sListNode* cur;
 *   for (cur = list->head; cur != NULL; cur = cur->next) {
 *       char* element = (char*) cur->data;
 *       // do stuff
 *       ...
 *   }
 *   ...
 *   // destroy list.
 *   if (list_destroy(list) != 0) {
 *       // report error
 *   }
 *
 *
 * This list type can also  be used as a queue or a stack, with their own set
 * of functions which are simply aliases for list functions. See the bottom of
 * this header file. These functions are of course optional to use, but are
 * supplied in order to make the code easier to understand when using queues
 * and stacks.
 *-----------------------------------------------------------------------*/

#ifndef LIST_H_
#define LIST_H_

#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

/* List node type. */
typedef struct sListNodeT {
  void* data;               /* Pointer to the data carried by the node. */
  struct sListNodeT* prev;  /* Previous node in the list. */
  struct sListNodeT* next;  /* Next node in the list. */
} sListNode;

typedef enum {
    LIST_THREADSAFE,
    LIST_NOT_THREADSAFE,
} eListThreadAlt;

/* Type for the list itself. */
typedef struct {
  sListNode* head;            /* Head node. */
  sListNode* tail;            /* Tail node. */
  size_t size;                /* Number of elements in the list. */
  void (*destroyFunc)(void*); /* Ptr to destructor function. */
  eListThreadAlt isThreadsafe;            /* 1 if this list is threadsafe 0 if not */
  pthread_mutex_t mutex;      /* Mutex lock to make the list thread safe. */
} sList;

/** 
 * Creates a new empty list. It should be freed with list_destroy() when no
 * longer needed. On failure returns NULL. 'destroyFunc' is called on all
 * remaining elements of the list when list_destroy() or list_clear() is called
 * on it, and is responsible for freeing up memory used by the element. The
 * destroyFunc can (and should) be used to perform more complex cleanup if the
 * list contains more complex elements.
 *
 * Some examples of what destroyFunc:s to use:
 *   Stored element                               destroyFunc
 *   --------------                               -----------
 *   Ptrs to data on the stack                    NULL (no cleanup is needed)
 *   Ptrs to malloc():ed data                     free()
 *   Ptrs to structs containing malloc():ed data  Custom destroyFunc is needed
 */
typedef void (*destroyFunc2)(void*);
sList* list_create(void (*destroyFunc)(void*));

/**
 * Frees up memory taken up by 'l'. If the list's 'destroyFunc' member is
 * non-NULL, it is called on all data elements before freeing 'l'. Returns 0 on
 * success, -1 on failure. After this call, the given sList can no longer be
 * used.
 */
int list_destroy(sList* l);

/**
 * Adds a new element to the front of the list. Returns 0 on success, -1 on
 * failure.
 */
int list_pushFront(sList* l, void* data);

/**
 * Adds a new element to the end of the list. Returns 0 on success, -1 on
 * failure.
 */
int list_pushBack(sList* l, void* data);

/**
 * Removes the head element from 'l' and returns it. Returns NULL if there are
 * no elements in the list, or if 'l' is NULL.
 */
void* list_popFront(sList* l);

/**
 * Removes the tail element from 'l' and returns it. Returns NULL if there are
 * no elements in the list, or if 'l' is NULL.
 */
void* list_popBack(sList* l);

/**
 * Removes the tail element from 'l' and returns it. Returns NULL if there are
 * no elements in the list, or if 'l' is NULL.
 */
//void* list_popBack(sList* l);

/** Retrieves the front element without removing it from the list. */
void* list_peekFront(sList* l);

/** Retrieves the back element without removing it from the list. */
void* list_peekBack(sList* l);

/**
 * Removes all elements from 'l'. Elements are cleaned up using the list's
 * destroyFunc if non-NULL, see comment about list_create() for details.
 */
int list_clear(sList* l);

/**
 * Returns 1 on success if 'data' exists in 'l', otherwise returns 0. Pointers,
 * not data, is compared, which might lead to surprising results if not
 * careful.
 */
int list_exists(sList* l, void* data);

/**
 * Performs a linear search of the list 'l' and returns the first element for
 * which 'cmpFunc' matches the given element 'el'. Returns NULL if no match was
 * found. 'cmpFunc' should be defined like the comparison function for the
 * standard library function qsort(), but distinction is only made between zero
 * and nonzero return values (different return values for elements being less
 * than or greater than a given element is not needed). O(n) complexity.
 */
void* list_find(sList* l, void* el, int (*cmpFunc)(void* a, void* b));

/**
 * Removes the first element from 'l' matching 'data'. Does _not_ free the
 * sListNode->data pointer, only the sListNode pointer. Returns 0 on success,
 * -1 on failure, or when no element was removed. Note that the comparison is
 * done as pointer equality, it doesn't inspect the actual data.
 */
int list_remove(sList* l, void* data);

/**
 * Removes a node that you have already found yourself. Does _not_ free
 * the sListNode->data pointer, only the node. Returns 0 on success,
 * <0 on failure or when no element was removed.
 */
int list_removeNode(sList *l, sListNode *node);

/** Returns the number of elements in the list, or 0 if 'l' is NULL. */
size_t list_size(sList* l);

/** 
 * Append src to dest. Return the concatenated list, dest, or NULL if dest 
 * or src is NULL or if they don have the same destroyFunc.
 * list_cat() does NOT destroy dest. src IS consumed and destroyed.
 */
sList* list_cat(sList* dest, sList* src);

/**
 * Adds a new element to the list, right before the node "next".
 * If next==NULL, the element will be placed at the end of the list.
 */
int list_insert(sList *list, void *data, sListNode *next);


typedef int32_t (*foreachFunc)(void *element, void *param);

/**
 * loop through the elements in list and call the foreach function with element
 * and param as arguments. If the foreach function returns something but a true
 * the list _foreach will return without calling the foreach function with any
 * remaining elements
 */
int list_foreach(sList *list, foreachFunc foreach, void *param);

#endif /* LIST_H_ */
