#include "icssh.h"
#include <readline/readline.h>
#include "helpers.h"
#include <signal.h>

volatile sig_atomic_t childTerminated = 0;

void sigchld_handler(int signum)
{
    childTerminated = 1;
}

int last_exit_status = 0;





int main(int argc, char* argv[]) 
{
    int max_bgprocs = 3;
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
	char* line;
	list_t* bgJobListOldest = CreateList();
	list_t* bgJobListNewest = CreateList();
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("sigaction\n");
		exit(EXIT_FAILURE);
	}


#ifdef GS
    rl_outstream = fopen("/dev/null", "w");
#endif

    // check command line arg
    if(argc > 1) {
        int check = atoi(argv[1]);
        if(check != 0)
            max_bgprocs = check;
        else {
            printf("Invalid command line argument value\n");
            exit(EXIT_FAILURE);
        }
    }

	// Setup segmentation fault handler
	if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) 
	{
		perror("Failed to set signal handler");
		exit(EXIT_FAILURE);
	}

    	// print the prompt & wait for the user to enter commands string
	while ((line = readline(SHELL_PROMPT)) != NULL) 
	{
		if (childTerminated)
		{
			childTerminated = 0;
			int status;
			pid_t pid;

			while((pid = waitpid(-1, &status, WNOHANG)) > 0)
			{
				node_t* curr = RemoveByPid(&bgJobListOldest->head,pid);
				node_t* curr2 = RemoveByPid(&bgJobListNewest->head,pid);
				if (curr == NULL || curr2 == NULL)
				{
					perror("removing node failure\n");
					exit(EXIT_FAILURE);
				}
				else
				{
					bgJobListOldest->length--;
					bgJobListNewest->length--;
					printf(BG_TERM, pid, curr->bgentry->job->line);


				}
            	// Print the defined BG_TERM message to STDOUT
        	}

		}

		// MAGIC HAPPENS! Command string is parsed into a job struct
		// Will print out error message if command string is invalid
		job_info* job = validate_input(line);
		if (job == NULL) 
		{ // Command was empty string or invalid
		free(line);
		continue;
		}

		//Prints out the job linked list struture for debugging
		#ifdef DEBUG   // If DEBUG flag removed in makefile, this will not longer print
				debug_print_job(job);
		#endif

		// example built-in: exit
		if (strcmp(job->procs->cmd, "exit") == 0) 
		{
			// Terminating the shell
			free(line);
			free_job(job);
            validate_input(NULL);   // calling validate_input with NULL will free the memory it has allocated
            return 0;
		}

		if (strcmp(job->procs->cmd, "estatus") == 0)
		{
			printf("%d\n", last_exit_status);
			free_job(job);
			free(line);
			continue;
		}

		if (strcmp(job->procs->cmd, "bglist") == 0)
		{
			// printf("Printing Linked List of length: %d\n",bgJobList->length);
			// printf("Oldest:\n");
			if (bgJobListOldest->length == 0)
			{
				fprintf(stderr,"ERROR: No Background Processes\n");
				last_exit_status = 1;
			}
			else
			{
				PrintLinkedList(bgJobListOldest,stderr);
				last_exit_status = 0;

			}
			// printf("\n\n");
			// printf("Newest\n");
			
			// PrintLinkedList(bgJobListNewest,stderr);

			continue;
		}

		if (strcmp(job->procs->cmd, "fg") == 0)
		{
			node_t* temp;

			if (bgJobListNewest->length == 0 || bgJobListOldest->length == 0)
			{
				fprintf(stderr,"%s",PID_ERR);
				last_exit_status = 1;
			}
			else
			{

				if (job->procs->argc > 1)
				{
				
					pid_t myPid = atoi(job->procs->argv[1]);
					temp = RemoveByPid(&bgJobListOldest->head,myPid);
				
					// check to see if the pid exist and remove or print error
					if(temp != NULL)
					{
						// desired pid successfully removed
						RemoveByPid(&bgJobListNewest->head,myPid);
						bgJobListNewest->length--;
						bgJobListOldest->length--;
						printf("%s\n",temp->bgentry->job->line);
					}
					else
					{
						fprintf(stderr,"%s",PID_ERR);
						last_exit_status = 1;
					}
				
					
				}
				else if (job->procs->argc == 1)
				{	
					temp = RemoveFromHead(&bgJobListNewest->head);

					if(temp != NULL)
					{
						// remove the newest background job
						RemoveByPid(&bgJobListOldest->head,temp->bgentry->pid);
						printf("%s\n",temp->bgentry->job->line);
						bgJobListNewest->length--;
						bgJobListOldest->length--;
					}
					else
					{
						fprintf(stderr,"%s",PID_ERR);
						last_exit_status = 1;
					}
				
						
		
				}

			}

			free_job(job);
			free(line);
			continue;

		}



		if (strcmp(job->procs->cmd, "cd") == 0)
		{
			char * new_dir = job->procs->argv[1];

			if (new_dir == NULL)
			{
				new_dir = getenv("HOME");
				if (new_dir == NULL)
				{
					fprintf(stderr, "Cannot get HOME environment variable\n");
            		free_job(job);
            		free(line);
            		continue;
				}
			}

			if (chdir(new_dir) == 0)
			{
				char cwd[1024];
				if(getcwd(cwd,sizeof(cwd)) != NULL)
				{
					printf("%s\n",cwd);
				}
				else
				{
					perror("getcwd() error\n");
				}
			}
			else
			{
				fprintf(stderr, "%s",DIR_ERR);
			}
			free_job(job);
			free(line);
			continue;
		}

		// example of good error handling!
        // create the child proccess
		if ((pid = fork()) < 0) 
		{
			perror("fork error");
			exit(EXIT_FAILURE);
		}
		if (pid == 0) 
		{  //If zero, then it's the child process
            //get the first command in the job list to execute
		    proc_info* proc = job->procs;
			exec_result = execvp(proc->cmd, proc->argv);
			if (exec_result < 0) {  //Error checking
				printf(EXEC_ERR, proc->cmd);
				
				// Cleaning up to make Valgrind happy 
				// (not necessary because child will exit. Resources will be reaped by parent)
				free_job(job);  
				free(line);
    				validate_input(NULL);  // calling validate_input with NULL will free the memory it has allocated

				exit(EXIT_FAILURE);
			}
		} 
		else 
		{		// As the parent, wait for the foreground job to finish

			if (job->bg && bgJobListOldest->length == max_bgprocs)
			{
				free_job(job);
				free(line);
				fprintf(stderr,"%s",BG_ERR);
				continue;
			}
			else if (job->bg)
			{
				node_t* newNode = malloc(sizeof(node_t));
				bgentry_t* newbgentry = malloc(sizeof(bgentry_t));

				if (newbgentry == NULL || newNode == NULL)
				{
					perror("malloc failssss\n");
					exit(EXIT_FAILURE);
				}
				newNode->bgentry = newbgentry;

				newbgentry->job = job;
				newbgentry->pid = pid;
				newbgentry->seconds = time(NULL);

				InsertNodeAtTail(bgJobListOldest,newNode);
				InsertNodeAtHead(bgJobListNewest,newNode);
				continue;
				
			}
			else
			{
				wait_result = waitpid(pid, &exit_status, 0);
				if (wait_result < 0) {
					printf(WAIT_ERR);
					exit(EXIT_FAILURE);
				}
				else
				{
					if (WIFEXITED(exit_status))
					{
						last_exit_status = WEXITSTATUS(exit_status);
					}
				}
			}
	}

		free_job(job);  // if a foreground job, we no longer need the data
		free(line);
	}

    	// calling validate_input with NULL will free the memory it has allocated
    	validate_input(NULL);

#ifndef GS
	fclose(rl_outstream);
#endif
	return 0;
}
