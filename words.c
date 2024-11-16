#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>

/**This defines the structure of  a typical word and its frequency.**/
struct wordNode
{
    char * phrase; //This is the word itself.
    unsigned int frequency; //This is the frequency of the unique word encountered.
};

/**Initialize the word list array.**/
void initializeWordList(struct wordNode * wordList, size_t start, size_t end)
{
    for (size_t index = start; index < end; index++)
    {
        wordList[index].phrase = malloc(4 * sizeof(char));
        strcpy(wordList[index].phrase, "???");
        
        wordList[index].frequency = 0;
    }
}

/**This is the compare function. Used to sort the struct wordNode array in lexicographical order**/
int compare(const void * first, const void * second)
{
    struct wordNode * firstElement = (struct wordNode *) first;
    struct wordNode * secondElement = (struct wordNode *) second;
    
    //Sort by lexicographical order.
    return strcmp(firstElement -> phrase, secondElement -> phrase);
}

int sort(const void * first, const void * second)
{
    struct wordNode * first_element = (struct wordNode *) first;
    struct wordNode * second_element = (struct wordNode *) second;
    
    if (first_element -> frequency != second_element -> frequency)
    {
        return second_element -> frequency - first_element -> frequency;
    }
    
    return strcmp(first_element -> phrase, second_element -> phrase);
}

/**This part puts the words into the struct wordNode array.**/
struct wordNode * populateWordList(struct wordNode * wordList, char * word, size_t * length, size_t special, size_t * position) //CHANGE
{
    size_t check = 0;
    
    if (special == 1)
    {
        if (*position == *length)
        {
            size_t temp = *length * 2;
            struct wordNode * replacement = realloc(wordList, temp * sizeof(struct wordNode));
            initializeWordList(replacement, *length, temp);
            
            wordList = replacement;
            *length = temp;
        }
        
        if (!strcmp(wordList[*position].phrase, "???"))
        {
            size_t wordLength = strlen(word) + 1;
            char * temp = realloc(wordList[*position].phrase, wordLength);
            wordList[*position].phrase = temp;
            
            strcpy(wordList[*position].phrase, word);
            wordList[*position].frequency++;
            
            return wordList;
        }
        
        size_t wordLength = strlen(wordList[*position].phrase) + strlen(word) + 1;
        char * temp = realloc(wordList[*position].phrase, wordLength);
        wordList[*position].phrase = temp;
        
        strcat(wordList[*position].phrase, word);
        
        return wordList;
    }
    
    for (check = 0; check < *length; check++)
    {
        if (!strcmp(wordList[check].phrase, word))
        {
            wordList[check].frequency++;
            
            return wordList;
        }
        
        if (!strcmp(wordList[check].phrase, "???"))
        {
            size_t wordLength = strlen(word) + 1;
            char * tmp = realloc(wordList[check].phrase, wordLength);
            wordList[check].phrase = tmp;
            
            strcpy(wordList[check].phrase, word);
            wordList[check].frequency++;
            
            (*position)++; //NEW
            
            return wordList;
        }
    }
    
    size_t temp = *length * 2;
    struct wordNode * replacement = realloc(wordList, temp * sizeof(struct wordNode));
    initializeWordList(replacement, *length, temp);
    
    wordList = replacement;
    *length = temp;
    
    size_t required = strlen(word) + 1;
    char * temporary = realloc(wordList[check].phrase, required);
    wordList[check].phrase = temporary;
    
    strcpy(wordList[check].phrase, word);
    wordList[check].frequency++;
    
    (*position)++; //NEW
    
    return wordList;
}

/**This part simplifies a wordList array, ensuring no wierd symbols exist.**/
struct wordNode * simplifyWordList(struct wordNode * wordList, size_t * length)
{
    size_t validLength = 0;
    
    for (size_t index = 0; index < *length; index++)
    {
        if ((!strcmp(wordList[index].phrase, "???")) && (wordList[index].frequency == 0))
        {
            break;
        }
        
        validLength++;
    }
    
    for (size_t index = validLength; index < *length; index++)
    {
        free(wordList[index].phrase);
    }
    
    struct wordNode * replacement = realloc(wordList, validLength * sizeof(struct wordNode));
    wordList = replacement;
    *length = validLength;
    
    return wordList;
}

/**This function reads the files indicated.**/
struct wordNode * readFiles(struct wordNode * wordList, size_t * wordStd, char * fileName, char * * word, size_t * standard, size_t * space, size_t * position) //CHANGED
{
    //Setting up the file.
    int folder = open(fileName, O_RDONLY);
    char buffer[128];
    
    size_t bytesRead;
    size_t special = 0; //NEW
    
    while ((bytesRead = read(folder, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytesRead] = '\0';
        
        for (int index = 0; index < bytesRead; index++)
        {
            if ((!isalpha(buffer[index])) && (buffer[index] != '-') && (buffer[index] != '\'')) //A separator is encountered.
            {
                if (*space > 1)
                {
                    if ((*word)[*space - 1] == '-')
                    {
                        (*word)[*space - 1] = '\0';
                    }
                }
                
                if ((*word)[0] != '\0')
                {
                    wordList = populateWordList(wordList, *word, wordStd, special, position);
                }
                
                *space = 0;
                (*word)[0] = '\0';
                
                if (special == 1)
                {
                    special = 0;
                    (*position)++;
                }
            }
            
            else //Normally read the characters of the file.
            {
                if (*space == *standard - 1)
                {
                    special = 1;
                    wordList = populateWordList(wordList, *word, wordStd, special, position);
                    
                    *space = 0;
                    (*word)[0] = '\0';
                }
                
                if ((*space == 0) && (buffer[index] == '-')) //A hyphen cannot appear in the first character in a valid word.
                {
                    if (special == 1)
                    {
                        special = 0;
                        
                        if (wordList[*position].phrase[strlen(wordList[*position].phrase) - 1] == '-')
                        {
                            wordList[*position].phrase[strlen(wordList[*position].phrase) - 1] = '\0';
                            (*position)++;
                            continue;
                        }
                        
                        if (wordList[*position].phrase[strlen(wordList[*position].phrase) - 1] == '\'')
                        {
                            (*position)++;
                            continue;
                        }
                    }
                    
                    else
                    {
                        continue;
                    }
                }
                
                if ((*space == 0) && (buffer[index] == '\'') && (special == 1))
                {
                    if (special == 1)
                    {
                        if (wordList[*position].phrase[strlen(wordList[*position].phrase) - 1] == '-')
                        {
                            special = 0;
                            wordList[*position].phrase[strlen(wordList[*position].phrase) - 1] = '\0';
                            (*position)++;
                        }
                    }
                }
                
                if ((buffer[index] == '\'') || (buffer[index] == '-'))
                {
                    if (*space > 0)
                    {
                        if ((*word)[*space - 1] == '-')
                        {
                            (*word)[*space - 1] = '\0';
                            wordList = populateWordList(wordList, *word, wordStd, special, position);
                            
                            *space = 0;
                            (*word)[0] = '\0';
                            
                            if (special == 1)
                            {
                                special = 0;
                                (*position)++;
                            }
                            
                            if (buffer[index] == '\'')
                            {
                                (*word)[*space] = buffer[index];
                                (*space)++;
                            }
                            
                            continue;
                        }
                        
                        if ((buffer[index] == '-') && ((*word)[*space - 1] == '\''))
                        {
                            wordList = populateWordList(wordList, *word, wordStd, special, position);
                            
                            *space = 0;
                            
                            if (special == 1)
                            {
                                special = 0;
                                (*position)++;
                            }
                            
                            continue;
                        }
                        
                        if ((buffer[index] == '\'') && ((*word)[*space - 1] == '-'))
                        {
                            (*word)[*space - 1] = '\0';
                            wordList = populateWordList(wordList, *word, wordStd, special, position);
                            
                            *space = 0;
                            
                            if (special == 1)
                            {
                                special = 0;
                                (*position)++;
                            }
                        }
                    }
                    
                    if ((*space == 0) && (buffer[index] == '-'))
                    {
                        continue;
                    }
                }
                
                (*word)[*space] = buffer[index];
                (*space)++;
                (*word)[*space] = '\0';
            }
        }
    }
    
    if ((*word)[0] != '\0') //Print any leftover words remaining.
    {
        if ((*word)[strlen(*word) - 1] == '-')
        {
            (*word)[strlen(*word) - 1] = '\0';
        }
        
        wordList = populateWordList(wordList, *word, wordStd, special, position);
    }
    
    (*word)[0] = '\0';
    *space = 0;
    
    if (special == 1)
    {
        special = 0;
        (*position)++;
    }
    
    close(folder);
    
    return wordList;
}

/**Read the directory.**/
struct wordNode * read_directory(struct wordNode * wordList, size_t * wordStd, char * path, char * word, size_t * standard, size_t * space, size_t * position)
{
    DIR * directory = opendir(path);
    
    struct dirent * entry;
    
    while ((entry = readdir(directory)) != NULL)
    {
        if (entry -> d_name[0] == '.')
        {
            continue;
        }
        
        struct stat path_stat;
        
        size_t total = strlen(path) + 2 + strlen(entry -> d_name);
        char * newPath = malloc(total);
        snprintf(newPath, total, "%s/%s", path, entry -> d_name);
        
        if (stat(newPath, &path_stat) != 0)
        {
            break;
        }
        
        if (S_ISDIR(path_stat.st_mode))
        {
            wordList = read_directory(wordList, wordStd, newPath, word, standard, space, position);
        }
        
        if (S_ISREG(path_stat.st_mode) && (strstr(entry -> d_name, ".txt")))
        {
            wordList = readFiles(wordList, wordStd, newPath, &word, standard, space, position);
            word[0] = '\0'; *space = 0;
        }
        
        free(newPath);
    }
    
    closedir(directory);
    
    return wordList;
}

/**This is the main function.**/
int main(int argc, char * * argv)
{
    //This array represents the list of words and their frequencies.
    size_t standard = 15;
    char * word = calloc(standard, sizeof(char)); size_t space = 0;
    
    size_t wordStd = 15;
    struct wordNode * wordList = calloc(wordStd, sizeof(struct wordNode));
    initializeWordList(wordList, 0, wordStd);
    
    size_t position = 0;
    
    //Start opening the directories.
    for (int input = 1; input < argc; input++)
    {
        struct stat path_type;
        
        if (stat(argv[input], &path_type) != 0)
        {
            break;
        }
        
        if (S_ISDIR(path_type.st_mode))
        {
            wordList = read_directory(wordList, &wordStd, argv[input], word, &standard, &space, &position);
        }
        
        if ((S_ISREG(path_type.st_mode)) && (strstr(argv[input], ".txt")))
        {
            wordList = readFiles(wordList, &wordStd, argv[input], &word, &standard, &space, &position);
        }
    }
    
    //Simplify the word list.
    wordList = simplifyWordList(wordList, &wordStd);
    qsort(wordList, wordStd, sizeof(struct wordNode), compare);
    
    //Count the number of unique words.
    size_t numUnique = 0;
    
    if (wordStd < 2)
    {
        numUnique = wordStd;
    }
    
    else
    {
        numUnique = 1;
        
        for (size_t index = 0; index < wordStd - 1; index++)
        {
            if (strcmp(wordList[index].phrase, wordList[index + 1].phrase))
            {
                numUnique++;
            }
        }
    }
    
    //Update the frequency of each words existing.
    for (size_t index = 0; index < wordStd - 1; index++)
    {
        if (!strcmp(wordList[index].phrase, wordList[index + 1].phrase))
        {
            size_t back = index + 1;
            
            while ((!strcmp(wordList[index].phrase, wordList[back].phrase)) && (back < wordStd))
            {
                wordList[index].frequency++;
                wordList[back].frequency = wordList[index].frequency;
                
                back++;
                
                if (back >= wordStd)
                {
                    break;
                }
            }
            
            index = back;
        }
    }
    
    //Copy the unique words into a separate folder
    struct wordNode * uniqueArr = calloc(numUnique, sizeof(struct wordNode));
    initializeWordList(uniqueArr, 0, numUnique);
    
    if (numUnique > 0)
    {
        size_t ref = 1;
        
        char * temp = realloc(uniqueArr[ref - 1].phrase, strlen(wordList[ref - 1].phrase) + 1);
        uniqueArr[ref - 1].phrase = temp;
        
        if (uniqueArr[ref - 1].phrase != NULL)
        {
            strcpy(uniqueArr[ref - 1].phrase, wordList[ref - 1].phrase);
        }
        
        uniqueArr[ref - 1].frequency = wordList[ref - 1].frequency;
        
        for (size_t index = 1; index < wordStd && ref < numUnique; index++)
        {
            if (strcmp(wordList[index].phrase, wordList[index - 1].phrase))
            {
                char * temp = realloc(uniqueArr[ref].phrase, strlen(wordList[index].phrase) + 1);
                uniqueArr[ref].phrase = temp;
                
                if (uniqueArr[ref].phrase != NULL)
                {
                    strcpy(uniqueArr[ref].phrase, wordList[index].phrase);
                }
                
                uniqueArr[ref].frequency = wordList[index].frequency;
                ref++;
            }
        }
    }
    
    qsort(uniqueArr, numUnique, sizeof(struct wordNode), sort);
    
    for (size_t index = 0; index < numUnique; index++)
    {
        printf("%s %u\n", uniqueArr[index].phrase, uniqueArr[index].frequency);
    }
    
    //Free the unique array.
    for (int index = 0; index < numUnique; index++)
    {
        free(uniqueArr[index].phrase);
    }
    
    //Free the word list.
    for (int index = 0; index < wordStd; index++)
    {
        free(wordList[index].phrase);
    }
    
    free(wordList);
    free(uniqueArr);
    free(word);
    
    return EXIT_SUCCESS;
}
