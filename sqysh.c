#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

#define BUFFERS 257

struct child {
	char* command;
	int pid;
	struct child* next;
	struct child* prev;
};

void execute(char** str, struct child** children);
char* trim(char* str);

int main(int argc, char** argv)
{
	/* Your code here! */
	FILE *fp;
	char string[10];
	char buffer[257];
//	char **buffers = (int **) malloc(sizeof(int*)*BUFFERS);
	struct child* children = NULL;
	if(argc > 2){
		printf("too many arguments");
		exit(1);
	}
	if(argc == 1 && isatty(0)){
		strcpy(string, "sqysh$ ");
		fp = stdin;
	}else{
		strcpy(string,  "");
		if(argc != 1){
			fp = fopen(argv[1], "r");
			if(fp == NULL){
				fprintf(stderr, "fopen(): %s\n", strerror(errno));
				exit(1);
			}
		}else{
			fp = stdin;
		}
	}
	int cont = 1;
	int concur = 0;
	char* strArray[257] = {NULL};
	printf("%s", string);
 	while(cont){
		
		if(fgets(buffer, sizeof(buffer), fp) == NULL){
			cont = 0;
			break;
		}
		if(strlen(buffer) !=0 && strcmp(buffer, "'\0'")!= 0 && buffer[0] != '\0'){
		int status;
		int freed = 0;
		struct child* curr;
			curr = children;
			while(curr != NULL){
				if(waitpid(curr->pid, &status, WNOHANG)){
				fprintf(stderr, "[%s (%d) completed with status %d]\n", curr->command, curr->pid, status);
				freed = 1; 	
					if(curr->prev != NULL){
						if(curr->next != NULL){
							curr->prev->next = curr->next;
							curr->next->prev = curr->prev;
						}else{
							curr->prev->next = NULL;
						}
					}else if(curr->prev == NULL){
						if(curr->next == NULL){
							children = NULL;
						}else{
							children = curr->next;
							curr->next->prev = NULL;
						}
					}
				}
				struct child* tmp = curr;
                                curr = curr->next;
				if(freed){
					tmp->next = NULL;
					tmp->prev = NULL;
					free(tmp->command);
					tmp->command = NULL;
					free(tmp);
				}
				freed = 0;
			}
			int j = 0;
	/*		for(j=0; j < 257; j++){
				free(strArray[j]);
				strArray[j] = NULL;
			}*/
			
			trim(buffer);
		if(strlen(buffer) !=0 && strcmp(buffer, "'\0'")!= 0 && buffer[0] != '\0'){
			char* token = strtok(buffer, " ");
	//		int i = 0;
		 	j =0;
			strArray[0] = malloc(sizeof(token));
			while(token != NULL){
			   if(j != 0){
				strArray[j] = malloc(sizeof(buffer));
				}
				strcpy(strArray[j], token);
				token = strtok(NULL, " ");
				j++;
			}
			strArray[j] = NULL;
		if(strArray[0] != '\0'){
			execute(strArray, &children);	
		}
		}

			j = 0;
			for(j =0; j < 257; j++){
				free(strArray[j]);
				strArray[j] = NULL;
			}
			


			curr = children;
			while(curr != NULL){
				if(waitpid(curr->pid, &status, WNOHANG)){
					fprintf(stderr, "[%s (%d) completed with status %d]\n", curr->command, curr->pid, status);
					freed = 1;
					if(curr->prev != NULL){
						if(curr->next != NULL){
							curr->prev->next = curr->next;
							curr->next->prev = curr->prev;
						}else{
							curr->prev->next = NULL;
						}
					}else if(curr->prev == NULL){
						if(curr->next == NULL){
							children = NULL;
						}else{
							children = curr->next;
							curr->next->prev = NULL;
						}
					}
				}
                                struct child* tmp = curr;
                                curr = curr->next;
                                if(freed){
                                        tmp->next = NULL;
                                        tmp->prev = NULL;
					free(tmp->command);
					tmp->command = NULL;	
                                        free(tmp);
                                }
                                freed = 0;
			}
		concur++;
		}
		printf("%s", string);
		/*	j = 0;
                         while(strArray[j] != NULL){
                                 free(strArray[j]);
                                 strArray[j] = NULL;
                                 j++;
                         }*/
	}			
	return 0;
}

char* trim(char *str){
	size_t len = 0;
    	char *frontp = str;
    	char *endp = NULL;

    	if( str == NULL ) { return NULL; }
    	if( str[0] == '\0' ) { return str; }

    	len = strlen(str);
    	endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    	while( isspace((unsigned char) *frontp) ) { ++frontp; }
    	if( endp != frontp )
    	{
        	while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    	}

    	if( str + len - 1 != endp )
            *(endp + 1) = '\0';
    	else if( frontp != str &&  endp == frontp )
            *str = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }


    return str;
} 

void execute(char** str, struct child** children){
	pid_t pid;
	int status;
	char* firstCommand = str[0];
	if(strcmp(firstCommand, "cd")==0){
		//run cd
		if(str[1] == NULL){
			if((chdir(getenv("HOME"))) == -1){
				fprintf(stderr, "cd: %s\n",strerror(errno));
				return;
			}
		}else if(str[2] != NULL){
                        fprintf(stderr, "cd: too many arguments\n");
			return;
                }else{
			char* path = str[1];
			if((chdir(path)) == -1){
				fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
				return;
			}
		}	
	}else if(strcmp(firstCommand, "pwd")==0){
		//run pwd
		char path[1024];
		if(getcwd(path, sizeof(path)) != NULL){
			printf("%s\n", path);
			return;
		}else{
			fprintf(stderr, "getcwd(): %s\n",  strerror(errno));
			return;
		}
	}else if(strcmp(firstCommand, "exit")==0){
		//run exit
		int i = 0; 
		while(str[i] != NULL){
			free(str[i]);
			str[i] = NULL;
			i++;
		}	
		exit(0);
	}else {
		int redirectLeft = 0;
		int redirectRight = 0;
		int backAnd = 0;
		int i = 0;
//		char buff[257];
		int descpLeft = 0;
		int descpRight = 0;
		while((str[i])!= NULL){
                        if(strstr(str[i], "<")){
                                redirectLeft = 1;
                                descpLeft = open(str[i+1], O_RDONLY, 0);
                                if(descpLeft == -1){
                                        fprintf(stderr, "open(): %s\n", strerror(errno));
                                        return;
                                }
				free(str[i]);
                                str[i] = '\0';
				free(str[i+1]);
                                str[i+1] = '\0';
				i = i+ 1;
                        }else if(strstr(str[i], ">")){
                                redirectRight  = 1;
                                descpRight = open(str[i+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                                if(descpRight == -1){
                                        fprintf(stderr, "open(): %s\n", strerror(errno));
                                        return;
                                }
				free(str[i]);
                                str[i] = '\0';
				free(str[i+1]);
                                str[i+1] = '\0';
				i = i + 1;
                        }else if(strstr(str[i], "&")){
                              	backAnd = 1;
				free(str[i]);
				str[i] = '\0';
                        }
                        i++;
                }
	
		if((pid = fork()) <0){
			fprintf(stderr, "%s: %s\n", firstCommand, strerror(errno));
			exit(1);
		}else if(pid == 0){
			if(redirectLeft){
                        	dup2(descpLeft, 0);
				close(descpLeft);
                	}
			if(redirectRight){
                        	dup2(descpRight, 1);
				close(descpRight);
                	}	
			

			if((execvp(firstCommand, str)) < 0){
				fprintf(stderr, "Error: %s\n", strerror(errno));
				firstCommand = NULL;
				int i = 0;
				for(i = 0; i < 257; i++){
					free(str[i]);
					str[i] = NULL;
				}
				str = NULL;	
				exit(1);
			}	
		}else{
			if(backAnd){
				struct child* newNode = malloc(sizeof(struct child));
				newNode->pid = pid;
				char* tmp = malloc(sizeof(firstCommand+1));
				strcpy(tmp, firstCommand);
				newNode->command = tmp;
				tmp = NULL;
				newNode->next = NULL;
				if(*children == NULL){
					newNode->prev = NULL;
					*children = newNode;
				}else{	
					struct child* curr = *children;
					while(curr->next !=NULL){
						curr = curr->next;
					}
					curr->next = newNode;
					newNode->prev = curr;
				}
							

			}else{
				waitpid(pid, &status, 0);
			}
		}

	}
}


