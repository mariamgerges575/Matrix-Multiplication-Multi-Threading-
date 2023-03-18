#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

//declaration of global variables
int a[20][20];
int b[20][20];
int c[20][20];
int raw_a,raw_b,col_a,col_b;

//struct that holds raw and col which will be multiplied
struct element{
    int raw;
    int col;
};

//function reads all information from a file which it take its name as a parameter
//and also takes pointers to store number of raws and columns in this address
//which array is a parameter to tell if it's the file of first or second array
void readFile(char str[100],int *r,int *c,int whichArray){
    FILE *file = fopen(str, "r");
    if (file == NULL)
    {
        perror("Error opening file\n");
        exit(1);
    }
    char ch='0';
    while(ch!='='){
        ch=fgetc(file); //skip all character and take the integer after "="
    }
    fscanf(file,"%d",r);
    ch='0';
    while(ch!='='){
        ch=fgetc(file); //skip all character and take the integer after "="
    }
    fscanf(file,"%d",c);
    for(int i=0;i<*r;i++){
        for(int j=0;j<*c;j++){
            if(whichArray==1)
                fscanf(file,"%d",&a[i][j]);
            else if(whichArray==2)
                fscanf(file,"%d",&b[i][j]);
        }
    }
    fclose(file);


}
/*
Function which takes number of raw ,but as a pointer of void and this is to be able to pass parameter while creating thread,
and multiply this with the other matrix to the find the ith raw of the result.
*/
void * M_multip_per_raw(void *arg){
    int raw=*(int*)arg;
    for(int k=0;k<col_b;k++){
        c[raw][k]=0;
        for(int j=0;j<raw_b;j++){
            c[raw][k]+=a[raw][j]*b[j][k];
        }
    }
    free(arg);
}

/*
Function that takes struct of element holds number of raw I and column j
,but as a pointer of void and this is because pthread_creat function requires that,
to multiply this raw of the first array with this column of the second array
and find the jth element of ith row of the result.
*/
void * M_multip_per_element(void *arg){
    struct element e=*(struct element*)arg;
    c[e.raw][e.col]=0;
    for(int j=0;j<raw_b;j++){
        c[e.raw][e.col]+=a[e.raw][j]*b[j][e.col];
    }
    free(arg);
}
/*
This function holds the implementation of normal matrix multiplication,
it doesn’t take any parameter as the matrices and its dimensions are global
*/
void * M_multip_per_mat(){

    for(int i =0;i<raw_a;i++){
        for(int k=0;k<col_b;k++){
            for(int j=0;j<raw_b;j++){
                c[i][k]+=a[i][j]*b[j][k];
            }
        }
    }
}
//functions that handle threads of the three methods
void th_handling_per_mat(){
    pthread_t thread_id;
    pthread_create(&thread_id,NULL,&M_multip_per_mat,NULL);
    pthread_join(thread_id, NULL);
}
void th_handling_per_row(){
    pthread_t thread_id[raw_a];

    for (int i =0;i<raw_a;i++){
        int *x=malloc(sizeof(int));
        *x=i;
        pthread_create(&thread_id[i],NULL,&M_multip_per_raw,x);

    }
    for (int i =0;i<raw_a;i++){
        pthread_join(thread_id[i], NULL);
    }
}
void th_handling_per_element(){
    pthread_t thread_id[raw_a][col_b];
    struct element e;
    for(int i=0;i<raw_a;i++){
        for(int j=0 ;j<col_b;j++){
            struct element *x=malloc(sizeof(struct element));
            (*x).col=j;
            (*x).raw=i;
            pthread_create(&thread_id[i][j],NULL,&M_multip_per_element,x);
        }
    }
     for(int i=0;i<raw_a;i++){
        for(int j=0 ;j<col_b;j++){
            pthread_join(thread_id[i][j], NULL);
        }
     }

}
//function to save the result in a specified file
void saveArray(char str[100]){
    FILE *f=fopen(str,"w");
    if (f == NULL){
        perror("Error opening file\n");
        exit(1);
    }
    for(int i =0;i<raw_a;i++){
        for(int j=0;j<col_b;j++){
            fprintf(f,"%d ",c[i][j]);
        }
        fprintf(f,"\n");
    }
    fclose(f);
}
int main(int argc,char *argv[])
{

    char afile[100],bfile[100],outputfile1[100],outputfile2[100],outputfile3[100];

    if(argc<4){ //make default arguments when there is no arguments
        strcpy(afile,"a");
        strcpy(bfile,"b");
        strcpy(outputfile1,"c");
        strcpy(outputfile2,"c");
        strcpy(outputfile3,"c");

    }
    else { //if there are arguments take them
        strcpy(afile,argv[1]);
        strcpy(bfile,argv[2]);
        strcpy(outputfile1,argv[3]);
        strcpy(outputfile2,argv[3]);
        strcpy(outputfile3,argv[3]);


    }
    //concatenate the file names with the text extension
    strcat(afile,".txt");
    strcat(bfile,".txt");
    strcat(outputfile1,"_per_matrix.txt");
    strcat(outputfile2,"_per_row.txt");
    strcat(outputfile3,"_per_element.txt");


    readFile(afile,&raw_a,&col_a,1);
    readFile(bfile,&raw_b,&col_b,2);


    struct timeval stop, start;

    gettimeofday(&start, NULL); //start checking time
    th_handling_per_mat();
    gettimeofday(&stop, NULL);//end checking time
    saveArray(outputfile1);
    printf("\nmultiplication per matrix\nSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("number of threads =1\n");

    gettimeofday(&start, NULL); //start checking time
    th_handling_per_row();
    gettimeofday(&stop, NULL);//end checking time
    saveArray(outputfile2);
    printf("\nmultiplication per raw\nSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("number of threads = %d\n",raw_a);

    gettimeofday(&start, NULL); //start checking time
    th_handling_per_element();
    gettimeofday(&stop, NULL);//end checking time
    saveArray(outputfile3);
    printf("\nmultiplication per raw\nSeconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("number of threads = %d\n",raw_a*col_b);




    return 0;
}
