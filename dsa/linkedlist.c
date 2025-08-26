#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

bool verify_index(int index,int n) {
    if(index >= n || index < 0) {
        printf("Index out of range\n");
        return false;
    }
    return true;
}

Node *create_node(int data) {
    Node *new_node = (Node*)malloc(sizeof(Node));
    if(new_node == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    new_node->data = data;
    new_node->next = NULL;
    return new_node;
}

Node* make_list(int n) {
    if(n <= 0) return NULL;

    printf("Enter %d elements:\n", n);

    Node* head = NULL;
    Node* curr = NULL;

    for(int i = 0;i < n;i++) {
        int input;
        if(!scanf("%d", &input)) {
            printf("Invalid input.\n");
            Node *temp;
            while(head!=NULL) {
                temp = head;
                head = head->next;
                free(temp);
            }
            return NULL;
        }

        Node *new_node = create_node(input);

        if(head == NULL) {
            head = new_node;
            curr = head;
        } else {
            curr->next = new_node;
            curr = new_node;
        }
    }

    return head;
}

void print_list(Node *head) {
    Node *curr = head;
    while(curr != NULL) {
        if(curr->next != NULL) {
            printf("%d -> ", curr->data);
        } else {
            printf("%d", curr->data);
        }
        
        curr = curr->next;
    }
    printf("\n");
}

Node* insert_node(Node *head, int n) {
    Node* dummy = create_node(0);
    dummy->next = head;
    Node* curr = dummy;

    int element_to_insert, position;
    printf("Enter the position and element to insert: \n");
    scanf("%d %d", &position, &element_to_insert);

    if(position > n || position < 0) {
        printf("position out of range\n");
        return head;
    }

    while(position > 0) {
        curr = curr->next;
        position--;
    }

    Node *new_node = create_node(element_to_insert);
    new_node->next = curr->next;
    curr->next = new_node;

    printf("\n");

    return dummy->next;
}

void delete_node(Node *head, int *n) {

    int position;
    printf("Enter the position to delete: \n");
    scanf("%d", &position);

    if(position >= *n || position < 0) {
        printf("position out of range\n");
        return;
    }

    while(position > 1) {
        head = head->next;
        position--;
    }

    Node *curr = head->next;
    head->next = curr->next;
    *n--;

    printf("\n");
}

void free_list(Node *head) {
    Node *temp;
    while(head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

int main(){
    int n;
    while(scanf("%d", &n)!=EOF) {
        Node *head = make_list(n);
        print_list(head);

        head = insert_node(head, n);
        print_list(head);

        delete_node(head, &n);
        print_list(head);

        free_list(head);
    }

    return 0;
}