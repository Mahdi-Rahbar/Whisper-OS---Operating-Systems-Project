//Madhdi Rahbar && Amir Reza Najafi


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <direct.h> 
#include <windows.h>
#include <time.h>

	 


#define PATH_SEPARATOR "\\"
#define MAX_FILES 100
#define MAX_FILENAME 50
#define MAX_CONTENT 1024



// Define color codes for better shell display
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"
#define RESET "\033[0m"



typedef struct {
    char name[MAX_FILENAME];
    char content[MAX_CONTENT];
    int isDeleted;
    int isOnDisk;
} File;



File *fileSystem[MAX_FILES];
int fileCount = 0;
char currentDirectory[MAX_FILENAME] = "root"; 





void initializeFileSystem() {
    for (int i = 0; i < MAX_FILES; i++) {
        fileSystem[i] = NULL;
    }
#ifdef _WIN32
    _mkdir(currentDirectory); 

#endif
}





void saveFileToDisk(File *file) {
    char fullPath[MAX_FILENAME * 2];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDirectory, file->name);

    FILE *diskFile = fopen(fullPath, "w");
    if (diskFile) {
        fprintf(diskFile, "%s", file->content);
        fclose(diskFile);

        file->isOnDisk = 1; 
        file->content[0] = '\0'; 
        printf(YELLOW "File '%s' saved to disk.\n" RESET, file->name);
    } else {
        printf(RED "Error: Could not save file '%s' to disk.\n" RESET, file->name);
    }
}






void loadFileFromDisk(File *file) {
    char fullPath[MAX_FILENAME * 2];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDirectory, file->name);

    FILE *diskFile = fopen(fullPath, "r");
    if (diskFile) {
        fread(file->content, 1, MAX_CONTENT, diskFile);
        fclose(diskFile);

        file->isOnDisk = 0; 
        printf(GREEN "File '%s' loaded from disk.\n" RESET, file->name);
    } else {
        printf(RED "Error: Could not load file '%s' from disk.\n" RESET, file->name);
    }
}




void manageMemory() {
    int saved = 0;
    if (fileCount >= MAX_FILES) {
        for (int i = 0; i < MAX_FILES; i++) {
            if (fileSystem[i] != NULL && !fileSystem[i]->isOnDisk) {
                saveFileToDisk(fileSystem[i]); 
                saved = 1;
                return;
            }
        }
        if (!saved) {
            printf(RED "Error: Memory is full, and all files are already saved to disk.\n" RESET);
        }
    }
}






void accessFile(File *file) {
    if (file->isOnDisk) {
        loadFileFromDisk(file);
    }
}






void loadFilesFromDirectory() {
    #ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    char searchPath[MAX_FILENAME * 2];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", currentDirectory);

    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf(RED "Error: Could not open directory '%s'.\n" RESET, currentDirectory);
        return;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            for (int i = 0; i < MAX_FILES; i++) {
                if (fileSystem[i] == NULL) {
                    fileSystem[i] = (File *)malloc(sizeof(File));
                    strncpy(fileSystem[i]->name, findFileData.cFileName, MAX_FILENAME);
                    fileSystem[i]->content[0] = '\0'; 
                    fileSystem[i]->isDeleted = 0;
                    fileSystem[i]->isOnDisk = 1; 
                    fileCount++;
                    break;
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    #else
    printf(RED "Error: Directory loading not implemented for non-Windows systems.\n" RESET);
    #endif
}









int isValidFilename(const char *name) {
    for (int i = 0; name[i] != '\0'; i++) {
        if (!isalnum(name[i]) && name[i] != '.' && name[i] != '_') {
            return 0; 
        }
    }
    return 1; 
}






void changeDirectory(const char *dir) {
    if (strcmp(dir, "..") == 0) {
        
        char *lastSlash = strrchr(currentDirectory, '\\');
        if (lastSlash != NULL) {
            *lastSlash = '\0'; 
        } else {
            strcpy(currentDirectory, "root");
        }
    } else {
        char newPath[MAX_FILENAME * 2];
        snprintf(newPath, sizeof(newPath), "%s", currentDirectory);

        
        char tempDir[MAX_FILENAME * 2];
        snprintf(tempDir, sizeof(tempDir), "%s", dir);

        char *token = strtok(tempDir, "/\\"); 
        while (token != NULL) {
            char intermediatePath[MAX_FILENAME * 2];
            if (snprintf(intermediatePath, sizeof(intermediatePath), "%s\\%s", newPath, token) >= sizeof(intermediatePath)) {
                printf(RED "Error: Path is too long.\n" RESET);
                return;
            }

            
#ifdef _WIN32
            DWORD attributes = GetFileAttributes(intermediatePath);
            if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
                printf(RED "Error: Directory '%s' does not exist.\n" RESET, token);
                return;
            }
#else
            struct stat st;
            if (stat(intermediatePath, &st) != 0 || !S_ISDIR(st.st_mode)) {
                printf(RED "Error: Directory '%s' does not exist.\n" RESET, token);
                return;
            }
#endif
            snprintf(newPath, sizeof(newPath), "%s", intermediatePath);
            token = strtok(NULL, "/\\"); 
        }

       
        snprintf(currentDirectory, sizeof(currentDirectory), "%s", newPath);
    }

    printf(CYAN "Current directory: %s\n" RESET, currentDirectory);
}








void makeDirectory(const char *dir) {
    if (!isValidFilename(dir)) {
        printf(RED "Error: Invalid directory name. Only alphanumeric characters, dots, and underscores are allowed.\n" RESET);
        return;
    }



    char fullPath[MAX_FILENAME * 2];
    
    snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDirectory, dir);
    

#ifdef _WIN32
    if (_mkdir(fullPath) == 0) {
#else
    if (mkdir(fullPath, 0777) == 0) {
#endif
        printf(GREEN "Directory '%s' created successfully in %s.\n" RESET, dir, currentDirectory);
    } else {
        printf(RED "Error: Could not create directory '%s'. It might already exist.\n" RESET, dir);
    }
}






void createFile(char *name) {
	
	manageMemory(); 
	
    if (!isValidFilename(name)) {
        printf(RED "Error: Invalid filename. Only alphanumeric characters, dots, and underscores are allowed.\n" RESET);
        return;
    }

    if (fileCount >= MAX_FILES) {
        printf(RED "Error: File system is full.\n" RESET);
        return;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (fileSystem[i] != NULL && strcmp(fileSystem[i]->name, name) == 0) {
            printf(RED "Error: File already exists.\n" RESET);
            return;
        }
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (fileSystem[i] == NULL) {
            fileSystem[i] = (File *)malloc(sizeof(File));
            if (!fileSystem[i]) {
                printf(RED "Error: Memory allocation failed.\n" RESET);
                return;
            }
            strcpy(fileSystem[i]->name, name);
            fileSystem[i]->content[0] = '\0';
            fileSystem[i]->isDeleted = 0;
            fileSystem[i]->isOnDisk = 0;
            fileCount++;

      
            char fullPath[MAX_FILENAME * 2];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDirectory, name);
            FILE *file = fopen(fullPath, "w");
            if (file) {
                fclose(file);
                printf(GREEN "File '%s' created successfully in %s.\n" RESET, name, currentDirectory);
            } else {
                printf(RED "Error: Could not create file '%s' on disk.\n" RESET, name);
            }
            return;
        }
    }
}







void deleteFile(char *name) {
    char fullPath[MAX_FILENAME * 2];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDirectory, name);

    for (int i = 0; i < MAX_FILES; i++) {
        if (fileSystem[i] != NULL && strcmp(fileSystem[i]->name, name) == 0) {
            if (remove(fullPath) == 0) {
                free(fileSystem[i]);
                fileSystem[i] = NULL;
                fileCount--;
                printf(GREEN "File '%s' deleted successfully from %s.\n" RESET, name, currentDirectory);
            } else {
                printf(RED "Error: Could not delete file '%s' from disk.\n" RESET, name);
            }
            return;
        }
    }
    printf(RED "Error: File not found.\n" RESET);
}






void renameFile(char *oldName, char *newName) {
    if (!isValidFilename(newName)) {
        printf(RED "Error: Invalid new filename. Only alphanumeric characters, dots, and underscores are allowed.\n" RESET);
        return;
    }

    char oldPath[MAX_FILENAME * 2], newPath[MAX_FILENAME * 2];
    snprintf(oldPath, sizeof(oldPath), "%s/%s", currentDirectory, oldName);
    snprintf(newPath, sizeof(newPath), "%s/%s", currentDirectory, newName);

    for (int i = 0; i < MAX_FILES; i++) {
        if (fileSystem[i] != NULL && strcmp(fileSystem[i]->name, oldName) == 0) {
            strcpy(fileSystem[i]->name, newName);

            if (rename(oldPath, newPath) == 0) {
                printf(GREEN "File '%s' renamed to '%s' in %s.\n" RESET, oldName, newName, currentDirectory);
            } else {
                printf(RED "Error: Could not rename file '%s' to '%s' on disk.\n" RESET, oldName, newName);
            }
            return;
        }
    }
    printf(RED "Error: File not found.\n" RESET);
}




void listFiles() {
 WIN32_FIND_DATA findFileData;
    HANDLE hFind;

    char searchPath[MAX_FILENAME * 2];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", currentDirectory);

    hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf(RED "Error: Could not open directory '%s'.\n" RESET, currentDirectory);
        return;
    }

    
    printf(YELLOW "\nFiles in directory '%s':\n" RESET, currentDirectory);
    printf("File Name            Size (KB)   Last Modified          Format      \n");
    printf("----------------------------------------------------------------------\n");

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
           
            char fileName[21]; 
            char *extension = strrchr(findFileData.cFileName, '.'); 
            if (extension) {
                size_t nameLength = extension - findFileData.cFileName; 
                if (nameLength > 20) nameLength = 17; 
                strncpy(fileName, findFileData.cFileName, nameLength);
                if (nameLength == 17) strcpy(&fileName[17], "..."); 
                else fileName[nameLength] = '\0'; 
                extension++; 
            } else {
                
                strncpy(fileName, findFileData.cFileName, 20);
                fileName[20] = '\0';
                extension = "Unknown"; 
            }

            
            ULONGLONG fileSizeBytes = ((ULONGLONG)findFileData.nFileSizeHigh << 32) + findFileData.nFileSizeLow;
            DWORD fileSizeInKB = (fileSizeBytes + 1023) / 1024;

          
            FILETIME localFileTime;
            FileTimeToLocalFileTime(&findFileData.ftLastWriteTime, &localFileTime);
            SYSTEMTIME st;
            FileTimeToSystemTime(&localFileTime, &st);
            char lastModified[21];
            snprintf(lastModified, sizeof(lastModified), "%04d-%02d-%02d %02d:%02d",
                     st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

          
            printf("%-20s %-10lu %-20s      %-10s\n",
                   fileName, fileSizeInKB, lastModified, extension);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

   
    FindClose(hFind);

    
    printf("----------------------------------------------------------------------\n");
}





void writeFile(char *name, char *content) {
	
	manageMemory();
	
    char fullPath[MAX_FILENAME * 2];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", currentDirectory, name);

    for (int i = 0; i < MAX_FILES; i++) {
        if (fileSystem[i] != NULL && strcmp(fileSystem[i]->name, name) == 0) {
            strncpy(fileSystem[i]->content, content, MAX_CONTENT);

            FILE *file = fopen(fullPath, "w");
            if (file) {
                fprintf(file, "%s", content);
                fclose(file);
                printf(GREEN "Content written to '%s' in %s.\n" RESET, name, currentDirectory);
            } else {
                printf(RED "Error: Could not write to file '%s' on disk.\n" RESET, name);
            }
            return;
        }
    }
    printf(RED "Error: File not found.\n" RESET);
}





void readFile(char *name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (fileSystem[i] != NULL && strcmp(fileSystem[i]->name, name) == 0) {
            if (fileSystem[i]->isOnDisk) {
                manageMemory();
                loadFileFromDisk(fileSystem[i]);
            }

            printf(YELLOW "Content of '%s':\n%s\n" RESET, name, fileSystem[i]->content);
            return;
        }
    }
    printf(RED "Error: File not found.\n" RESET);
}






void runFile(char *name) {
    char fullPath[MAX_FILENAME * 2];
   
    snprintf(fullPath, sizeof(fullPath), "%s\\%s", currentDirectory, name);


    for (int i = 0; i < MAX_FILES; i++) {
        if (fileSystem[i] != NULL && strcmp(fileSystem[i]->name, name) == 0) {
            printf(YELLOW "Executing file '%s':\n" RESET, name);

            if (system(fullPath) == 0) {
                printf(GREEN "File '%s' executed successfully.\n" RESET, name);
            } else {
                printf(RED "Error: Could not execute file '%s'. Make sure it's executable.\n" RESET, name);
            }
            return;
        }
    }
    printf(RED "Error: File not found.\n" RESET);
}





void showHelp() {
    printf(YELLOW "\nWhisper OS Help Menu:\n" RESET);
    printf("========================================\n");
    printf(GREEN "File Commands:\n" RESET);
    printf("  CREATE <filename>       - Create a new file\n");
    printf("  DELETE <filename>       - Delete a file\n");
    printf("  RENAME <old> <new>      - Rename a file\n");
    printf("  WRITE <filename> <text> - Write content to a file\n");
    printf("  READ <filename>         - Read content of a file\n");
    printf("  RUN <filename>          - Execute a file\n\n");

    printf(GREEN "Directory Commands:\n" RESET);
    printf("  DIR                     - List all files\n");
    printf("  CD <directory>          - Change directory\n");
    printf("  MKDIR <directory>       - Create a new directory\n\n");

    printf(GREEN "Other Commands:\n" RESET);
    printf("  HELP                    - Show this help menu\n");
    printf("  EXIT                    - Exit the Whisper OS\n");
    printf("========================================\n");
    
}



void toUpperCase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}






void commandHandler(char *command) {
    char cmd[20], arg1[MAX_FILENAME], arg2[MAX_FILENAME + MAX_CONTENT] = "";

    sscanf(command, "%s", cmd);
    toUpperCase(cmd);

    if (strcmp(cmd, "CREATE") == 0) {
        sscanf(command, "%*s %s", arg1);
        createFile(arg1);
    } else if (strcmp(cmd, "DELETE") == 0) {
        sscanf(command, "%*s %s", arg1);
        deleteFile(arg1);
    } else if (strcmp(cmd, "RENAME") == 0) {
        sscanf(command, "%*s %s %s", arg1, arg2);
        renameFile(arg1, arg2);
    } else if (strcmp(cmd, "DIR") == 0) {
        
        listFiles();
    } else if (strcmp(cmd, "CD") == 0) {
        sscanf(command, "%*s %s", arg1);
        changeDirectory(arg1);
    } else if (strcmp(cmd, "MKDIR") == 0) {
        sscanf(command, "%*s %s", arg1);
        makeDirectory(arg1);
    } else if (strcmp(cmd, "WRITE") == 0) {
        char *textStart = strchr(command, '"');
        if (textStart) {
            char *textEnd = strrchr(command, '"');
            if (textEnd && textEnd > textStart) {
                *textEnd = '\0';
                strcpy(arg2, textStart + 1);
                sscanf(command, "%*s %s", arg1);
                writeFile(arg1, arg2);
                return;
            }
        }
        printf(RED "Error: Invalid WRITE command format. Use: WRITE <filename> \"content\"\n" RESET);
    } else if (strcmp(cmd, "READ") == 0) {
        sscanf(command, "%*s %s", arg1);
        readFile(arg1);
    } else if (strcmp(cmd, "RUN") == 0) {
        sscanf(command, "%*s %s", arg1);
        runFile(arg1);
    } else if (strcmp(cmd, "HELP") == 0) {
        
        showHelp();
    } else if (strcmp(cmd, "EXIT") == 0) {
        printf(GREEN "Exiting Whisper OS...\n" RESET);
        exit(0);
    } else {
        printf(RED "Error: Unknown command. Type HELP for a list of commands.\n" RESET);
    }
}







int main() {
    char command[256];

    initializeFileSystem();
    loadFilesFromDirectory();
    printf(GREEN "Welcome to the Whisper OS! Type HELP for a list of commands.\n" RESET);

    while (1) {
        printf(CYAN "\n%s> " RESET, currentDirectory);
        if (fgets(command, sizeof(command), stdin)) {
            command[strcspn(command, "\n")] = '\0'; 
            commandHandler(command); 
        }
    }

    return 0;
}