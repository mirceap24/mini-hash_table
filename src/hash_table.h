// single key-value pair
typedef struct {
    char* key; 
    char* value; 
} ht_item; 

typedef struct {
    int size; // total number of buckets
    int count; // number of items currently stored
    ht_item** items; // pointer to an array of pointers to `ht_item` structures
} ht_hash_table;



