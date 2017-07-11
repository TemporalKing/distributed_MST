#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <limits>
#include <stdlib.h>
#include <boost/algorithm/string.hpp>

using namespace std; 

int main(){
  
   
   fstream fichier;
   fichier.open("barabasi_n25_m020_m2_1.txt", ios::in | ios::binary);
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
    
    cout<< "taille de la matrice : " << matrix.size() << endl;
    


      //on initialise la matrice d'adjacence avec que des 0
    
    int V = matrix.size();
    
      /*int adj_matrix[V][V];
      for(int i = 0 ; i < V ; ++i){
        for(int j = 0; j < V ; ++j){
            adj_matrix[i][j] = 0;
	       }
      }*/
    
    int** adj_matrix;
    adj_matrix = new int*[V];
     for(int i = 0 ; i < V ; ++i){
         adj_matrix[i] = new int[V];
         for(int j = 0; j < V ; ++j){
             adj_matrix[i][j] = 0;
         }
     }
    
      //on remplit la matrice d'adjacence
      // random weight quand il existe un arc
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

    cout<<"Matrice d'adjacence :"<<endl;
    for(int b = 0; b < V; ++b){
         for(int a = 0; a < V ; ++a)
               cout << adj_matrix[b][a] << "  ";
     cout<<endl;
    }
     
      fichier.close();
   
   
   return 0;
}
