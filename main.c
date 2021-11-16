#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//API Project 2020
//Lorenzo Campana
//lorecampa99@gmail.com
//2° year of Engineering of Computing Systems at Politecnico di Milano
//Project of the Algorithms and Pinciples of Computer Science module

/*
Final grade: 30L
WriteOnly: 5/5
BulkReads: 5/5
TimeForAChange: 5/5
RollingBack: 5/5
AlteringHistory: 5/5
Rollercoaster: 5/5
Lode: 1/1
 */


//length buffer row
#define MAX_SIZE 1024
//length initial vector text editor
#define START_SIZE 150
//coefficient of reallocation
#define COEFF_REALLOC 2




//Idea: use of dynamic arrays, without freeing the strings
// due to the fact that they will be reused somehow.

//struct commands
typedef struct command{
    int ind1;
    int ind2;
    char letter;
}Command;

//struct array text editor
typedef struct array{
    char** array;
    int used;
    int size;
}Array;

//undo and redo list node
typedef struct stacknode{
    char** array;
    int used;
    int size;
    int riga1;
    int riga2;
    struct stacknode* next;
}stackNode;


//global variables for Undo and Redo and their sizes.
int undoStackSize, redoStackSize;
stackNode* undoStack, *redoStack;


//manage array functions
void initArray(Array* arr);
void reallocArray(Array* arr);



//manage stack
stackNode* popStack(stackNode** root);
void pushStack(stackNode** root, stackNode* node);



//manage commands
int commandUtil(Array* arr, Command* command);

//commands
void changeCommand(Array* arr, Command* command);
void printCommand(Array* arr, Command* command);
void deleteCommand(Array* arr, Command* command);
void undoCommand(Array* arr, Command* command);
void redoCommand(Array* arr, Command* command);

//retrieve command from stdin
Command getCommand();



int main(){

    Array arrayTextEditor;
    initArray(&arrayTextEditor);

    //initialization stacks undo e redo
    undoStack = NULL;
    undoStackSize = 0;
    redoStack = NULL;
    redoStackSize = 0;

    //loop terminate if 'q' command appears (return 0)
    Command currCommand = getCommand();
    while (commandUtil(&arrayTextEditor, &currCommand)){
        currCommand = getCommand();
    }

    return 0;
}


void initArray(Array* arr){

    //initial allocation of the dynamic array (look at START SIZE)
    //then for each string will be allocated a proportional number of byte based of their length
    //way to expensive saving string with 1024 character for each cell
    arr->array = (char**) malloc(START_SIZE*sizeof(char*));
    arr->used = 0;
    arr->size = START_SIZE;
}

void reallocArray(Array* arr){
    while (arr->used >= arr->size){
        arr->size *= COEFF_REALLOC;
    }
    arr->array = realloc(arr->array, sizeof(char*) * (arr->size));
}

Command getCommand(){
    //command parsing, could have been developed in many ways
    Command command;
    char stringTemp[MAX_SIZE];
    int length, nDigits;
    fgets(stringTemp, MAX_SIZE, stdin);

    //-1 for the \n
    length = strlen (stringTemp) - 1;

    command.letter = stringTemp[length-1];

    //Case 'q'
    if (command.letter == 'q'){
        command.ind1 = -1;
        command.ind2 = -1;
        return command;
    }

    //first address
    command.ind1 = atoi(stringTemp);


    nDigits = floor(log10(abs(command.ind1))) + 1;

    if (length == nDigits+1) {
        command.ind2 = -1;
        return command;

    }else{
        char s1[2] = ",";
        char s2[1];
        s2[0] = command.letter;
        char* token;
        token = strtok(stringTemp, s1);
        token = strtok(NULL, s2);
        command.ind2 = atoi(token);

        return command;
    }
}


void changeCommand(Array* arr, Command* command) {
    int ind1 = command->ind1, ind2 = command->ind2;
    int oldUsed = arr->used;
    int oldSize = arr->size;


    //free the redo stack after a change
    if (redoStack != NULL){
        //don't free the memory as it would take too long
        redoStack = NULL;
        redoStackSize = 0;

    }


    //allocation node for Undo stack
    stackNode* newNode = malloc(sizeof(stackNode));

    //managing two different cases
    if (ind1 > oldUsed){
        //adding new lines to the text editor
        newNode->used = 0;
        newNode->size = ind2 - ind1 + 1;
        newNode->riga1 = ind1;
        newNode->riga2 = ind2;

        //undo consists on setting NULL in specific addresses
        newNode->array = calloc(newNode->size, sizeof(char*));

    }else{
        //overriding strings
        newNode->riga1 = -1;
        newNode->riga2 = -1;
        newNode->used = oldUsed;
        newNode->size = oldSize;

        //vector backup just by coping the pointer of the old strings (without freeing them)
        char** oldArray = malloc(sizeof(char*) * oldSize);
        for (int i = 0; i < oldUsed; i++){
            oldArray[i] = arr->array[i];
        }
        newNode->array = oldArray;
    }

    //node insertion in undoStack
    pushStack(&undoStack, newNode);
    undoStackSize += 1;


    //computation of change
    int j = 0;
    char stringTemp[MAX_SIZE];


    for (int i = ind1- 1; i < ind2; i++){
        //control if reallocation is needed
        if (arr->used >= arr->size){
            reallocArray(arr);
        }

        fgets(stringTemp, MAX_SIZE, stdin);

        //increment arr->used if new strings are added
        if (i >= oldUsed){
            arr->used += 1;
        }

        //alloc a new string without freeing the previous (undo and redo stacks nodes point to them)
        arr->array[i] = malloc (strlen(stringTemp) + 1);
        strcpy(arr->array[i], stringTemp);

        j += 1;
    }
    //reading the last '.' at the end of the command
    fgets(stringTemp, MAX_SIZE, stdin);

}

void printCommand(Array* arr, Command* command){
    int ind1 = command->ind1, ind2 = command->ind2, used = arr->used;

    //managing edge cases such as 0, Xp or 0,0p
    if (ind1 == 0){
        printf(".\n");
        if (ind2 == 0) return;
        else{
            ind1 = 1;
        }
    }

    for (int i = ind1 - 1; i < ind2; i++){
        if (i < used){
            printf("%s", arr->array[i]);

        }else{
            printf(".\n");
        }
    }
}

void deleteCommand(Array* arr, Command* command) {
    int ind1 = command->ind1, ind2 = command->ind2;
    char* stringTemp;
    int oldUsed = arr->used;
    int oldSize = arr->size;


    //free the redo stack in case of delete
    if (redoStack != NULL){
        //do not free the node due to the fact that occupy a little portion of memory and it would require
        //a lot of time
        redoStack = NULL;
        redoStackSize = 0;
    }

    //creation node for UndoStack
    stackNode* newNode = malloc(sizeof(stackNode));
    newNode->riga1 = -1;
    newNode->riga2 = -1;
    newNode->used = oldUsed;
    newNode->size = oldSize;

    //backup old array
    char** oldArray = malloc(sizeof(char*) * oldSize);
    for (int i = 0; i < oldUsed; i++){
        //backup of pointers
        oldArray[i] = arr->array[i];
    }

    newNode->array = oldArray;

    //UndoStack node insertion
    pushStack(&undoStack, newNode);
    undoStackSize += 1;



    //Edge cases

    //address not present
    if (ind1 > oldUsed){
        return;
    }
    if (ind1 == 0){
        if (ind2 == 0){
            //0,0d
            return;
        }else{
            //0,Xd
            ind1 = 1;

        }
    }

    //second address not present
    if (ind2 > oldUsed){
        ind2 = arr->used;
    }

    int gap = ind2 - ind1 + 1;

    //normal case
    for (int i = ind1 - 1; i < oldUsed; i++){
        //translations without freeing strings
        if (i + gap < oldUsed){
            stringTemp = arr->array[i];
            arr->array[i] = arr->array[i+gap];
            arr->array[i+gap] = stringTemp;

        }else{
            arr->array[i] = NULL;
        }
    }
    //updates used attribute
    arr->used = oldUsed - gap;

}


int commandUtil(Array* arr, Command* command){
    char letter = command->letter;
    int result, value, oldUndoSize, oldRedoSize;
    Command newCommand;

    switch (letter) {
        case 'c':
            changeCommand(arr, command);
            return 1;

        case 'd':
            deleteCommand(arr, command);
            return 1;

        case 'p':
            printCommand(arr, command);
            return 1;

        case 'u':
            //Merging sequential Undo and Redo commands

            oldUndoSize = undoStackSize;
            oldRedoSize = redoStackSize;
            result = command->ind1;

            if (result > oldUndoSize){
                result = oldUndoSize;
            }
            //update size
            oldUndoSize -= result;
            oldRedoSize += result;

            newCommand = getCommand();

            while (newCommand.letter == 'u' || newCommand.letter == 'r'){
                value = newCommand.ind1;
                if (newCommand.letter == 'u'){
                    if (value > oldUndoSize){
                        value = oldUndoSize;
                    }
                    result += value;
                    //update size
                    oldUndoSize -= value;
                    oldRedoSize += value;

                }else{
                    if(value > oldRedoSize){
                        value = oldRedoSize;
                    }
                    result -= value;
                    //update size
                    oldUndoSize += value;
                    oldRedoSize -= value;
                }
                newCommand = getCommand();
            }

            //execute undo/redo based on result value
            if (result >= 0){
                command->ind1 = result;
                undoCommand(arr, command);
            }else{
                command->ind1 = abs(result);
                command->letter = 'r';
                redoCommand(arr, command);
            }

            //little recursion
            return commandUtil(arr, &newCommand);

        case 'r':
            //Merging sequential redo commands
            result = command->ind1;
            newCommand = getCommand();
            while (newCommand.letter == 'r'){
                result += newCommand.ind1;
                newCommand = getCommand();
            }
            command->ind1 = result;
            redoCommand(arr, command);

            return commandUtil(arr, &newCommand);

        case 'q':
            return 0;

    }

}



stackNode* popStack(stackNode** root){
    stackNode* temp = *root;
    *root = (*root) -> next;
    temp->next = NULL;
    return temp;
}

void pushStack(stackNode** root, stackNode* node){
    node->next = *root;
    *root = node;
}


void undoCommand(Array* arr, Command* command){
    int ind1 = command->ind1;
    stackNode* temp;
    char* tempPunt;
    int tempUsed, tempSize, riga1, riga2, arrUsed, h;
    char** tempArray;


    int j = 0;
    while (undoStack != NULL && j < ind1){
        //pop of node from undo stack
        temp = popStack(&undoStack);
        undoStackSize -= 1;

        //pop of a snapshot
        if (temp->riga1 == -1){
            //simple switch of pointers
            tempArray= temp->array;
            tempUsed = temp->used;
            tempSize = temp->size;

            temp->array = arr->array;
            temp->used = arr->used;
            temp->size = arr->size;
            temp->next = NULL;

            arr->array = tempArray;
            arr->used = tempUsed;
            arr->size = tempSize;
        }
        //pop of a change that incremented a vector
        else{

            riga1 = temp->riga1;
            riga2 = temp->riga2;
            tempUsed = temp->used;
            arrUsed = arr->used;

            h = 0;
            for (int i = riga1-1; i < riga2; i++){
                if (i < arrUsed){
                    if (h < tempUsed){
                        //switch of pointers
                        tempPunt = arr->array[i];
                        arr->array[i] = temp->array[h];
                        temp->array[h] = tempPunt;

                    }else{
                        //switch with temp[i] NULL
                        temp->array[h] = arr->array[i];
                        arr->array[i] = NULL;
                        //update values
                        temp->used += 1;
                        arr->used -= 1;
                    }
                }else{
                    if (h < tempUsed){
                        //switch with arr[i] NULL
                        arr->array[i] = temp->array[h];
                        temp->array[h] = NULL;
                        //update values
                        temp->used -= 1;
                        arr->used += 1;
                    }
                }

                h+=1;
            }
        }

        //insert new node in the redo stack
        pushStack(&redoStack, temp);
        redoStackSize += 1;

        j+=1;
    }

}

void redoCommand(Array* arr, Command* command){
    int ind1 = command->ind1;
    stackNode* temp;
    char* tempPunt;
    int tempUsed, tempSize, riga1, riga2, arrUsed, h;
    char** tempArray;


    int j = 0;
    while (redoStack != NULL && j < ind1){

        //pop the node from the redo stack
        temp = popStack(&redoStack);
        redoStackSize -= 1;

        //snapshot case
        if (temp->riga1 == -1){
            //switch of pointers
            tempArray= temp->array;
            tempUsed = temp->used;
            tempSize = temp->size;

            temp->array = arr->array;
            temp->used = arr->used;
            temp->size = arr->size;
            temp->next = NULL;

            arr->array = tempArray;
            arr->used = tempUsed;
            arr->size = tempSize;
        }

        else{
            riga1 = temp->riga1;
            riga2 = temp->riga2;
            tempUsed = temp->used;
            arrUsed = arr->used;

            h = 0;
            for (int i = riga1-1; i < riga2; i++){

                if (i < arrUsed){
                    if (h < tempUsed){
                        tempPunt = arr->array[i];
                        arr->array[i] = temp->array[h];
                        temp->array[h] = tempPunt;

                    }else{
                        temp->array[h] = arr->array[i];
                        arr->array[i] = NULL;

                        temp->used += 1;
                        arr->used -= 1;
                    }
                }else{
                    if (h < tempUsed){
                        arr->array[i] = temp->array[h];
                        temp->array[h] = NULL;

                        temp->used -= 1;
                        arr->used += 1;
                    }
                }

                h+=1;
            }
        }

        //insertion of a node in the undo stack
        pushStack(&undoStack, temp);
        undoStackSize += 1;

        j+=1;
    }

}


