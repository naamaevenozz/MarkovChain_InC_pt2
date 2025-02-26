#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY -1
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60
#define BASE 10

#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20

#define NUM_ARGS_ERROR "Usage: invalid number of arguments"

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell {
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the
    // ladder in case there is one from this square
    int snake_to;  // snake_to represents the jump of
    // the snake in case there is one from this square
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;

MarkovChain  *create_markov_chain2(void(*print_func)(void *),
                                  int (*comp_func)(void *, void *),
                                  void(*free_data )(void *),
                                  void *(*copy_func)(void *),
                                  bool(*is_last )(void *)) {
    MarkovChain *markov_chain = malloc(sizeof(MarkovChain));
    if (markov_chain == NULL) {
        printf("%s\n", ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }
    markov_chain->database = malloc(sizeof(LinkedList));
    if (markov_chain->database == NULL) {
        printf("%s\n", ALLOCATION_ERROR_MASSAGE);
        free(markov_chain);
        return NULL;
    }
    markov_chain->database->first = NULL;
    markov_chain->database->last = NULL;
    markov_chain->database->size = 0;
    markov_chain->copy_func = copy_func;
    markov_chain->is_last = is_last;
    markov_chain->print_func = print_func;
    markov_chain->comp_func = comp_func;
    markov_chain->free_data = free_data;

    return markov_chain;
}
void print_func2(void *data) {
    Cell *cell = (Cell *)data;
    printf("[%d]", cell->number);
}

int comp_func2(void *data1, void *data2) {
    Cell *cell1 = (Cell *)data1;
    Cell *cell2 = (Cell *)data2;
    return cell1->number - cell2->number;
}

void free_data2(void *data) {
    free(data);
}

void *copy_func2(void *data) {
    Cell *cell = (Cell *)data;
    Cell *new_cell = malloc(sizeof(Cell));
    if (new_cell == NULL) {
        return NULL;
    }
    *new_cell = *cell;
    return new_cell;
}

bool is_last2(void *data) {
    Cell *cell = (Cell *)data;
    return cell->number == BOARD_SIZE;
}


/** Error handler **/
int handle_error_snakes(char *error_msg, MarkovChain **database)
{
    printf("%s", error_msg);
    if (database != NULL)
    {
        free_markov_chain(database);
    }
    return EXIT_FAILURE;
}


int create_board(Cell *cells[BOARD_SIZE])
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        cells[i] = malloc(sizeof(Cell));
        if (cells[i] == NULL)
        {
            for (int j = 0; j < i; j++) {
                free(cells[j]);
            }
            handle_error_snakes(ALLOCATION_ERROR_MASSAGE,NULL);
            return EXIT_FAILURE;
        }
        *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
    }

    for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
    {
        int from = transitions[i][0]; //current place
        int to = transitions[i][1]; //place to move to
        if (from < to)
        {
            cells[from - 1]->ladder_to = to;
        }
        else
        {
            cells[from - 1]->snake_to = to;
        }
    }
    return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int fill_database_snakes(MarkovChain *markov_chain)
{Cell* cells[BOARD_SIZE];
    if(create_board(cells) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;}
    MarkovNode *from_node = NULL, *to_node = NULL;
    size_t index_to;
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        add_to_database(markov_chain, cells[i]);
    }

    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        from_node = get_node_from_database(markov_chain,cells[i])->data;

        if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
        {
            index_to = MAX(cells[i]->snake_to,cells[i]->ladder_to) - 1;
            to_node = get_node_from_database(markov_chain,
                                             cells[index_to])->data;
            add_node_to_frequency_list(from_node,
                                       to_node, markov_chain);
        }
        else
        {
            for (int j = 1; j <= DICE_MAX; j++)
            {
                index_to = ((Cell*) (from_node->data))->number + j - 1;
                if (index_to >= BOARD_SIZE)
                {
                    break;}
                to_node = get_node_from_database(markov_chain,
                                                 cells[index_to])->data;
                int  res = add_node_to_frequency_list(from_node,
                                                      to_node, markov_chain);
                if(res==EXIT_FAILURE)
                {
                    return EXIT_FAILURE;}
            }
        }
    }
    // free temp arr
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        free(cells[i]);}
    return EXIT_SUCCESS;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[])
{if(argc!=3)
    {
        printf("%s\n",NUM_ARGS_ERROR);
        return EXIT_FAILURE;}
    const unsigned int seed = (unsigned int) strtol
            (argv[1], NULL, BASE);
    const int num_paths= atoi(argv[2]);
    if(num_paths<=0)
    {
        printf("%s\n",NUM_ARGS_ERROR);
        return EXIT_FAILURE;}
    MarkovChain *markov_chain= create_markov_chain2(print_func2,
                                                   comp_func2,free_data2,
                                                   copy_func2,is_last2);
    if(markov_chain==NULL)
    {
        return EXIT_FAILURE;}
    if(fill_database_snakes(markov_chain)==EXIT_FAILURE)
    {
        free_markov_chain(&markov_chain);
        return EXIT_FAILURE;}
    srand(seed);
    for(int i=0;i<num_paths;i++)
    {
        printf("Random Walk %d: ",i+1);
        MarkovNode *current=markov_chain->database->first->data;
        for (int j = 0; j < MAX_GENERATION_LENGTH; j++) {
            if (current == NULL) {
                break;}
            Cell *cell = (Cell *)current->data;
            markov_chain->print_func(cell);
            if (cell->number == BOARD_SIZE) {
                break;
            }
            if (cell->ladder_to != EMPTY) {
                printf(" -ladder to-> ");
            } else if (cell->snake_to != EMPTY) {
                printf(" -snake to-> ");
            } else {
                printf(" -> ");
            }
            current = get_next_random_node(current);
        }
        printf("\n");
    }
    free_markov_chain(&markov_chain);
    return EXIT_SUCCESS;
}
