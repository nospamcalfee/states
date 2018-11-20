/**********************************************************************
 *
 * Filename:    list_unit_test.c
 *
 * Description: test the linked list functionality
 *
 * Notes:       This software should be portable to Posix compatible systems.
 *
 *
 * Copyright (c) 2018 by Steve Calfee.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include "cdll.h"
#include <states.h>

struct cdll myvalvelist;
struct teststruct {
	int struct_no;	/* struct number */
	struct cdll vlist;     /* list of all known valves */
};

#define cast_cdll_to_teststruct(pt) (cast_p_to_outer( \
            struct cdll *, pt, \
            struct teststruct, vlist))

void removevalvelist()
{
	struct cdll *vlist;
	struct teststruct *test;

	cdll_for_each(vlist, &myvalvelist) {
		test = cast_cdll_to_teststruct(vlist);
		printf("fifo delete %p, number %d\n", test, test->struct_no);
		cdll_delete_node(vlist);
		free(cast_cdll_to_teststruct(vlist));
	}
}

void removevalvelist_rev()
{
	struct cdll *pos;
	struct teststruct *test;
	//for a complete test, actually remove the fifo backwards
	cdll_for_each_rev(pos, &myvalvelist) {
		test = cast_cdll_to_teststruct(pos);
		printf("filo delete %p, number %d\n", test, test->struct_no);
		cdll_delete_node(pos);
		free(cast_cdll_to_teststruct(pos));
	}
}

void printlist(struct cdll *p)
{
	struct cdll *vlist;
	struct teststruct *test;
	cdll_for_each(vlist, p) {
		test = cast_cdll_to_teststruct(vlist);
		printf("list %p, number %d\n", test, test->struct_no);
	}

}
#define VERSION "1.0"
int main( int argc, char *argv[] )
{
	printf("C based unittest, version %s\n", VERSION);

	cdll_init(&myvalvelist);
	removevalvelist();
	cdll_init(&myvalvelist);
	for (int i = 0; i < 8; i++) {
		struct teststruct *listtest = calloc(1, sizeof(*listtest));
		cdll_init(&listtest->vlist);
		listtest->struct_no = i;
		cdll_insert_node_tail(&listtest->vlist, &myvalvelist);
		printf("new filo %p, number %d\n", listtest, listtest->struct_no);
		//printlist(&myvalvelist);
		//printf("\n");
	}
	printlist(&myvalvelist);
	removevalvelist_rev();
	for (int i = 0; i < 3; i++) {
		struct teststruct *listtest = calloc(1, sizeof(*listtest));
		cdll_init(&listtest->vlist);
		listtest->struct_no = i + 100;
		cdll_insert_node_head(&listtest->vlist, &myvalvelist);
		printf("new fifo item %p, number %d\n", listtest, listtest->struct_no);
		//printlist(&myvalvelist);
		//printf("\n");
	}
	printlist(&myvalvelist);
	removevalvelist();
}
