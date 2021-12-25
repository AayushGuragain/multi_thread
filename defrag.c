#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>


typedef struct _location_array 
{
    char *name;
    int total_count;
} LOC_INFO;

char main_dir[200];

typedef struct _mp_record 
{
    char *name;
    char *data;
    long data_length;
} MP_DATA;


MP_DATA **musics;

void sort(int n, MP_DATA** pointers_to_cards)
{ 

    int i, j; 
    MP_DATA *t; 
    for (i = 0; i < n; i++) { 
        for (j = i+1; j < n; j++) {
        //if cards name is not in alphabetical order, puts in alphabetical order.

           if (strcmp( pointers_to_cards[j] -> name, pointers_to_cards[i] -> name) < 0){ 
                t = pointers_to_cards[i];
                pointers_to_cards[i] = pointers_to_cards[j];
                pointers_to_cards[j] = t;
            }
            
           if (strlen( pointers_to_cards[j] -> name)< strlen(pointers_to_cards[i] -> name)){ 
                t = pointers_to_cards[i];
                pointers_to_cards[i] = pointers_to_cards[j];
                pointers_to_cards[j] = t;
            }
       
             
        } 
    } 
}
pthread_mutex_t count_elements;
int total_elements = 0;

void read_mp(char *path, int placement){
	FILE *cards_bin = fopen(path, "rb");
	long last;
	fseek(cards_bin, 0, SEEK_END); //end
	
	last = ftell(cards_bin);
	
	fseek(cards_bin, 0, SEEK_SET); //end
		
	///What should I fread in a mp3 file????
	
	musics[placement]->data = malloc(last +1);
	fread(musics[placement]->data, last, 1,cards_bin);
	
	musics[placement]->data_length = last;
	
	fclose(cards_bin);
	
}

void display(MP_DATA **musics){
    printf("The total elements are: %d \n", total_elements);
    for (int i=0; i< total_elements; i++){
        printf("name: %s \n",musics[i]->name);
    }
    printf("The total elements are: %d \n", total_elements);
}

//Recurvesivly travse the directory and store it in the 'MP_DATA' type
//Credit: https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
void listdir(const char *name){
    int tmp_count = 0;
    DIR *dir;
    struct dirent *entry;
    //printf("DEBUG:::Pathname::::%s \n", name);
    if (!(dir = opendir(name))){
        printf("Invalid directory! \n");
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        char path[1024];
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        if ( (entry->d_name[0]) ==  '.' ){
            continue;
        }
        
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        //If it is a directory type travrse it further
        if (entry->d_type == DT_DIR) {
            listdir(path);
        } 
        
        //If it is a file type pass through here
        else if (entry->d_type != DT_LNK) {
            int dir_len = strlen(entry->d_name);
            char *last_four = &entry->d_name[dir_len-4];
            //printf("Check here: %s\n",entry->d_name);
            if (strncmp(last_four, ".mp3", 4) == 0){
                //Controlled access to the total_element index
		pthread_mutex_lock(&count_elements); //locking it 
			musics = realloc(musics, (total_elements+2)*sizeof(MP_DATA*) );
			tmp_count = total_elements;
			total_elements++;
			musics[tmp_count] = malloc(sizeof(MP_DATA));
			//length for strndup
			int name_len = strlen(entry->d_name);
					
			musics[tmp_count] -> name = strndup(entry->d_name, name_len);
			read_mp(path, tmp_count);
		pthread_mutex_unlock(&count_elements);//Unlocking it 
                

            }
        }
    }
    closedir(dir); //close the directory
}


int mp3_out(MP_DATA **musics, char* mp3_out_format){
    //Get the total elements
    int totalElem = total_elements;
    FILE *music_w = fopen(mp3_out_format,"wb");
    
    if (music_w == NULL) 
    { 
        printf("Could not open file"); 
        return 0; 
    }
    for (int i=0; i<totalElem; i++) 
    {   
        fwrite(musics[i] -> data,musics[i] ->data_length,1,music_w);
    }
    fclose(music_w); 
    return 0;
}

void* thread_func( void *arg){
	char *v = (char *)arg;
	char *location = calloc(1, 500);
	
	strcat(location, main_dir);
	strcat(location, "/");
	strcat(location, v);
	listdir(location);
	//printf("%s\n",location);
	
	free(location);
	return NULL;

}

// recursive directory traversing code borrowed from: https://iq.opengenus.org/traversing-folders-in-c/ 

LOC_INFO ** top_dir_search(LOC_INFO **dirnames, char * filename){
    struct dirent *de;  // Pointer for directory entry 
    int counter =0;
    dirnames[counter] = malloc(sizeof(LOC_INFO));
    dirnames[counter]-> total_count = 0;
    
    // opendir() returns a pointer of DIR type.  
    DIR *dr = opendir(filename); 
    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 
    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html 
    // for readdir() 
    while ((de = readdir(dr)) != NULL){
        //if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0){
        if ((de->d_name[0] != '.') && (de->d_type == DT_DIR)){
            dirnames[counter]-> name =  malloc(strlen(de->d_name) + 2);
            strncpy(dirnames[counter]-> name, de->d_name,  strlen(de->d_name) +1);
            dirnames = realloc(dirnames, (counter+2)*sizeof(LOC_INFO*) );
            counter++;
        
            dirnames[counter] = malloc(sizeof(LOC_INFO));
        }
    }
    dirnames[0]-> total_count = counter;
    closedir(dr); 
    //free(dirnames[counter]);
    return dirnames;
}

  
int main(int argc,char **argv)
{

	if (argc != 3) {
		printf("ERROR!\n");
		return -1;
	}
	strcpy(main_dir, argv[1]);
	LOC_INFO **top_list = malloc(sizeof(LOC_INFO*));
	//top_list[0] = malloc(sizeof(LOC_INFO));
	//top_list[0]-> total_count = 0;
	top_list = top_dir_search(top_list, argv[1]);
	//int top_dir_count = top_list[0]-> total_count;
	//printf("%d\n", top_list[0]-> total_count);
	musics = malloc(sizeof(MP_DATA*));
	int total = top_list[0]->total_count;
	
	
	pthread_t threads[total];
	int ctr;
	for (ctr=0; ctr<total;ctr++ ){
		pthread_create(&threads[ctr], NULL, thread_func, top_list[ctr]->name);

	}

	for (ctr=0; ctr<total;ctr++ ){
		pthread_join(threads[ctr], NULL);
	}
	sort(total_elements, musics);
	//display(musics);
	mp3_out(musics, argv[2]);

	int i;
	for(i = 0; i < total; ++i){
		//printf("%s\n", top_list[i]->name);
		free(top_list[i]-> name);
		free(top_list[i]);
	}
	free(top_list[i]);
	free(top_list);
	for(int free_val = 0; free_val<total_elements;free_val++){
		free(musics[free_val]->name);
		free(musics[free_val]->data);
		free(musics[free_val]);
	}
	free(musics);
    
    	return 0; 
} 
