#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ===
// Region: Constants
// ===

#define UNFILLED_FRAME -1
#define MAX_QUEUE_SIZE 10
#define MAX_FRAMES_AVAILABLE 10
#define MAX_REFERENCE_STRING 30

#define ALGORITHM_FIFO "FIFO"
#define ALGORITHM_OPT "OPT"
#define ALGORITHM_LRU "LRU"


// Keywords (to be used when parsing the input)
#define KEYWORD_ALGORITHM "algorithm"
#define KEYWORD_FRAMES_AVAILABLE "frames_available"
#define KEYWORD_REFERENCE_STRING_LENGTH "reference_string_length"
#define KEYWORD_REFERENCE_STRING "reference_string"



// Assume that we only need to support 2 types of space characters:
// " " (space), "\t" (tab)
#define SPACE_CHARS " \t"


char algorithm[10];
int reference_string[MAX_REFERENCE_STRING];
int reference_string_length;
int frames_available;
int frames[MAX_FRAMES_AVAILABLE];


int is_blank(char *line)
{
    char *ch = line;
    while (*ch != '\0')
    {
        if (!isspace(*ch))
            return 0;
        ch++;
    }
    return 1;
}

int is_skip(char *line)
{
    if (is_blank(line))
        return 1;
    char *ch = line;
    while (*ch != '\0')
    {
        if (!isspace(*ch) && *ch == '#')
            return 1;
        ch++;
    }
    return 0;
}

void parse_tokens(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

void parse_input()
{
    FILE *fp = stdin;
    char *line = NULL;
    ssize_t nread;
    size_t len = 0;

    char *two_tokens[2];                                 // buffer for 2 tokens
    char *reference_string_tokens[MAX_REFERENCE_STRING]; // buffer for the reference string
    int numTokens = 0, n = 0, i = 0;
    char equal_plus_spaces_delimiters[5] = "";

    strcpy(equal_plus_spaces_delimiters, "=");
    strcat(equal_plus_spaces_delimiters, SPACE_CHARS);

    while ((nread = getline(&line, &len, fp)) != -1)
    {
        if (is_skip(line) == 0)
        {
            line = strtok(line, "\n");
            if (strstr(line, KEYWORD_ALGORITHM))
            {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    strcpy(algorithm, two_tokens[1]);
                }
            }
            else if (strstr(line, KEYWORD_FRAMES_AVAILABLE))
            {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    sscanf(two_tokens[1], "%d", &frames_available);
                }
            }
            else if (strstr(line, KEYWORD_REFERENCE_STRING_LENGTH))
            {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2)
                {
                    sscanf(two_tokens[1], "%d", &reference_string_length);
                }
            }
            else if (strstr(line, KEYWORD_REFERENCE_STRING))
            {
                parse_tokens(two_tokens, line, &numTokens, "=");
                if (numTokens == 2)
                {
                    parse_tokens(reference_string_tokens, two_tokens[1], &n, SPACE_CHARS);
                    for (i = 0; i < n; i++)
                    {
                        sscanf(reference_string_tokens[i], "%d", &reference_string[i]);
                    }
                }
            }
        }
    }
}

void print_parsed_values()
{
    int i;
    printf("%s = %s\n", KEYWORD_ALGORITHM, algorithm);
    printf("%s = %d\n", KEYWORD_FRAMES_AVAILABLE, frames_available);
    printf("%s = %d\n", KEYWORD_REFERENCE_STRING_LENGTH, reference_string_length);
    printf("%s = ", KEYWORD_REFERENCE_STRING);
    for (i = 0; i < reference_string_length; i++)
        printf("%d ", reference_string[i]);
    printf("\n");
}


const char template_total_page_fault[] = "Total Page Fault: %d\n";
const char template_no_page_fault[] = "%d: No Page Fault\n";


void display_fault_frame(int current_frame)
{
    int j;
    printf("%d: ", current_frame);
    for (j = 0; j < frames_available; j++)
    {
        if (frames[j] != UNFILLED_FRAME)
            printf("%d ", frames[j]);
        else
            printf("  ");
    }
    printf("\n");
}

void init_frames()
{
    int i;
    for (i = 0; i < frames_available; i++)
        frames[i] = UNFILLED_FRAME;
}

void FIFO_algorithm()
{
    int frameExists[MAX_FRAMES_AVAILABLE];
    int Location = 0;
    int faults [MAX_REFERENCE_STRING];

    for (int i = 0; i < frames_available; i++){
        frameExists[i] = 0;
    }

    for (int i = 0; i < MAX_REFERENCE_STRING; i++){
        faults[i] = 0;
    }

    for (int i = 0; i < reference_string_length; i++){
        int currentPage = reference_string[i];
        int pageFound = 0; //false

        for (int j = 0; j < frames_available; j++) {
            if (frames[j] == currentPage) {
                pageFound = 1;
            }
        }
        if (pageFound == 0) {
            int victimFrame = frames[Location];
            frameExists[victimFrame] = 0;  // Mark victim frame as empty
            frames[Location] = currentPage;
            frameExists[currentPage] = 1;  // Mark current page as occupying a frame
            Location = (Location + 1) % frames_available;  // Update replace location
            faults[i] = 1;
            display_fault_frame (currentPage); 
        } 
        else if (pageFound == 1) {
            faults[i] = 0;
            printf(template_no_page_fault, currentPage);
        }
    }
    
    //Handle Faults
    int totalFaults = 0;
    for (int k = 0; k < MAX_REFERENCE_STRING; k++){
        if (faults[k] == 1){
            totalFaults = totalFaults + faults[k];
        }
    }
    
    printf(template_total_page_fault, totalFaults);
    
}

void OPT_algorithm()
{
    int frameExists[MAX_FRAMES_AVAILABLE];
    int faults [MAX_REFERENCE_STRING];

    for (int i = 0; i < frames_available; i++) {
        frameExists[i] = 0;  // Initialize frameExists with false
    }

    for (int i = 0; i < MAX_REFERENCE_STRING; i++){
        faults [i] = 0;
    }

    for (int i = 0; i < reference_string_length; i++) {
        int currentPage = reference_string[i];
        int pageFound = 0;

        for (int j = 0; j < frames_available; j++) {
            if (frames[j] == currentPage) {
                pageFound = 1;
            }
        }

        if (pageFound == 0) {
                      
            int victimFrame = -1;
            int maxDistance = -1;
            int min_frame = 2147483647;

            for (int j = 0; j < frames_available; j++) {
                int nextPageIndex = i + 1;
                int nextUsage = 0;
                int currentFrame = frames[j];
                int doReplace = 0;

                while (nextPageIndex < reference_string_length) { //next usage
                    if (currentFrame != reference_string[nextPageIndex]) {
                        nextUsage = nextUsage + 1;
                        nextPageIndex = nextPageIndex + 1;
                    }
                    else{
                        break;
                    }
                    
                }

                if (nextUsage >= maxDistance) { 
                    doReplace = 1;
                    if (nextUsage == maxDistance && currentFrame < min_frame){
                        doReplace = 1;
                    }
                    else if (nextUsage == maxDistance && currentFrame >= min_frame){
                        doReplace = 0;
                    }
                }

                if (doReplace == 1){
                    min_frame = currentFrame;
                    victimFrame = j;
                    maxDistance = nextUsage;
                }
            }

            if (victimFrame != -1) {
                frameExists[frames[victimFrame]] = 0;  // Mark victim frame as empty
                frames[victimFrame] = currentPage;
                frameExists[currentPage] = 1;  // Mark current page as occupying a frame
            }
            
            faults[i] = 1;
            display_fault_frame (currentPage);

        } 

        else if (pageFound == 1){
            faults[i] = 0;
            printf(template_no_page_fault, currentPage);
        }
    }

    //Handle Faults
    int totalFaults = 0;
    for (int k = 0; k < MAX_REFERENCE_STRING; k++){
        if (faults[k] == 1){
            totalFaults = totalFaults + faults[k];
        }
    }

    printf(template_total_page_fault, totalFaults);
}

void LRU_algorithm()
{
    int last_used[MAX_FRAMES_AVAILABLE];
    int faults [MAX_REFERENCE_STRING];

    for (int i = 0; i < frames_available; i++) {
        last_used[i] = -1;  // Initialize last_used with -1
    }
    
    for (int i = 0; i < MAX_REFERENCE_STRING; i++){
        faults [i] = 0;
    }


    for (int i = 0; i < reference_string_length; i++) {
        int currentPage = reference_string[i];
        int pageFound = 0;
        int leastRecentIndex = 0;
        int endPoint = frames_available;

        for (int j = 0; j < frames_available; j++) {
            if (frames[j] == currentPage) {
                pageFound = 1;
            }
        }

         for (int j = 0; j < frames_available; j++) {
                if (frames[j] == currentPage) {
                    last_used[j] = i;  // Update last used index for the page
                    endPoint = j;
                    break;
                }
            }

        
        if (pageFound == 0) {
            for (int i = 0; i < frames_available; i++){ // Find the index of the least recently used page
                if ((i <= endPoint) && (last_used[i] < last_used[leastRecentIndex])) {
                    leastRecentIndex = i;  
                }
            }

            frames[leastRecentIndex] = currentPage;
            last_used[leastRecentIndex] = i;
            faults[i] = 1;
            display_fault_frame(currentPage);
        } 

        else if (pageFound == 1){
            faults [i] = 0;
            printf(template_no_page_fault, currentPage);
        }
    }

    //Handle faults
    int totalFaults = 0;
    for (int k = 0; k < MAX_REFERENCE_STRING; k++){
        if (faults[k] == 1){
            totalFaults = totalFaults + faults[k];
        }
    }
    printf(template_total_page_fault, totalFaults);
}


int main()
{
    parse_input();              
    print_parsed_values();      
    init_frames();              
    if (strcmp(algorithm, ALGORITHM_FIFO) == 0)
    {
        FIFO_algorithm();
    }
    else if (strcmp(algorithm, ALGORITHM_OPT) == 0)
    {
        OPT_algorithm();
    }
    else if (strcmp(algorithm, ALGORITHM_LRU) == 0)
    {
        LRU_algorithm();
    }

    return 0;
}