#include <stdio.h>
#include <stdlib.h>

struct list_item {
	int value;
    struct list_item * next;
	};
//typedef struct list_item link;


void append(struct list_item *first, int x) {
	struct list_item *temp = first;
	while(temp->next != NULL) {
		temp = temp->next;
	}
	struct list_item *last;
	last = malloc(sizeof(struct list_item));
	temp->next = last;
	last->value = x;
	last->next = NULL;
}

void prepend(struct list_item *first, int x) {
	struct list_item *new_first = malloc(sizeof(struct list_item));
	new_first->value = x;
	new_first->next = first->next;
	first->next = new_first;
}

void print(struct list_item *first) {
	struct list_item *temp = first->next;
	while (temp != NULL) {
		printf("%i\n", temp->value);
		temp = temp->next;
	}
}

void input_sorted(struct list_item *first, int x) {
	struct list_item *temp = first; 	
	while(temp->next != NULL) {
		if(temp->next->value > x) {
			struct list_item *inp_sort = malloc(sizeof(struct list_item));
			inp_sort->value = x;
			inp_sort->next = temp->next;
			temp->next = inp_sort;
			break;
		}
		temp = temp->next;
	}
	if(temp->next == NULL) {
		struct list_item *last = malloc(sizeof(struct list_item));
		temp->next = last;
		last->value = x;
		last->next = NULL;
	}
}

void clear(struct list_item *first) {
	struct list_item *temp = first->next;
	first->next = NULL;
	while (temp != NULL) {
		struct list_item *temp2 = temp->next;
		free(temp);
		temp = temp2;
	}
}

int main(int argc, char ** argv){
	struct list_item root;
	root.value = -1; /* This value is always ignored */
	root.next = NULL;
	print(&root);
	append(&root, 1);
	print(&root);
	prepend(&root, 4);
	print(&root);
	append(&root, 3);
	append(&root, 3);
	input_sorted(&root, 2);
	append(&root, 7);
	print(&root);
	clear(&root);
	clear(&root);
	print(&root);
	prepend(&root, 10);
	print(&root);
	input_sorted(&root, 20);
	print(&root);
	clear(&root);
}