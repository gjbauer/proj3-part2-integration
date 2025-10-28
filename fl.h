#ifndef FL_H
#define FL_H

typedef struct FL_LL
{
	int index;
	struct FL_LL *next;
} FL_LL;

FL_LL *fl_push(FL_LL *list, int index);
FL_LL *fl_pop(FL_LL *list);

#endif
