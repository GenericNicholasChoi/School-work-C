#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include "freq_list.h"
#include "worker.h"

/* The function get_word should be added to this file */

FreqRecord *get_word(char *word, Node **head, char **filename)
{
	Node **currhead = head;
	FreqRecord newrecordarray[MAXFILES];
	FreqRecord *newrecord;
	FreqRecord *newrecordfinal;
	FreqRecord *recordarray = newrecordarray;
	int arrayposition = 0;
	// printf("Im in getword \n");
	while (*currhead != NULL)
	{
		// printf("This is currhead word: %s \n", (*currhead)->word);
		// printf("Im in first loop of getword \n");
		if (strcmp((*currhead)->word, word) == 0)
		{
			int filefreqcounter = 0;
			while (filefreqcounter < MAXFILES)
			{
				if ((*currhead)->freq[filefreqcounter] > 0)

				{
					newrecord = malloc(sizeof(FreqRecord));
					newrecord->freq = (*currhead)->freq[filefreqcounter];
					strcpy(newrecord->filename, filename[filefreqcounter]);
					newrecordarray[arrayposition] = *newrecord;
					arrayposition += 1;
				}
				filefreqcounter += 1;
			}
		}

		*currhead = (*currhead)->next;
	}
	newrecordfinal = malloc(sizeof(FreqRecord));
	newrecordfinal->freq = 0;
	newrecordarray[arrayposition] = *newrecordfinal;
	return recordarray;
}

/* Print to standard output the frequency records for a word.
* Used for testing.
*/
void print_freq_records(FreqRecord *frp)
{
	int i = 0;
	while (frp != NULL && frp[i].freq != 0)
	{
		printf("%d    %s\n", frp[i].freq, frp[i].filename);
		i++;
	}
}

/* run_worker
* - load the index found in dirname
* - read a word from the file descriptor "in"
* - find the word in the index list
* - write the frequency records to the file descriptor "out"
*/
void run_worker(char *dirname, int in, int out)
{ /*Open the file called index and filenames
	 create a pipe*/
	char buf[MAXWORD];
	char listfile[PATHLENGTH];
	strcpy(listfile, dirname);
	strcat(listfile, "/index");

	Node *newhead = malloc(sizeof(Node));
	Node *head = newhead;
	char namefile[PATHLENGTH];
	strcpy(namefile, dirname);
	strcat(namefile, "/filenames");
	char **filenames = init_filenames();
	//Load the indexes
	read_list(namefile, listfile, &head, filenames);
	//continuously keep asking for a word until fd is exited
	int read_success = read(in, buf, MAXWORD);
	while (read_success != 0)
	{
		buf[read_success - 1] = '\0';
		FreqRecord *word = get_word(buf, &head, filenames);
		write(out, word, sizeof(FreqRecord) * MAXRECORDS);
		read_success = read(in, buf, MAXWORD);
		read_list(namefile, listfile, &head, filenames);
	}
}
