#include <iostream>
#include <limits.h>
#include <mpi.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <stdlib.h>
#include <ctime>


using namespace std;


// Fonction d'affichage du MST
int printMST(int pred[], int** graph, int V)
{
    cout<<"Edge   Weight"<< endl;
    for (int i = 1; i < V; i++)
        cout<<pred[i]<<" - "<<i<<"    "<<graph[i][pred[i]]<<endl;
}


void primMST(int** graph, int V, int numtasks, int taskid)
{
    int pred[V]; // tableau des sommets choisis successivement
    int cost[V];   // cout de chaque sommet a ajouter, mis a jour a chaque ajout
    // Pas un tableau de booleens mais de 0,1 pour envoyer par MPI
    int isInMST[V];  // chaque fois qu'on ajoute un sommet a pred, insertion "logique"
    
    
    const int root = 0;
    
    // On commence par ajouter le sommet 0
    cost[0] = INT_MAX;
    pred[0] = -1; // C'est le premier : il n'a pas de predecesseur
    isInMST[0] = 1;
    
    
    MPI_Status stat;
    MPI_Request request = MPI_REQUEST_NULL;
    

    //INITIALISATION
    for (unsigned int v = 1; v < V; v++){
        if (graph[0][v] > 0){
            isInMST[v] = 0, pred[v] = 0, cost[v] = graph[0][v];
        }
        else {
            isInMST[v] = 0, pred[v] = 0, cost[v] = INT_MAX;
        }
    }
    
    
    // Ajout des sommets a pred
    for (unsigned int i = 1; i < V - 1; i++)
    {
        
        int local_u = 0;
        int local_min = INT_MAX;
        
        // Chaque processus s'occupe de ses sommets et trouve le sommet de cout minimum localement
        for (int v = taskid; v < V; v+=numtasks){
            if (isInMST[v] == 0 && cost[v] < local_min){
                    local_min = cost[v], local_u = v;
            }
        }
        
        MPI_Send(&local_u,1,MPI_INT,root,0,MPI_COMM_WORLD);
        
        int u_min = 0;
        int u = 0;
        
        if (taskid == root){
            int min = INT_MAX;
            for (int p = 0; p < numtasks; p++){
                // processus root recoit les minima locaux et choisit le bon candidat
                MPI_Recv(&u,1,MPI_INT,p,0,MPI_COMM_WORLD,&stat);
                if (cost[u] < min){
                    min = cost[u], u_min = u;
                }
            }
            // insertion "logique"
            isInMST[u_min] = 1;
        }
        
        MPI_Bcast(&u_min, 1, MPI_INT, root, MPI_COMM_WORLD);
        MPI_Bcast(&isInMST, V, MPI_INT, root, MPI_COMM_WORLD);



        // Mise a jour de pred par chaque processus
        // Mise a jour des couts des sommets qui ne sont pas encore dans le MST
        for (unsigned int v = taskid; v < V; v+=numtasks){
            if (graph[u_min][v] > 0 && isInMST[v] == 0 && graph[u_min][v] <  cost[v] && v > 0){
                pred[v] = u_min, cost[v] = graph[u_min][v];
            }
        }

        // Envoi des tableaux cost et pred mis a jour a ROOT
        MPI_Send(&cost,V,MPI_INT,root,2,MPI_COMM_WORLD);
        MPI_Send(&pred,V,MPI_INT,root,2,MPI_COMM_WORLD);

        if (taskid == 0){
            int tmp_cost[V];
            int tmp_pred[V];
            for (int p = 0; p < numtasks; p++){
                MPI_Recv(&tmp_cost,V,MPI_INT,p,2,MPI_COMM_WORLD,&stat);
                MPI_Recv(&tmp_pred,V,MPI_INT,p,2,MPI_COMM_WORLD,&stat);
                for (int j = p ; j < V ; j+=numtasks){
                    cost[j] = tmp_cost[j];
                    pred[j] = tmp_pred[j];
                }
            }
        }
        MPI_Bcast(&pred, V, MPI_INT, root, MPI_COMM_WORLD);
        MPI_Bcast(&cost, V, MPI_INT, root, MPI_COMM_WORLD);

        MPI_Wait(&request,&stat);
    }
    
    // Imprimer le MST construit
    if (taskid==0)
        printMST(pred,graph,V);
}



int main(int argc, char *argv[])
{
    /*int graph[7][7] = {{0, 28, 0, 0, 0, 10, 0},
        		{28, 0, 16, 0, 0, 0, 14},
        		{0, 16, 0, 12, 0, 0, 0},
        		{0, 0, 12, 0, 22, 0, 18},
        		{0, 0, 0, 22, 0, 25, 24},
        		{10, 0, 0, 0, 25, 0, 0},
        		{0, 14, 0, 18, 24, 0, 0},
    };*/
    
    
    
    // LECTURE DU GRAPHE DANS UN FICHIER
    
    fstream fichier;
    fichier.open("barabasi_n100_m020_m2_1.txt", ios::in | ios::binary);
    string contenu;
    getline(fichier, contenu);
    
    // But : arriver a la premiere ligne de donnees
    for(int i = 0; i < 4; i++){
        getline(fichier, contenu);
    }
    
    // Lecture du graphe
    // matrice qui va stocker les donnees du graphes
    vector< vector<int> > matrix(0);
    
    while(getline(fichier, contenu)){
        istringstream iss(contenu);
        string array_number = contenu.substr(contenu.find("[")+1,contenu.size());
        string number_as_string;
        int array_size = 0;
        while (iss){
            if (!getline( iss, number_as_string, ',' )) break;
            array_size ++;
        }
        vector<int> neighbours(0);
        for (int k = 0 ; k < array_size - 1 ; k++){
            number_as_string = array_number.substr(0,array_number.find(", "));
            char ns[number_as_string.size()];
            for (int j = 0 ; j < number_as_string.size() ; j++) ns[j]=number_as_string[j];
            neighbours.push_back(atoi(ns)); //transforme string en int
            array_number = array_number.substr(array_number.find(", ")+2, array_number.size());
            
        }
        // on arrive au dernier neighbour de array_number
        number_as_string = array_number.substr(0,array_number.find(", ")-1);
        char ns[number_as_string.size()];
        for (int j = 0 ; j < number_as_string.size() ; j++) ns[j]=number_as_string[j];
        neighbours.push_back(atoi(ns)); //transforme string en int
        
        // on ajoute le vecteur de neighbours a la matrice de lecture
        matrix.push_back(neighbours);
    }
    
    //on initialise la matrice d'adjacence avec des 0
    int V = matrix.size();
    
    int** adj_matrix;
    adj_matrix = new int*[V];
    for(int i = 0 ; i < V ; ++i){
        adj_matrix[i] = new int[V];
        for(int j = 0; j < V ; ++j){
            adj_matrix[i][j] = 0;
        }
    }
    
    //on remplit la matrice d'adjacence
    //on attribue des poids random aux arcs
    vector<int> neighbours;
    for(int i = 0 ; i < V ; ++i){
	       neighbours = matrix[i];
        for(int j = 0; j < neighbours.size() ; ++j){
            int r = rand()%10;
            adj_matrix[neighbours[j]-1][i] = r;
            adj_matrix[i][neighbours[j]-1] = r;
            //adj_matrix[i][neighbours[j]-1] = 1;
	       }
    }
    
     /*cout<<"Matrice d'adjacence :"<<endl;
     for(int b = 0; b < V; ++b){
        for(int a = 0; a < V ; ++a)
            cout << adj_matrix[b][a] << "  ";
        cout<<endl;
    }*/
    
    fichier.close();
    
    // FIN DE LA LECTURE

    
    
    int name_len;
    int numtasks, taskid;
    
    
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    clock_t start;
    double duration;
    start = clock();
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    

    primMST(adj_matrix,V, numtasks,taskid);

    MPI_Finalize();

    
    duration = ( clock() - start ) / (double) CLOCKS_PER_SEC;
    cout<<"Temps d'execution : "<< duration <<'\n';
    return 0;
}

