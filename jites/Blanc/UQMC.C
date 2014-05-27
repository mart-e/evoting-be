/* START CONFIDENTIAL */  

/****************************************************************/
/*                                                              */
/*    Character Queue Manager                                   */
/*                                                              */
/*                                                              */
/*   K. Craeynest          n.v. dZine      Created: Jun, 92     */
/*   Adapted by E. Franssen                                     */
/*                                                              */
/****************************************************************/

#include <stdio.h>  /* Needed only for NULL */
#include <string.h>

#pragma check_stack( off) /* Functions may be called frrom interrupt */

/**************************************************************************
*
* This function initialises the Character Queue Manager
*
*  Params:
*    count count of character to queue
*    ptr   pointer to queue space (thus count bytes)
*    uqmc  address of UQMC structure to link with queue
*
*  Returns:
*    0     if error, not initialised
*    !0    if initialised
*
*/
int uqmc_init( uint count, uchar far *ptr, UQMC *uqmc)
{
    int retval;

    memset(uqmc, 0, sizeof(UQMC));
    uqmc->start = ptr;
    if (uqmc->start == NULL || count == 0)
	retval = 0;
    else
    {
	uqmc->next_write = uqmc->start;
	uqmc->next_read = uqmc->start;
	uqmc->end = uqmc->start + count;
	uqmc->element_count = 0;
	uqmc->max_elements = count;
	retval = 1;
    }

    return( retval);
}

/**************************************************************************
*
* This function reads an element out of the queue
*
*  Params:
*    uqmc  address of the involved queue structure
*
*  Returns:
*    >= 0  the character that is read
*    -1    if unable to read
*
*/
int uqmc_read( UQMC *uqmc)
{
    int retval = -1;                    /* Suppose nothing available */

    if ( uqmc->element_count)
    {
	retval = *uqmc->next_read++;
	if ( uqmc->next_read == uqmc->end)
	    uqmc->next_read = uqmc->start;
	uqmc->element_count--;
    }

    return( retval);
}

/**************************************************************************
*
* This function reads an element ahead from the queue, but the element
* is not removed from the queue, this is only done in uqmc_read function.
*
*  Params:
*    uqmc  address of the involved queue structure
*
*  Returns:
*    >= 0  the character that is read ahead
*    -1    if unable to read
*
*/
int uqmc_read_ahead( UQMC *uqmc)
{
    return( (uqmc->element_count) ? *uqmc->next_read : -1);
}

/**************************************************************************
*
* This function writes an element into the queue
*
*  Params:
*    c     element to write
*    uqmc  address of the involved queue structure
*
*  Returns:
*    0     if unable to write (buffer full)
*    !0    if element written
*
*/
int uqmc_write( uchar c, UQMC *uqmc)
{
    int retval = 0;                     /* Suppose won't succeed */

    if ( uqmc->start != NULL && uqmc->element_count < uqmc->max_elements)
    {
	*uqmc->next_write++ = c;
	if (uqmc->next_write == uqmc->end)
	    uqmc->next_write = uqmc->start;
	uqmc->element_count++;
	retval = 1;
    }

    return( retval);
}

/**************************************************************************
*
* This function returns the number of elements in the queue
*
*  Params:
*    uqmc  address of the involved queue structure
*
*  Returns:
*    the number of elements in the buffer
*
*/
uint uqmc_check_used( UQMC *uqmc)
{
    return( uqmc->element_count);
}

/**************************************************************************
*
* This function returns the number of elements that can be put in the queue
*
*  Params:
*    uqmc  address of the involved queue structure
*
*  Returns:
*    the number of elements that can be put into the buffer
*
*/
uint uqmc_check_free( UQMC *uqmc)
{
    return( uqmc->max_elements - uqmc->element_count);
}

/**************************************************************************
*
* This function clears the queue (removes all elements)
*
*  Params:
*    uqmc  address of the involved queue structure
*
*  Returns: nothing
*
*/
void uqmc_flush( UQMC *uqmc)
{
    if ( uqmc->start != NULL)
    {
	uqmc->next_write = uqmc->start;
	uqmc->next_read = uqmc->start;
	uqmc->element_count = 0;
    }
}

/**************************************************************************
*
* This function stops the Queue Manager
*
*  Params:
*    uqmc  address of UQM structure to link with queue
*
*  Returns:
*    nothing
*/
void uqmc_exit( UQMC *uqmc)
{
    memset(uqmc, 0, sizeof(UQMC));
}

#pragma check_stack()
/* END CONFIDENTIAL */  
