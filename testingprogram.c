#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include "worker.c"
void read_list(char *namefile, char *listfile,
			   Node **head, char **filenames)
{

	/* Read in the linked list */
	FILE *list_fp;
	if ((list_fp = fopen(listfile, "r")) == NULL)
	{
		perror("List file");
		exit(1);
	}

	Node *cur = malloc(sizeof(Node));
	Node *prev = NULL;
	/* fread is a function similar to the read function we have seen in lecture
           except that it works on FILE * instead of file descriptors;
           it is used to read binary input (rather than characters).
        */
	if ((fread(cur, sizeof(Node), 1, list_fp)) == 0)
	{
		free(cur);
		*head = NULL;
		return;
	}
	*head = cur;
	do
	{
		cur->next = NULL;
		if (cur == *head)
		{
			prev = cur;
		}
		else
		{
			prev->next = cur;
			prev = cur;
		}
		cur = malloc(sizeof(Node));
	} while ((fread(cur, sizeof(Node), 1, list_fp)) != 0);
	free(cur);
	if ((fclose(list_fp)))
	{
		perror("fclose");
		exit(1);
	}

	/* Read in the file names */
	FILE *fname_fp;
	if ((fname_fp = fopen(namefile, "r")) == NULL)
	{
		perror("Name file");
		exit(1);
	}
	char line[MAXLINE];
	int i = 0;
	while ((fgets(line, MAXLINE, fname_fp)) != NULL)
	{
		line[strlen(line) - 1] = '\0';
		char *name = malloc(strlen(line) + 1);
		strncpy(name, line, (strlen(line) + 1));
		filenames[i] = name;
		i++;
	}
	if ((fclose(fname_fp)))
	{
		perror("fclose");
		exit(1);
	}
}
char **init_filenames()
{
	int i;
	char **fnames = malloc(MAXFILES * sizeof(char *));
	for (i = 0; i < MAXFILES; i++)
	{
		fnames[i] = NULL;
	}
	return fnames;
}

int main(int argc, char **argv)
{

	Node *newhead = malloc(sizeof(Node));
	Node *head = newhead;
	char **filenames = init_filenames();
	read_list("/courses/courses/cscb09w19/nizamnau/a3/testing/big/dir7/filenames", "/courses/courses/cscb09w19/nizamnau/a3/testing/big/dir7/index", &head, filenames);
	print_freq_records(get_word("able", &head, filenames));
	return 0;
}
