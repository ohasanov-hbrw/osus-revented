#include "linkedListImpl.hpp"

Node::Node(){ 
    object = NULL; 
    next = NULL; 
    prev = NULL;
} 

Node::Node(void * data){ 
    this->object = data; 
    this->next = NULL; 
    this->prev = NULL;
}

Linkedlist::Linkedlist(){
    head = NULL;
    tail = NULL;
    size = 0;
} 

void Linkedlist::insertHead(void * data){  
    Node *newNode = new Node(data);  
    newNode->next = head;
    newNode->prev = NULL;  
    head = newNode; 
    size++;
    if(tail == NULL){
        tail = newNode;
        return;
    }
    newNode->next->prev = newNode;
}  

void Linkedlist::insertTail(void * data){  
    Node *newNode = new Node(data);  
    if(head == NULL) {  
        head = newNode;  
        tail = newNode;
        newNode->next = NULL;
        newNode->prev = NULL;
        size++;
        return;  
    }
    newNode->next = NULL;
    newNode->prev = tail;
    tail = newNode;  
    newNode->prev->next = newNode;
    size++;
}  

void Linkedlist::deleteHead(){  
    if(head == NULL){  
        return;  
    }  
    Node *temp = head;  
    Node *temp2 = temp->next;   
    if(temp2 == NULL){
        size = 0;
        delete temp;  
        head = NULL;
        tail = NULL;
        return;
    }
    temp2->prev = NULL; 
    head = temp2;
    if(size > 0){
        size--;
    }
    delete temp;  
}  

void Linkedlist::deleteTail(){  
    if(tail == NULL){  
        return;  
    }  
    Node *temp = tail;  
    Node *temp2 = temp->prev;   
    if(temp2 == NULL){
        size = 0;
        delete temp;  
        head = NULL;
        tail = NULL;
        return;
    }
    temp2->next = NULL; 
    tail = temp2;
    if(size > 0){
        size--;
    }
    delete temp;  
}  

void Linkedlist::deleteNode(int index){
    Node *temp1 = head, *temp2 = NULL; 
    int ListLen = 0; 
    if(head == NULL || tail == NULL){ 
        std::cout << "List empty." << std::endl; 
        return; 
    }

    while(temp1 != NULL){ 
        temp1 = temp1->next; 
        ListLen++; 
    } 

    if(ListLen > size){
        std::cout << "huh?\n";
    }

    if(ListLen <= index || index < 0){ 
        std::cout << "Index out of range" << std::endl; 
        return; 
    } 

    if(index == 0){
        deleteHead();
        return;
    }
    else if(index == ListLen - 1){
        deleteTail();
        return;
    }
    else{
        temp1 = head;
        for(int i = 0; i < index; i++){
            temp1 = temp1->next; 
        }
        Node* temp2 = temp1->prev; 
        Node* temp3 = temp1->next;
        temp3->prev = temp1->prev;
        temp2->next = temp1->next; 
        delete temp1; 
        if(size > 0){
            size--;
        }
    }
}

size_t Linkedlist::getSize(){
    return size;
}

Node* Linkedlist::getTail(){
    return tail;
}

Node* Linkedlist::getHead(){
    return head;
}

void Linkedlist::deleteNodeUnsafe(Node* node){
    if(node == NULL){
        return;
    }
    //std::cout << "Shit.\n";
    if(node->next == NULL){
        if(node->prev == NULL){
            if(size > 0){
                size--;
            }
            head = NULL;
            tail = NULL;
            delete node;
            return;
        }
        else{
            if(size > 0){
                size--;
            }
            tail = node->prev;
            node->prev->next = NULL;
            delete node;
            return;
        }
    }
    if(node->prev == NULL){
        if(node->next == NULL){
            if(size > 0){
                size--;
            }
            head = NULL;
            tail = NULL;
            delete node;
            return;
        }
        else{
            if(size > 0){
                size--;
            }
            head = node->next;
            node->next->prev = NULL;
            delete node;
            return;
        }
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    delete node;
    if(size > 0){
        size--;
    }
    return;
}