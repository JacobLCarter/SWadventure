/*******************************************************************************
* Program Name: cartjaco.buildrooms
* Author: Jacob Carter
* Date Modified: 10/22/2018
* Course: CS344
* Description: Creates a map from a series of randomly generated rooms that is
* then used by cartjaco.adventure
*******************************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//define global constant variables
#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define MAX_ROOMS 7
#define NUM_ROOM_POOL 10
#define NAME_SIZE 9

//create an array holding the name of all available rooms in the pool
char* ROOM_NAMES[NUM_ROOM_POOL] =
{
    "Tatooine",
    "Hoth",
    "Alderaan",
    "Endor",
    "Yavin",
    "Bespin",
    "Dagobah",
    "Kashyyyk",
    "Corellia",
    "Korriban"
};

//global enumeration to make defining room type easier
enum ROOM_TYPE { START_ROOM, MID_ROOM, END_ROOM };

//enumeration to mimic the behavior of the boolean type
typedef enum { FALSE, TRUE } bool;

struct ROOM
{
    int numConnections;
    enum ROOM_TYPE type;
    char name[NAME_SIZE];
    struct ROOM* connections[MAX_CONNECTIONS];
    bool isInitialized;
};

//create an array of rooms to hold the map as it is created
struct ROOM* GeneratedMapList[MAX_ROOMS];

struct ROOM RoomPool[NUM_ROOM_POOL];

//the name of the directory in which to write files 
char directoryName[256];

/*******************************************************************************
* Function Name: IsMapFull
* Description: helper function to check if the map is at capacity 
* Arguments: none 
* Return: bool 
*******************************************************************************/
bool IsMapFull()
{
    int i;
    
    for (i = 0; i < MAX_ROOMS; i++)
    {
        //executes if any of the rooms are under the minimum connections
        if (GeneratedMapList[i]->numConnections < 3)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*******************************************************************************
* Function Name: IsNewConnectionAvailable
* Description: helper function to check if a room can accept a new connection 
* Arguments: none 
* Return: bool 
*******************************************************************************/
bool IsNewConnectionAvailable(struct ROOM* x)
{
    //returns TRUE if the room's number of connections is less than max allowed
    if (x->numConnections < MAX_CONNECTIONS)
    {
        return TRUE;
    }
        
    return FALSE;
}

/*******************************************************************************
* Function Name: IsSameRoom
* Description: helper function to check if a room and a potential connection
* refer to the same struct
* Arguments: none 
* Return: bool 
*******************************************************************************/
bool IsSameRoom(struct ROOM* x, struct ROOM* y)
{
    //returns true if the room and the potential connection have the same name
    if (strcmp(x->name, y->name) == 0)
    {
        return TRUE;
    }
        
    return FALSE;
}

/*******************************************************************************
* Function Name: DoesConnectionExist 
* Description: helper function to check if a room and a potential connection
* are already connected
* Arguments: none 
* Return: bool 
*******************************************************************************/
bool DoesConnectionExist(struct ROOM* x, struct ROOM* y)
{
    int i;
   
    //loop over each connection in x
    for (i = 0; i < x->numConnections; i++)
    {
        //break out of function if x has no connections
        //if (x->connections[i] == 0)
        //{
         //   return FALSE;
        //}
        //returns true if the room has a connection with the same name as the 
        //potential
        if (strcmp(x->connections[i]->name, y->name) == 0)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

/*******************************************************************************
* Function Name: ConnectRooms 
* Description: creates a connection between the passed room and a random room 
* are already connected
* Arguments: none 
* Return: none 
*******************************************************************************/
void ConnectRooms(struct ROOM* x)
{
    bool madeConnection = FALSE;

    if (IsNewConnectionAvailable(x))
    {
        do
        {
            //get the index of a random room in map 
            int selectedRoom = GetRandomRoom(MAX_ROOMS);

            //executes if a connection does not already exist, not the same rooms,
            //and the potential connection has room for a new connection
            if (!DoesConnectionExist(x, GeneratedMapList[selectedRoom]) &&
                    !IsSameRoom(x, GeneratedMapList[selectedRoom]) &&
                    IsNewConnectionAvailable(GeneratedMapList[selectedRoom]))
            {
                //add a new connection to x
                x->connections[x->numConnections] = GeneratedMapList[selectedRoom];
                //increase the number of connections in x
                x->numConnections++;
                //add a new connection to the randomly selected room 
                GeneratedMapList[selectedRoom]->connections[GeneratedMapList[selectedRoom]->numConnections] = x;
                //increase the number of connections in the randomly selected room 
                GeneratedMapList[selectedRoom]->numConnections++;
                
                madeConnection = TRUE;
            }
        }
        while (madeConnection == FALSE);
    }
}

/*******************************************************************************
* Function Name: GetRandomRoom 
* Description: generates a random number from 0 to the limiter 
* Arguments: integer that is max range for the random value 
* Return: randomly generated integer 
*******************************************************************************/
int GetRandomRoom(int limiter)
{
    return rand() % limiter;
}

/*******************************************************************************
* Function Name: GenerateMap 
* Description: chooses 7 rooms from the pool and connects them 
* Arguments: none 
* Return: none 
*******************************************************************************/
void GenerateMap()
{
    int i, j, k, randRoom;

    //set all rooms to not initialized
    for (i = 0; i < NUM_ROOM_POOL; i++)
    {
        RoomPool[i].isInitialized == FALSE;
    }

    //choose 7 random rooms from the room pool
    for (j = 0; j < MAX_ROOMS; j++)
    {
        bool roomAdded = FALSE;

        do 
        {
            //get a random room from the pool
            randRoom = GetRandomRoom(NUM_ROOM_POOL);
          
            //execute if the room has not already been added to the map
            if (RoomPool[randRoom].isInitialized == FALSE)
            {
                //set the number of outgoing connections to 0
                RoomPool[randRoom].numConnections = 0;
               
                //add the name of the the chosen pool item to the room
                strcpy(RoomPool[randRoom].name, ROOM_NAMES[randRoom]); 

                //chooses the type of room based on the loop counter
                //ie. the first room is START, last is END, and rest are MID
                if (j == 0)
                {
                    RoomPool[randRoom].type = START_ROOM;
                }
                else if (j == MAX_ROOMS - 1)
                {
                    RoomPool[randRoom].type = END_ROOM;
                }
                else
                {
                    RoomPool[randRoom].type = MID_ROOM;
                }
               
                //set the room as initialized
                RoomPool[randRoom].isInitialized = TRUE;
               
                //add a pointer in the map array to the chosen room
                GeneratedMapList[j] = &RoomPool[randRoom];

                //toggle loop condition
                roomAdded = TRUE;
            }
        }
        while (roomAdded == FALSE);
    }

    //add a new connection to a random room until the map is filled
    do
    {
        ConnectRooms(GeneratedMapList[GetRandomRoom(MAX_ROOMS)]);
    }
    while (!IsMapFull());
}

/*******************************************************************************
* Function Name: CreateDirectory 
* Description: makes a new directory with the pid of the current process 
* Arguments: none 
* Return: integer representing success/failure
*******************************************************************************/
int CreateDirectory()
{
    //the static string that will be used for each run of the program
    char* permanentDir = "./cartjaco.rooms.";

    //add the pid to the end of the static string
    sprintf(directoryName, "%s%d", permanentDir, getpid());

    return mkdir(directoryName, 0755);
}

/*******************************************************************************
* Function Name: GenerateFiles 
* Description: creates all room files to match the generated map 
* Arguments: none 
* Return: none 
*******************************************************************************/
void GenerateFiles()
{
    int i, j;
    //holds success value of creating a new directory
    int generatedDir = CreateDirectory();
    //holds the name of a file to be created
    char newFilePath[32];
    char writeBuffer[256];
    //filestream object
    FILE* fout;

    //executes upon successful navigation to the new directory
    if (chdir(directoryName) == 0)
    {
        //loop over each room in the map
        for (i = 0; i < MAX_ROOMS; i++)
        {
            //copy the name of the room to the name of the file to open
            strcpy(newFilePath, GeneratedMapList[i]->name);

            //open the file
            fout = fopen(newFilePath, "w"); 

            if (fout == 0)
            {
                perror("File could not be opened\n");
            }

            //clean out the writeBuffer
            memset(writeBuffer, '\0', sizeof(writeBuffer));
            
            //write the room name to the file
            sprintf(writeBuffer, "ROOM NAME: %s\n", GeneratedMapList[i]->name);
            fputs(writeBuffer, fout);

            for (j = 0; j < GeneratedMapList[i]->numConnections; j++)
            {
                memset(writeBuffer, '\0', sizeof(writeBuffer));
           
                //write the connection name and number to the file 
                sprintf(writeBuffer, "CONNECTION %d: %s\n", j + 1, GeneratedMapList[i]->connections[j]->name);
                fputs(writeBuffer, fout);
            }

            memset(writeBuffer, '\0', sizeof(writeBuffer));

            //get the type of the room
            if (GeneratedMapList[i]->type == START_ROOM)
            {
                sprintf(writeBuffer, "ROOM TYPE: START_ROOM\n"); 
            }
            else if (GeneratedMapList[i]->type == END_ROOM)
            {
                sprintf(writeBuffer, "ROOM TYPE: END_ROOM\n"); 
            }
            else
            {
                sprintf(writeBuffer, "ROOM TYPE: MID_ROOM\n");
            }
            //write the room type to the file 
            fputs(writeBuffer, fout);
  
        //close the file
        fclose(fout);
        }
    }
    else
    {
        printf("Directory could not be opened\n");
        return;
    }
}

int main()
{
    //provide a seed for random number generation
    srand(time(0));

    GenerateMap();
   
    GenerateFiles();

    return 0;
}
