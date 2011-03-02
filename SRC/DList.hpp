#pragma once

/*
DList.hpp
������ ������ � ���������� �������.
Type ������ ����� ����������� �� ���������, ���� ������������ ������
Push, Unshift, InsertBefore ��� InsertAfter � item ������ ��� ��
������������ �������� �����������:
      const Type& operator=(const Type &)
*/
/*
Copyright (c) 2009 lort
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "VisRen.hpp"

class CDList
{
		size_t count;
	protected:
		struct Node
		{
			Node *next;
			Node *prev;
		};
		CDList();
		void *CInsertBefore(void *b, void *item);
		void *CInsertAfter(void *a, void *item);
		void CMoveBefore(void *b, void *item);
		void CMoveAfter(void *a, void *item);
		void *CDelete(void *item);
		void CSwap(CDList &l);
		Node root;

		virtual Node *AllocNode(void *key)=0;
		virtual void DeleteNode(Node *node)=0;

	public:
		void Clear();
		size_t Count()const {return count;};
		bool Empty()const {return !count;};
};

template <typename Type>
class DList : public CDList
{
		struct TNode : Node
		{
			Type type;
			TNode(Type *t) {if (t) type=*t;}
		};
		Node *AllocNode(void *key) {return new TNode((Type*)key);}
		void DeleteNode(Node *node) {delete(TNode*)node;}

		Type *Node2Type(Node *node) {return node!=&root ? (Type*)((BYTE*)node+sizeof(Node)) : NULL;}
		Node *Type2Node(const Type *item) {return item ? (Node*)((BYTE*)item-sizeof(Node)) : &root;}

	public:
		~DList() {Clear();};

		//������� ����� ������� � ��������� ��� � ����� �������
		//���������� ��������� �� ��������� �������
		Type *Push(const Type *item=NULL) {return (Type*)CInsertBefore(NULL, (void*)item);}

		//������� ����� ������� � ��������� ��� � ������ �������
		//���������� ��������� �� ��������� �������
		Type *Unshift(const Type *item=NULL) {return (Type*)CInsertAfter(NULL, (void*)item);}

		//������� ����� ������� � ��������� ��� � ������ ����� before
		//���� before==NULL ������� ���������� � ����� ������
		//���������� ��������� �� ��������� �������
		Type *InsertBefore(const Type *before, const Type *item=NULL) {return (Type*)CInsertBefore((void*)before, (void*)item);}

		//������� ����� ������� � ��������� ��� � ������ ����� after
		//���� after==NULL ������� ���������� � ������ ������
		//���������� ��������� �� ��������� �������
		Type *InsertAfter(const Type *after, const Type *item=NULL) {return (Type*)CInsertAfter((void*)after, (void*)item);}

		void MoveBefore(const Type *before, const Type *item) {CMoveBefore((void*)before, (void*)item);}
		void MoveAfter(const Type *after, const Type *item) {CMoveAfter((void*)after, (void*)item);}

		//������� ������� item �� ������, ������������ ��������� �� ���������� �������,
		//���� �������� ������ ������� ������������ NULL
		Type *Delete(Type *item) {return (Type*)CDelete(item);}

		//���������� ������ ������� ������ ��� NULL ���� ������ ������
		Type *First() {return Node2Type(root.next);}

		//���������� ��������� ������� ������ ��� NULL ���� ������ ������
		Type *Last() {return Node2Type(root.prev);}

		//���������� ������� ��������� �� item ��� NULL ���� item ��������� �������.
		//Next(NULL) ���������� ������ �������
		Type *Next(const Type *item) {return Node2Type(Type2Node(item)->next);}

		//���������� ������� ������ � ������ ����� item ��� NULL ���� item ������ �������.
		//Prev(NULL) ���������� ��������� �������
		Type *Prev(const Type *item) {return Node2Type(Type2Node(item)->prev);}

		//������ ������� ���������� �������
		void Swap(DList<Type> &l) {CSwap(l);}
};
