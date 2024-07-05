#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "prime.h"

static ht_item HT_DELETED_ITEM = {NULL, NULL};

static ht_item* ht_new_item(const char* k, const char* v) {
    ht_item* i = malloc(sizeof(ht_item));
    i -> key = strdup(k);
    i -> value = strdup(v);
    return i;
}

static ht_hash_table* ht_new_sized(const int base_size) {
    ht_hash_table* ht = malloc(sizeof(ht_hash_table));
    ht -> base_size = base_size;
    ht -> size = next_prime(ht -> base_size);
    ht -> count = 0; 
    ht -> items = calloc((size_t)ht -> size, sizeof(ht_item*));
    return ht;
}

ht_hash_table* ht_new() {
    return ht_new_sized(HT_INITIAL_BASE_SIZE);
}

static void ht_del_item(ht_item* i) {
    free(i -> key);
    free(i -> value);
    free(i);
}

void ht_del_hash_table(ht_hash_table* ht) {
    for (int i = 0; i < ht -> size; i ++) {
        ht_item* item = ht -> items[i];
        if (item != NULL) {
            ht_del_item(item);
        }
    }
    free(ht -> items);
    free(ht);
}

static int ht_hash(const char* s, const int a, const int m) {
    long hash = 0; 
    const int len_s = strlen(s);

    for (int i = 0; i < len_s; i ++) {
        // hash of the current character
        hash += (long)pow(a, len_s - (i + 1)) * s[i];
        // take modulus m to keep the hash value within the range [0, m - 1]
        hash = hash % m;
    }
    return (int)hash;
}

// computes the hash for a given key using double hashing 
// parameters: input string key, total number of buckets in the hash table, the number of collision attempts
static int ht_get_hash(const char* s, const int num_buckets, const int attempt) {
    const int hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
    const int hash_b = ht_hash(s, HT_PRIME_2, num_buckets);
    // addition of (hash_b + 1) ensures that hash_b is never zero, preventing infinite loops
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

void ht_insert(ht_hash_table* ht, const char* key, const char* value) {
    const int load = ht -> count * 100 / ht -> size; 
    if (load > 70) {
        ht_resize_up(ht);
    }
    ht_item* item = ht_new_item(key, value); // Create a new item with the given key and value
    int index = ht_get_hash(item->key, ht->size, 0); // Compute the initial index using the primary hash function
    ht_item* cur_item = ht->items[index]; // Get the current item at the computed index
    int i = 1; // Initialize the attempt counter for double hashing

    // If the computed index is not empty or marked as deleted, we have a collision, so use double hashing
    while (cur_item != NULL && cur_item != &HT_DELETED_ITEM) {
        // Check if the current item is the same key we are trying to insert
        if (cur_item != &HT_DELETED_ITEM) {
            if (strcmp(cur_item->key, key) == 0) {
                ht_del_item(cur_item); // Delete the old item
                ht->items[index] = item; // Insert the new item
                return; // Exit the function
            }
        }
        // Compute a new index using the secondary hash function and attempt counter
        index = ht_get_hash(item->key, ht->size, i);
        cur_item = ht->items[index];
        i++;
    }

    // Place the new item at the computed (or re-computed) index
    ht->items[index] = item;

    // Increment the count of items in the hash table
    ht->count++;
}


char* ht_search(ht_hash_table* ht, const char* key) {
    int index = ht_get_hash(key, ht -> size, 0);
    ht_item* item = ht -> items[index];
    int i = 1; 
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM) {
            if (strcmp(item -> key, key) == 0) {
                return item -> value;
            }
        }
        index = ht_get_hash(key, ht -> size, i);
        item = ht -> items[index];
        i ++;
    }
    return NULL;
}

void ht_delete(ht_hash_table* ht, const char* key) {
    const int load = ht -> count * 100 / ht -> size; 
    if (load < 10) {
        ht_resize_down(ht);
    }
    // compute initial index for key and get item at that index 
    // counter for double hashing 
    int index = ht_get_hash(key, ht -> size, 0);
    ht_item* item = ht -> items[index];
    int i = 1; 

    while (item != NULL) {
        if (item != &HT_DELETED_ITEM) {
            // check if this is item to delete
            if (strcmp(item -> key, key) == 0) {
                ht_del_item(item);
                ht -> items[index] = &HT_DELETED_ITEM;
            }
        }
        // double hashing 
        index = ht_get_hash(key, ht -> size, i);
        item = ht -> items[index];
        i ++;
    }
    ht -> count--;
}

static void ht_resize(ht_hash_table* ht, const int base_size) {
    // Prevent resizing to a size smaller than the initial base size
    if (base_size < HT_INITIAL_BASE_SIZE) {
        return;
    }

    // Create a new hash table with the new base size
    ht_hash_table* new_ht = ht_new_sized(base_size);

    // Rehash all items from the old table to the new table
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_insert(new_ht, item->key, item->value);
        }
    }

    // Update the old hash table's properties to match the new hash table
    ht->base_size = new_ht->base_size;
    ht->count = new_ht->count;

    // Swap sizes
    const int tmp_size = ht->size;
    ht->size = new_ht->size;
    new_ht->size = tmp_size;

    // Swap items
    ht_item** tmp_items = ht->items;
    ht->items = new_ht->items;
    new_ht->items = tmp_items;

    // Delete the temporary new hash table structure (which now contains the old data)
    ht_del_hash_table(new_ht);
}

static void ht_resize_up(ht_hash_table* ht) {
    const int new_size = ht->base_size * 2;
    ht_resize(ht, new_size);
}


static void ht_resize_down(ht_hash_table* ht) {
    const int new_size = ht->base_size / 2;
    ht_resize(ht, new_size);
}