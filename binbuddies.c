#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct mem_block{
	void *startaddr;  /*  pointer to the start of the memory  */
	int size;  /*  Measures size in bytes  */
}mem_block;

/*  Begin global variables  */

mem_block mainblock;  /*  Pointer to the main allocated  block of memory.  */

/*  Implement a function to go through the memory and check if any memory leaks occoured.  Print any memory leaks if they exist and free the initial
	memory chunk.  */

void end_memory()
{
	/*  Declare a temporary pointer to the start of memory to cycle and check for leaks. */

	int *temp = (int *)mainblock.startaddr;

	/*  Go through memory byte by byte.  If we come across a non-zero byte, we have a child node that hasnt been released.  */

	int i;

	for(i = 0; i<mainblock.size; i++)
	{
		/*  Memory not freed.  */

		if(temp+i != 0)
		{
			printf("Memory leak at block of size %d at address %d.\n", *(temp+3), *(temp));
			temp += *(temp+3);  /*  Increment temp to point to the next byte after the leaked block.  */
		}
		else
		{
			temp += i;  /*  Go check the next byte.  */
		}
	}

	/*  Now that we have identified all the leaks, we will free the initial memory.  Assignment was unclear if at this point
		memory leaks should be released or not.  */

	free(mainblock.startaddr);


}

/*  Implement a function to split a node in our binary tree.  This is done to create a new memory partition.  

	Arguments: Pointer to parent node	*/

void add_node(int *address)
{
	int *temp;
	temp = address;

	/*  Make a temporary integer to store the offset representing the indexes of parent node information  */

	int offset = 1;
	int tempsize = *(address+3);

	while(tempsize < mainblock.size)
	{
		tempsize = tempsize*2;
		offset++;
	}

	/*  Shift the parent nodes up 4 bytes to make room for this nodes information.  */

	/*  Point temp to the last item of parent node information.  */
	temp += (offset * 4) - 1;

	/*  Shift each parent item down by 4 bytes.  */
	int i = 0;

	for(i = 0; i < offset*4; i++)
	{
		*(temp+4) = *temp;
		temp--;
	}

	/*  Point temp back to the start of our new partition  */
	temp = address;

	/*  Add a left node  */
	*temp = 1;  /*  left node true  */
	*(temp+1) = 0;	/*  right node false  */
	*(temp+2) = 1;  /*  free true  */
	*(temp+3) = *(address+3) / 2;  /*  size = half the size of parent node  */

	/*  Add a right node  */
	temp += *(address+3);  /*  Go to the right child slot  */
	*temp = 0;  /*  left node false  */
	*(temp+1) = 1;  /*  right node true  */
	*(temp+2) = 1;  /*  free true  */
	*(temp+3) = *(address+3);  /*  size = half the size of parent node  */

	/*  Copy the rest of the parent node information to the right child.  */

	/*  Make a new pointer to point at the start of the parent node info, using the left child as a refrence  */
	int *temp2 = address+4;

	/*  Point temp to the first empty parent node info slot, using the right child as a refrence  */
	temp += 4;

	/*  Copy to right child  */
	i=0;

	for(i = 0; i < offset*4; i++)
	{
		*(temp+i) = *(temp2+i);
	}



}

void release_memory(void *p)
{
	/*  Create a temporary pointer to point to integers at r  */
	int *temp = (int *)p;

	/*  Make a temporary intelger to store the offset representing the indexes of parent node information  */

	int offset = 1;
	int tempsize = *(temp+3);

	while(tempsize < mainblock.size)  
	{
		tempsize = tempsize*2;
		offset++;
	}

	/*  Erase the partition before doing any joining of nodes.  */

	/*  Go to the end of all the nodal information and erase the user memory.  */
	int i = 0;

	for(i = (offset * 4); i <* (temp+3); i++)
	{
		*(temp+i) = 0;
	}

	/*  Mark the node as free.  */
	*(temp+2) = 1;

	if(*(temp+1) == 1 && *(temp) == 0)  /*  We are the right child node of this size  */
	{
		/*  Look one partition above.  If the node is the same size and it is free, then it is our left brother node, so we can join the
			two together.  */

		temp -= *(temp+3);

		if(*(temp+3) == *((int*)p+3) && *(temp+2) == 1 && *(temp) == 1 && *(temp+1) == 0)  /*  If the previous node is a left child, the same size and free, join  */
		{
			/*  Delete the child node information from the left nodes bit sequence.  */
			i = 0;

			for(i = 0; i < (offset*4)-4; i++)
			{
				*(temp) = *(temp+4);  /*  Shift the parent information unit down */
				*(temp+4) = 0;  /*  Erase the old information unit from this slot to eliminate copies.  */
				temp++;
			}

			/*  Since we are joining the left child to our desired pointer (p), so we will append p to the end of the left child.  */

			/*  Navigate up in memory to the start of the left child  */
			temp = (int *)p;
			temp -= *(temp+3);

			/*  Now from the start of our newly sized block, erase all the memory including the bit patterns of the right node we are joining.  */
			i = 0;

			for(i = (offset*4)-4; i < *(temp+3); i++)
			{
				*(temp+i) = 0;
			}

			/*  Now that we have a new free node, see if it can be joined up again.  */
			release_memory(temp);


		}
	}
	else if(*(temp+1) == 0 && *(temp) == 1)  /*  We are the left child node of this size.  */
	{
		/*  Look one partition below.  If the node is the same size, the opposite child, and is free, then it is our right brother node,
			so we can join the two together.  */

		temp += *(temp+3);

		if(*(temp+3) == *((int*)p+3) && *(temp+2) == 1 && *(temp) == 0 && *(temp+1) == 1)  /*  If the next node is a right child, the same size and free, join  */
		{

			/*  Go back up to our left child node  */

			temp -= *(temp+3);

			i = 0;

			for(i = 0; i < (offset*4)-4; i++)  /*  Erase child node information of the left node before we join  */
			{
				*(temp) = *(temp+4);
				*(temp+4) = 0;  /*  Erase the old information unit from this slot to eliminate copies.  */
				temp++;
			}


			temp = (int *)p;

			/*  Now clear all the way down to the end of the right child, including its nodal information.  */

			i = 0;

			for(i = (offset*4)-4; i < *(temp+3); i++)
			{
				*(temp+i) = 0;
			}

			/*  Now that we have a new free node, see if it can be joined up again.  */
			release_memory(temp);

		}
	}
}


/*  Implement a function to assign an appropriately sized chunk of memory to our requesting program  */

void * get_memory(int size)
{

	/*  Check the size of our initial partition against the binary numbers.  Start i at 1 since binary buddies do not include
		memory partitions of 1 size unit (in our case kb).  */
	int chunksize = 0;
	int i = 1;

	while(1)
	{
		chunksize = pow(2, i);

		if(chunksize > size) /*  We have gone too far.  The largest block is one permutation of i back.  */
		{
			i--;
			chunksize = pow(2, i);
			break;
		}
		else /*  Keep incrementing our largest chunk size.  */
		{
			i++;
		}
	}

	/*  Now that we have our largest chunk size, loop through the partitions.  Keep going either until
		we find an appropriately sized partition that is free, or until we reach the end of our memory.   */

	/*  Make a temporary pointer to the global pointer.  */

	int *temp;
	temp = (int *)mainblock.startaddr;


	/*  Check the bit pattern in the next 4 bytes.  

		Byte 0: Left Child (boolean value)

		Byte 1: Right Child (boolean value)

		Byte 2: Free (boolean value)

		Bytes 3: Size of this partition

	*/

	/*  Make a temporary integer to store the offset representing the indexes of parent node information and an integer to track the loop  */

	int offset = 0;
	int tempsize;

	while(temp <= (int *)mainblock.startaddr+mainblock.size)
	{

		offset = 1;
		tempsize = *(temp+3);

		while(tempsize < mainblock.size)
		{
			tempsize = tempsize*2;
			offset++;
		}

		if(temp+2 == 0)  /*  The block is not free  */
		{
			/*  Increment our temp pointer to the next block  */
			temp += *(temp+3);
		}
		else if(*(temp+3) < size)  /*  If the block is smaller than our requested size  */
		{
			/*  Increment our temp pointer to the next block  */
			temp += *(temp+3);
		}
		else if( *(temp+3) - (offset*4)  >= (size*2))  /*  If the block is big enough to split in half and satisfy our size plus bit pattern(s), split it.  */
		{
			add_node(temp);
			temp = (int *)get_memory(size);  /*  Recursively break down our partitions until we get an address of a good partition.  */
			*(temp+2) = 0;  /*  Change the free bit to indicate this partition is taken  */
			return temp;
		}
		else  /*  This block is big enough to space efficiently satisfy this memory request.  */
		{
			mainblock.startaddr = mainblock.startaddr;
			*(temp+2) = 0;  /*  Change the free bit to indicate this partition is taken  */
			return temp;
		}

	}

	return NULL;

}

/*  Implement a function that allocates the amount of memory requested by user.  This function will return a pointer to the 
	free spot in memory.  If the requested size is not availble, return 1.  */

void *start_memory(int size)
{
	void *memstart;

	memstart = malloc(size);

	if (memstart == 0)  /*  Malloc call failed  */
	{
		printf("Memory allocation failed.\n");
		return NULL;
	}
	else /*  Initialize our global pointer  */
	{
		/*  Initialize the global memory chunk, including its base address and size.  */
		mainblock.startaddr = memstart;
		mainblock.size = size;

		/*  Set the bit pattern at the top of memory to refer to our main block.  */
		int *temp;
		temp = (int *)mainblock.startaddr;

		*temp = 0;  temp++;  /*  Left Child  */
		*temp = 0; temp++;  /*  Right Child  */
		*temp = 1; temp++;  /*  Free  */
		*temp = size;  /*  Size  */

		return memstart; /*  Return the void pointer to the first free spot.  */
	}

}


int main(int argc, char *argv[])
{
	/*  Declare a void pointer to point to the address returned by get_memory().  */

	void *memstart;

	/*  Declare a variable to hold the requested amount of memory.  */
	int size;

	/*  Check to see if a size is requested in command line.  If not, prompt the user for a size.  */

	if(argc < 2)
	{
		printf("Please enter an initial memory block size in bytes.\n");
		scanf("%d", &size);
		memstart = start_memory(size);
	}
	else
	{
		sscanf(argv[1], "%d", &size);
		memstart = start_memory(size);
	}

	/*  TESTS GO HERE  */


	end_memory();

}