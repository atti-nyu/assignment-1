#include "dictionary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <ctype.h>

node* create_node (char* word) {
    if (strlen (word) > LENGTH) {
        // printf ("Invalid word size\n");
        return NULL;
    }
    node *tmp = (node*) malloc (sizeof(node));
    strncpy (tmp->word, word, LENGTH);
    tmp->next = NULL;

    return tmp;
}

void remove_punct (char* word) { 
    while (word[0] && isalpha (word[0]) == 0) memmove (word, word + 1, strlen (word));
    while (word[0] && isalpha (word [strlen(word) - 1] ) == 0) word [strlen (word) - 1] = '\0';
    }

void make_lowercase (char* word) {
    for (int i = 0; word[i]; i++) {
        word[i] = tolower (word[i]);
    }
}

bool check_len(char* word) {
    if (strlen(word) == 0) return false;
    if (strlen(word) > LENGTH) return false;

    for(int i = 0; word[i]; i++) {   
        if ((unsigned int)word[i] > 1023) return false;  
    }
    return true;
}

bool check_word (const char* word, hashmap_t hashtable[]) {
    // lowering the string
    char* lower_word = strdup (word);
    if (!check_len (lower_word)) {
        free (lower_word);
        return false;
    }
    make_lowercase (lower_word);

    //search list for the word
    node* list_item = hashtable [hash_function (lower_word)];
    if (list_item == NULL) {
        free (lower_word);
        return false;
    }

    while (1) {
        if (!strncmp (list_item->word, lower_word, strlen (list_item->word))) {
            free (lower_word);
            return true;
        }
        if (list_item->next == NULL) break;
        list_item = list_item->next;
    }

    free (lower_word);
    return false;
}

bool load_dictionary (const char* dictionary_file, hashmap_t hashtable[]) {
    //Initialize all value in hash table to NULL
    for (int i = 0; i < HASH_SIZE; i++) hashtable[i] = NULL;

    FILE *fp = fopen (dictionary_file, "r");
    if (fp == NULL) {
          perror ("Failed to open file\n");
          exit (1);
    }
 
    char* word = NULL;
    size_t len = 0;

    while (getline (&word, &len, fp) != -1) {
        word [strcspn (word, "\n")] = 0;
        char* parse_word = strdup (word);
        remove_punct (parse_word);

        if (parse_word != NULL && check_len (parse_word)) {
            make_lowercase (parse_word);
            node* new_word = create_node (parse_word);

            if (hashtable [hash_function (parse_word)] != NULL) {
                node* last = hashtable [hash_function (parse_word)];
                while (last->next != NULL) {
                    last = last->next;
                }
                last->next = new_word;
            }
            else {
                hashtable [hash_function (parse_word)] = new_word;
            }
        }
        free (parse_word);
    }
    fclose (fp);
    free (word); 

    return true;
}

int check_words (FILE* fp, hashmap_t hashtable[], char* misspelled[]) {
    char *line = NULL;
    size_t len = 0;
    int mispelledCount = 0;
    char delim[] = " ";

    while (getline (&line, &len, fp) != -1) {
        line [strcspn(line, "\r\n")] = 0;
        char* word = strtok (line, delim); 
  
        while (word != NULL) { 
            char* check_this_word = strdup(word);

            if (strlen (check_this_word) > LENGTH) check_this_word[LENGTH] = '\0';
            remove_punct (check_this_word);

            if (strlen(check_this_word) && !check_word (check_this_word, hashtable)) {
        
                misspelled [mispelledCount++] = check_this_word;
                if (mispelledCount > MAX_MISSPELLED - 1) break;
            }
            else {
               free (check_this_word);
            }

            word = strtok (NULL, delim); 
        }   
        if (mispelledCount > MAX_MISSPELLED - 1) break;

    }
    free (line);

    return mispelledCount;
}