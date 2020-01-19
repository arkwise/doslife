#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "DOSTCP.H"

char square[10] = { 'o', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

int checkwin();
void board();

void convertUsableToRaw(char *raw, int *nChoice, bool bUR)
{
	// Usable => Raw
	if(bUR)
	{
		strcpy(raw, "");
		itoa(*nChoice, raw, 10);
	}
	// Raw => Usable
	else 
	{
		*nChoice = atoi(raw);
	}
}

bool bServer;

int main(int argc, char *argv[])
{
	startConnecting(argc, argv);

    int player = 1, i, choice;
    bool bCorrect;
	if(argc != 1) {
		bServer = false;
        player = 2;
    }
    else if(argc == 1)
        bServer = true;
    
    char rawData[4];

    char mark;
    do
    {
        board();
        player = (player % 2) ? 1 : 2;

        strcpy(rawData, "");
        bCorrect = true;
        // Networking
        if(player == 1) // Your(Player1) turn
        {
          printf("Your turn, enter a number: ");
          scanf("%d", &choice);
        }
        else // Player2 turn
        {
        //   printf("Waiting Other Player");
          recvData(rawData);
          convertUsableToRaw(rawData, &choice, false);
          printf("Recv Choice %d\n", choice);
        }
        
        if(bServer)
            mark = (player == 1) ? 'X' : 'O';
        else
            mark = (player != 1) ? 'X' : 'O';

        if (choice == 1 && square[1] == '1')
            square[1] = mark;
            
        else if (choice == 2 && square[2] == '2')
            square[2] = mark;
            
        else if (choice == 3 && square[3] == '3')
            square[3] = mark;
            
        else if (choice == 4 && square[4] == '4')
            square[4] = mark;
            
        else if (choice == 5 && square[5] == '5')
            square[5] = mark;
            
        else if (choice == 6 && square[6] == '6')
            square[6] = mark;
            
        else if (choice == 7 && square[7] == '7')
            square[7] = mark;
            
        else if (choice == 8 && square[8] == '8')
            square[8] = mark;
            
        else if (choice == 9 && square[9] == '9')
            square[9] = mark;
            
        else
        {
            printf("Invalid move ");

            player--;
            bCorrect = false;
            getch();
        }
        if(bCorrect)
        {
            if(player == 1)
            {
                convertUsableToRaw(rawData, &choice, true);
                sendData(rawData);
            }
        }
        i = checkwin();

        player++;
    }while (i ==  - 1);

    board();
    if (i == 1)
    {
        --player;
        player = (player % 2) ? 1 : 2;
        if(player == 1)
            if(bServer)
                printf("==>\aServer win");
            else
                printf("==>\aClient win");
        else
            if(bServer)
                printf("==>\aClient win");
            else
                printf("==>\aServer win");
    }
    else
        printf("==>\aGame draw");

    closeSocket();

    return 0;
}

/*********************************************

FUNCTION TO RETURN GAME STATUS
1 FOR GAME IS OVER WITH RESULT
-1 FOR GAME IS IN PROGRESS
O GAME IS OVER AND NO RESULT
 **********************************************/

int checkwin()
{
    if (square[1] == square[2] && square[2] == square[3])
        return 1;
        
    else if (square[4] == square[5] && square[5] == square[6])
        return 1;
        
    else if (square[7] == square[8] && square[8] == square[9])
        return 1;
        
    else if (square[1] == square[4] && square[4] == square[7])
        return 1;
        
    else if (square[2] == square[5] && square[5] == square[8])
        return 1;
        
    else if (square[3] == square[6] && square[6] == square[9])
        return 1;
        
    else if (square[1] == square[5] && square[5] == square[9])
        return 1;
        
    else if (square[3] == square[5] && square[5] == square[7])
        return 1;
        
    else if (square[1] != '1' && square[2] != '2' && square[3] != '3' &&
        square[4] != '4' && square[5] != '5' && square[6] != '6' && square[7] 
        != '7' && square[8] != '8' && square[9] != '9')

        return 0;
    else
        return  - 1;
}


/*******************************************************************
FUNCTION TO DRAW BOARD OF TIC TAC TOE WITH PLAYERS MARK
 ********************************************************************/


void board()
{
    system("cls");
    printf("\n\n\tTic Tac Toe\n\n");

    printf("Server : X  -  Client : O\n\n\n");


    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", square[1], square[2], square[3]);

    printf("_____|_____|_____\n");
    printf("     |     |     \n");

    printf("  %c  |  %c  |  %c \n", square[4], square[5], square[6]);

    printf("_____|_____|_____\n");
    printf("     |     |     \n");

    printf("  %c  |  %c  |  %c \n", square[7], square[8], square[9]);

    printf("     |     |     \n\n");
}

/*******************************************************************
END OF PROJECT
 ********************************************************************/