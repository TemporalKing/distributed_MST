#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "node.h"
#include <mpi.h>

using namespace std;


int main(int argc, char *argv[]){
    int k,rc,numNodes,w,*weights;
    FILE *fp;
    fp = fopen("input.txt","r");
    fscanf(fp,"%d\n",&numNodes);
    cout<<numNodes<<endl;
    
    Node *nodes;
    Node *nbrs[numNodes];
    
    
    nodes = new Node[numNodes];
    // cree de la  memoire pour mettre le tableau
    weights = (int *)malloc(numNodes*sizeof(int));
    for(int i=0;i<numNodes;i++){
        //nombre de edges du node nodes[i]
        k=0;
        //printf("debut boucle for");
        for(int j=0;j<numNodes;j++){
            fscanf(fp,"%d\t",&w);
            if(w!=0){
                weights[k]=w;
                nbrs[k]=&nodes[j];
                k++;
                
                if(j<i){
                    // construction edgeInput --> allEdges
                    edgeInput e;
                    e.left = j;
                    e.right= i;
                    allEdges.insert(pair<int,edgeInput>(w,e));
                }
            }
        }
        fscanf(fp,"\n");
        printf("fini de lire");
        
        nodes[i].init(k,weights,nbrs);
        nodes[i].ind=i;
    }
    fclose(fp);
    run = 1;
    printf("le fichier est ferme");
    
    int taskid, numtasks;
    
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    
    int i = taskid%numtasks;
    nodes[i].ind = i;
    nodes[i].readMessage();

    nodes[0].wakeup();
    
    MPI_Finalize();

    return 0;
}
