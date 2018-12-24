#ifndef SET_LS_H
#define SET_LS_H

#include <functional>
#include <pthread.h>
#include <bits/stdc++.h>
#include <syslog.h>

template <class T>
class SetLS{
private:
  class Node{
  public:
    T item; 
    size_t key;
    Node* next;
    bool marked;
    pthread_mutex_t mutex;
  
    Node(T init_value){
    	item = init_value;
    	key = std::hash<T>()(init_value);
    	next = nullptr;
    	marked = false;
    	mutex = PTHREAD_MUTEX_INITIALIZER;
    }
	
	void lock(){
      if(pthread_mutex_lock(&mutex) != 0)
        syslog(LOG_INFO, "Error mutex lock");   
    }

    void unlock(){
      if(pthread_mutex_unlock(&mutex) != 0)
        syslog(LOG_INFO, "Error mutex unlock");   
    }
   };

  Node* head; 

public:
  SetLS(){
    head = new Node(0);
    if (!head)
      syslog(LOG_INFO, "Error creating node");
    head->next = new Node(INT_MAX);
    if (!head->next)
      syslog(LOG_INFO, "Error creating node");
}

  ~SetLS(){
    for (Node *cur = head, *next = nullptr; cur != nullptr; cur = next){
      next = cur->next;
      delete cur;
    }
  }

  size_t generate_hash(const T& item){
    return std::hash<T>()(item);
  }

  bool validate(Node* prev, Node* cur){
    return !prev->marked && !cur->marked && (prev->next == cur);
  }

  bool add(const T& item){
    size_t key = generate_hash(item);
    while (true){
      Node* prev = head;
      Node* cur = head->next;
      while (cur->key < key){
        prev = cur;
        cur = cur->next;
      }

      prev->lock();
      cur->lock();
     
      if (validate(prev, cur)){
        if (cur->key == key){
          prev->unlock();
          cur->unlock();
          return false;
        }
        else{
          Node* node = new Node(item);
          if (!node)
            syslog(LOG_INFO, "Error creating node");
          node->next = cur;
          prev->next = node;
          prev->unlock();
          cur->unlock();
          return true;
        }
      }
      prev->unlock();
      cur->unlock();
    }
  }

  bool remove(const T& item){
    size_t key = generate_hash(item);
    while (true){
      Node* prev = head;
      Node* cur = head->next;
      while (cur->key < key){
        prev = cur;
        cur = cur->next;
      }

      prev->lock();
      cur->lock();

      if (validate(prev, cur)){
        if (cur->key == key){
          cur->marked = true;
          prev->next = cur->next;
          delete cur;
          prev->unlock();
          cur->unlock();
          return true;
        }
        else{
          prev->unlock();
          cur->unlock();
          return false;
        }
      }
      prev->unlock();
      cur->unlock();
    }
  }

  bool contains(const T& item){
    size_t key = generate_hash(item);
    Node* cur = head;
    while(cur->key < key)
    	cur = cur->next;

    return (cur->key == key) && !cur->marked;
  }
};

#endif