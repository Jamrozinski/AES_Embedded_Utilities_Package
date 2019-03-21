/********************************************************************************
 * Description: Circular queue implementation using double linked list, leveraging
 * dynamic memory allocation for nodes.
 *
 * Remarks: Requires AES_Mem_Manager from AES_Utilities package
 *
 * Example:
 *
 * Author: Peter S Procek
 *
 * Copyright Notice:
 *
 * Copyright (c) 2017 by Apex Embedded Systems. All
 * rights reserved. This document is the confidential
 * property of Apex Embedded Systems Any reproduction
 * or dissemination is prohibited unless specifically
 * authorized in writing.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * *****************************************************************************/
#include "stdint.h"
#include "stdbool.h"
#include "assert.h"

#include "../Include/AES_Queue.h"
#include "../../AES_Memory/AES_Heap/Include/mem.h"

/***********************************************************************************************************************************************************
 *                                                                   Private XMACROS
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
#define XPRINTF(...)            container->printf_data_func( __VA_ARGS__ )
XPRINTF_CONTAINER container;

/***********************************************************************************************************************************************************
 *                                                             Private Variable Declarations
 *
 *
 *
 *
 ************************************************************************************************************************************************************/

typedef struct AES_Queue_Node
{
	/* data packet */
	AES_Queue_Node_Data_Packet data_packet;

    /* queue links */
    struct AES_Queue_Node * next;
    struct AES_Queue_Node * previous;

} AES_Queue_Node;

/***********************************************************************************************************************************************************
 *                                                             Private Function Declarations
 *
 *
 *
 *
 ************************************************************************************************************************************************************/

/*FUNCTION**********************************************************************
 * Function Name : New_AES_Queue_Node()
 * Description   : Allocates memory for a new node
 *
 * parameters:
 * -void
 *
 * Return value:
 * -location of new queue node
 *END**************************************************************************/

AES_Queue_Node * New_AES_Queue_Node( void )
{
	AES_Queue_Node * temp = (AES_Queue_Node *)Mem_Alloc( sizeof(struct AES_Queue_Node));
	return temp;
}

/*FUNCTION**********************************************************************
 * Function Name : AES_Queue_Node_Load
 * Description   :
 *
 * parameters:
 * -
 *
 * Return value:
 * -
 *
 * Todo: write a safe memcpy - memcpy is inherently unsafe!
 *END**************************************************************************/

void * AES_Queue_Node_Load_Object(AES_Queue_Node * node, void * object, int object_size)
{
	/* validate parameters */
	if( node == NULL || object == NULL || object_size < (int)sizeof(void *))
	{
		return NULL;
	}

	/* copy the object into a new memory location pointed to by the node */
	node->data_packet.data = Mem_Alloc( object_size );
	node->data_packet.data_size = object_size;

	if(node->data_packet.data != NULL)
	{
		return memcpy(node->data_packet.data, object, object_size);
	}

	return node->data_packet.data;
}

/***********************************************************************************************************************************************************
 *                                                             Public Function Declarations
 *
 *
 *
 *
 ************************************************************************************************************************************************************/

/*FUNCTION**********************************************************************
 * Function Name : AES_Queue_enQueue
 * Description   : enQueues a new item passed in by the user
 *
 * parameters:
 * - AES_Queue * Queue :: the queue itself
 * - void * object     :: the item to be enQueued
 * - in object_size    :: the size of the item to be enQueued
 *
 * Return value:
 * - Address of Queue if successful, NULL otherwise
 *END**************************************************************************/
void * AES_Queue_enQueue(AES_Queue * Queue, void * object, int object_size)
{
	/* spawn a new node */
	AES_Queue_Node * temp = New_AES_Queue_Node();

	/* copy the object into the node */
	if( AES_Queue_Node_Load_Object(temp, object, object_size) != NULL)
	{
		/* if queue is empty, new node becomes temp
		 * otherwise new node becomes tail
		 */
	    if (Queue->head == NULL)
	    {
	    	/* both heads and tails become initialized by first node */
	    	Queue->head = temp;
		    Queue->tail = temp;

		    /* link the nodes to eachother */
		    ((AES_Queue_Node *)(Queue->head))->previous = Queue->tail; /* link head back to tail */
		    ((AES_Queue_Node *)(Queue->head))->next = Queue->tail; /* link head forward to tail */

		    ((AES_Queue_Node *)(Queue->tail))->previous = Queue->head; /* link tail back to head */
		    ((AES_Queue_Node *)(Queue->tail))->next = Queue->head; /* link tail forward to head */

		    Queue->Queue_number_of_nodes = 1;

	    }
	    else
	    {
	    	/* link the nodes first, then update the tail pointer */
		    ((AES_Queue_Node *)(Queue->tail))->next = temp; /* link tail forward to new node */

		    temp->previous = ((AES_Queue_Node *)(Queue->tail)); /* link new node back to original tail */
		    temp->next = ((AES_Queue_Node *)(Queue->head)); /* link new node back to head */

	    	/* move the tail pointer to the next connected node*/
		    Queue->tail = ((AES_Queue_Node *)(Queue->tail))->next;

		    Queue->Queue_number_of_nodes += 1;

	    }


	    return (void *)Queue;
	}

	return NULL;

}

/*FUNCTION**********************************************************************
 * Function Name : AES_Queue_deQueue
 * Description   : deQueues the tail object. **IMPORTANT: User must free the
 * returned data packet when done with it.**
 *
 * parameters:
 * - AES_Queue * Queue :: the queue itself
 *
 * Return value:
 * - AES_Queue_Node_Data_Packet * :: pointer pointing to address of the data packet
 *END**************************************************************************/
int AES_Queue_deQueue(AES_Queue * Queue, AES_Queue_Node_Data_Packet * data_packet)
{
	AES_Queue_Node_Data_Packet * deQueued_data_packet;
	AES_Queue_Node * tmp_nxt;
	AES_Queue_Node * tmp_prev;

	/* check that the queue is valid */
	if( Queue == NULL || Queue->head == NULL || Queue->tail == NULL)
	{
		return 0;
	}

	/* grab the data in the head node to be deQueued */
	deQueued_data_packet = &(((AES_Queue_Node *)(Queue->head))->data_packet);

	if(data_packet != NULL)
	{
		memcpy(data_packet, deQueued_data_packet, sizeof(AES_Queue_Node_Data_Packet));
	}


    if (Queue->head == Queue->tail) // There is only one node
    {
        if ( Mem_Free((void *)Queue->head) < 0 )
        	return 0;

        Queue->head = NULL;
        Queue->tail = NULL;
    }
    else // There are more than one nodes
    {
    	tmp_nxt = ((AES_Queue_Node *)(Queue->head))->next;
    	tmp_prev = ((AES_Queue_Node *)(Queue->head))->previous; //tail

    	/* unlink the head node by connecting the next node's previous link to the tail node */
    	tmp_nxt->previous = tmp_prev;

    	/* link the tail node to the new head node */
    	tmp_prev->next = tmp_nxt;

    	/* Free the data */
        if ( Mem_Free((void *)(deQueued_data_packet->data)) < 0 )
        	return 0;

    	/* Free the head */
        if ( Mem_Free((void *)(Queue->head)) < 0 )
        	return 0;

    	/* set new head */
    	Queue->head = tmp_nxt;
    }

    Queue->Queue_number_of_nodes--;

    return 1;
}

/*FUNCTION**********************************************************************
 * Function Name : AES_Queue_Peak
 * Description   : peaks the data from the tail node without actually deQueueing
 *
 * parameters:
 * - AES_Queue * Queue :: the queue itself
 *
 * Return value:
 * - AES_Queue_Node_Data_Packet * :: pointer pointing to address of the data packet
 *END**************************************************************************/
AES_Queue_Node_Data_Packet * AES_Queue_Peak( AES_Queue * Queue)
{
	AES_Queue_Node_Data_Packet * deQueued_data;

	/* check that the queue is valid */
	if( Queue == NULL || Queue->head == NULL || Queue->tail == NULL)
	{
		return NULL;
	}

	/* grab the data in the head node to be deQueued */
	deQueued_data = &((AES_Queue_Node *)(Queue->head))->data_packet;

	return deQueued_data;
}

/*FUNCTION**********************************************************************
 * Function Name : AES_Queue_Node_Count
 * Description   : returns how many nodes exist in the queue
 *
 * parameters:
 * - AES_Queue * Queue :: the queue itself
 *
 * Return value:
 * - int Queue_number_of_nodes :: number of nodes in the queue
 * - < 0 if error
 *END**************************************************************************/
int AES_Queue_Node_Count(AES_Queue * Queue)
{
	if( Queue == NULL )
		return -1;

	return Queue->Queue_number_of_nodes;
}

/*FUNCTION**********************************************************************
 * Function Name : AES_Queue_isEmpty
 * Description   : checks if the queue is empty or not
 *
 * parameters:
 * -
 *
 * Return value:
 * -
 *END**************************************************************************/
bool AES_Queue_isEmpty( AES_Queue * Queue )
{
	if( Queue->head == NULL && Queue->tail == NULL && Queue->Queue_number_of_nodes == 0)
		return true;

	return false;
}

/*FUNCTION**********************************************************************
 * Function Name : AES_Queue_Delete
 * Description   : Deletes the queue and frees any allocated memory associated
 * 				   with the queue
 * parameters:
 * -
 *
 * Return value:
 * -
 *END**************************************************************************/
int AES_Queue_Delete( AES_Queue * Queue )
{
	int number_nodes_deleted = 0;
	AES_Queue_Node * temp = NULL;
	AES_Queue_Node * temp_previous = NULL;
	AES_Queue_Node * temp_next = NULL;

	//start at the head
	temp = (AES_Queue_Node *)Queue->head;

	while( !AES_Queue_isEmpty( Queue ) )
	{
		if( temp->next != temp->previous)
		{
			//save previous and next indices
			temp_previous = temp->previous;
			temp_next = temp->next;

			//deallocate node data packet and the node itself
			if( Mem_Free(temp->data_packet.data) < 0)
			{
				return -1;
			}

			if( Mem_Free(temp) < 0)
			{
				return -1;
			}

			number_nodes_deleted = number_nodes_deleted + 1;
			Queue->Queue_number_of_nodes--;

			//reconnect the nodes
			temp_previous->next = temp_next;
			temp_next->previous = temp_previous;

			temp = temp_next;

		}
		else
		{
			//Only one node exists

			//deallocate node data packet and the node itself
			if( Mem_Free(temp->data_packet.data) < 0)
			{
				return -1;
			}

			if( Mem_Free(temp) < 0)
			{
				return -1;
			}

			number_nodes_deleted = number_nodes_deleted + 1;
			Queue->Queue_number_of_nodes--;

			Queue->head = NULL;
			Queue->tail = NULL;

		}
	}



	return number_nodes_deleted;
}

/* Initializes test -- really it just sets a pointer for the print function */
int AES_Queue_Print_Init( void * ptr)
{
    container = (XPRINTF_CONTAINER)ptr;

    return 1;
}
#if 1
/*FUNCTION**********************************************************************
 * Function Name :
 * Description   :
 *
 * parameters:
 * -
 *
 * Return value:
 * -
 *END**************************************************************************/
void AES_Queue_Print( XPRINTF_CONTAINER container )
{
	(void)container;
#if 0
	 int counter;
	  block_header* current = NULL;
	  char* t_Begin = NULL;
	  char* Begin = NULL;
	  int Size;
	  int t_Size;
	  char* End = NULL;
	  int free_size;
	  int busy_size;
	  int total_size;
	  char status[5];

	  free_size = 0;
	  busy_size = 0;
	  total_size = 0;
	  current = list_head;
	  counter = 1;
	  XPRINTF("************************************Block list***********************************\r\n");
	  XPRINTF("No.\tStatus\tBegin\t\tEnd\t\tSize\tt_Size\tt_Begin\r\n");
	  XPRINTF("---------------------------------------------------------------------------------\r\n");
	  while(NULL != current)
	  {
	    t_Begin = (char*)current;
	    Begin = t_Begin + (int)sizeof(block_header);
	    Size = current->size_status;
	    strcpy(status,"Free");
	    if(Size & 1) /*LSB = 1 => busy block*/
	    {
	      strcpy(status,"Busy");
	      Size = Size - 1; /*Minus one for ignoring status in busy block*/
	      t_Size = Size + (int)sizeof(block_header);
	      busy_size = busy_size + t_Size;
	    }
	    else
	    {
	      t_Size = Size + (int)sizeof(block_header);
	      free_size = free_size + t_Size;
	    }
	    End = Begin + Size;
	    XPRINTF("%d\t%s\t0x%08lx\t0x%08lx\t%d\t%d\t0x%08lx\r\n",counter,status,(unsigned long int)Begin,(unsigned long int)End,Size,t_Size,(unsigned long int)t_Begin);
	    total_size = total_size + t_Size;
	    current = current->next;
	    counter = counter + 1;
	  }
	  XPRINTF("---------------------------------------------------------------------------------\r\n");
	  XPRINTF("*********************************************************************************\r\n");

	  XPRINTF("Total busy size = %d\r\n",busy_size);
	  XPRINTF("Total free size = %d\r\n",free_size);
	  XPRINTF("Total size = %d\r\n",busy_size+free_size);
	  XPRINTF("*********************************************************************************\r\n");

	  return;
#endif
}

#endif
