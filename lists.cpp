#include "lists.h"

int insert_to_list(DiseaseListNode*& diseases_list_ptr, string new_disease) {
	if (diseases_list_ptr == NULL) {
		diseases_list_ptr = new DiseaseListNode;
		diseases_list_ptr->Next = NULL;
		diseases_list_ptr->data = new_disease;
	}
	else {
		if (diseases_list_ptr->data == new_disease)
			return 1;
		DiseaseListNode* temp_node = diseases_list_ptr;
		while (temp_node->Next != NULL) {
			temp_node = temp_node->Next;
			if (temp_node->data == new_disease)
				return 1;
		}
		temp_node->Next = new DiseaseListNode;
		temp_node->Next->Next = NULL;
		temp_node->Next->data = new_disease;
	}
	return 0;
}

int delete_list(DiseaseListNode*& diseases_list_ptr) {
	DiseaseListNode* current_node = diseases_list_ptr;
	DiseaseListNode* to_delete;
	while (current_node != NULL) {
		// current_node->date_ptr->print();
		// delete current_node->date_ptr;
		to_delete = current_node;
		current_node = current_node->Next;
		delete to_delete;
	}
	diseases_list_ptr = NULL;
	return 0;
}

int date_list_insert(DateListNode*& ListNode, char* new_date) {
	DateListNode* temp_node = ListNode;
	if (ListNode == NULL) {
		// cout << "1-return" << endl;
		ListNode = new DateListNode;
		ListNode->Next = NULL;
		ListNode->data = new char[15];
		strcpy(ListNode->data, new_date);
	}
	else if (earlier_or_equal(new_date, ListNode->data) == 1) {
		DateListNode* new_node = new DateListNode;
		new_node->data = new char[15];
		strcpy(new_node->data, new_date);
		ListNode = new_node;
		new_node->Next = temp_node;
	}
	else {
		// cout << "2" << endl;
		while (temp_node->Next != NULL) {
			// cout << "3" << endl;
			if (earlier_or_equal(new_date, temp_node->Next->data) == 1) {
				// cout << "4" << endl;
				break;
			}
			// cout << "5" << endl;
			temp_node = temp_node->Next;
		}
		// cout << "6" << endl;
		DateListNode* new_node = new DateListNode;
		new_node->data = new char[15];
		// cout << "7" << endl;
		new_node->Next = temp_node->Next;
		// cout << "8" << endl;
		strcpy(new_node->data, new_date);
		// cout << "9" << endl;
		temp_node->Next = new_node;

		// ListNode = new_node;
		// new_node->Next = temp_node;
	}
	return 0;
}

void print_date_list(DateListNode* ListNode) {
	// cout << "-----------------------------------------------" << endl;
	while (ListNode != NULL) {
		cout << ListNode->data << endl;
		ListNode = ListNode->Next;
	}
	// cout << "-----------------------------------------------" << endl;
}

int delete_date_list(DateListNode*& date_list_ptr) {
	DateListNode* current_node = date_list_ptr;
	DateListNode* to_delete;
	while (current_node != NULL) {
		// current_node->date_ptr->print();
		// delete current_node->date_ptr;
		to_delete = current_node;
		current_node = current_node->Next;
		delete[] to_delete->data;
		delete to_delete;
	}
	date_list_ptr = NULL;
	return 0;
}

int earlier_or_equal(char* date1, char* date2) {
	char date11[15];
	strcpy(date11, date1);
	char date22[15];
	strcpy(date22, date2);
	// cout << "1111111111" << endl;
	if (date11 == NULL || date22 == NULL) {
		// cout << "NULL" << endl;
		return -1;
	}
	// cout << date11 << endl;
	// cout << date22 << endl;
	// cout << "2222222222222" << endl;
	char* saveptr = date11;
	int day1 = atoi(strtok_r(saveptr, "-", &saveptr));
	int month1 = atoi(strtok_r(saveptr, "-", &saveptr));
	int year1 = atoi(strtok_r(saveptr, "-", &saveptr));

	saveptr = date22;
	int day2 = atoi(strtok_r(saveptr, "-", &saveptr));
	int month2 = atoi(strtok_r(saveptr, "-", &saveptr));
	int year2 = atoi(strtok_r(saveptr, "-", &saveptr));
	// cout << "333333333" << endl;

	if (year1 < year2)
		return 1;
	else if (year1 > year2)
		return 0;
	else if (month1 < month2)
		return 1;
	else if (month1 > month2)
		return 0;
	else if (day1 < day2)
		return 1;
	else if (day1 > day2)
		return 0;
	else
		return 1;
}