#ifndef __ODYS_LIST__
#define __ODYS_LIST__
#include <string>
#include <cstring>
#include <iostream>
using namespace std;

typedef struct disease_list_node DiseaseListNode;
struct disease_list_node {
	// Date* date_ptr;
	string data;
	DiseaseListNode* Next;
};
int insert_to_list(DiseaseListNode*&, string);
int delete_list(DiseaseListNode*&);
int earlier_or_equal(char*, char*);

typedef struct date_list_node DateListNode;
struct date_list_node {
	char* data;
	DateListNode* Next;
};
void print_date_list(DateListNode*);
int date_list_insert(DateListNode*&, char*);
int delete_date_list(DateListNode*&);
#endif