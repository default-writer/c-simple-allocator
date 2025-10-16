#include <stdio.h>
#include <stdlib.h>

struct C { int x; };
struct B { struct C c; };
struct A { struct B b; };

struct node { int x; };
struct linked_list
{
    struct node l;
    struct linked_list *r;
};

typedef unsigned long long _u64;

int main(void) {
    struct A a;
    printf("&a:         0x%016lld\n", (_u64)&a);
    printf("&a.b:       0x%016lld\n", (_u64)&a.b);
    printf("&a.b.c:     0x%016lld\n", (_u64)&a.b.c);

    struct linked_list _ll;
    _ll.l.x = 1;
    _ll.r = (struct linked_list*)malloc(sizeof(struct linked_list));
    _ll.r->l.x = 2;
    _ll.r->r = NULL;
    
    printf("&ll:        0x%016lld\n", (_u64)&_ll);
    printf("&ll.l:      0x%016lld\n", (_u64)&_ll.l);
    printf("&ll.r:      0x%016lld\n", (_u64)&_ll.r);
    printf("&ll.r.l:    0x%016lld\n", (_u64)&_ll.r->l);
    printf("&ll.r.l.x:  0x%016lld\n", (_u64)&_ll.r->l.x);

    free(_ll.r);
    return 0;
}