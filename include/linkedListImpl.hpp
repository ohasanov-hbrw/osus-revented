#pragma once
#include <raylib.h>


class Node{ 
    public: 
        void* object;
        
        Node* next; 
        Node* prev; 

        // Default constructor 
        Node();
    
        // Parameterised Constructor 
        Node(void * data);
}; 

class Linkedlist { 
    public: 
        // Default constructor 
        
        int size;
        Node* head; 
        Node* tail;

        Linkedlist();

        void insertHead(void * data);
        
        void insertTail(void * data); 
        
        void deleteHead();
        
        void deleteTail(); 
        
        void deleteNode(int index);
        
        void deleteNodeUnsafe(Node* node);

        void init();

        Node* getTail();

        Node* getHead();

        int getSize();
}; 