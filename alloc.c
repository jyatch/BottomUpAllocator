#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_VR 256
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct{
	unsigned length;
	unsigned capacity;
	char **data;
} array_t;

struct node {
    char *operation_name;
    char *input1;
    char *input2;
    char *output;
    int num;
};

struct node2 {
    int upper_bound;
    int lower_bound;
    int next_use;
    int max;
    int check;
};

struct node3 {
    int min;
    int index;
    char *name;
};

void al_init(array_t *L, unsigned cap){
	L->length = 0;
	L->capacity = cap;
	L->data = malloc((sizeof(char*)) * cap);
}

void al_destroy(array_t *L){
	for(unsigned i = 0; i < L->length; i++){
		free(L->data[i]);
	}
		free(L->data);
}

void al_append(array_t *L, char* item){
	if(L->length == L->capacity){
		L->capacity *= 2;
		//printf("doubling capacity to %d\n", L->capacity);
		L->data = realloc(L->data, L->capacity * sizeof(char *));
	}
	L->data[L->length] = strdup(item);
	L->length++;
}

void al_expand(array_t *L) {
	L->capacity *= 2;
	L->data = realloc(L->data, L->capacity * sizeof(char *));
}

void al_insert(array_t *L, unsigned index, char *item){
	if (L->length == L->capacity){
		L->capacity *= 2;
		L->data = realloc(L->data, L->capacity * sizeof(char *));
	}

	for(unsigned i = L->length; i > index; i--) {
		L->data[i] = L->data[i-1];
	}
	L->data[index] = item;
	L->length++;
}

void al_remove(array_t *L, unsigned index) {
	for(unsigned i = index; i < L->length - 1; i++){
		L->data[i] = L->data[i+1];
	}
	L->length--;
}

void al_removelast(array_t *L) {
	free(L->data[L->length - 1]);
	L->length--;
}

int string_to_int(char *name) {
    //converts a string to an integer (specifically the format r31 -> 31)
    int length = strlen(name);
    char *number = malloc(length);

    memmove(number, name + 1, length - 1);
    *(number + length - 1) = '\0';

    int num = atoi(number);
    free(number);
    return num;
}

int min3(int a, int b, int c) {
    return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

void range_initializer(struct node2 *rangeArray) {
    struct node2 *current = rangeArray;

    // initializes the fields of the structs with -1
    for(int i = 0; i <= MAX_VR; i++) {
        current->upper_bound = -1;
        current->lower_bound = -1;
        current->next_use = -1;
        current++;
    }
}

// determines if the given string matches any of the input1 options
int compare_input1(int start, char *str, struct node *structArray, array_t *ILOCcode) {
    struct node *current = structArray + start + 1;
    int count = 0;

    for(int i = start + 1; i < ILOCcode->length; i++) {
        if(strcmp(current->input1, str) == 0) {
            return count + 1;
        }
        current++;
        count++;
    }
    return -1;
}

int compare_input2(int start, char *str, struct node *structArray, array_t *ILOCcode) {
    struct node *current = structArray + start + 1;
    int count = 0;

    for(int i = start + 1; i < ILOCcode->length; i++) {
        if(current->input2 != NULL) {
            if(strcmp(current->input2, str) == 0) {
                return count + 1;
            }
        }
        current++;
        count++;
    }
    return -1;
}

int compare_output(int start, char *str, struct node *structArray, array_t *ILOCcode) {
    struct node *current = structArray + start + 1;
    int count = 0;

    for(int i = start + 1; i < ILOCcode->length; i++) {
        if(current->output != NULL) {
            if(strcmp(current->output, str) == 0) {
                return count + 1;
            }
        }
        current++;
        count++;
    }
    return -1;
}

void dead_checker(int k, int line, struct node2 *rangeArray, struct node *structArray, array_t *ILOCcode, array_t *k_registers, array_t *spillArray, array_t *spillCount, array_t *finalArray) {
    // determine what to do with overflow
    int check = 0;
    struct node3 temp;
    temp.min = -2;

    //STEP 1: check if we have dead registers in the main set

    for(int i = 0; i < k; i++) {
        int num = string_to_int(k_registers->data[i]);
        if((rangeArray + num)->upper_bound <= line) {
            char *old = k_registers->data[i];
            char *new = malloc(strlen(k_registers->data[k]) + 1);
            strcpy(new, k_registers->data[k]);
            k_registers->data[i] = new;
            free(k_registers->data[k]);
            al_remove(k_registers, k);
            free(old);
            check = 1;
            break;
        }
    }

    // STEP 2: If we have no dead registers, pick a register to spill instead
    if(check == 0) {
        for(int i = 0; i < k_registers->length; i++) {
            int input1 = compare_input1(line, k_registers->data[i], structArray, ILOCcode);
            int input2 = compare_input2(line, k_registers->data[i], structArray, ILOCcode);
            int output = compare_output(line, k_registers->data[i], structArray, ILOCcode);

            int min = min3(input1, input2, output);

            if(min > temp.min) {
                temp.min = min;
                temp.index = i;
                temp.name = k_registers->data[i];
            }
        }
        printf("SPILLING %s\n", temp.name);
        al_append(spillArray, temp.name);
        char new[20];
        sprintf(new, "%d", line);
        al_append(spillCount, new);
        
        if(temp.index == k) {
            free(k_registers->data[k]);
            al_remove(k_registers, k);
        } else {
            char *old = k_registers->data[temp.index];
            char *new = malloc(strlen(k_registers->data[k]) + 1);
            strcpy(new, k_registers->data[k]);
            k_registers->data[temp.index] = new;
            free(k_registers->data[k]);
            al_remove(k_registers, k);
            free(old);
        }
    }
    check = 0;
}

// returns 0 if the string is in spillArray
int spill_check(char *str, array_t *spillArray) {
    for(int i = 0; i < spillArray->length; i++) {
        if(str != NULL) {
            if(strcmp(str, spillArray->data[i]) == 0) {
                return 0;
            }
        }
    }
    return 1;
}

void store_spill(int k, int index, int line, int l, array_t *ILOCcode, array_t *ultimateArray) {
    char *ultimate_str = malloc(strlen(ILOCcode->data[line]) * 2);

    char *loadI = "loadI";
    memmove(ultimate_str, loadI, strlen(loadI) + 1);
    int temp_length = strlen(ultimate_str);

    *(ultimate_str + temp_length) = ' ';

    index = (index + 3) * 4;
    char new[20];
    sprintf(new, "%d", index);
    memmove(ultimate_str + temp_length + 1, new, strlen(new) + 1);
    temp_length = strlen(ultimate_str);

    *(ultimate_str + temp_length) = ' ';
    *(ultimate_str + temp_length + 1) = '=';
    *(ultimate_str + temp_length + 2) = '>';
    *(ultimate_str + temp_length + 3) = '\0';
    temp_length = strlen(ultimate_str);

    sprintf(new, "%d", k);
    *(ultimate_str + temp_length) = ' ';
    *(ultimate_str + temp_length + 1) = 'r';
    memmove(ultimate_str + temp_length + 2, new, strlen(new) + 1);
    //printf("LINE: %d\n", line + 1);
    //printf("%s\n", ultimate_str);
    al_append(ultimateArray, ultimate_str);

    char *store = "store";
    memmove(ultimate_str, store, strlen(store) + 1);
    temp_length = strlen(ultimate_str);

    *(ultimate_str + temp_length) = ' ';
    *(ultimate_str + temp_length + 1) = 'r';
    sprintf(new, "%d", l);
    memmove(ultimate_str + temp_length + 2, new, strlen(new) + 1);
    temp_length = strlen(ultimate_str);

    *(ultimate_str + temp_length) = ' ';
    *(ultimate_str + temp_length + 1) = '=';
    *(ultimate_str + temp_length + 2) = '>';
    *(ultimate_str + temp_length + 3) = '\0';
    temp_length = strlen(ultimate_str);

    sprintf(new, "%d", k);
    *(ultimate_str + temp_length) = ' ';
    *(ultimate_str + temp_length + 1) = 'r';
    memmove(ultimate_str + temp_length + 2, new, strlen(new) + 1);
    //printf("%s\n", ultimate_str);
    al_append(ultimateArray, ultimate_str);

    free(ultimate_str);
}

int single_use_check(int line, int k, int index, struct node *current, array_t *finalArray) {
    if(current->input1 != NULL) {
        if(strcmp(current->input1, finalArray->data[((line - 1) * k) + index]) == 0) {
            return 1;
        }
    }
    if(current->input2 != NULL) {
        if(strcmp(current->input2, finalArray->data[((line - 1) * k) + index]) == 0) {
            return 1;
        }
    }
    if(current->output != NULL) {
        if(strcmp(current->output, finalArray->data[((line) * k) + index]) == 0) {
            return 1;
        }
    }
    return 0;
}

void top_half_gen(int reg_num, int line, int marker, int k, int memory_location, int spill_index, struct node *current, array_t *ILOCcode, array_t *spillArray, array_t *finalArray, array_t *ultimateArray) {
    // when we get to the generator function, we should already have are register number lined up
    // before we generate anything, we will call a live check function and determine if we need to add 1
    // one more time before moving on. We can do the same in bottom half as well

    if(single_use_check(line, k, reg_num, current, finalArray) == 1) {
        reg_num++;
    }
    
    
    char *ultimate_str = malloc(strlen(ILOCcode->data[line]) * 2);
    
    sprintf(ultimate_str, "loadI %d => r%d", memory_location, k);
    al_append(ultimateArray, ultimate_str);

    sprintf(ultimate_str, "store r%d => r%d", reg_num, k);
    al_append(ultimateArray, ultimate_str);

    spill_index = (spill_index + 3) * 4;
    sprintf(ultimate_str, "loadI %d => r%d", spill_index, k);
    al_append(ultimateArray, ultimate_str);

    sprintf(ultimate_str, "load r%d => r%d", k, reg_num);
    al_append(ultimateArray, ultimate_str);

    free(ultimate_str);
}

void bottom_half_gen(int reg_num, int line, int marker, int k, int memory_location, int spill_index, struct node *current, array_t *ILOCcode, array_t *spillArray, array_t *finalArray, array_t *ultimateArray) {
    if(single_use_check(line, k, reg_num, current, finalArray) == 1) {
        reg_num++;
    }
    
    
    char *ultimate_str = malloc(strlen(ILOCcode->data[line]) * 2);

    sprintf(ultimate_str, "loadI %d => r%d", memory_location, k);
    al_append(ultimateArray, ultimate_str);

    sprintf(ultimate_str, "load r%d => r%d", k, reg_num);
    al_append(ultimateArray, ultimate_str);

    free(ultimate_str);
}

int top_half(int start, int line, int marker, int k, int memory_location, int spill_index, struct node *current, array_t *ILOCcode, array_t *spillArray, array_t *finalArray, array_t *ultimateArray) {
    int temp1 = -1;
    int temp2 = -1;
    
    if(marker == 0) {
        for(int i = 0; i < k; i++) {
            if(current->input2 != NULL) {
                if(strcmp(current->input2, finalArray->data[((line - 1) * k) + i]) == 0) {
                    temp1 = i;
                }
            }
            if(current->output != NULL) {
                if(strcmp(current->output, finalArray->data[((line) * k) + i]) == 0) {
                    temp2 = i;
                }
            }
        }
    } else if(marker == 1) {
        for(int i = 0; i < k; i++) {
            if(current->input1 != NULL) {
                if(strcmp(current->input1, finalArray->data[((line - 1) * k) + i]) == 0) {
                    temp1 = i;
                }
            }
            if(current->output != NULL) {
                if(strcmp(current->output, finalArray->data[((line) * k) + i]) == 0) {
                    temp2 = i;
                }
            }
        }
    } else if(marker == 2) {
        for(int i = 0; i < k; i++) {
            if(current->input1 != NULL) {
                if(strcmp(current->input1, finalArray->data[((line - 1) * k) + i]) == 0) {
                    temp1 = i;
                }
            }
            if(current->input2 != NULL) {
                if(strcmp(current->input2, finalArray->data[((line - 1) * k) + i]) == 0) {
                    temp2 = i;
                }
            }
        }
    }

    // this returns the register number that we will be temporarily replacing
    if(temp1 != 0 && temp2 != 0) {
        top_half_gen(0 + start, line, marker, k, memory_location, spill_index, current, ILOCcode, spillArray, finalArray, ultimateArray);
        return 0 + start;
    }
    if(temp1 != 1 && temp2 != 1) {
        top_half_gen(1 + start, line, marker, k, memory_location, spill_index, current, ILOCcode, spillArray, finalArray, ultimateArray);
        return 1 + start;
    }
    if(temp1 != 2 && temp2 != 2) {
        top_half_gen(2 + start, line, marker, k, memory_location, spill_index, current, ILOCcode, spillArray, finalArray, ultimateArray);
        return 2 + start;
    }
    return 0;
}

// code for rewriting a normal instruciton without any spills. assigns read from previous assignment and writes from current assignment
void normal_instruction(int global_input1, int global_input2, int global_output, int k, int line, struct node *current, array_t *ILOCcode, array_t *finalArray, array_t *ultimateArray) {
    char *ultimate_str = malloc(strlen(ILOCcode->data[line]) * 2);

    // append the operation name to the string
    memmove(ultimate_str, current->operation_name, strlen(current->operation_name));
    *(ultimate_str + strlen(current->operation_name)) = '\0';

    // append input1 to the string
    if(global_input1 == -1) {
        if(*(current->input1) == 'r') {
            for(int i = 0; i < k; i++) {
                if(strcmp(current->input1, finalArray->data[((line - 1) * k) + i]) == 0) {
                    *(ultimate_str + strlen(current->operation_name)) = ' ';
                    *(ultimate_str + strlen(current->operation_name) + 1) = 'r';
                    char str[20];
                    sprintf(str, "%d", i);
                    memmove(ultimate_str + strlen(current->operation_name) + 2, str, strlen(str) + 1);
                }
            }

        } else if(*(current->input1) != 'h') {
            *(ultimate_str + strlen(current->operation_name)) = ' ';
            memmove(ultimate_str + strlen(current->operation_name) + 1, current->input1, strlen(current->input1) + 1);
        }
    } else {
        *(ultimate_str + strlen(current->operation_name)) = ' ';
        *(ultimate_str + strlen(current->operation_name) + 1) = 'r';
        char str[20];
        if(single_use_check(line, k, global_input1, current, finalArray) == 1) {
            global_input1++;
        }
        sprintf(str, "%d", global_input1);
        memmove(ultimate_str + strlen(current->operation_name) + 2, str, strlen(str) + 1);
    }
    int temp_length = strlen(ultimate_str);

    // append input2 to the string
    if(global_input2 == - 1) {
        if(current->input2 != NULL) {
            if(*(current->input2) == 'r') {
                for(int i = 0; i < k; i++) {
                    if(strcmp(current->input2, finalArray->data[((line - 1) * k) + i]) == 0) {
                        *(ultimate_str + temp_length) = ',';
                        *(ultimate_str + temp_length + 1) = ' ';
                        *(ultimate_str + temp_length + 2) = 'r';
                        char str[20];
                        sprintf(str, "%d", i);
                        memmove(ultimate_str + temp_length + 3, str, strlen(str) + 1);
                    }
                }

            } else if(*(current->input2) != 'h') {
                *(ultimate_str + temp_length) = ' ';
                memmove(ultimate_str + temp_length + 1, current->input2, strlen(current->input2) + 1);
            }
        }
    } else {
        *(ultimate_str + temp_length) = ',';
        *(ultimate_str + temp_length + 1) = ' ';
        *(ultimate_str + temp_length + 2) = 'r';
        char str[20];
        if(single_use_check(line, k, global_input2, current, finalArray) == 1) {
            global_input2++;
        }
        sprintf(str, "%d", global_input2);
        memmove(ultimate_str + temp_length + 3, str, strlen(str) + 1);
    }

    // append the arrow symbol
    temp_length = strlen(ultimate_str);
    if(current->output != NULL) {
        *(ultimate_str + temp_length) = ' ';
        *(ultimate_str + temp_length + 1) = '=';
        *(ultimate_str + temp_length + 2) = '>';
        *(ultimate_str + temp_length + 3) = '\0';
    }
    temp_length = strlen(ultimate_str);

    // append the output to the string
    if(global_output == -1) {
        if(current->output != NULL) {
            if(*(current->output) == 'r') {
                for(int i = 0; i < k; i++) {
                    if(strcmp(current->output, finalArray->data[(line * k) + i]) == 0) {
                        *(ultimate_str + temp_length) = ' ';
                        *(ultimate_str + temp_length + 1) = 'r';
                        char str[20];
                        sprintf(str, "%d", i);
                        memmove(ultimate_str + temp_length + 2, str, strlen(str) + 1);
                    }
                }
            } else if(*(current->output) != 'h') {
                *(ultimate_str + temp_length) = ' ';
                memmove(ultimate_str + temp_length + 1, current->output, strlen(current->output) + 1);
            }   
        }
    } else {
        *(ultimate_str + temp_length) = ' ';
        *(ultimate_str + temp_length + 1) = 'r';
        char str[20];
        if(single_use_check(line, k, global_output, current, finalArray) == 1) {
            global_output++;
        }
        sprintf(str, "%d", global_output);
        memmove(ultimate_str + temp_length + 2, str, strlen(str) + 1);
    }
    //printf("LINE %d\n", line);
    //printf("%s\n", ultimate_str);
    al_append(ultimateArray, ultimate_str);
    free(ultimate_str);
}



void code_generator(int k, struct node *structArray, struct node2* rangeArray, array_t *ILOCcode, array_t *k_registers, array_t *spillArray, array_t *spillCount, array_t *finalArray, array_t *ultimateArray) {
    struct node *current = structArray;
    struct node2 *current2 = rangeArray + 1;

    // initializes the check field of rangeArray
    for(int i = 1; i <= rangeArray->max; i++) {
        rangeArray->check = 0;
        current2++;
    }

    // for every instruction
    for(int i = 0; i < ILOCcode->length; i++) {
        int global_input1 = -1;
        int global_input2 = -1;
        int global_output = -1;
        int start = 0;
        
        // check for any new spills
        for(int j = 0; j < spillArray->length; j++) {
            int new = atoi(spillCount->data[j]);

            if(new == i) {
                for(int l = 0; l < k; l++) {
                    if(strcmp(finalArray->data[((i - 1) * k) + l], spillArray->data[j]) == 0) {
                        store_spill(k, j, i, l, ILOCcode, ultimateArray);
                    }
                }
            }
        }

        // check if any of the registers need to be loaded
        int memory_location = 0;

        for(int j = 0; j < spillArray->length; j++) {
            int new = atoi(spillCount->data[j]);

            if(new < i) {
                if(current->input1 != NULL) {
                    if(strcmp(current->input1, spillArray->data[j]) == 0) {
                        if(global_input2 != -1 || global_output != -1) {
                            start = MAX(global_input2, global_output) + 1;
                        }
                        global_input1 = top_half(start, i, 0, k, memory_location, j, current, ILOCcode, spillArray, finalArray, ultimateArray);
                        memory_location = memory_location + 4;
                        printf("INPUT 1 ON LINE %d\n", i);
                    }
                }
                if(current->input2 != NULL) {
                    if(strcmp(current->input2, spillArray->data[j]) == 0) {
                        if(global_input1 != -1 || global_output != -1) {
                            start = MAX(global_input1, global_output) + 1;
                            printf("%d STARING FROM HERE!!!!\n", start);
                        }
                        global_input2 = top_half(start, i, 1, k, memory_location, j, current, ILOCcode, spillArray, finalArray, ultimateArray);
                        memory_location = memory_location + 4;
                        printf("INPUT 2 ON LINE %d\n", i);
                    }
                }
                if(current->output != NULL) {
                    if(strcmp(current->output, spillArray->data[j]) == 0) {
                        if(global_input1 != -1 || global_input2 != -1) {
                            start = MAX(global_input1, global_input2) + 1;
                        }
                        global_output = top_half(start, i, 2, k, memory_location, j, current, ILOCcode, spillArray, finalArray, ultimateArray);
                        memory_location = memory_location + 4;
                        printf("OUTPUT ON LINE %d\n", i);
                    }
                }
            }
        }

        normal_instruction(global_input1, global_input2, global_output, k, i, current, ILOCcode, finalArray, ultimateArray);

        memory_location = 0;

        for(int j = 0; j < spillArray->length; j++) {
            int new = atoi(spillCount->data[j]);

            if(new < i) {
                if(current->input1 != NULL) {
                    if(strcmp(current->input1, spillArray->data[j]) == 0) {
                        bottom_half_gen(global_input1, i, 0, k, memory_location, j, current, ILOCcode, spillArray, finalArray, ultimateArray);
                        memory_location = memory_location + 4;
                        printf("INPUT 1 ON LINE %d\n", i);
                    }
                }
                if(current->input2 != NULL) {
                    if(strcmp(current->input2, spillArray->data[j]) == 0) {
                        bottom_half_gen(global_input2, i, 1, k, memory_location, j, current, ILOCcode, spillArray, finalArray, ultimateArray);
                        memory_location = memory_location + 4;
                        printf("INPUT 2 ON LINE %d\n", i);
                    }
                }
                if(current->output != NULL) {
                    if(strcmp(current->output, spillArray->data[j]) == 0) {
                        bottom_half_gen(global_output, i, 2, k, memory_location, j, current, ILOCcode, spillArray, finalArray, ultimateArray);
                        memory_location = memory_location + 4;
                        printf("OUTPUT ON LINE %d\n", i);
                    }
                }
            }
        }

        current++;
    }
}

// goal of this function is to create a k sized arraylist for each line and mark spill registers
void register_assignment(struct node2 *rangeArray, struct node *structArray, array_t *ILOCcode, array_t *k_registers, array_t *spillArray, array_t *spillCount, array_t *finalArray) {
    struct node *current = structArray;
    int k = k_registers->capacity;
    int count = 0;
    int same = 0;

    // loops over each intruction and adds registers to k_registers
    for(int i = 0; i < ILOCcode->length; i++) {
        if(current->input1 != NULL) {
            if(*(current->input1) == 'r') {
                if(count < k) {
                    for(int j = 0; j < count; j++) {
                        if(strcmp(k_registers->data[j], current->input1) == 0) {
                            same = 1;
                        }
                    }
                    if(same != 1) {
                        if(spill_check(current->input1, spillArray)) {
                            al_append(k_registers, current->input1);
                        }
                        count++;
                        if(k_registers->length > k) {
                            dead_checker(k, i, rangeArray, structArray, ILOCcode, k_registers, spillArray, spillCount, finalArray);
                        }
                    }

                } else {
                    for(int j = 0; j < k_registers->length; j++) {
                        if(strcmp(k_registers->data[j], current->input1) == 0) {
                            same  = 1;
                        }
                    }
                    if(same != 1) {
                        if(spill_check(current->input1, spillArray)) {
                            al_append(k_registers, current->input1);
                        }
                        count++;
                        if(k_registers->length > k) {
                            dead_checker(k, i, rangeArray, structArray, ILOCcode, k_registers, spillArray, spillCount, finalArray);
                        }
                    }
                }
                same = 0;
            }
        }

        if(current->input2 != NULL) {
            if(*(current->input2) == 'r') {
                if(count < k) {
                    for(int j = 0; j < count; j++) {
                        if(strcmp(k_registers->data[j], current->input2) == 0) {
                            same = 1;
                        }
                    }
                    if(same != 1) {
                        if(spill_check(current->input2, spillArray)) {
                            al_append(k_registers, current->input2);
                        }
                        count++;
                        if(k_registers->length > k) {
                            dead_checker(k, i, rangeArray, structArray, ILOCcode, k_registers, spillArray, spillCount, finalArray);
                        }
                    }

                } else {
                    for(int j = 0; j < k_registers->length; j++) {
                        if(strcmp(k_registers->data[j], current->input2) == 0) {
                            same  = 1;
                        }
                    }
                    if(same != 1) {
                        if(spill_check(current->input2, spillArray)) {
                            al_append(k_registers, current->input2);
                        }
                        count++;
                        if(k_registers->length > k) {
                            dead_checker(k, i, rangeArray, structArray, ILOCcode, k_registers, spillArray, spillCount, finalArray);
                        }
                    }
                }
                same = 0;
            }
        }

        if(current-> output != NULL) {
            if(*(current->output) == 'r') {
                if(count < k) {
                    for(int j = 0; j < count; j++) {
                        if(strcmp(k_registers->data[j], current->output) == 0) {
                            same = 1;
                        }
                    }
                    if(same != 1) {
                        if(spill_check(current->output, spillArray)) {
                            al_append(k_registers, current->output);
                        }
                        count++;
                        if(k_registers->length > k) {
                            dead_checker(k, i, rangeArray, structArray, ILOCcode, k_registers, spillArray, spillCount, finalArray);
                        }
                    }

                } else {
                    for(int j = 0; j < k_registers->length; j++) {
                        if(strcmp(k_registers->data[j], current->output) == 0) {
                            same  = 1;
                        }
                    }
                    if(same != 1) {
                        if(spill_check(current->output, spillArray)) {
                            al_append(k_registers, current->output);
                        }
                        count++;
                        if(k_registers->length > k) {
                            dead_checker(k, i, rangeArray, structArray, ILOCcode, k_registers, spillArray, spillCount, finalArray);
                        }
                    }
                }
                same = 0;
            }
        }

        //insert hi at beggining
        (structArray + i)->num = count;
        
        printf("K REGISTERS: %d\n", i + 1);
        for(int j = 0; j < k_registers->length; j++) {
            printf("%s\n", k_registers->data[j]);
            al_append(finalArray, k_registers->data[j]);
        }

        char *hi = "hi";       
        if(k_registers->length < k) {
            for(int l = 0; l < k - k_registers->length; l++) {
                al_append(finalArray, hi);
            }
        }
        
        current++;
    }

}

struct node2 *register_counter(struct node *structArray, array_t *ILOCcode) {
    int max = 0;
    int temp;
    struct node *current = structArray;

    // initialize rangeArray
    struct node2 test;
    struct node2 *rangeArray = malloc(sizeof(test) * (MAX_VR + 5));
    range_initializer(rangeArray);

    // locate each register and set its lower and upper bound
    // also helps to locate the max register number
    for(int i = 0; i < ILOCcode->length; i++) {
        if(current->input1 != NULL) {
            if(*(current->input1) == 'r') {
                temp = string_to_int(current->input1);
                if(temp > max) {
                    max = temp;
                }
                if((rangeArray + temp)->lower_bound == -1) {
                    (rangeArray + temp)->lower_bound = i;
                }
                (rangeArray + temp)->upper_bound = i;
            }
        }

        if(current->input2 != NULL) {
            if(*(current->input2) == 'r') {
                temp = string_to_int(current->input2);
                if(temp > max) {
                    max = temp;
                }
                if((rangeArray + temp)->lower_bound == -1) {
                    (rangeArray + temp)->lower_bound = i;
                }
                (rangeArray + temp)->upper_bound = i;
            }
        }
        if(current->output != NULL) {
            if(*(current->output) == 'r') {
                temp = string_to_int(current->output);
                if(temp > max) {
                    max = temp;
                }
                if((rangeArray + temp)->lower_bound == -1) {
                    (rangeArray + temp)->lower_bound = i;
                }
                (rangeArray + temp)->upper_bound = i;
            }
        }
        current++;
    }
    // populates the max field of the struct
    for(int i = 0; i <= max; i++) {
        (rangeArray + i)->max = max;
    }
    return rangeArray;
}

void free_struct_array(struct node *structArray, array_t *ILOCcode) {
    // performs all free() calls for the structArray
    for(int i = 0; i < ILOCcode->length; i++) {
        struct node *current = structArray + i;
        char *operation = current->operation_name;

        char *add = "add";
        char *sub = "sub";
        char *mult = "mult";
        char *output_literal = "output";
        char *rshift = "rshift";
        char *lshift = "lshift";

        if(strcmp(operation, add) == 0 || strcmp(operation, sub) == 0 || strcmp(operation, mult) == 0 || strcmp(operation, rshift) == 0 || strcmp(operation, lshift) ==0) {
            free(current->input2);
        }

        if(strcmp(operation, output_literal) != 0) {
            free(current->output);
        }

        free(current->operation_name);
        free(current->input1);
    }
}

struct node *struct_creator(array_t *ILOCcode) {

    struct node test;
    struct node *structArray = malloc(ILOCcode->length * sizeof(test) * 2);

    // creates an array of structs where each element is a line of code
    for(int i = 0; i < ILOCcode->length; i++) {
        char *line = ILOCcode->data[i];
        char *ptr = line;
        char *start = line;
        int count = 0;

        while(*ptr != ' ' && *ptr != '\t') {
            ptr++;
            count++;
        }

        char *operation = malloc(count + 1);
        memmove(operation, start, count);
        *(operation + count) = '\0';

        struct node *current;
        current = structArray + i;

        current->operation_name = operation;

        //printf("%s\n", current->operation_name);

        while(*ptr == ' ' || *ptr == '\t') {
            ptr++;
        }

        start = ptr;
        count = 0;

        while(*ptr != ',' && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\0') {
            ptr++;
            count++;
        }

        char *first_input = malloc(count + 1);
        memmove(first_input, start, count);
        *(first_input + count) = '\0';

        current->input1 = first_input;

        //printf("%s\n", current->input1);

        char *add = "add";
        char *sub = "sub";
        char *mult = "mult";
        char *output_literal = "output";
        char *rshift = "rshift";
        char *lshift = "lshift";

        // check if we need to populate the second_input field. If not, initialize it as NULL
        if(strcmp(operation, add) == 0 || strcmp(operation, sub) == 0 || strcmp(operation, mult) == 0 || strcmp(operation, rshift) == 0 || strcmp(operation, lshift) ==0) {
            while(*ptr == ' ' || *ptr == '\t' || *ptr == ',') {
                ptr++;
            }
    
            start = ptr;
            count = 0;
    
            while(*ptr != ',' && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\0') {
                ptr++;
                count++;
            }
    
            char *second_input = malloc(count + 1);
            memmove(second_input, start, count);
            *(second_input + count) = '\0';
    
            current->input2 = second_input;
    
            //printf("%s\n", current->input2);
        } else {
            current->input2 = NULL;
        }

        if(strcmp(operation, output_literal) != 0) {
            while(*ptr == ' ' || *ptr == '\t' || *ptr == '=' || *ptr == '>') {
                ptr++;
            }
    
            start = ptr;
            count = 0;

            while(*ptr != ',' && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\0') {
                ptr++;
                count++;
            }

            char *first_output = malloc(count + 1);
            memmove(first_output, start, count);
            *(first_output + count) = '\0';
    
            current->output = first_output;
    
            //printf("%s\n", current->output);
        } else {
            current->output = NULL;
        }
    }
    return structArray;
}

//populates array list with each line of ILOC code
void file_opener(char *filename, array_t* ILOCcode) {

    // open file
    FILE *pF;
    pF = fopen(filename, "r");

    // string to store line
    char lineOfFile[100];

    // append each line to our arraylist
    if(pF == NULL) {
        printf("File does not exist\n");
    } else {
        while (fgets(lineOfFile, 100, pF)) {
            if(*lineOfFile != '/' && *lineOfFile != ' ' && *lineOfFile != '\n')
                al_append(ILOCcode, lineOfFile);
        }
    }

    if(pF != NULL) {
        fclose(pF);
    }
}

int main(int argc, char **argv) {

    array_t ILOCcode;
    array_t k_registers;
    array_t spillArray;
    array_t finalArray;
    array_t ultimateArray;
    array_t spillCount;
    al_init(&ILOCcode, 10);
    int k = atoi(argv[1]);
    al_init(&k_registers, k);
    al_init(&spillArray, 10);
    al_init(&spillCount, 10);
    al_init(&finalArray, 10);
    al_init(&ultimateArray, 10);
    
    file_opener(argv[2], &ILOCcode);
    struct node *structArray = struct_creator(&ILOCcode);
    struct node2 *rangeArray = register_counter(structArray, &ILOCcode);
    register_assignment(rangeArray, structArray, &ILOCcode, &k_registers, &spillArray, &spillCount, &finalArray);
    code_generator(k, structArray, rangeArray, &ILOCcode, &k_registers, &spillArray, &spillCount, &finalArray, &ultimateArray);

    /*
    for(int i = 0; i < spillCount.length; i++) {
        printf("%s\n", spillCount.data[i]);
    }
    
    for(int i = 0; i < spillArray.length; i++) {
        printf("%s\n", spillArray.data[i]);
    }
    */
    for(int i = 0; i < ultimateArray.length; i++) {
        printf("%s\n", ultimateArray.data[i]);
    }

    FILE *file = fopen(argv[3], "w");
    
    for(int i = 0; i < ultimateArray.length; i++) {
        fprintf(file, "%s\n", ultimateArray.data[i]);
    }

    fclose(file);

    
    free_struct_array(structArray, &ILOCcode);
    free(structArray);
    free(rangeArray);
    al_destroy(&ILOCcode);
    al_destroy(&k_registers);
    al_destroy(&spillArray);
    al_destroy(&spillCount);
    al_destroy(&finalArray);
    al_destroy(&ultimateArray);
}