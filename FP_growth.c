#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

//Macros
#define PATH "groceries_subset.csv" //Input test file
#define DEST "header.txt" //File to write the header table
#define MAXITEMS_IN_TR 30 //Maximum no of items in a transaction
#define ITEMSTRLEN 30 //Maximum string length of a transaction item
#define MAXTRANSACTIONS 1000 //No of transactions in the test file
#define UNIQUESTRINGS 200 //No of unique items in the test file 
#define MIN_SUPPORT 20 //Minimum support count
#define MAX 10000

//Set the write_flag to 0 if you want to write header table to DEST file
int write_flag = 1, hash_index = -1, is_multipath = 0;

//Structure that reads the transactions
struct transaction{
    int hash_id[MAXITEMS_IN_TR];
    int id;
    int item_count;
};

//Hash table
struct hash{
    char str[ITEMSTRLEN];
    int index;
}Hash[UNIQUESTRINGS];

//Structure for fp-tree
struct tree{
    int hash_id;
    struct tree *parent;
    struct tree *next;
    int frequency;
    int is_visited;
};

//structure that stores the header tables
struct header{
    int hash_id;
    int frequency;
    struct tree *ptr;
};

//Search the hash table and return the index
int return_index_and_search(char S[]){
    int i;
    for (i=0; i<=hash_index; i++){
        if (strcmp(Hash[i].str,S)==0)
            return Hash[i].index;
    }
    return -1;
}

//Function to read the file and return a array of transaction structures
void readfile(struct transaction T[]){
    FILE *fp;
    int index,tid=-1,j,item=-1;
    fp = fopen(PATH,"r");
    for (int i=0; 0<1; i++) {
        char ch = fgetc(fp), item_name[ITEMSTRLEN];

        if (isdigit(ch)) {
            if(tid>-1){
                T[tid].item_count = item;
                //printf("%d\n",T[tid].item_count);
                item = -1;
            }
            
            int x = ch - '0';
            while ((ch = fgetc(fp)) != ',') {
                x = (x * 10) + (ch - '0');
            }
            T[++tid].id = x;
            //printf("%d: ",T[tid].id);
        }

        else if(isalpha(ch)) {
            item_name[0] = ch;
            j = 1;
            ch = fgetc(fp);
            while (ch != ',' && ch != '\n') {
                item_name[j] = ch;
                j++;
                ch = fgetc(fp);
            }
            if (ch == '\n') {
                item_name[j-1] = '\0';
            }
            item_name[j] = '\0';
            //printf("%s, ",item_name);
            int index = return_index_and_search(item_name);
            if (index>-1 ){
                T[tid].hash_id[++item] = index;
            }
            else{
                strcpy(Hash[++hash_index].str,item_name);
                Hash[hash_index].index = hash_index;
                T[tid].hash_id[++item] = hash_index;
            }
        }
        else if (ch == EOF) {
            T[tid].item_count = item;
            //printf("%d\n",T[tid].item_count);
            item = -1;
            break;
        }
    }
    fclose(fp);
    //printf("%d\n",hash_index);
}

//Search for presence of string using linear search
int search_header_linear(int hash_id, int count,struct header H[], int *pos){
    int i;
    for (i=0; i<=count; i++){
        if (hash_id == H[i].hash_id){
            H[i].frequency+=1;
            *pos = i;
            return 0;
        }
    }
    return 1;
}

//Function to create the header table
void get_header(struct transaction T[],struct header H[],int *header_strings_count,int limit){
    int i,j,temp;
    for (i=0; i<=limit; i++){
        for (j=0; j<=T[i].item_count; j++){
            int flag;
            flag = search_header_linear(T[i].hash_id[j],*header_strings_count,H,&temp);
            *header_strings_count += flag;
            if (flag == 1){
                //printf("Found: %s %d\n",Hash[T[i].hash_id[j]].str,j);
                H[*header_strings_count].hash_id = T[i].hash_id[j];
                H[*header_strings_count].frequency=1;
                H[*header_strings_count].ptr = NULL;
            }
        }
    }
}

//Prints the header table
void print_header(struct header H[],int header_strings){
    printf("\033[1;32m");
    for(int i=0;i<=header_strings;i++){
        printf("%s %d\n",Hash[H[i].hash_id].str,H[i].frequency);
    }
    printf("Total strings in header= %d\n",header_strings+1);
    printf("\033[01;33m\n");
}

//Prints the transactions
void print_transactions(struct transaction T[], int tid){
    int i,j;
    printf("\033[1;35m");
    for (i=0; i<=tid; i++){
        printf("%d: ",T[i].id);
        for (j=0; j<=T[i].item_count; j++){
            printf("%s, ",Hash[T[i].hash_id[j]].str);
        }
        printf("\n");
    }
    printf("\033[01;33m\n");
}

//Performs swap operation
void swap(int i, int j,struct header H[]){
    struct header temp;
    temp = H[i];
    H[i] = H[j];
    H[j] = temp;
}

//Sort the header file
void sort_header(struct header H[],int *header_strings_count){
    int i,j;
    for (i=*header_strings_count-1; i>=0; i--){
        for (j=0; j<=i; j++){
            if (H[j].frequency<H[j+1].frequency){
                swap(j,j+1,H);
            }
        }
    }
    if (write_flag == 0){
        FILE *fp;
        fp = fopen(DEST,"w");
        for (i=0; i<=*header_strings_count; i++){
            fwrite(Hash[H[i].hash_id].str,1,strlen(Hash[H[i].hash_id].str),fp);
            fwrite(" ",1,strlen(" "),fp);
            int *p = &(H[i].frequency); 
            fprintf(fp,"%d",H[i].frequency);
            fwrite("\n",1,strlen("\n"),fp);
            //printf("%s %d\n",H[i].str,H[i].frequency);
        }
        fclose(fp);
        write_flag = 1;
    }
}

//Eliminating items with less than min support
void eliminate_less_than_min_support(struct header H[],struct header H_final[],int *header_strings_count){
    int min_support,i,j=0;
    min_support = MIN_SUPPORT;
    for (i=0; i<=*header_strings_count; i++){
        if (H[i].frequency >= min_support){
            H_final[j].hash_id = H[i].hash_id;
            H_final[j].frequency = H[i].frequency;
            H_final[j].ptr = NULL;
            //printf("%s %d ",H_final[j].str,H_final[j].frequency);
            j++;
        }
    }
    *header_strings_count = j-1;
}

//Sorts strings
void string_sort(int S[], int item_count, int id, int S1[],struct header H_final[],int *header_strings_count){
    //printf("%d\n",*header_strings_count);
    int i,j,k=0;
    for (i=0; i<=*header_strings_count; i++){
        for (j=0; j<=item_count; j++){
            if (H_final[i].hash_id == S[j]){
                S1[k] = S[j];
                k++;
                break;
            }
        }
    }
}

// Eliminating and sorting the items of transactions
void eliminate_and_sort_transaction_items(struct transaction T[],struct transaction T_final[],struct header H_final[],int *header_strings_count, int limit){
    //printf("%d\n",*header_strings_count);
    int i,j;
    for (i=0; i<=limit; i++){
        int m=-1;
        for(j=0; j<=T[i].item_count; j++){
            T_final[i].id = T[i].id;
            int k, flag=0;
            for (k=0 ;k<=*header_strings_count; k++){
                if (T[i].hash_id[j] == H_final[k].hash_id){
                    flag = 1;
                    break;
                }
            }
            if (flag == 1){
                T_final[i].hash_id[++m] = T[i].hash_id[j];
            }
        }
        T_final[i].item_count=m;
        if (m >= 1){
            int S[UNIQUESTRINGS];
            string_sort(T_final[i].hash_id, m, T_final[i].id, S, H_final, header_strings_count);
            int l = 0;
            while (l <= m){
                T_final[i].hash_id[l] = S[l];
                l++;
            }
        }
    }
}

//Prints the content of the file read
void printfile(struct transaction T_final[]){
    int i;
    for (i=0; i<MAXTRANSACTIONS; i++){
        printf("%d ",T_final[i].id);
        int j;
        if (T_final[i].item_count>0){
            for (j=0; j<=T_final[i].item_count; j++){
                printf("%s, ",Hash[T_final[i].hash_id[j]].str);
            }
            printf("\n");
        }
    }
}

//Initializing a node for tree construction
struct tree* init_node(int S){
    struct tree *p = (struct tree*) malloc(sizeof(struct tree));
    p->hash_id = S;
    p->frequency = 1;
    p->next = NULL;
    p->parent = NULL;
    p->is_visited = 0;
    return p;
}

//Applying the specific property for insertion 
struct tree* find_and_operate(int S,int prev, struct tree* parent, struct tree* root,struct header H_final[],int *header_strings_count){
    int i,flag;
    for (i=0; i<=*header_strings_count; i++){
        if (H_final[i].hash_id == S){
            if (H_final[i].ptr == NULL){
                struct tree* node = init_node(S);
                node->parent=parent;
                parent->is_visited=0;
                H_final[i].ptr = node; 
                return node;
            }   
            else{
                struct tree* p = H_final[i].ptr;
                while (p->parent!=parent){
                    if(p->next!=NULL)
                        p = p->next;
                    else    break;
                }
                if(p!=NULL && p->parent == parent){
                    p->frequency+=1;
                    parent->is_visited=0;
                    return p;
                }
                else if (p->next == NULL){
                    struct tree *q = init_node(S);
                    q->parent=parent;
                    parent->is_visited=0;
                    p->next = q;
                    return q;
                }
            }
        }
    }
}

//Tree construction function
struct tree* construct_tree(struct transaction T_final[],struct header H_final[],int *header_strings_count,int t_count){
    struct tree *root = init_node(-1);
    int i,j;
    struct tree *temp;
    if(t_count==-1){
        t_count = MAXTRANSACTIONS-1;
    }
    int previous;
    for (i=0; i<=t_count; i++){
        temp = root;
        previous = -1;
        for (j=0; j<=T_final[i].item_count; j++){
            //printf("%d %s %d\n",T_final[i].id,T_final[i].str[j],T_final[i].item_count);
            if(T_final[i].hash_id[j]>=0){
                temp = find_and_operate(T_final[i].hash_id[j],previous,temp,root,H_final,header_strings_count);
            }
        }
    }
    /*for (i=*header_strings_count;i>=0; i--){
        struct tree *p;
        int val = 0;
        p = H_final[i].ptr;
        printf("%s: ",Hash[p->hash_id].str);
        while (p!=NULL){
            val += p->frequency;
            printf ("%d, ",p->frequency);
            p=p->next;
        }
        printf("Total= %d\n",val);
    }*/
    return root;
}

//check for multiple paths in the constructed tree
int check_multipath(struct header H_final[], int *header_strings_count){
    int i;
    for(i=0; i<=*header_strings_count; i++){
        if (H_final[i].ptr->next!=NULL){
            //printf("Multiple paths exist\n\n");
            return 1;
        }
    }
    //printf("Single path\n\n");
    return 0;
}

//Functions for printing
int get_frequency(int hash_id,struct header H[], int cnt){
    int i;
    for(i=0;i<=cnt;i++){
        if(H[i].hash_id == hash_id)
            return H[i].frequency;
    }
}

void combinationUtil(int arr[], int data[], int start, int end, int index, int r, int head[], int head_count, int frequency[], int cnt, struct header H[]);

void printCombination(int arr[], int n, int r, int head[],int head_count, int frequency[],int cnt, struct header H[]){
	int data[r];
	combinationUtil(arr, data, 0, n-1, 0, r, head,head_count,frequency,cnt,H);
}

void combinationUtil(int arr[], int data[], int start, int end,int index, int r, int head[], int head_count,int frequency[],int cnt, struct header H[]){
	if (index == r)
	{
        for (int j=0; j<=head_count; j++)
            printf("%s, ",Hash[head[j]].str);

        int ff = MAX, temporary;
		for (int j=0; j<r; j++)
        {   
            temporary = get_frequency(data[j],H,cnt);
            if(ff > temporary)  ff = temporary;
            if(j < r-1) 
                printf("%s, ", Hash[data[j]].str);
            else if (j == r-1)  printf("%s ", Hash[data[j]].str);
        }
        printf(" %d\n",ff);
		return;
	}
    for (int i=start; i<=end && end-i+1 >= r-index; i++)
	{
		data[index] = arr[i];
		combinationUtil(arr, data, i+1, end, index+1, r, head, head_count,frequency,cnt,H);
	}
}

void print_output(struct header H[], int *count, int head[], int head_count, int frequency[],int cnt){
    int i, hashes[100],j;
    //printf("%d\n",head_count);
    for (int j=0; j<=head_count; j++){
        if(j == head_count) printf("%s ",Hash[head[j]].str); 
        else    printf("%s, ",Hash[head[j]].str);
    }
    printf(" %d\n",frequency[cnt]);
    if(*count == -1){
        return;
    }    
    i=*count;
    struct tree *node;
    node = H[i].ptr;
    j = 0;
    while (node->parent != NULL){
        hashes[j] = node->hash_id;
        j++;
        node = node->parent;
    }
    for(int r =1; r<=j; r++)
        printCombination(hashes,j,r,head,head_count,frequency,cnt,H);
}

//Declaration
void fp_growth(struct tree *root,struct header H_final[],int *header_strings_count, int head[], int head_count);

//Construction of the sub trees
void construct_subtree(int S,int x, struct tree *root,struct header H_final[],int hashes[], int head){
    struct tree *node, *chain, *root1;
    chain = H_final[x].ptr;
    struct header mini_header[UNIQUESTRINGS];
    int i = -1,pos;
    struct transaction mini_T[MAXTRANSACTIONS],mini_T_final[MAXTRANSACTIONS];
    int tid=-1;
    while (chain!= NULL){
        int x=chain->frequency,y;
        y=x;
        while( x > 0){
            node = chain->parent;
            int item = -1;
            mini_T[++tid].id = tid + 1;
            while (node->parent != NULL){
                if(y-x < node->frequency){
                    int flag = search_header_linear(node->hash_id,i,mini_header,&pos);
                    mini_T[tid].hash_id[++item] = node->hash_id;
                }
                node = node->parent;
            }
            x--;
            mini_T[tid].item_count = item;
        }
        chain = chain->next;
    }
    struct header mini_header_final[UNIQUESTRINGS];
    //print_transactions(mini_T,tid);
    get_header(mini_T,mini_header,&i,tid);
    //print_header(mini_header,i);
    sort_header(mini_header,&i);
    
    eliminate_less_than_min_support(mini_header,mini_header_final,&i);
    //print_header(mini_header_final,i);
    eliminate_and_sort_transaction_items(mini_T,mini_T_final,mini_header_final,&i,tid);
    
    //print_transactions(mini_T_final,tid);
    
    struct tree* temporary = construct_tree(mini_T_final,mini_header_final,&i,tid);
    
    is_multipath = check_multipath(mini_header_final,&i);
    
    if (is_multipath == 1){
        int j;
        for (j=0; j<=head; j++){
            printf("%s ",Hash[hashes[j]].str);
        }
        printf("%d\n",H_final[x].frequency);
        
        fp_growth(temporary,mini_header_final,&i,hashes,head);
    }
    else{
        int frequency[i],j;
        //frequency[0] = tid+1;
        for (int j=0;j<=i;j++){
            frequency[j] = mini_header_final[j].frequency;
        }
        j=i+1;
        frequency[j] = tid+1;
        /*printf("Frequency: ");
        printf("%s %d\n",Hash[H_final[x].hash_id].str,tid+1);
        for(int k=1;k<=j;k++)
            printf("%s %d\n",Hash[mini_header_final[j-k].hash_id].str,frequency[k]);
        printf("\n");*/
        printf("\033[1;36m");
        print_output(mini_header_final,&i, hashes, head, frequency,j);
    }
}

//FP-growth algorithm
void fp_growth(struct tree *root,struct header H_final[],int *header_strings_count,int head[], int head_count){
    int i;
    for (i=0; i<=*header_strings_count; i++){
        //printf("Current: %s %d\n",Hash[H_final[i].hash_id].str,H_final[i].frequency);
        //int hashes[UNIQUESTRINGS],head_count=0;
        head[++head_count]=H_final[i].hash_id;
        construct_subtree(H_final[i].hash_id,i,root,H_final,head,head_count);
        head_count--;
    }    
}

int main(){
    int header_strings_count=-1;
    struct transaction T[MAXTRANSACTIONS],T_final[MAXTRANSACTIONS];
    struct header H[UNIQUESTRINGS],H_final[UNIQUESTRINGS];

    printf("\033[01;33m");
    printf("\nReading file...\n\n");
    
    readfile(T);
    //Uncomment this to read the transactions read from the file
    /*
    print_transactions(T,MAXTRANSACTIONS-1);
    */
    
    
    printf("Finished reading the file\n\n");
    printf("Processing data...\n\n");
    get_header(T,H,&header_strings_count,MAXTRANSACTIONS-1);

    sort_header(H,&header_strings_count);
    
    eliminate_less_than_min_support(H,H_final,&header_strings_count);
    
    //Uncomment this to print the header table
    /*
    print_header(H_final,header_strings_count);
    */
    
    
    eliminate_and_sort_transaction_items(T,T_final,H_final,&header_strings_count,MAXTRANSACTIONS-1);
    
    //Uncomment this to print the processed transactions
    /*
    print_transactions(T_final,MAXTRANSACTIONS-1);
    */
    
    printf("Finished processing data! Constructing FP tree..\n\n");
    
    struct tree* root = construct_tree(T_final,H_final,&header_strings_count,-1);
    
    printf("Constructed tree successfully\n\n");
    
    int hashes[UNIQUESTRINGS],head_count=-1;

    printf("Output after Running FP growth algorithm for Minimum support count = %d\n\n",MIN_SUPPORT);
    
    printf("\033[1;36m");
    
    fp_growth(root,H_final,&header_strings_count,hashes,head_count);
    
    printf("\033[01;33m\n");

    printf("Done\n");
}
