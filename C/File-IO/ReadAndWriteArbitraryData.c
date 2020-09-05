// /////////////////////////////////////////////////////////////////////////////////////////////////
// Program to read and write arbitrary data
// /////////////////////////////////////////////////////////////////////////////////////////////////
//
// Write a program in C that reads arbitrary data from standard input and writes it to a file. 
// By arbitrary data, we mean that we don’t want to put any limitations or restrictions whatsoever 
// on the data a user provides via standard input. 
//
// /////////////////////////////////////////////////////////////////////////////////////////////////
// OS verified: Windows, Linux
// Compilers verified: cl for Windows, gcc for Linux
// /////////////////////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/stat.h>

#define ONE_BYTE 1
#define ONE_KILO_BYTE 1024

#define FREE(x) { if(x) free(x); }
#define FCLOSE(x) { if(x) fclose(x); }	

// Returns a pointer to the extension of 'string'.
// If no extension is found, returns a pointer to the end of 'string'. 
char* getFileExtension(const char *pFileName)
{
    assert(pFileName != NULL);
    char *pFileExtension = strrchr(pFileName, '.');
 
    if (pFileExtension == NULL)
        return (char *) pFileName + strlen(pFileName);
 
    for (char *pIterator = pFileExtension + 1; *pIterator != '\0'; pIterator++)
	{
        if (!isalnum((unsigned char)*pIterator))
            return (char *) pFileName + strlen(pFileName);
    }
 
    return pFileExtension;
}

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char * trimLeadingAndTrailingWhiteSpaces(char *pString)
{
	char *pLastChar;
	
	// Trim leading spaces
	while(isspace((unsigned char)*pString))
		pString++;
	
	// if input string has only White spaces, we can simply return here 
	if(NULL == pString)	
		return pString;
		
	// Trim trailing spaces
	pLastChar = pString + strlen(pString) - 1;
	while(pLastChar > pString && isspace((unsigned char)*pLastChar))
		pLastChar--;
	
  // Write null terminator after the last character 
  *(pLastChar+1) = '\0';

  return pString;
}

// Returns true if file name string contains valid characters 
// Returns false if invalid characters are found in file name string
bool isValidFileName(char *pFileName)
{
	char invalidCharacters[] = "<> :\"/\\|?*";
	bool invalidCharFound = false;
	
	for (int i = 0; i < strlen(invalidCharacters); ++i)
	{
		if (strchr(pFileName, invalidCharacters[i]) != NULL)
		{
			printf("Characters <>:\"/\\|?*] are not allowed in file name. ");
			invalidCharFound = true;
			break;
		}
	}
	return !invalidCharFound;
}

// Returns true if given file exists
// Returns false if given file not found
bool isFileExists(char *pFileName) {
	
	if(NULL == pFileName)
		return false;
	
	struct stat buffer;
	bool fileExists = (stat(pFileName, &buffer) == 0);
	
	if(!fileExists)
		printf("File not found! ");
	else
		printf("File '%s' exists\n",pFileName);
	
	return fileExists;
}

// Gets file name string from standard input and returns a pointer to the dynamically allocated string.
// Clients of this function should should take care of deallocating memory by calling free  
char * getFileName(const char *pFileDescription)
{
	char *pFileName= (char *)malloc(FILENAME_MAX); 
	printf("Please enter a valid '%s' file name (Max %d characters only): ", pFileDescription, FILENAME_MAX);
	fgets(pFileName, FILENAME_MAX, stdin);
	
	//fegets adds a new line character at the end, so using strchr to find and replace it with '\0' 
	char *pNewLinePosition=strchr(pFileName, '\n');
	if (NULL != pNewLinePosition)
		*pNewLinePosition = '\0';
	
	return pFileName;
}

// Returns a string with output file name
// If the extension of output file name provided by user doesn't match with the input parameter 'pFileExtension', a warning is displayed to the user.
char * getOutputFileName(const char *pFileExtension)
{
	char *pOutputFileName = NULL, *pOutputFileExt = NULL;
	
	do
	{
		pOutputFileName=getFileName("Output");
	} while(!isValidFileName(pOutputFileName));
	
	pOutputFileExt = getFileExtension(pOutputFileName); //Do not free pOutputFileExt(stored in stack memory not heap) as it is only a substring of pOutputFileName
	
	if(pFileExtension)
	{
		if(strcmp(pOutputFileExt,pFileExtension) != 0)
		{
			printf("Warning: Output file might become unusable as ouput file extension '%s' is not matching with input file extension '%s'\n",pOutputFileExt,pFileExtension);
		}
	}
	
	return pOutputFileName;
}

// Returns a string with input file name
// if the given file name doesn't exists, users will be prompted to provide a valid file.
// the function returns only when a valid file name is provided.
char * getInputFileName()
{
	char *pInputFileName = NULL;
	
	do
	{
		pInputFileName = getFileName("Input");
	} while(!isFileExists(pInputFileName));
	
	return pInputFileName;
}

// Reads input data from input stream and writes the same to the output stream
// If read/write failed, the program terminates wih exit code 1; 
void writeData(FILE *pInputStream, FILE *pOutputStream)
{
	char buffer[ONE_KILO_BYTE];
	memset(buffer,'\0',sizeof(buffer));
	size_t size;
	while (size = fread(buffer, ONE_BYTE, ONE_KILO_BYTE, pInputStream))
	{
		size = fwrite(buffer, ONE_BYTE, size, pOutputStream);
		
		//HandleError
		if(ferror(pOutputStream))
		{
			printf("Write Failed\n");
			
			FCLOSE(pOutputStream);
			FCLOSE(pInputStream);
			
			exit(1);
		}
	}
	
	//HandleError
	if(ferror(pInputStream))
	{
		printf("Read Failed\n");
		fclose(pInputStream);
		exit(1);
	}
	
	printf("Success! Write Data Completed.\n");
}

int main()
{
	char *pInputFileName = NULL, *pOutputFileName = NULL, *pInputFileExt = NULL;
	FILE *pInputStream = NULL, *pOutputStream = NULL;
		
	char c;
	char str[ONE_KILO_BYTE];
	
	do
	{
		printf("Enter 0 to read from console (standard input) or 1 to provide input file name:\n");
		scanf("%s",str);
		
		// To ignore the additional newline character from previous scanf() call
		getchar();
		
		c=str[0];
		
		switch (c)
		{
		case '1':
			pInputFileName = getInputFileName();
			if(!pInputFileName)
			{
				printf("Error: Unable to get Input File Name.\n");
				//Free resources before exiting
				exit(1);
			}
			
			pInputStream = fopen(pInputFileName, "rb");
			//Handle Error

			break;
		case '0':
			pInputStream = stdin;
			break;
		default:
			printf("Invalid input! ");
			break;
		}
	} while (c != '1' && c != '0');
	
	if(pInputFileName)
	{
		pInputFileExt = getFileExtension(pInputFileName); //Do not free pInputFileExt ((stored in stack memory not heap)) as it is only a substring of pInputFileName
	}
	
	pOutputFileName	= getOutputFileName(pInputFileExt);
	//Handle Error
	if(!pOutputFileName)
	{
		printf("Error: Unable to get Output File Name.\n");
		//Free resources before exiting
		exit(1);
	}
	
	pOutputStream = fopen(pOutputFileName,"wb");
	// HandleError
	if(!pOutputStream)
	{
		printf("Error: Output File Stream pointer is NULL\n");
		//Free resources before exiting
		exit(1);
	}
	
	if(!pInputStream)
	{
		printf("Error: Input File Stream pointer is NULL\n");
		//Free resources before exiting		
		exit(1);
	}
	
	if(stdin == pInputStream)
		printf("Enter the input data (press ctrl+z twice & press 'Enter' to process input): \n");
	
	writeData(pInputStream, pOutputStream);
	
	//FREE(pInputFileExt);	
	
	FREE(pInputFileName);
    FREE(pOutputFileName);
	
	FCLOSE(pInputStream);
	FCLOSE(pOutputStream);
	
	printf("Done\n");	
	
    return 0;
}
