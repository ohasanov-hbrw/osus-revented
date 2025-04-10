#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <raylib.h>
#include <climits>

#include "hitobject.hpp"

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
    size_t size;
    Node* head; 
    Node* tail;

    public: 
        // Default constructor 
        
    

        Linkedlist();

        void insertHead(void * data);
        
        void insertTail(void * data); 
        
        void deleteHead();
        
        void deleteTail(); 
        
        void deleteNode(int index);
        
        void deleteNodeUnsafe(Node* node);

        Node* getTail();

        Node* getHead();

        size_t getSize();
}; 