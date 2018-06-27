/* C standard lib port. */

#include "../../src/MpsPort.h"

#include <stdlib.h>
#include <time.h>

void *MpsPortMalloc(uint32_t size)
{
	return malloc(size);
}

void MpsPortFree(void *ptr)
{
	free(ptr);
}

uint32_t MpsPortTimeMillis(void)
{
	return time();
}

uint32_t MpsPortTimeHours(void)
{
	return 0;
}