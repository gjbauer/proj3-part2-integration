#ifndef GDL_H
#define GDL_H

typedef struct GDL
{
	int64_t index;
	struct GDL *next;
	struct GDL *prev;
} GDL;

#endif
