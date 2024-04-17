/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    /**
    * TODO: implement per description
    */
    
    int sub_byte_pos = 0;
    int byte_pos = 0;
    int offset_count = 0;
    struct aesd_buffer_entry *ptrRet;


    if (buffer == NULL)
    {
      perror("Null pointer");
      exit(EXIT_FAILURE);
    }

    ptrRet = (struct aesd_buffer_entry*) malloc(sizeof(struct aesd_buffer_entry));

   while ((offset_count < char_offset) && (byte_pos < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED -1u))
   {
	   offset_count += buffer->entry[byte_pos++].size;

   }

    //printf("The offset_count is: %d\n", offset_count);
    //printf("The buffer size is: %ld\n", buffer->entry[byte_pos].size);

    entry_offset_byte_rtn = (size_t *) &byte_pos;

    if (offset_count >= char_offset)
    {
    	circ_bbuf_pop(buffer, ptrRet);
    }
    else
    {
    	// Verify if the char_offset is inside the actual byte of buffer[]
        sub_byte_pos = char_offset - offset_count;

    	// if offset (0) + SUB_BYTE_POS (7) < offset (0) + buffer[tail] (7)
        if ((offset_count + sub_byte_pos) < (offset_count + buffer->entry[byte_pos].size) )
        {
        	printf("The sub-byte position is: %d\n", sub_byte_pos);
    		//the element will be in actual buffer[byte_pos]
    		//calculate sub-byte position

            ptrRet->buffptr = buffer->entry[byte_pos].buffptr + sub_byte_pos;
            ptrRet->size = sizeof(buffer->entry[byte_pos].buffptr[sub_byte_pos]);
        }
        else
        {

        	return NULL;
        }

    }

    printf("The byte position is: %d\n", byte_pos);
    printf("The value is: %s\n", ptrRet->buffptr);
    printf("The size is: %ld\n", ptrRet->size);
    //ptrRet = (struct aesd_buffer_entry*) &buffer->entry[byte_pos];


    return ptrRet; 

    
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description
    */
   // If buffer is empty then initialize in_off and out_off
   // if buff is full then set in_off to out_off and increment out_off by 1
   /*buff is full when: */
     /* in_off is == max of write op */
     /* in_off is == out_off */

   /* Check if pointers are valid */
   if ( (buffer == NULL) && (add_entry == NULL) )
   {
      perror("Null pointer error");
      exit(EXIT_FAILURE);
   }

   /* Check if buffer was previously full */
   if (buffer->full == true)
   {
	   //This flag will be set after a next write after buffer is full
	   buffer->overlap_flag = true;
   }

   /* Check if buffer is full 
    ** head (in_offs) is one position before tail (out_offs)
    ** head is at the last position and tail is the zero pos */
   if ( (buffer->out_offs == buffer->in_offs + 1) ||
      ((buffer->in_offs == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED -1u) && (buffer->out_offs == 0)) )
   {
	   buffer->full = true;
   }

   buffer->entry[buffer->in_offs].buffptr = (const char*) add_entry->buffptr;
   buffer->entry[buffer->in_offs].size = strlen(add_entry->buffptr);

   printf("The buffer value is: %s\n", buffer->entry[buffer->in_offs].buffptr);
   printf("The size of the element is: %d\n", (int) buffer->entry[buffer->in_offs].size);

   
   if (buffer->in_offs == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED -1u)
   {
      /* set head to the initial position */
      buffer->in_offs = 0;
   }
   else
   {
	   buffer->in_offs++;
   }



}

int circ_bbuf_pop(struct aesd_circular_buffer *buffer,  struct aesd_buffer_entry *data_entry)
{
    int next;

    if ( (buffer->full == true) && (buffer->overlap_flag == true))
    {

    	buffer->out_offs++;
    	buffer->full = false;
    	buffer->overlap_flag = false;

    }

    next = buffer->out_offs + 1;  // next is where tail will point to after this read.
    if(next >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
    {
    	next = 0;
    }


    *data_entry = buffer->entry[buffer->out_offs];  // Read data and then move
    buffer->out_offs = next;                        // tail to next offset.

    //printf("The read buffer value is: %s\n", data_entry->buffptr);
    //printf("The read size is: %ld\n", data_entry->size);
    return 0;  // return success to indicate successful push.
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}

