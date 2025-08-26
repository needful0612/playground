#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//all index and input is 0 based
bool verify_index(int index,int n) {
    if(index >= n || index < 0) {
        printf("Index out of range\n");
        return false;
    }
    return true;
}

void merge(int *arr, int *temp_arr,int l, int m, int r) {
    int i = l, j = m + 1,k = l;

    while(i <= m && j <= r) {
        if(arr[i] <= arr[j]) {
            temp_arr[k++] = arr[i++];
        } else {
            temp_arr[k++] = arr[j++];
        }
    }

    while(i <= m) temp_arr[k++] = arr[i++];
    while(j <= r) temp_arr[k++] = arr[j++];

    for(int idx = l;idx <= r;idx++) arr[idx] = temp_arr[idx];
}

void merge_sort(int *arr, int *temp_arr, int l, int r) {
    if(l < r) {
        int m = l + (r - l) / 2;

        merge_sort(arr,temp_arr,l,m);
        merge_sort(arr,temp_arr,m+1,r);

        merge(arr,temp_arr,l,m,r);
    }
}

int *read_array(int n) {
    int *arr = (int *)malloc(n * sizeof(int));

    if (arr == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }

    printf("Enter %d elements:\n", n);
    for (int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
    }
    return arr;
}

void traverse_array(const int *arr, int n) {
    if(arr == NULL) return;
    for(int i = 0;i < n;i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void search_array(const int *arr, int n) {
    if(arr == NULL) return;
    
    int element;
    bool found = false;
    printf("Enter the element to search for:\n");
    scanf("%d", &element);

    for(int i = 0;i < n;i++) {
        if(arr[i] == element) {
            found = true;
            printf("Found at position %d\n", i);
            break;
        }
    }

    if(!found) printf("Element not found!\n");
}

int *insert_element(int *arr, int *n) {
    if(arr == NULL) return arr;

    int element_to_insert, index;
    printf("Enter the index and element to insert: \n");
    scanf("%d %d", &index, &element_to_insert);

    if(index > *n || index < 0) {
        printf("Index out of range\n");
        return arr;
    }

    int *new_arr = (int *)realloc(arr, (*n + 1) * sizeof(int));
    if(new_arr == NULL) {
        printf("Memory reallocation failed!\n");
        return arr;
    }

    for(int i = *n;i > index;i--) new_arr[i] = new_arr[i-1];
    new_arr[index] = element_to_insert;
    *n += 1;
    return new_arr;
}

int *remove_element(int *arr, int *n) {
    if(arr == NULL || *n <= 0) return arr;

    int index;
    printf("Enter the index to delete: \n");
    scanf("%d", &index);

    if(!verify_index(index, *n)) return arr;
    
    for(int i = index;i < *n - 1;i++) arr[i] = arr[i+1];

    int *new_arr = (int *)realloc(arr, (*n - 1) * sizeof(int));
    if(new_arr == NULL) {
        printf("Memory reallocation failed!\n");
        return arr;
    }
    *n -= 1;
    return new_arr;
}

void sort_array(int *arr, int n) {
    int *temp_arr = (int *)malloc(n * sizeof(int));
    if (temp_arr == NULL) {
        printf("Memory allocation failed for temporary array!\n");
        return;
    }

    merge_sort(arr, temp_arr, 0, n-1);
    free(temp_arr);
}

void modify_element(int *arr, int n) {
    if(arr == NULL) return;

    int element_to_modify, index;
    printf("Enter the index and the element to modify: \n");
    scanf("%d %d", &index, &element_to_modify);

    if(!verify_index(index, n)) return;

    arr[index] = element_to_modify;
}

int main(){
    int n;
    while(scanf("%d", &n)!=EOF) {
        int *arr = read_array(n);
        if(arr == NULL) continue;

        search_array(arr, n);

        arr = insert_element(arr, &n);
        traverse_array(arr, n);

        arr = remove_element(arr, &n);
        traverse_array(arr, n);

        sort_array(arr, n);
        traverse_array(arr, n);

        modify_element(arr, n);
        traverse_array(arr, n);

        free(arr);
    }

    return 0;
}