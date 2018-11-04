/*******************************************************************************
* Program Name: cartjaco.adventure
* Author: Jacob Carter
* Date Modified: 10/25/2018
* Course: CS344
* Description: A playable game in which the player must navigate a series of 
* connected rooms to find the end
*******************************************************************************/
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

//define global constant variables
#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define MAX_ROOMS 7
#define NUM_ROOM_POOL 10
#define NAME_SIZE 9

//global enumeration to make defininig room type easier
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
struct ROOM SelectedRooms[MAX_ROOMS];

// the name of the directory in which to write files 
char directoryName[256];
//title of the time file
char* tFile = "currentTime.txt";
//mutex to lock thread when needed
pthread_mutex_t timeLock = PTHREAD_MUTEX_INITIALIZER;
//name of thread
pthread_t timeThread;

/*******************************************************************************
* Function Name: GetStart
* Description: finds the starting room for the game 
* Arguments: none 
* Return: int index of the start room
*******************************************************************************/
struct ROOM* GetStart()
{
    int i;
    struct ROOM* startRoom;

    //loop through each member of the array and search for type "START_ROOM"
    for (i = 0; i < MAX_ROOMS; i++)
    {
        if (SelectedRooms[i].type == START_ROOM)
        {
            startRoom = &SelectedRooms[i];
            return startRoom;
        }
    }
}

/*******************************************************************************
* Function Name: PrintPossibleConnections
* Description: Lists all possible connections from the passed in room 
* Arguments: pointer to a struct 
* Return: none 
*******************************************************************************/
void PrintPossibleConnections(struct ROOM* x)
{
    int i;

    //loop through all number of connections
    for (i = 0; i < x->numConnections; i++)
    {
        //if not the last connection, append a comma after the connection
        if (i != x->numConnections - 1)
        {
            printf("%s, ", x->connections[i]->name);
        }
        //if the last connection, append a period
        else
        {
            printf("%s.", x->connections[i]->name);
        } 
    }
}

/*******************************************************************************
* Function Name: WriteTimeFile 
* Description: creates a text file to hold the current time when requested by
* the user
* Arguments: none 
* Return: none 
*******************************************************************************/
void* WriteTimeFile()
{
    char timeString[256];
    time_t currentTime;
    struct tm* timeStats;
    FILE* timeFile;

    //clean up the timeString before use
    memset(timeString, '\0', sizeof(timeString));

    //get the current time
    time(&currentTime);
    //adjust the current time to the user's local time
    timeStats = localtime(&currentTime);
    //format the time string as specified in the assignment directives
    strftime(timeString, 256, "%I:%M%P, %A, %B %d, %Y", timeStats);

    //open the global time file for writing
    timeFile = fopen(tFile, "w");
    //write the current time to the file
    fprintf(timeFile, "%s\n", timeString);
    //close the time file
    fclose(timeFile);
}

/*******************************************************************************
* Function Name: ReadTimeFile 
* Description: reads the value in the time file to the screen 
* Arguments: none 
* Return: none 
*******************************************************************************/
void ReadTimeFile()
{
    char timeBuffer[256];
    FILE* timeFile;

    //clean up the timeBuffer before use
    memset(timeBuffer, '\0', sizeof(timeBuffer));

    //open the time file
    timeFile = fopen(tFile, "r");

    //execute if the file opened successfully 
    if (!timeFile == 0)
    {
        //put each line of the file into timeBuffer and print
        while (fgets(timeBuffer, 256, timeFile))
        {
            printf("%s\n", timeBuffer);
        }

        fclose(timeFile);
    }
    else
    {
        printf("Unable to read file: %s\n", tFile);
    }
}

/*******************************************************************************
* Function Name: ManageThreads 
* Description: handles the switching of threads throughout the program 
* Arguments: none 
* Return: none 
* Source: Course Lecture 2.3 - Concurrency, instruction by Ben Brewster
*******************************************************************************/
void ManageThreads()
{
    //unlock the mutex
    pthread_mutex_unlock(&timeLock);
    //join the time thread, will block the main thread until complete
    pthread_join(timeThread, 0);
    //re-lock the mutex
    pthread_mutex_lock(&timeLock);
    //re-create the time thread for use in the next call
    pthread_create(&timeThread, 0, WriteTimeFile, 0);
}

/*******************************************************************************
* Function Name: GetFolderToUse 
* Description: finds the most current room folder to run the program with 
* Arguments: none 
* Return: none 
* Source: Course Reading 2.4, implementation written by Ben Brewster
*******************************************************************************/
void GetFolderToUse()
{
    //timestamp of newest subdirectory
    time_t newestTimeDir = -1;
    char* permanentDir = "cartjaco.rooms.";
    //the name of the newset directory found
    char newDirName[256];
    //the directory to start in
    DIR* currentDir;
    //the current subdirectory of the starting directory
    struct dirent* dirStream;
    //information about the above subdirectory
    struct stat dirAttributes;

    //clean out strings to be used
    memset(directoryName, '\0', sizeof(directoryName));
    memset(newDirName, '\0', sizeof(newDirName));

    //open the directory that the program exists in
    currentDir = opendir(".");

    //execute if the directory was successfully opened
    if (currentDir != 0)
    {
        //loop over every entry in the directory
        while (dirStream = readdir(currentDir))
        {
            //execute if the name of the directory matches the string searched for
            if (strstr(dirStream->d_name, permanentDir) != 0)
                {
                    //get information about the selected directory
                    stat(dirStream->d_name, &dirAttributes);

                    //execute if the time of the selected directory is larger
                    //than that of the current newest directory
                    if ((int)dirAttributes.st_mtime > newestTimeDir)
                    {
                        //set the newest directory to the selected directory
                        newestTimeDir = (int)dirAttributes.st_mtime;
                        //clean out the name string before copying new name
                        memset(newDirName, '\0', sizeof(newDirName));
                        //copy the name of the selected directory
                        strcpy(newDirName, dirStream->d_name);
                    }
                }
            }
    }

    //close the directory stream
    closedir(currentDir);
   
    //copy the name of the newest directory into the global directoryName
    strcpy(directoryName, newDirName);
}

/*******************************************************************************
* Function Name: GetRoomIndex
* Description: finds the index of a room in the map 
* Arguments: string holding the name of the room to find 
* Return: integer index of the room 
*******************************************************************************/
int GetRoomIndex(char* roomToFind)
{
    int i;

    //loop over each room in the map
    for (i = 0; i < MAX_ROOMS; i++)
    {
        //copy the room index, if the searched name matches the room name
        if (strcmp(roomToFind, SelectedRooms[i].name) == 0)
        {
            return i;
        }
    }
}

/*******************************************************************************
* Function Name: CreateRooms
* Description:  
* Arguments: string holding the name of the room to find 
* Return: integer index of the room 
*******************************************************************************/
void CreateRooms()
{
    int i = 0;
    DIR* roomDir;
    struct dirent *dirStream;
    
    roomDir = opendir(directoryName);

    if (roomDir != 0)
    {
        while ((dirStream = readdir(roomDir)) != 0)
        {
            strcpy(SelectedRooms[i].name, dirStream->d_name);
            i++;
        }
    }
}

/*******************************************************************************
* Function Name: InitializeRooms
* Description: sets default values of rooms before they are used 
* Arguments: none 
* Return: none 
*******************************************************************************/
void InitializeRooms()
{
    int i, j;

    for (i = 0; i < MAX_ROOMS; i++)
    {
        memset(SelectedRooms[i].name, '\0', sizeof(SelectedRooms[i].name));
        
        SelectedRooms[i].numConnections = 0;
        
        for (j = 0; j < MAX_CONNECTIONS; j++)
        {
            SelectedRooms[i].connections[j] = 0;
        }
        
        SelectedRooms[i].isInitialized = TRUE;
    }
}

/*******************************************************************************
* Function Name: GenerateMap
* Description: creates the map that the game will be played on, using room files 
* Arguments: none 
* Return: none 
*******************************************************************************/
void GenerateMap()
{
    //holds a line from a room file
    char line[512];
    //holds the type of information stripped from a line
    char *lineBuffer;
    //holds the value of the information stripped from a line
    char *wordBuffer;
    //the room file to open
    FILE* roomFile;
    int i = 0;
    int j;
    DIR* roomDir;
    struct dirent *dirStream;
   
    //open the directory for the most recent room files
    roomDir = opendir(directoryName);

    InitializeRooms();

    
    if (roomDir != 0)
    {
        //loop through all files in the directory
        while ((dirStream = readdir(roomDir)) != 0)
        {
            //skip the current and previous directories
            if (strlen(dirStream->d_name) > 2)
            {
                //copy the name of the file to a room in the map
                strcpy(SelectedRooms[i].name, dirStream->d_name);
                i++;
            }
        }
    }

    //move into the directory holding the room files
    if (chdir(directoryName) == 0)
    {
        for (j = 0; j < MAX_ROOMS; j++)
        {
            //open the room file matching the name of the current room
            roomFile = fopen(SelectedRooms[j].name, "r");

            if (roomFile != 0)
            {
                //loop over each line in the file
                while (fgets(line, sizeof(line), roomFile))
                {
                    //cut the line as delimited by the colon
                    lineBuffer = strtok(line, ":");
                    //put the rest of the line into the wordbuffer
                    wordBuffer = strtok(0, "");
                    //remove the leading space from the value in wordBuffer
                    char* formattedValue = wordBuffer + 1;
                    //remove the trailing new line from the formatted value
                    strtok(formattedValue, "\n");
                   
                    //compare the lineBuffer to find what kind of information is
                    //held in the current line
                    if (strstr(lineBuffer, "CONNECTION "))
                    {
                        //get the index of the room that matches the name value in the file
                        int index = GetRoomIndex(formattedValue);
                        //add a connection from the operative room to the indexed connection
                        SelectedRooms[j].connections[SelectedRooms[j].numConnections] = &SelectedRooms[index];
                        SelectedRooms[j].numConnections++;
                    }
                    else if (strcmp(lineBuffer, "ROOM TYPE") == 0)
                    {
                        //set the type of room based on file value
                        if (strcmp(formattedValue, "START_ROOM") == 0)
                        {
                            SelectedRooms[j].type = START_ROOM;
                        }
                        else if (strcmp(formattedValue, "END_ROOM") == 0)
                        {
                            SelectedRooms[j].type = END_ROOM;
                        }
                        else
                        {
                            SelectedRooms[j].type = MID_ROOM;
                        }
                    }
                }

                //close the room file
                fclose(roomFile);
            }
            //print message if the file could not be opened
            else
            {
                printf("Unable to open file: %s\n", SelectedRooms[j].name);
            }
        }
    }

    //move back to the program directory
    chdir("..");
}

/*******************************************************************************
* Function Name: PlayGame
* Description: runs the game simulation by taking user input 
* Arguments: none 
* Return: none 
*******************************************************************************/
void PlayGame()
{
    //array of room names to hold the player's path
    char pathTaken[256][NAME_SIZE];
    //holds the users's input
    char inputBuffer[256];
    //pointer to the current location of the player
    struct ROOM* currentLocation = GetStart();
    int stepCount = 0;
    int i, j;
    bool foundEnd = FALSE;
    bool roomMatch;

    do
    {
        //clean out the inputBuffer
        memset(inputBuffer, '\0', sizeof(inputBuffer));
       
        //set that no room is found
        roomMatch = FALSE;

        printf("CURRENT LOCATION: %s\n", currentLocation->name);
        
        printf("POSSIBLE CONNECTIONS: "); 
        PrintPossibleConnections(currentLocation);
        printf("\n");
        
        printf("WHERE TO? >");
        //get the users input
        scanf("%s", inputBuffer);
        printf("\n");

        //execute if the input is to get the time
        if (strcmp(inputBuffer, "time") == 0)
        {
            ManageThreads();
            ReadTimeFile();
            roomMatch = TRUE;
        }
        else
        {
            //loop over each connection for the current location
            for (i = 0; i < currentLocation->numConnections; i++)
            {
                //execute if the user's input matches a valid connection
                if (strcmp(inputBuffer, currentLocation->connections[i]->name) == 0)
                {
                    roomMatch = TRUE;
                    //set the current location to the chosen connection
                    currentLocation = currentLocation->connections[i];
                    //add the chosen connection to the total path taken
                    strcpy(pathTaken[stepCount], currentLocation->name);
                    stepCount++;
                   
                    //execute if the user is in the ending room
                    if (currentLocation->type == END_ROOM)
                    {
                        printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
                        printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);

                        //print the total path taken to get to the end
                        for (j = 0; j < stepCount; j++)
                        {
                            printf("%s\n", pathTaken[j]);
                        }

                        foundEnd = TRUE;
                    }
                }
            }

            //execute if the user input is invalid
            if (roomMatch == FALSE)
            {
                printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN\n\n");
            }
        }
    }
    while (foundEnd != TRUE);
}

int main()
{
    pthread_mutex_lock(&timeLock);
    pthread_create(&timeThread, 0, WriteTimeFile, 0);
    
    GetFolderToUse();
    
    GenerateMap();

    PlayGame();

    return 0;
}
