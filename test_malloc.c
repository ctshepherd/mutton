#include "malloc.h"

#define MAXP	400
#define SUBP	20
#define NPASS	25
#define NLOOP	12

double metrics[NLOOP];

int lrand()
{
  static unsigned long long next = 0;
  next = next * 0x5deece66dLL + 11;
  return (int)((next >> 16) & 0x7fffffff);
}

char *pointers[MAXP];
int size[MAXP];

int rsize()
{
  int rv = 8 << (lrand() % 15);
  rv = lrand() & (rv-1);
  return rv;
}

void test_malloc(void)
{
	int i, r, loop, pass, subpass;
	int absmax=0, curmax=0;
	int realloc_mask = -1;

	for (loop=0; loop<NLOOP; loop++) {
		for (pass=0; pass<NPASS; pass++) {
			for (subpass=0; subpass<SUBP; subpass++) {
				for (i=0; i<MAXP; i++) {
					int rno = lrand();
					if (rno & 8) {
						if (pointers[i]) {
							if (!(rno & realloc_mask)) {
								r = rsize();
								curmax -= size[i];
								curmax += r;
								pointers[i] = realloc(pointers[i], rsize());
								size[i] = r;
								if (absmax < curmax)
									absmax = curmax;
							} else {
								curmax -= size[i];
								free(pointers[i]);
								pointers[i] = 0;
							}
						} else {
							r = rsize();
							curmax += r;
							pointers[i] = malloc(r);
							size[i] = r;
							if (absmax < curmax)
								absmax = curmax;
						}
					}
				}
			}
		}
	}
}
