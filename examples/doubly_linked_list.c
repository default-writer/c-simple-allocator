#include <stdio.h>
#include <stdlib.h>

struct C { int x; };
struct B { struct C c; };
struct A { struct B b; };

struct node
{
    int x;
};

struct doubly_linked_list
{
    struct node n;
    struct doubly_linked_list *r;
    struct doubly_linked_list *l;
};

typedef unsigned long long _u64;

int main(void) {
    void* memory_block = calloc(1,
        sizeof(struct doubly_linked_list) * 3 +
        sizeof(struct node) * 2);

    struct doubly_linked_list dll;

    struct doubly_linked_list* dll_ptr = (struct doubly_linked_list*)memory_block;
    struct doubly_linked_list a = *(dll_ptr+0);
    struct doubly_linked_list b = *(dll_ptr+1);
    struct doubly_linked_list c = *(dll_ptr+2);

    struct node* node_ptr = (struct node*)((struct doubly_linked_list*)memory_block+3);
    struct node n1 = *(node_ptr + 0);
    struct node n2 = *(node_ptr + 1);

    //     \
    //      A
    //     / \
    //    B   C
    //    |   |
    //    N1  N2

    dll.r = &a;
    a.l = &b;
    a.r = &c;

    n1.x = 1;

    b.n = n1;
    c.n = n2;

    n2.x = 2;

    printf("&ll:          0x%016lld\n", (_u64)&dll);
    printf("&ll.l:        0x%016lld\n", (_u64)&dll.l);
    printf("&ll.r:        0x%016lld\n", (_u64)&dll.r);
    printf("&ll.r.l:      0x%016lld\n", (_u64)&dll.r->l);
    printf("&ll.r.l.n:    0x%016lld\n", (_u64)&dll.r->l->n);
    printf("&ll.r.r.n:    0x%016lld\n", (_u64)&dll.r->r->n);
    printf("ll.r.l.n.x:   0x%016lld\n", (_u64)dll.r->l->n.x);
    printf("ll.r.r.n.x:   0x%016lld\n", (_u64)dll.r->r->n.x);

    free(memory_block);
    return 0;
}