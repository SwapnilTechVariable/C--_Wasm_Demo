#pragma once
#include <string>
#include <sstream>
#include "cpp-base64/base64.h"

namespace collection {
    template <typename T>
    struct Node {
        T data;
        Node *next;

        Node(T data) {
            this->data = data;
        }
    };

    template <typename T>
    class LinkedList{
    private:
        Node<T>* m_head;
        int m_length;

    public:   
        LinkedList() {
            m_head = nullptr;
            m_length = 0;
        }

        ~LinkedList() {
            Node<T>* ptr = m_head;
            while(ptr != nullptr) {
                Node<T>* temp = ptr;
                ptr =  ptr->next;
                delete temp;
            }

        }

        void insert(T data) {
            Node<T> *temp = new Node<T>(data);

            if(this->m_head == nullptr) {
                this->m_head = temp;

            } else {
                Node<T>* ptr = this->m_head;
                while(ptr->next != nullptr)
                    ptr = ptr->next;
                ptr->next = temp;
            }
            m_length++;
        }

        void insertR(T data) {
            Node<T>* temp = new Node<T>(data);
            temp->next = this->m_head;
            this->m_head = temp;

            m_length++;
        }

        void remove() {
            Node<T>* ptr = m_head;
            if(ptr == nullptr)
                return;
            if(ptr->next == nullptr) {
                m_head = nullptr;
                delete ptr;
                return;
            }
            while(ptr->next->next != nullptr) 
                ptr = ptr->next;
            Node<T>* temp = ptr->next;
            ptr->next = nullptr;
            delete temp;
            m_length--;
        }

        void removeR() {
            if(m_head == nullptr)
                return;
            Node<T>* temp = m_head;
            m_head = m_head->next;

            delete temp;
            m_length--;
        }

        int length() const {
            return m_length;
        }

        std::string display() {
            std::stringstream ss;

            Node<T> *ptr = m_head;


            if(ptr == nullptr) return "EMPTY_LIST";

            ss << ptr->data;

            ptr = ptr-> next;

            while(ptr != nullptr) {
                ss << "->" <<  ptr->data;
                ptr = ptr->next;
            }

            return ss.str();
        }
    };
}