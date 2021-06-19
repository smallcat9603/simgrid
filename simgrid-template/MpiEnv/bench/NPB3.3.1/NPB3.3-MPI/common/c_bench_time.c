/*****************************************************************/
/******     C  _  B E N C H _ T I M E                       ******/
/*****************************************************************/
#include "c_bench_time.h"

list_element_t* init_list()
{
  list_element_t* el=(list_element_t*)malloc(sizeof(list_element_t));
  el->sibling=NULL;
  el->next=NULL;
  el->count=0;
  return el;  
}

void insert_value(list_element_t* root, unsigned int id, double value){
  printf("Insert (%i,%g)\n",id,value);
  while(root->sibling!=NULL && id>0)
    {
      id--;
      root=root->sibling;
    }

  if(id==0)
  {
    //Data already exist for this identifier -> find element that is not full
    while(root->next!=NULL && root->count>=NBENCHVALUES)
      root=root->next;
    if(root->count>=NBENCHVALUES)
     {
       //insert a new child
       list_element_t* el=(list_element_t*)malloc(sizeof(list_element_t));
       el->sibling=NULL;
       el->next=NULL;
       el->count=0;
       root->next=el;
       root=el;
     }
    
  }else
  {
    //Create new siblings until identifier is reached
    while(root->sibling!=NULL && id>0)
     {
        id--;
        //insert a new child
        list_element_t* el=(list_element_t*)malloc(sizeof(list_element_t));
        el->sibling=NULL;
        el->next=NULL;
        el->count=0;
        root->sibling=el;
        root=el;
     }
  }

  assert(root->count<NBENCHVALUES);

  root->values[root->count++]=value;
}

double bench_start()
{
  return MPI_Wtime();
  //  return SIMIX_get_clock();
}

void bench_stop(list_element_t* root, unsigned int id, unsigned int idstart,double start)
{
  // double stop=SIMIX_get_clock();
  double stop=MPI_Wtime();
  assert(id==idstart);
  insert_value(root,id,stop-start);
}
  

void bench_finalize(list_element_t* root)
{
  char filename[]="bench.csv";
  //char filename[100];
  //sprintf(filename,"bench_%i.csv",rank);
  FILE* file;      
  char hasData;
  //The current display row
  unsigned int row=0;
  //The relative value index
  unsigned int index;
  //Element on top of each column
  list_element_t* base;
  //Element descending in each column to get the value to display 
  list_element_t* el;
  //Temporary element for deletion 
  list_element_t* del;

  MPI_Status  status;
  int comm_size,my_rank;
  MPI_Comm_rank( MPI_COMM_WORLD, &my_rank );
  MPI_Comm_size(MPI_COMM_WORLD,&comm_size);
  //Init the table entries
  if(my_rank==0)
   {
     file=fopen(filename,"w");      
     fprintf(file,"Rank;\tIteration;\t");
     base=root;
     index=0;
     do{   
       fprintf(file,"E%i;\t",index++);
       base=base->sibling;
     }while(base!=NULL);
     fprintf(file,"\n");
   }else{
    MPI_Recv(NULL,0,MPI_INT,my_rank-1,1234,MPI_COMM_WORLD,&status);
    file=fopen(filename,"a");      
  }


  
  //Save bench data
  do{
    base=root;
    hasData=0;
    do{
	el=base;
	index=row;
	fprintf(file,"%i;\t%i;\t",my_rank,row);
	while(el->next!=NULL && index>NBENCHVALUES)
	{
	  el=el->next;
	  index-=NBENCHVALUES;
	}
	if(index>NBENCHVALUES || index>=el->count)
	  {
	    //Empty value
	    fprintf(file," ;\t");
	  }else
	  {
	    //Get the value
	    fprintf(file,"%g;\t",el->values[index]);
	    hasData=1;
	  }
	base=base->sibling;
    }while(base!=NULL);
    fprintf(file,"\n");
    row++;
  } while(hasData>0);

  fclose(file);

  if(my_rank+1<comm_size)
  {
    MPI_Send(NULL,0,MPI_INT,my_rank+1,1234,MPI_COMM_WORLD);
  }
  
  //Free heap
  
  base=root;
  do{
    el=base;
    base=base->sibling;
    do{
      del=el;
      el=el->next;
      free(del);
    }while(el!=NULL);
  }while(base!=NULL);

}
