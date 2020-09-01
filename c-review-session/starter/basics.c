

// Print out all the elements of the array,
// each element on a newline
void print_array (int* arr) {
    while (*arr != NULL) {
        printf ("%d\n", *arr);
        arr += sizeof (int);
    }
}

int main () {
    // Array intended to consist of 1, 2, 0, 5
    int a[] = {1, 2, 0, 5, NULL};
    // Should print:
    // 1
    // 2
    // 0
    // 5
    print_array (a);
}
