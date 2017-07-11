#include <iostream>
#include <limits.h>
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


void primMST(int** graph, int V)
{
    int pred[V]; // tableau des sommets choisis successivement
    int cost[V];   // cout de chaque sommet a ajouter, mis a jour a chaque ajout
    bool isInMST[V];  // chaque fois qu'on ajoute un sommet a pred, insertion "logique"
    
    // Initialisation : couts infinis
    for (int i = 0; i < V; i++)
        cost[i] = INT_MAX, isInMST[i] = false;
    
    // On commence par le sommet 0
    cost[0] = 0;
    pred[0] = -1; // C'est le premier : il n'a pas de predecesseur
    
    // Ajout des sommets a pred
    for (unsigned int i = 0; i < V-1; i++)
    {
        // on ajoute le sommet u de cout minimal qui n'est pas encore inclus dans le MST
        int u;
        int min = INT_MAX;
        
        for (int v = 0; v < V; v++)
            if (isInMST[v] == false && cost[v] < min)
                min = cost[v], u = v;
        
        
        // insertion "logique"
        isInMST[u] = true;
        
        // Mise a jour de pred
        // Mise a jour des couts des sommets qui ne sont pas encore dans le MST
        for (unsigned int v = 0; v < V; v++){
            if (graph[u][v] > 0 && isInMST[v] == false && graph[u][v] <  cost[v]){
                pred[v] = u, cost[v] = graph[u][v];
            }
        }
    }
    
    // Affichage du MST construit
    printMST(pred, graph, V);
}



int main(int argc, char *argv[])
{
    
    
// LECTURE DU GRAPHE DANS UN FICHIER
    
    fstream fichier;
    fichier.open("barabasi_n150_m020_m2_1.txt", ios::in | ios::binary);
    string contenu;
    getline(fichier, contenu);
    
    //arriver a la premiere ligne de donnees
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
    fichier.close();
    
    //on initialise la matrice d'adjacence avec que des 0
    
    int V = matrix.size();
    
    int** adj_matrix;
    adj_matrix = new int*[V];
    for(int i = 0 ; i < V ; ++i){
        adj_matrix[i] = new int[V];
        for(int j = 0; j < V ; ++j){
            adj_matrix[i][j] = 0;
        }
    }
    
    // on remplit la matrice d'adjacence
    // on met des poids random aux arcs
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
    
// FIN DE LA LECTURE
    
    cout<< "Matrice d'adjacence :"<<endl;
    for(int b = 0; b < V; ++b){
        for(int a = 0; a < V ; ++a)
            cout << adj_matrix[b][a] << "  ";
        cout<<endl;
    }

    clock_t start;
    double duration;
    start = clock();
    
    /* Your algorithm here */

    primMST(adj_matrix,V);
    
    duration = ( clock() - start ) / (double) CLOCKS_PER_SEC;
    cout<<"Temps d'execution : "<< duration <<'\n';
    
    return 0;

}

