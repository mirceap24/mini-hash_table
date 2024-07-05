#define HT_PRIME_1 151
#define HT_PRIME_2 163


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

void ht_insert(ht_hash_table* ht, const char* key, const char* value);
char* ht_Search(ht_hash_table* ht, const char* key);
void ht_delete(ht_hash_table* h, const char* key);





