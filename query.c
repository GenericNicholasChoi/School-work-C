#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "freq_list.h"
#include "worker.h"

int main(int argc, char **argv)
{

    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";
    //make array for the infomation to be stored in
    FreqRecord big_array[MAXRECORDS][MAXRECORDS];
    // make an array to store the paths
    char all_paths[MAXRECORDS][PATHLENGTH];
    int directorypaths = 0;

    while ((ch = getopt(argc, argv, "d:")) != -1)
    {
        switch (ch)
        {
        case 'd':
            startdir = optarg;
            break;
        default:
            fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
            exit(1);
        }
    }
    // Open the directory provided by the user (or current working directory)

    DIR *dirp;
    if ((dirp = opendir(startdir)) == NULL)
    {
        perror("opendir");
        exit(1);
    }

    /* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then call run_worker
	* to process the index file contained in the directory.
 	* Note that this implementation of the query engine iterates
	* sequentially through the directories, and will expect to read
	* a word from standard input for each index it checks.
	*/

    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL)
    {

        if (strcmp(dp->d_name, ".") == 0 ||
            strcmp(dp->d_name, "..") == 0 ||
            strcmp(dp->d_name, ".svn") == 0)
        {
            continue;
        }
        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path) - 1);
        strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

        struct stat sbuf;
        if (stat(path, &sbuf) == -1)
        {
            //This should only fail if we got the path wrong
            // or we don't have permissions on this entry.
            perror("stat");
            exit(1);
        }

        // Only call run_worker if it is a directory
        // Otherwise ignore it.
        if (S_ISDIR(sbuf.st_mode))
        {
            strncpy(all_paths[directorypaths], path, PATHLENGTH);
            directorypaths += 1;
        }
    }
    printf("Enter the word: \n");
    char buf[MAXWORD];
    int read_success = read(STDIN_FILENO, buf, MAXWORD);
    buf[read_success - 1] = '\0';
    //create a pipe for parent to child communication
    int parent_child[MAXRECORDS][2];
    //create a pipe ofr child to parent communication
    int child_parent[MAXRECORDS][2];
    int pid;
    int i = 0;
    // create this pipe for reading
    while (i < directorypaths)
    {
        parent_child[i][0] = STDIN_FILENO;
        parent_child[i][1] = STDOUT_FILENO;
        if (pipe(parent_child[i]) == -1)
        {
            perror("pipe");
            exit(1);
        }
        i += 1;
    }
    i = 0;
    // create this pipe for writing
    while (i < directorypaths)
    {
        child_parent[i][0] = STDIN_FILENO;
        child_parent[i][1] = STDOUT_FILENO;
        if (pipe(child_parent[i]) == -1)
        {
            perror("pipe");
            exit(1);
        }
        i += 1;
    }
    //write to child pipe before forking
    i = 0;
    while (i < directorypaths)
    {
        write(parent_child[i][1], buf, MAXWORD);
        i += 1;
    }
    // create a variables to hold the freqRecords
    FreqRecord *buffer = malloc(sizeof(FreqRecord));
    i = 0;
    //start forking and run run worker.
    while (i < directorypaths)
    {
        pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(1);
        }
        else if (pid > 0)
        {
            //This is a parent
            close(child_parent[i][1]);
            close(parent_child[i][0]);
        }
        else
        {
            //this is a child
            //read from parent child pipesvn
            close(parent_child[i][1]);
            close(child_parent[i][0]);
            run_worker(all_paths[i], parent_child[i][0], child_parent[i][1]);
            return 0;
        }
        i += 1;
    }

    i = 0;
    // loop through and grab all the pipes information
    while (i < directorypaths)
    {
        //check if the record is already in the file
        read(child_parent[i][0], buffer, sizeof(FreqRecord) * MAXRECORDS);
        int k = 0;
        while (buffer != NULL && buffer[k].freq != 0)
        {
            strcpy(big_array[i][k].filename, buffer[k].filename);
            big_array[i][k].freq = buffer[k].freq;
            k += 1;
        }
        i += 1;
    }

    i = 0;
    while (i < directorypaths)
    {
        print_freq_records(big_array[i]);
        i += 1;
    }
    i = 0;
    //closing pipes
    return 0;
}
