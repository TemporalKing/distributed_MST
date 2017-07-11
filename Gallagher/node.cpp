#include <mpi.h>
#include "node.h"

#define INF 1000000

void Node::init(int numEdges, int weights[], Node* nodes[]) {
    this->numEdges = numEdges;
    this->edges = (edge *)malloc(sizeof(edge) * numEdges);
    for (int i = 0; i < numEdges; ++i) {
        this->edges[i].state = 0;
        this->edges[i].weight = weights[i];
        this->edges[i].node = nodes[i];
    }
    this->state = 0;
    this->bestEdge = -1;
    this->bestWeight = INF;
    this->testEdge = -1;
    this->parent = -1;
    this->level = -1;
    this->findCount = -1;
    this->id = INF;
}

Node::Node() {}

Node::~Node() {
    delete(this->edges);
}


void Node::readMessage() {
        int data[5];
        MPI_Status status;
        MPI_Recv(&data, 4, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    // Identification du edge (i ici = j du pseudo code)
        int i = data[4];
    
        if(this->state == 0)
            this->wakeup();
    
        int id = data[3];
    
        switch(id) {
            case 0:
                connect(data[0], i);
                break;
            case 1:
                initiate(data[0], data[1], data[2], i);
                break;
            case 2:
                testMessage(data[0], data[1], i);
                break;
            case 3:
                accept(i);
                break;
            case 4:
                reject(i);
                break;
            case 5:
                reportMessage(data[0], i);
                break;
            case 6:
                changeRootMessage(i);
                break;
            case 7:
                this->wakeup();
        }

}

int Node::findMinEdge() {
    int min = INF, index = 0;
    for (int i = 0; i < numEdges; i++) {
        if (this->edges[i].weight < min) {
            min = this->edges[i].weight;
            index = i;
        }
    }
    return index;
}

void Node::wakeup() {
    int minEdge = findMinEdge();
    // Branch state
    this->edges[minEdge].state = 1;
    //HORROR: MST IS GLOBAl
    // But it is not in pseudo code: maybe we can throw it
    mst.insert(pair<int,int>(this->edges[minEdge].weight,1));
    this->level = 0;
    // Found State
    this->state = 2;
    this->findCount = 0;
    int data[5];
    data[3] = 0; // message id
    data[0] = 0;
    data[4] = this->edges[minEdge].weight;
    MPI_Send(&data, 5, MPI_INT, this->edges[minEdge].node->ind, 0, MPI_COMM_WORLD);
}

void Node::connect(int L, int j) {
    if (L < this->level) {
        // Branch state
        this->edges[j].state = 1;
        int data[5];
        data[3] = 1;
        data[0] = this->level;
        data[1] = this->id;
        data[2] = this->state;
        data[4] = this->edges[j].weight;
        MPI_Send(&data, 5, MPI_INT, this->edges[j].node->ind, 1, MPI_COMM_WORLD);
    }
    else if (this->edges[j].state == 0) {
        int data[5];
        data[3] = 0;
        data[0] = L;
        data[4] = this->edges[j].weight;
        MPI_Send(&data, 5, MPI_INT, this->ind, 0, MPI_COMM_WORLD);
    }
    else {
        int data[5];
        data[3] = 1; // Initiate Message
        data[0] = this->level + 1;
        data[1] = this->edges[j].weight;
        data[2] = 1; // Find State
        data[4] = this->edges[j].weight;
        MPI_Send(&data, 5, MPI_INT, this->edges[j].node->ind, 1, MPI_COMM_WORLD);
    }
}

void Node::initiate(int L, int id, int stateOfNode, int j) {
    this->level = L; // LN <- L
    this->id = id; // FN <- F
    this->state = stateOfNode; // SN <- S
    this->parent = j; // in-branch <- j
    this->bestEdge = -1; // best-edge <- nil
    this->bestWeight = INF; // best-wt <- infinity
    for (int i = 0; i < numEdges; ++i) {
        if (i != j && this->edges[i].state == 1) {
            int data[5];
            data[3] = 1; // Initiate Message
            data[0] = L;
            data[1] = id;
            data[2] = stateOfNode; // Find State
            data[4] = this->edges[i].weight;
            MPI_Send(&data, 5, MPI_INT, this->edges[i].node->ind, 1, MPI_COMM_WORLD);
        }
    }
    if (stateOfNode == 1) {
        this->findCount = 0;
        test();
    }
}

void Node::test() {
    int i, min, min_ind;
    min = INF;
    min_ind = -1;
    for (i = 0; i < numEdges; i++) {
        if (this->edges[i].state == 0) {
            if(min>this->edges[i].weight){
                min = this->edges[i].weight;
                min_ind = i;
            }
        }
    }
    if(min_ind>=0){
        this->testEdge = min_ind;
        int data[5];
        data[3] = 2; // Test Message
        data[0] = this->level;
        data[1] = this->id;
        data[4] = this->edges[min_ind].weight;
        MPI_Send(&data, 5, MPI_INT, this->edges[min_ind].node->ind, 2, MPI_COMM_WORLD);
    }
    else{
        this->testEdge = -1;
        report();
    }
}

void Node::testMessage(int L, int id, int j) {
    if (L > this->level) {
        int data[5];
        data[3] = 2; // Test Message
        data[0] = L;
        data[1] = id;
        data[4] = this->edges[j].weight;
        MPI_Send(&data, 5, MPI_INT, this->ind, 2, MPI_COMM_WORLD);
    }
    else if (id == this->id) {
        if (this->edges[j].state == 0)
            this->edges[j].state = 2;
        if (j != this->testEdge) {
            int data[5];
            data[3] = 4; // Reject Message
            data[4] = this->edges[j].weight;
            MPI_Send(&data, 5, MPI_INT, this->edges[j].node->ind, 4, MPI_COMM_WORLD);
        }
        else
            test();
    }
    else{
        int data[5];
        data[3] = 3; // Accept Message
        data[4] = this->edges[j].weight;
        MPI_Send(&data, 5, MPI_INT, this->edges[j].node->ind, 3, MPI_COMM_WORLD);
    }
}

void Node::accept(int j) {
    this->testEdge = -1;
    if(this->edges[j].weight<this->bestWeight){
        this->bestEdge = j;
        this->bestWeight = this->edges[j].weight;
    }
    this->report();
}

void Node::reject(int j) {
    if(this->edges[j].state == 0)
        this->edges[j].state = 2;
    this->test();
}

void Node::report() {
    int i,k;
    k=0;
    for(i=0;i<this->numEdges;i++){
        if(this->edges[i].state==1 && i!=this->parent)
            k++;
    }
    if(findCount == k && this->testEdge == -1){
        this->state = 2;
        int data[5];
        data[3]=5;
        data[0]=this->bestWeight;
        data[4] = this->edges[this->parent].weight;
        MPI_Send(&data, 5, MPI_INT, this->edges[this->parent].node->ind, 5, MPI_COMM_WORLD);
    }
}

void quicksort(vector<int> &dist, vector<int> &ind, int low, int high) {
    int i = low;
    int j = high;
    int z = dist[(low + high) / 2];
    do {
        while(dist[i] < z) i++;
        
        while(dist[j] > z) j--;
        
        if(i <= j) {
            /* swap two elements */
            int t = dist[i];
            int ti = ind[i];
            dist[i]=dist[j];
            dist[j]=t;
            
            ind[i] = ind[j];
            ind[j]=ti;
            i++;
            j--;
        }
    } while(i <= j);
    
    if(low < j)
        quicksort(dist,ind, low, j);
    
    if(i < high)
        quicksort(dist,ind, i, high);
}
void Node::print_output(){
    FILE *fp;
    vector<int> n1;
    vector<int> n2;
    vector<int> ind;
    int i;
    i = 0;
    for(map<int,int>::iterator it = mst.begin(); it != mst.end(); ++it){
        n1.push_back(allEdges[it->first].left);
        n2.push_back(allEdges[it->first].right);
        ind.push_back(i++);
    }
    quicksort(n1,ind,0,n1.size()-1);
    fp = fopen("output.txt","w");
    for(i=0;i<n1.size();i++){
        fprintf(fp,"%d->%d\n",n1[i],n2[ind[i]]);
    }
    fclose(fp);
}


void Node::reportMessage(int weight, int j) {
    if(j != this->parent){
        if(weight < this->bestWeight){
            this->bestEdge = j;
            this->bestWeight = weight;
        }
        this->findCount += 1;
        report();
    }
    else{
        if(this->state == 1){
            int data[5];
            data[3] = 5;
            data[0]=weight;
            data[4] = this->edges[j].weight;
            MPI_Send(&data, 5, MPI_INT, this->ind, 5, MPI_COMM_WORLD);
        }
        else if(weight > this->bestWeight){
            changeRoot();
        }
        else if (weight == INF && this->bestWeight == INF){
            print_output();
            run = 0;
        }
    }
}	
void Node::changeRoot() {
    int data[5];
    if(this->edges[this->bestEdge].state == 1){
        data[3]=6;
        data[4] = this->edges[this->bestEdge].weight;
        MPI_Send(&data, 5, MPI_INT, this->edges[this->bestEdge].node->ind, 6, MPI_COMM_WORLD);
    }
    else{
        data[3]=0;
        data[0]=this->level;
        data[4] = this->edges[this->bestEdge].weight;
        MPI_Send(&data, 5, MPI_INT, this->edges[this->bestEdge].node->ind, 0, MPI_COMM_WORLD);
        this->edges[this->bestEdge].state = 1;
        //HORROR: MST IS GLOBAl
        // But it is not in pseudo code: maybe we can throw it
        mst.insert(pair<int,int>(this->edges[this->bestEdge].weight,1));
    }
}

void Node::changeRootMessage(int j) {
    changeRoot();
}
