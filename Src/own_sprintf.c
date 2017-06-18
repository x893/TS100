#include <stdarg.h>
#include "own_sprintf.h"

static char * __own_itoa(int num, char *dst)
{
	int i = 0, digits = 0, n;
	if (num < 0)
	{
		dst[i++] = '-';
		num = -num;
	}
	n = num;
	n /= 10;
	digits = 1;
	while (n)
	{
		n /= 10;
		digits *= 10;
	}

	while (digits)
	{
		dst[i++] = (num / digits) + '0';
		num %= digits;
		digits /= 10;
	}

	dst[i] = '\0';
	return dst;
}

char* own_sprintf(char* dest, const char* cntrl_string, ...)
{
	int pos = 0, cnt_printed_chars = 0, i;
	unsigned char* chptr;
	char str[8];
	int flp;
	va_list ap;

	if (!dest )
		return 0;

	// Causes ap to point to the first optional parameter
	va_start(ap, cntrl_string);

	while (cntrl_string[pos])
	{
		if (cntrl_string[pos] == '%')
		{
			pos++;
			switch (cntrl_string[pos])
			{
			case 'c':
				dest[cnt_printed_chars++] = va_arg (ap, int); // Va_arg returns the current argument in the argument list and points the ap to the next argument in the argument list
				break;
			case 's':
				chptr = va_arg (ap, unsigned char*);
				i = 0;
				while (chptr [i])
					dest[cnt_printed_chars++] = chptr [i++];
				break;
			case 'd':
				__own_itoa (va_arg (ap, int), str);
				i = 0;
				while (str [i])
					dest[cnt_printed_chars++] = str [i++];
				break;
			case 'f':
				flp = va_arg (ap, int);
				if(flp <= 9 && flp > 0)
				{	//Less than 0.1 greater than 0.01
				}
				else if(flp > 9 && flp <= 99)
				{	// Greater than 0.1 less than 1
				}
				else if(flp > 99 && flp <= 999)
				{	// Greater than 1 less than 10
					dest[3] = (flp % 10) + '0';
					flp /= 10;
					dest[2] = (flp % 10) + '0';
					dest[1] = '.';
					flp /= 10;
					dest[0] = (flp % 10) + '0';
					cnt_printed_chars = 4;
				}
				else if(flp > 999 && flp <= 9999)
				{	// Greater than 10 is less than 100
					dest[4] = (flp % 10) + '0';
					flp /= 10;
					dest[3] = (flp % 10) + '0';
					dest[2] = '.';
					flp /= 10;
					dest[1] = (flp % 10) + '0';
					flp /= 10;
					dest[0] = (flp % 10) + '0';
					cnt_printed_chars = 5;
				}
				break;
			default:
				dest[cnt_printed_chars++] = cntrl_string[pos];
				break;
			}
		}
		else
			dest[cnt_printed_chars++] = cntrl_string[pos];

		pos++;
	}
	va_end(ap); // Va_end clear the ap pointer to NULL
	dest[cnt_printed_chars] = '\0';
	return dest;
}
