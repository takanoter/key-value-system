#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void log_err(const char* filename, const int line, FILE* fp, const char* fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);

	if (fp) {
		fprintf (fp, "[%d]:", line);
		vfprintf (fp, fmt, ap);	
		fprintf (fp, "\n");
		fflush(fp);
	} else {
		printf ("%s %d:", filename, line);
		vprintf (fmt, ap);
		printf ("\n");
	}

	va_end(ap);
	return;
}

