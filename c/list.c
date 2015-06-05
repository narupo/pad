/**
 * @file List.c
 * @author Yuta Aizawa
 * @date 29 5 2015
 * @brief List from CAP.
 */

// List::Include

#include <stdlib.h>

// List::Types

typedef struct ListNode ListNode;
typedef struct List List;
typedef int List_type;

// List::Structure

struct ListNode {
	List_type value;
	ListNode* prev;
	ListNode* next;
};

struct List {
	size_t length;
	ListNode* head;
	ListNode* tail;
};

// ListNode::New

ListNode* ListNode_New(List_type value)
{
	ListNode* self = (ListNode*) malloc(sizeof(ListNode));
	if (!self) {
		// Error::Malloc
		return NULL;
	}

	self->value = value;
	self->next = NULL;
	self->prev = NULL;

	return self;
}

// ListNode::Delete

void ListNode_Delete(ListNode* self)
{
	if (self) {
		free(self);
	}
}

// List::New

List* List_New(void)
{
	List* self = (List*) malloc(sizeof(List));
	if (!self) {
		// Error::Malloc
		return NULL;
	}

	self->length = 0;
	self->head = NULL;
	self->tail = NULL;

	return self;
}

// List::Delete

void List_Delete(List* self)
{
	if (self) {
		for (ListNode* curnode = self->head; curnode; ) {
			ListNode* delnode = curnode;
			curnode = curnode->next;
			ListNode_Delete(delnode);
		}
		free(self);
	}
}

// List::Getter

size_t List_Len(List const* self)
{
	return self->length;
}

List_type List_BackCopy(List const* self)
{
	if (!self->tail) {
		// Error::Null pointer
		return (List_type){0};
	}
	return self->tail->value;
}

List_type* List_BackPointer(List* self)
{
	if (!self->tail) {
		// Error::Null pointer
		return &(List_type){0};
	}
	return &self->tail->value;
}

List_type const* List_BackConstPointer(List const* self)
{
	if (!self->tail) {
		// Error::Null pointer
		return &(List_type){0};
	}
	return &self->tail->value;
}

List_type List_FrontCopy(List const* self)
{
	if (!self->head) {
		// Error::Null pointer
		return (List_type){0};
	}
	return self->head->value;
}

List_type* List_FrontPointer(List* self)
{
	if (!self->head) {
		// Error::Null pointer
		return &(List_type){0};
	}
	return &self->head->value;
}

List_type const* List_FrontConstPointer(List const* self)
{
	if (!self->head) {
		// Error::Null pointer
		return &(List_type){0};
	}
	return &self->head->value;
}

// List::Setter
 
void List_PushBackCopy(List* self, List_type value)
{
	ListNode* newtail = ListNode_New(value);
	if (!newtail) {
		// Error::ListNode_New
		return;
	}
	// Update length
	++self->length;

	if (!self->head) {
		self->head = self->tail = newtail;
		return; // Success
	}

	// Change links
	ListNode* oldtail = self->tail;

	newtail->prev = oldtail;
	oldtail->next = newtail;

	self->tail = newtail;
}

void List_PushFrontCopy(List* self, List_type value)
{
	ListNode* newhead = ListNode_New(value);
	if (!newhead) {
		// Error::ListNode_New
		return;
	}
	// Update length
	++self->length;

	if (!self->head) {
		self->head = self->tail = newhead;
		return; // Success
	}

	// Change links
	ListNode* oldhead = self->head;

	newhead->next = oldhead;
	oldhead->prev = newhead;

	self->head = newhead;
}

// List::Remove

List_type List_PopBack(List* self)
{
	List_type value = (List_type){0};

	if (!self->head) {
		// Error::Null pointer
		return value;
	}
	// Update length
	--self->length;

	ListNode* delnode = self->tail;

	if (self->head == self->tail) {
		self->head = self->tail = NULL;
	} else {
		self->tail = delnode->prev;
		self->tail->next = NULL;
		if (self->head == self->tail) {
			self->head->next = NULL;
		}
	}

	value = delnode->value;
	ListNode_Delete(delnode);

	return value;
}

List_type List_PopFront(List* self)
{
	List_type value = (List_type){0};

	if (!self->head) {
		// Error::Null pointer
		return value;
	}
	// Update length
	--self->length;

	ListNode* delnode = self->head;

	if (self->head == self->tail) {
		self->head = self->tail = NULL;
	} else {
		self->head = delnode->next;
		self->head->prev = NULL;
		if (self->head == self->tail) {
			self->tail->prev = NULL;
		}
	}

	value = delnode->value;
	ListNode_Delete(delnode);

	return value;
}

// List::Test

#include <stdio.h>

int main(void)
{
	List* intList = List_New();

	{
		int value;

		List_PushBackCopy(intList, 10);
		List_PushBackCopy(intList, 11);

		for (ListNode* curnode = intList->head; curnode; curnode = curnode->next) {
			printf("[%p][%p][%p]\n", curnode->prev, curnode, curnode->next);
		}
		
		value = List_PopBack(intList);
		fprintf(stderr, "%d\n", value);
		value = List_PopBack(intList);
		fprintf(stderr, "%d\n", value);
		
		List_PushFrontCopy(intList, 20);
		List_PushFrontCopy(intList, 21);
		
		value = List_PopFront(intList);
		fprintf(stderr, "%d\n", value);
		value = List_PopFront(intList);
		fprintf(stderr, "%d\n", value);

		List_Delete(intList);
		return 0;
	}


	for (int i = 1; i <= 4; ++i) {
		List_PushBackCopy(intList, i);
	}
	for (int i = -1; i >= -4; --i) {
		List_PushFrontCopy(intList, i);
	}
	// for (int i = 0; i < List_Len(intList); ++i) {
	// 	int value = List_BackCopy(intList);
	// 	printf("%d\n", value);
	// }
	// TODO: 
	for (ListNode* curnode = intList->head; curnode; curnode = curnode->next) {
		printf("%d\n", curnode->value);
	}

	size_t midlen;

	printf("\n");
	// midlen = List_Len(intList) / 2;
	// while (List_Len(intList) != midlen) {
	// 	int value = List_PopBack(intList);
	// 	printf("%d\n", value);
	// }

	// printf("\n");
	// midlen = List_Len(intList) / 2;
	// while (List_Len(intList) != midlen) {
	// 	int value = List_PopFront(intList);
	// 	printf("%d\n", value);
	// }

	List_Delete(intList);

	return 0;
}
