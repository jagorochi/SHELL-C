/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: jago
 *
 * Created on 3 janvier 2018, 14:52
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define  DEFAULT_BUFFER_LENGTH 32
#define  FREE_SIZE_BEFORE_REALLOC 3

// afin de définir le début et fin de chaque token
#define  TOKEN_BEGIN 1
#define  TOKEN_END 0
// le type de quote detecté dans la commande utilisateur
#define  NO_QUOTE 0
#define  SINGLE_QUOTE 1
#define  DOUBLE_QUOTE 2


char * ExtendBufferSize(char *pBuffer, int BufferLength, int index);

/*
 * 
 */
int main(int argc, char** argv) {
    //char *pInputBuffer = malloc(sizeof (char) * DEFAULT_BUFFER_LENGTH); //definition d'un pointeur de type char pour récuperer le contenu de stdin
    char * pfBuffer = malloc(sizeof (char) * DEFAULT_BUFFER_LENGTH);
    //int maxIndex = 0;
    if (pfBuffer == NULL) {
        perror("malloc");
        return (EXIT_FAILURE);
    }

    int nInput_Buffer_Length = DEFAULT_BUFFER_LENGTH; // definition de la longueur du buffer par defaut
    int nIndex = 0; // index de la chaine traitée


    int tokenMode;
    int nTokenCount;
    int quoteMode; // variable de controle si guillemets dans la commande
    char cCurrent;
    char cPrevious;


    while (1) {
        printf("prompt:");
        nIndex = 0;
        tokenMode = TOKEN_END;
        nTokenCount = 0;
        quoteMode = 0; // variable de controle si guillemets dans la commande
        cPrevious = ' ';


        memset(pfBuffer, '\0', sizeof (char) * nInput_Buffer_Length);

        while ((cCurrent = getchar()) && cCurrent != '\n') { // tant qu'on peut lire un char dans stdin

            if (quoteMode) { // si guillemets ouvert 
                pfBuffer[nIndex++] = cCurrent; // tous les char font partis de l'argument
                if ((cCurrent == '\"' && quoteMode == DOUBLE_QUOTE) || (cCurrent == '\'' && quoteMode == SINGLE_QUOTE)) { // si on arrive a nouveau sur un guillemet
                    quoteMode = 0; // fin des guillemets
                    tokenMode = TOKEN_END;
                }
            } else {
                switch (cCurrent) {
                    case '\'':
                        if (!quoteMode) {
                            quoteMode = SINGLE_QUOTE; // debut des guillemets
                        }
                    case '\"':
                        if (!quoteMode) {
                            quoteMode = DOUBLE_QUOTE; // debut des guillemets
                        }
                        if (nTokenCount > 0) { // si ce n'est pas le premier token
                            pfBuffer[nIndex++] = '\0'; // nouveau token
                        }
                        nTokenCount++;
                        pfBuffer[nIndex++] = cCurrent;
                        tokenMode = TOKEN_BEGIN;
                        break;
                    case ' ': // traitement des espaces
                        if (tokenMode == TOKEN_BEGIN) {
                            if (cPrevious != '\\') {
                                tokenMode = TOKEN_END;
                            } else {
                                pfBuffer[nIndex++] = cCurrent;
                            }
                        }
                        /*
                        if (previous != ' ') { // si le précédent charactère n'était pas déjà un espace
                            pfBuffer[nWritePos++] = '\0'; // nouveau token
                            nTokenCount++;
                        }*/
                        break;
                    case '>': // write to file

                    case '<': // read from file

                    case '|': // pipe

                    case '&': // background operation

                    case ';':

                    default: // charactère lambda
                        if (tokenMode == TOKEN_END) {
                            tokenMode = TOKEN_BEGIN;
                            nTokenCount++;
                            if (nTokenCount > 1) { // il faut separer les tokens après le premier
                                pfBuffer[nIndex++] = '\0'; // nouveau token
                            }
                        }
                        pfBuffer[nIndex++] = cCurrent;
                }
            }
            cPrevious = cCurrent;


            if (nIndex > (nInput_Buffer_Length - FREE_SIZE_BEFORE_REALLOC)) { // si on arrive a la taille max du buffer
                // on doit réallouer le tableau

                nInput_Buffer_Length *= 2;
                // printf("Extend : %d", nInput_Buffer_Length);
                char * newBuffer = malloc(sizeof (char) * nInput_Buffer_Length);
                if (newBuffer != NULL) {
                    memcpy(newBuffer, pfBuffer, sizeof (char) * nIndex);
                }
                free(pfBuffer);
                pfBuffer = newBuffer;
            }
        }
        pfBuffer[nIndex++] = '\0'; // rajout d'un dernier caractère de fin de chaine..
        
        /*---------------------------------------------------------------------------
        // a partir d'ici on peut commencer à traiter la commande de l'utilisateur
        //---------------------------------------------------------------------------
         il faut formater la chaine de charactère de manière a pouvoir récuperer :
            - la commande de l'utilisateur
            - la liste des arguments
            - delimiter les chaines de soumises entre guillemets
            - les caractères de redirection
         */
        printf("------------------------------------------\nnb token = : %d\n", nTokenCount);
        char *cmd;
        cmd = malloc(sizeof(char) * (5 + (int)strlen(pfBuffer)));
        strcpy(cmd, "/bin/");
        strcat(cmd, pfBuffer); // copie du premier token qui est censé être le programme a démarrer
        
        if (access(cmd, F_OK) != -1) {
            // le fichier existe
            printf("cmd %s exist..\n", pfBuffer);
            // il faut construire la liste d'argument
            char **lArgs;
            lArgs = malloc(sizeof(char*) * nTokenCount);
            
            int nPos = 0; //(int)strlen(&pfBuffer[0])+1; // positionnement au second token
            for (int i = 0; i < nTokenCount; i++) {
                int nSize = (int) strlen(&pfBuffer[nPos]);
                lArgs[i] = malloc(sizeof (char) * (nSize + 1));
                //memset(lArgs[i], '\0', sizeof (char) * (nSize + 1)); // on remplit de \0
                strcpy(lArgs[i], &pfBuffer[nPos]); // copie de l'argument dans le buffer
                printf("lArgs[%d] (s=%d) :  %s\n",i,nSize, &pfBuffer[nPos]);
                nPos += nSize + 1;
            }
            lArgs[nTokenCount] = NULL;
            if (execv(cmd, lArgs) == -1) {
                perror("execv");
                fprintf(stderr, "error running execv\n");
            }
        } else {
            // file doesn't exist
            printf("cmd %s doesn't exist..\n", pfBuffer);
        }
        // printf("pfBuffer = %s, length = %d\n", pfBuffer, (int) strlen(pfBuffer));
        /*char *token;
        token = strtok(pfBuffer, '\0');
        while (token != NULL) {
            printf(" %s\n", token);
            token = strtok(NULL, '\0');
        }*/

        // prompt:echo "iojlo ji' " 1 5 7 8 9 6 12,5
        
        if (strstr(pfBuffer, "exit") == pfBuffer) { //si la commande commence par la chaine "exit"
            // l'utilisateur pourrait aussi taper plus de charactère après mais suffisant pour le projet..
            return (EXIT_SUCCESS);
        }
    }


    return (EXIT_SUCCESS);
}

char * ExtendBufferSize(char *pBuffer, int BufferLength, int index) {
    char* ptempBuffer;
    ptempBuffer = malloc(sizeof (char) * BufferLength);
    if (ptempBuffer != NULL) {
        if (index > 0) {
            memcpy(ptempBuffer, pBuffer, sizeof (char) * index); // on copie le contenu de l'ancien dans le nouveau buffer
        }
    } else {
        perror("malloc");
        printf("erreur pour malloc ptempBuffer\n");
        exit(EXIT_FAILURE); // plus de mémoire.. inutile de continuer
    }
    if (pBuffer != NULL) {
        free(pBuffer);
    }
    pBuffer = ptempBuffer;

    return pBuffer;
}
