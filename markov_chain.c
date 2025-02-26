#include "markov_chain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define constants
#define SUCCESS 0
#define FAILURE 1
#define INITIAL_FREQUENCY 1


/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number)
{
    return rand() % max_number;
}

Node* get_node_from_database(MarkovChain *markov_chain, void *data_ptr)
{
    if (markov_chain == NULL || data_ptr == NULL) {
        return NULL;
    }

    Node *current_node = markov_chain->database->first;

    while (current_node != NULL ) {
        if (markov_chain->comp_func(current_node->data->
                data, data_ptr) == 0) {
            return current_node;
        }
        current_node = current_node->next;
    }
    return NULL;
}

void free_frequency_lst(MarkovNode *markov_node)
{
    if(markov_node==NULL)
    {
        return;
    }
    free(markov_node->data);
    free(markov_node->frequency_list);
    markov_node->frequency_list=NULL;
    markov_node->frequency_list_length=0;
}
Node* add_to_database(MarkovChain *markov_chain, void *data_ptr)
{
    if (markov_chain == NULL || data_ptr == NULL) {
        return NULL;
    }
    Node *node_exist = get_node_from_database(markov_chain, data_ptr);
    if (node_exist != NULL) {
        return node_exist;
    }
    MarkovNode *markov_node = calloc(1, sizeof(MarkovNode));
    if (markov_node == NULL) {
        return NULL;
    }
    markov_node->data=markov_chain->copy_func(data_ptr);
    if(!markov_node->data)
    {
        free(markov_node);
        return NULL;
    }
    markov_node->frequency_list_length=0;
    if(add(markov_chain->database,
           markov_node)==1)
    {
        free(markov_node->data);
        free(markov_node);
        return NULL;
    }

    return markov_chain->database->last;

}

int add_node_to_frequency_list(MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain)
{
    if (first_node == NULL || second_node == NULL) {
        return FAILURE;
    }
    for (size_t i = 0; i < first_node->
            frequency_list_length; i++) {
        if (markov_chain->comp_func(first_node->frequency_list[i].
                markov_node->data,second_node->data)==0) {
            first_node->frequency_list[i].frequency++;
            return SUCCESS;
        }
    }
    MarkovNodeFrequency *new_frequency_list =
            realloc(first_node->frequency_list,
                    sizeof(MarkovNodeFrequency) *
                    (first_node->frequency_list_length + 1));
    if (new_frequency_list == NULL) {
        return FAILURE;
    }
    first_node->frequency_list = new_frequency_list;
    first_node->frequency_list[first_node->frequency_list_length]
            .markov_node = second_node;
    first_node->frequency_list[first_node->frequency_list_length]
            .frequency = INITIAL_FREQUENCY;
    first_node->frequency_list_length++;
    return SUCCESS;
}

void free_markov_chain(MarkovChain **chain_ptr)
{
    if(chain_ptr==NULL || (*chain_ptr)==NULL)
    {
        return;
    }
    if((*chain_ptr)->database==NULL)
    {
        free(*chain_ptr);
        (*chain_ptr)=NULL;
        return;
    }
    if((*chain_ptr)->database->first==NULL)
    {
        free((*chain_ptr)->database);
        free(*chain_ptr);
        (*chain_ptr)=NULL;
        return;
    }
    Node *temp_markov_chain=(*chain_ptr)->database->first;
    while (temp_markov_chain!=NULL)
    {
        Node *current_node=temp_markov_chain;
        temp_markov_chain=temp_markov_chain->next;
        if(current_node->data!=NULL)
        {
            free_frequency_lst
                    (current_node->data);
        }
        (*chain_ptr)->free_data(current_node->data);
        free(current_node);
    }
    free(((*chain_ptr)->database));
    free(*chain_ptr);
    (*chain_ptr)=NULL;
}

MarkovNode* get_first_random_node(MarkovChain *markov_chain)
{
    if(markov_chain==NULL || markov_chain->database==NULL
       || markov_chain->database->size==0)
    {
        return NULL;
    }
    int flag = 1;
    MarkovNode *ret_node = NULL;
    while (flag == 1) {
        int random_index = get_random_number
                (markov_chain->database->size);
        Node *current_node=markov_chain->database->first;
        for (int i = 0; i < random_index; i++) {
            current_node=current_node->next;
        }
        ret_node=current_node->data;
        if(!markov_chain->is_last(ret_node->data))
        {
            flag=0;
        }
    }
    return ret_node;
}

MarkovNode* get_next_random_node(MarkovNode *cur_markov_node)
{
    if (cur_markov_node == NULL || cur_markov_node->
            frequency_list_length == 0) {
        return NULL;
    }
    int total_frequency = 0;
    for (size_t i = 0; i < cur_markov_node->
            frequency_list_length; i++) {
        total_frequency += cur_markov_node->
                frequency_list[i].frequency;
    }
    if(total_frequency==0)
    {return NULL;}
    int random_index = get_random_number
            (total_frequency);
    for(size_t i=0;i<cur_markov_node->
            frequency_list_length;i++)
    {
        random_index -= cur_markov_node->
                frequency_list[i].frequency;

        if(random_index < 0)
        {
            return cur_markov_node->
                    frequency_list[i].markov_node;
        }
    }
    return NULL;
}

void generate_random_sequence(MarkovChain *markov_chain, MarkovNode *
first_node, int max_length)
{
    MarkovNode *current_node = first_node;
    markov_chain->print_func(current_node->data);
    printf(" ");
    int length = 1;
    while (length < max_length) {
        MarkovNode *next_node= get_next_random_node
                (current_node);
        markov_chain->print_func(next_node->data);
        printf(" ");
        length++;
        if (markov_chain->is_last(next_node->data )) {
            break;
        }
        current_node = next_node;
    }
    printf("\n");
}