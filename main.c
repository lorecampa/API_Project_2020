#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//PROGETTO API 2020
//Lorenzo Campana
//lorecampa99@gmail.com
//2° anno ingegneria informatica al Politecnico di Milano
//Progetto relativo al corso di Algoritmi e strutture dati

/*
Valutazione progetto: 30L
WriteOnly: 5/5
BulkReads: 5/5
TimeForAChange: 5/5
RollingBack: 5/5
AlteringHistory: 5/5
Rollercoaster: 5/5
Lode: 1/1
 */


//lunghezza buffer riga
#define MAX_SIZE 1024
//grandezza vettore text editor iniziale
#define START_SIZE 150
//coefficente di riallocazione
#define COEFF_REALLOC 2

//Idea di base: usere array dinamici e non fare mai la free delle stringhe del vettore dinamico
//in quanto in qualche modo verrano riutilizzate


//struct comandi
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

//lista per implementare la undo e la redo come stack
typedef struct stacknode{
    char** array;
    int used;
    int size;
    int riga1;
    int riga2;
    struct stacknode* next;
}stackNode;


//variabili globali della stack Undo e Redo e le rispettive grandezze
int undoStackSize, redoStackSize;
stackNode* undoStack, *redoStack;


//funzioni gestione array
void initArray(Array* arr);
void reallocArray(Array* arr);


//funzioni gestione stack
stackNode* popStack(stackNode** root);
void pushStack(stackNode** root, stackNode* node);



//funzione gestione comandi
int commandUtil(Array* arr, Command* command);

//funzioni dei comandi
void changeCommand(Array* arr, Command* command);
void printCommand(Array* arr, Command* command);
void deleteCommand(Array* arr, Command* command);
void undoCommand(Array* arr, Command* command);
void redoCommand(Array* arr, Command* command);

//funzione per presa del comando da stdin
Command getCommand();



int main(){

    Array arrayTextEditor;
    initArray(&arrayTextEditor);

    //inizializazzione stack undo e redo
    undoStack = NULL;
    undoStackSize = 0;
    redoStack = NULL;
    redoStackSize = 0;

    //ciclo while che termina nel caso di 'q' (return 0)
    Command currCommand = getCommand();
    while (commandUtil(&arrayTextEditor, &currCommand)){
        currCommand = getCommand();
    }

    return 0;
}


void initArray(Array* arr){
    //allocazione array parziale char* del numero di elementi
    //poi ogni elemento dell'array verrà riservato in futuro un numero di byte
    //proporzionale al numero di caratteri della stringa da inserire
    //troppo oneroso salvare subito 1024 carattare per cella
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
    Command command;
    char stringTemp[MAX_SIZE];
    int length, nDigits;
    fgets(stringTemp, MAX_SIZE, stdin);

    //-1 perchè bisogna considerare il \n
    length = strlen (stringTemp) - 1;
    //questa cosa vale sempre
    command.letter = stringTemp[length-1];

    //Caso 'q'
    if (command.letter == 'q'){
        command.ind1 = -1;
        command.ind2 = -1;
        return command;
    }

    //primo indirizzo sempre calcolabile così
    command.ind1 = atoi(stringTemp);

    //calcolo numero di cifre dell'indirizzo 1
    nDigits = floor(log10(abs(command.ind1))) + 1;

    //Caso 'ind1X'
    if (length == nDigits+1) {
        command.ind2 = -1;
        return command;

    }else{
        //Caso 'ind1,ind2X'
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


    //faccio la free della redo stack dopo una change
    if (redoStack != NULL){
        //non sto a liberare la memoria mi occuperebbe troppo tempo
        redoStack = NULL;
        redoStackSize = 0;

    }


    //allocazione nodo per undoStack (caso change)
    stackNode* newNode = malloc(sizeof(stackNode));

    //gestione dei due diversi casi
    if (ind1 > oldUsed){
        //caso in cui si aggiungono nuove righe all'editor di testo
        newNode->used = 0;
        newNode->size = ind2 - ind1 + 1;
        newNode->riga1 = ind1;
        newNode->riga2 = ind2;
        //faccio il salvataggio di stringhe a NULL in quanto non sono presenti
        //per tornare indietro(fare l'undo) non devo fare altro che mettere
        //da riga1 a riga2 del vettore a NULL
        newNode->array = calloc(newNode->size, sizeof(char*));

    }else{
        //caso in cui si cambiano righe già scritte
        newNode->riga1 = -1;
        newNode->riga2 = -1;
        newNode->used = oldUsed;
        newNode->size = oldSize;
        //faccio il backup del dello stato del vettore allocando solo il il vettore e copiando i
        //puntatori delle stringhe
        char** oldArray = malloc(sizeof(char*) * oldSize);
        for (int i = 0; i < oldUsed; i++){
            oldArray[i] = arr->array[i];
        }
        newNode->array = oldArray;
    }

    //inserimento newNode nell'undoStack
    pushStack(&undoStack, newNode);
    undoStackSize += 1;


    //procedo con l'eseguimento della change e modifica dell'array attuale
    int j = 0;
    char stringTemp[MAX_SIZE];


    for (int i = ind1- 1; i < ind2; i++){
        //rialloco in caso di vettore dinamico esaurito
        if (arr->used >= arr->size){
            reallocArray(arr);
        }
        //presa riga
        fgets(stringTemp, MAX_SIZE, stdin);

        //aumento arr->used nel caso di inserimento di nuove righe nell'editor
        if (i >= oldUsed){
            arr->used += 1;
        }
        //alloco una nuova stringa senza fare la free di quella precedente
        //in quanto serve ancora nei backup precedenti (ho passato solo il puntatore)
        arr->array[i] = malloc (strlen(stringTemp) + 1);
        strcpy(arr->array[i], stringTemp);

        j += 1;
    }
    //lettura del punto finale '.' prima di uscire dalla funzione
    fgets(stringTemp, MAX_SIZE, stdin);

}

void printCommand(Array* arr, Command* command){
    int ind1 = command->ind1, ind2 = command->ind2, used = arr->used;

    //gestione casi particolari come 0,Xp o 0,0p
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


    //faccio la free della redo stack nel caso di delete
    if (redoStack != NULL){
        //non faccio la free di stackNode in quanto occupa poca memoria e mi costerebbe solo tempo
        redoStack = NULL;
        redoStackSize = 0;
    }

    //creazione nodo backup da inserire in undoStack
    stackNode* newNode = malloc(sizeof(stackNode));
    newNode->riga1 = -1;
    newNode->riga2 = -1;
    newNode->used = oldUsed;
    newNode->size = oldSize;

    //backup array vecchio
    char** oldArray = malloc(sizeof(char*) * oldSize);
    for (int i = 0; i < oldUsed; i++){
        //passo solo il puntatore senza fare la copia della stringa
        //troppo oneroso in termini di tempo fare la copia della stringa
        //bisogna stare poi attenti a non fare la free delle stringhe vecchie
        oldArray[i] = arr->array[i];
    }
    newNode->array = oldArray;
    //inserimento nodo backup nell'undoStack
    pushStack(&undoStack, newNode);
    undoStackSize += 1;



    //gestione casi particolari
    //gestione casi con indirizzo non presente
    if (ind1 > oldUsed){
        return;
    }
    if (ind1 == 0){
        if (ind2 == 0){
            //caso per gestire 0,0d
            return;
        }else{
            //caso per gestire 0,Xd
            ind1 = 1;

        }
    }
    //secondo indirizzo non presente nel text editor
    if (ind2 > oldUsed){
        ind2 = arr->used;
    }

    int gap = ind2 - ind1 + 1;

    for (int i = ind1 - 1; i < oldUsed; i++){
        //caso in cui si può fare la traslazione
        //inverto semplicemente i puntatori
        if (i + gap < oldUsed){
            stringTemp = arr->array[i];
            arr->array[i] = arr->array[i+gap];
            arr->array[i+gap] = stringTemp;

        }else{
            //caso in cui bisogna iniziare a fare la free del vettore
            //mettendo il puntatore a NULL per sapere quindi se è inizializzato o no
            arr->array[i] = NULL;
        }
    }
    //aggiorno used del vettore dinamico
    arr->used = oldUsed - gap;

}


int commandUtil(Array* arr, Command* command){
    char letter = command->letter;
    int result, value, oldUndoSize, oldRedoSize;
    Command newCommand;

    //switch per gestione vari comandi
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
            //accorpamento istruzioni posto Undo
            //idea: controllo di volta in volta se si possono eseguire quelle operazioni di undo/redo
            //andando a modificare il valore della grandezza della undoStack e redoStack
            //i comandi leciti poi verranno sommati (caso undo) o sottratti (caso redo) a result e poi eseguiti
            oldUndoSize = undoStackSize;
            oldRedoSize = redoStackSize;
            result = command->ind1;

            if (result > oldUndoSize){
                result = oldUndoSize;
            }
            //aggiornamento size
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
                    //aggiornamento size
                    oldUndoSize -= value;
                    oldRedoSize += value;

                }else{
                    if(value > oldRedoSize){
                        value = oldRedoSize;
                    }
                    result -= value;
                    //aggiornamento size
                    oldUndoSize += value;
                    oldRedoSize -= value;
                }
                newCommand = getCommand();
            }

            //eseguimento undo/redo in base al valore di result
            if (result >= 0){
                command->ind1 = result;
                undoCommand(arr, command);
            }else{
                command->ind1 = abs(result);
                command->letter = 'r';
                redoCommand(arr, command);
            }

            //mini ricorsione per gestire l'ultimo comando che ci ha fatto
            //uscire dal ciclo while
            return commandUtil(arr, &newCommand);

        case 'r':
            //gestione accorpamento delle redo una dietro l'altra
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

    //ciclo while che termina solo nel caso di stack vuota o undo terminati
    int j = 0;
    while (undoStack != NULL && j < ind1){
        //presa del nodo dalla undo stack
        temp = popStack(&undoStack);
        undoStackSize -= 1;

        //caso in cui si è fatto la pop di un backup
        if (temp->riga1 == -1){
            //semplice switch dei puntatori con appoggio
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
        //caso in cui si è fatto la pop di una change che aveva aumentato il vettore
        else{

            riga1 = temp->riga1;
            riga2 = temp->riga2;
            tempUsed = temp->used;
            arrUsed = arr->used;

            h = 0;
            for (int i = riga1-1; i < riga2; i++){
                if (i < arrUsed){
                    if (h < tempUsed){
                        //caso switch dei puntatori
                        tempPunt = arr->array[i];
                        arr->array[i] = temp->array[h];
                        temp->array[h] = tempPunt;

                    }else{
                        //caso switch con temp[i] NULL
                        temp->array[h] = arr->array[i];
                        arr->array[i] = NULL;
                        //aggiornamento grandezze vettori
                        temp->used += 1;
                        arr->used -= 1;
                    }
                }else{
                    if (h < tempUsed){
                        //caso switch con arr[i] NULL
                        arr->array[i] = temp->array[h];
                        temp->array[h] = NULL;
                        //aggiornamento grandezze vettori
                        temp->used -= 1;
                        arr->used += 1;
                    }
                }

                h+=1;
            }
        }

        //inserimento nuovo temp modifica nella redoStack
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


    //ciclo che termina in caso di pila vuota o redo terminati
    int j = 0;
    while (redoStack != NULL && j < ind1){

        //presa nodo dalla redoStack
        temp = popStack(&redoStack);
        redoStackSize -= 1;

        //caso in cui bisogna ripristinare un backup
        if (temp->riga1 == -1){
            //semplice switch di puntatori con appoggio
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
        //caso in cui bisogna ripristinare una change con incremento vettore
        else{
            riga1 = temp->riga1;
            riga2 = temp->riga2;
            tempUsed = temp->used;
            arrUsed = arr->used;

            h = 0;
            for (int i = riga1-1; i < riga2; i++){

                if (i < arrUsed){
                    if (h < tempUsed){
                        //caso switch puntatori non nulli
                        tempPunt = arr->array[i];
                        arr->array[i] = temp->array[h];
                        temp->array[h] = tempPunt;

                    }else{
                        //caso switch con temp[i] NULL
                        temp->array[h] = arr->array[i];
                        arr->array[i] = NULL;
                        //aggiornamento grandezza vettore
                        temp->used += 1;
                        arr->used -= 1;
                    }
                }else{
                    if (h < tempUsed){
                        //caso switch con arr[i] NULL
                        arr->array[i] = temp->array[h];
                        temp->array[h] = NULL;
                        //aggiornamento grandezza vettore
                        temp->used -= 1;
                        arr->used += 1;
                    }
                }

                h+=1;
            }
        }

        //inserimento nuovo temp modificato nello stack Undo
        pushStack(&undoStack, temp);
        undoStackSize += 1;

        j+=1;
    }

}


