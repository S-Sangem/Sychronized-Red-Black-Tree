/**
* Author @Sushanth Sangem
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <string>
#include <semaphore.h>
#include <pthread.h>

struct Node
{
    int key;
    struct Node *parent;
    struct Node *left;
    struct Node *right;
    int color; //black = 0, red = 1
};
typedef Node *NodePtr;

using namespace std;

//Global Variables
NodePtr root;
NodePtr TNULL;
string outputString;
sem_t sem;
pthread_t *reader;
pthread_t *writer;

//The following 2 methods are related to searching the RBT
NodePtr searchRBTHelper(NodePtr node, int data)
{
    if (node == TNULL || data == node->key)
    {
        return node;
    }

    if (data < node->key) {
        return searchRBTHelper(node->left, data);
    }
    return searchRBTHelper(node->right, data);
}

NodePtr searchRBT(int k)
{
    return searchRBTHelper(root, k);
}

//The following 4 methods are related to balancing the RBT
NodePtr minimum(NodePtr node)
{
    while (node->left != TNULL)
    {
        node = node->left;
    }
    return node;
}


NodePtr maximum(NodePtr node)
{
    while (node->right != TNULL)
    {
        node = node->right;
    }
    return node;
}

void leftRotate(NodePtr x)
{
    NodePtr y = x->right;
    x->right = y->left;
    if (y->left != TNULL)
    {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr)
    {
        root = y;
    }
    else if (x == x->parent->left)
    {
        x->parent->left = y;
    }
    else
    {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

void rightRotate(NodePtr x)
{
    NodePtr y = x->left;
    x->left = y->right;
    if (y->right != TNULL)
    {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr)
    {
        root = y;
    }
    else if (x == x->parent->right)
    {
        x->parent->right = y;
    }
    else
    {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

//Rebalances the RBT after a node deletion
void fixDelete(NodePtr x)
{
    NodePtr s;
    while (x != root && x->color == 0)
    {
        if (x == x->parent->left)
        {
            s = x->parent->right;
            if (s->color == 1)
            {
                s->color = 0;
                x->parent->color = 1;
                leftRotate(x->parent);
                s = x->parent->right;
            }

            if (s->left->color == 0 && s->right->color == 0)
            {
                s->color = 1;
                x = x->parent;
            }
            else
            {
                if (s->right->color == 0)
                {
                    s->left->color = 0;
                    s->color = 1;
                    rightRotate(s);
                    s = x->parent->right;
                }
                s->color = x->parent->color;
                x->parent->color = 0;
                s->right->color = 0;
                leftRotate(x->parent);
                x = root;
            }
        }
        else
        {
            s = x->parent->left;
            if (s->color == 1)
            {
                s->color = 0;
                x->parent->color = 1;
                rightRotate(x->parent);
                s = x->parent->left;
            }

            if (s->right->color == 0 && s->right->color == 0)
            {
                s->color = 1;
                x = x->parent;
            }
            else
            {
                if (s->left->color == 0)
                {

                    s->right->color = 0;
                    s->color = 1;
                    leftRotate(s);
                    s = x->parent->left;
                }

                s->color = x->parent->color;
                x->parent->color = 0;
                s->left->color = 0;
                rightRotate(x->parent);
                x = root;
            }
        }
    }
    x->color = 0;
}

//Sets nodes to red or black according to the logic of RBTs
void swapColors(NodePtr u, NodePtr v)
{
    if (u->parent == nullptr)
    {
        root = v;
    }
    else if (u == u->parent->left)
    {
        u->parent->left = v;
    }
    else
    {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

//Helper for the deleteNode method
void deleteNodeHelper(NodePtr node, int data)
{
    NodePtr z = TNULL;
    NodePtr x, y;
    while (node != TNULL){
        if (node->key == data)
        {
            z = node;
        }

        if (node->key <= data)
        {
            node = node->right;
        } else {
            node = node->left;
        }
    }

    if (z == TNULL)
    {
        cout<<"Key not found for: "<< data <<endl;
        return;
    }

    y = z;
    int y_original_color = y->color;
    if (z->left == TNULL)
    {
        x = z->right;
        swapColors(z, z->right);
    }
    else if (z->right == TNULL)
    {
        x = z->left;
        swapColors(z, z->left);
    }
    else
    {
        y = minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z)
        {
            x->parent = y;
        }
        else
        {
            swapColors(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        swapColors(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    delete z;
    if (y_original_color == 0){
        fixDelete(x);
    }
}

//Rebalances the RBT after a node insertion
void fixInsert(NodePtr k)
{
    NodePtr u;
    while (k->parent->color == 1)
    {
        if (k->parent == k->parent->parent->right)
        {
            u = k->parent->parent->left;
            if (u->color == 1)
            {
                u->color = 0;
                k->parent->color = 0;
                k->parent->parent->color = 1;
                k = k->parent->parent;
            }
            else
            {
                if (k == k->parent->left)
                {
                    k = k->parent;
                    rightRotate(k);
                }
                k->parent->color = 0;
                k->parent->parent->color = 1;
                leftRotate(k->parent->parent);
            }
        }
        else
        {
            u = k->parent->parent->right;

            if (u->color == 1)
            {
                u->color = 0;
                k->parent->color = 0;
                k->parent->parent->color = 1;
                k = k->parent->parent;
            }
            else
            {
                if (k == k->parent->right)
                {
                    k = k->parent;
                    leftRotate(k);
                }
                k->parent->color = 0;
                k->parent->parent->color = 1;
                rightRotate(k->parent->parent);
            }
        }
        if (k == root)
        {
            break;
        }
    }
    root->color = 0;
}

//Inserts a node into the RBT
void insert(int data)
{
    sem_post(&sem);

    NodePtr node = new Node;
    node->parent = nullptr;
    node->key = data;
    node->left = TNULL;
    node->right = TNULL;
    node->color = 1;

    NodePtr y = nullptr;
    NodePtr x = root;

    while (x != TNULL)
    {
        y = x;
        if (node->key < x->key)
        {
            x = x->left;
        } else
        {
            x = x->right;
        }
    }


    node->parent = y;
    if (y == nullptr)
    {
        root = node;
    } else if (node->key < y->key)
    {
        y->left = node;
    } else
    {
        y->right = node;
    }


    if (node->parent == nullptr)
    {
        node->color = 0;
        return;
    }


    if (node->parent->parent == nullptr)
    {
        return;
    }

    fixInsert(node);

    sem_wait(&sem);
}

//Deletes a node in the RBT
void * deleteNode(void * key)
{
    sem_post(&sem);
    int node = *((int *)key);
    deleteNodeHelper(root, node);
    sem_wait(&sem);
}

//Helper for the printRBT method
void printRBTHelper(NodePtr root, bool last)
{
    if (root == TNULL)
    {
        outputString += "f,";
    }
    else {
        string sColor = root->color?"r":"b";
        int skey = root->key;

        outputString += to_string(skey) + sColor + ",";

        //recursively call print
        printRBTHelper(root->left, false);
        printRBTHelper(root->right, true);
    }
}

//Prints the RBT in preorder
void printRBT()
{
    if (root)
    {
        outputString = "";
        printRBTHelper(root, true);
        outputString.pop_back();
        cout << outputString << endl;
    }
}

//Concurrently deletes nodes
void deleteNodesConcurrently(int key, int wthreads)
{
    int * b = new int(key);

    pthread_create(&writer[0], nullptr, deleteNode, (void *)b);
    pthread_join(writer[0], nullptr);
}

//Initiallizes a new RBT
void init_RBT()
{
    TNULL = new Node;
    TNULL->color = 0;
    TNULL->left = nullptr;
    TNULL->right = nullptr;
    root = TNULL;
}

int main()
{
    //initiallizes a new RBT
    init_RBT();


    ifstream input;
    int insertSearch;
    int insertModify;


    input.open("input.txt"); //change the file name here for alternate input

    while(input) //loop for parsing string
    {
        string line;
        //string testcase = "Test case1";
        string testcase = "Test case";
        string searchthreads = "Search";
        string modifythreads = "Modify";
        getline(input, line);

        if (testcase == line.substr(0,9)) //starts a test case
        {
            init_RBT();
            cout << endl;
            cout << "Working on " << line << endl;
            string rbtfill;
            getline(input, rbtfill);
            getline(input, rbtfill); //should be the rbt key

            int i = 0;
            string tempInt;
            while(rbtfill[i])
            {
                if(isdigit((int)rbtfill[i])) //Checks for numbers and saves it to a string
                {
                    tempInt += rbtfill[i];
                }
                //inserts a node
                if(rbtfill[i] == 'b')
                {
                    int insertInt = stoi(tempInt);
                    tempInt = "";
                    //cout << insertInt << endl;
                    insert(insertInt);
                }
                if(rbtfill[i] == 'r')
                {
                    int insertInt = stoi(tempInt);
                    tempInt = "";
                    //cout << insertInt << endl;
                    insert(insertInt);
                }
                i++;
            }

            //move to the thread count line
            getline(input, line);
            getline(input, line);

            //logic for thread count
            i = 16;
            //Creating the search threads
            if (searchthreads == line.substr(0,6)) //starts a test case
            {
                while(line[i])
                {
                    if(isdigit((int)line[i]))
                    {
                        tempInt += line[i];
                    }
                    i++;
                }
                insertSearch = stoi(tempInt);
                reader = new pthread_t[insertSearch];
                cout << "Search threads: " << insertSearch << endl;



                tempInt = "";
            }

            getline(input, line);
            i = 16;
            //Creating the modify threads
            if (modifythreads == line.substr(0,6)) //starts a test case
            {
                while(line[i])
                {
                    if(isdigit((int)line[i]))
                    {
                        tempInt += line[i];
                    }
                    i++;
                }
                insertModify = stoi(tempInt);
                writer = new pthread_t[insertModify];
                cout << "Modify threads: " << insertModify << endl;



                tempInt = "";
            }

            getline(input, line);
            getline(input, line);
            //logic for commands
            i = 0;
            int rthreads = 0;
            int wthreads = 0;

            string tempString;
            while(line[i])
            {
                //tempInt stores the strings
                tempString += line[i];
                if(line[i] == ')')
                {
                    if(tempString[0] == 's')
                    {
                        int runTemp = stoi(tempInt);
                        cout << "searching" << endl;
                        searchRBT(runTemp);
                        //pthread_create(&r[rthreads],NULL,searchRBT(runTemp), NULL);
                        rthreads++;
                    }
                    if(tempString[0] == 'i')
                    {
                        int runTemp = stoi(tempInt);
                        insert(runTemp);
                        cout << "Inserting----" << runTemp << "IntoRBT------";
                        printRBT();
                        cout << endl;
                        //pthread_create(&w[wthreads],NULL,insert(runTemp), NULL);
                        wthreads++;
                    }
                    if(tempString[0] == 'd')
                    {
                        int runTemp = stoi(tempInt);
                        deleteNodesConcurrently(runTemp, wthreads);
                        cout << "Deleting------" << runTemp << "fromRBT-------";
                        printRBT();
                        cout << endl;
                        //pthread_create(&w[wthreads],NULL,deleteNode(runTemp), NULL);
                        wthreads++;
                    }
                    i += 4;
                    tempInt = "";
                    tempString = "";
                }
                if(isdigit((int)line[i]))
                {
                    tempInt += line[i];
                }
                i++;
            }

            //cout << rbtfill << endl;
            printRBT(); //prints the RBT
        }
    }

    input.close();


    return 0;
}
