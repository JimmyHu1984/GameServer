#ifndef LIST_MANAGER_H
#define LIST_MANAGER_H
#pragma once
#include "StdAfx.h"
#include <list>

class CListLock{
public:
	CListLock(){ InitializeCriticalSection( &m_crit);};
	virtual ~CListLock(){ DeleteCriticalSection( &m_crit);};

public:
	void Lock(){ EnterCriticalSection( &m_crit);};
	void UnLock(){ LeaveCriticalSection( &m_crit);};
private:
	CRITICAL_SECTION m_crit;
};

class CAutoListLocker{
public:
	explicit CAutoListLocker(CListLock & lk):m_lock(lk){m_lock.Lock();}
	~CAutoListLocker(){m_lock.UnLock();}
private:
	CListLock & m_lock;
};

template<class T>
class CListManager{
public:
	CListManager(){};
	~CListManager(){};
	CListLock dataLock;
	CListLock processLock;

	//new method;
	BOOL removeAll();
	T getFirstElement();
	T getNextElement(T pTemp);
	T findElement(T pTemp);
	BOOL removeElement(T pTemp);
	BOOL addTail(T pTemp);
	INT getCount();
	BOOL getAndPopFirstElement(T& _takeOutLog);
private:
	std::list<T> listPool;
};

template<class T>
BOOL CListManager<T>::removeAll(){
	CAutoListLocker locker(this->dataLock);
	try{
		this->listPool.clear();
		return TRUE;
	}catch(...){
		return FALSE;
	}
}

template<class T>
T CListManager<T>::findElement(T pTemp){
	CAutoListLocker locker(this->dataLock);
	try{
		for(auto iter = this->listPool.begin(); iter != this->listPool.end(); ++iter ){
			if(pTemp == (*iter)){
				return *iter;
			}
		}
		return NULL;
	}catch(...){
		return NULL;
	}
}

template<class T>
T CListManager<T>::getFirstElement(){
	CAutoListLocker locker(this->dataLock);
	try{
		if(!this->listPool.empty()){
			return this->listPool.front();	
		}
		return NULL;
	}catch(...){
		return NULL;
	}
}

template<class T>
T CListManager<T>::getNextElement(T pTemp){
	CAutoListLocker locker(this->dataLock);
	try{
		for(auto iter = this->listPool.begin(); iter != this->listPool.end(); ++iter){
			if(pTemp == (*iter)){
				++iter;
				if(iter == this->listPool.end()){
					return NULL;
				}else{
					return *iter;
				}
			}
		}
		return NULL;
	}catch(...){
		return NULL;
	}
}

template<class T>
BOOL CListManager<T>::removeElement(T pTemp){
	CAutoListLocker locker(this->dataLock);
	try{
		for(auto iter = this->listPool.begin(); iter != this->listPool.end(); ++iter){
			if(pTemp == (*iter)){
				this->listPool.erase(iter);
				return TRUE;
			}
		}
		return FALSE;
	}catch(...){
		return FALSE;
	}
}

template<class T>
BOOL CListManager<T>::addTail(T pTemp){
	CAutoListLocker locker(this->dataLock);
	try{
		this->listPool.push_back(pTemp);
	//	::OutputDebugStringA("addTail");
		return TRUE;
	}catch(...){
		return FALSE;
	}
}

template<class T>
INT CListManager<T>::getCount(){
	CAutoListLocker locker(this->dataLock);
	try{
		return this->listPool.size();
	}catch(...){	
		return 0;
	}
}

template<class T>
BOOL CListManager<T>::getAndPopFirstElement(T& takeOutElement){
	CAutoListLocker locker(this->dataLock);
	try{
		if(!this->listPool.empty()){
			takeOutElement = this->listPool.front();
			this->listPool.pop_front();
			return TRUE;
		}
		takeOutElement = NULL;
		return FALSE;
	}catch(...){	
		takeOutElement = NULL;
		return FALSE;
	}
}

#endif