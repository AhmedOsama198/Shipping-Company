#include "PromotionEvent.h"

void PromotionEvent::setExtraMoney(int exm)
{
	this->ExtraMoney = exm;
}

int PromotionEvent::getExtraMoney()
{
	return this->ExtraMoney;
}
void PromotionEvent::Execute() {
	Cptr->UpdatetoVIP(getID());
}
VIPCargo* PromotionEvent::getVIPCargo() {
	return VCargoptr;
}
/*
VIPCargo* PromotionEvent::Execute(NormalCargo* & ptr)
{
	VIPCargo* temp=new VIPCargo;
	temp->SetCost(ptr->getCost() + this->getExtraMoney());
	temp->SetDdes(ptr->getDdes());
	temp->SetID(ptr->GetID());
	temp->SetLT(ptr->getLT());
	temp->setPT(ptr->getPT().first, ptr->getPT().second);
	return temp;
	ptr = NULL;			//LAST NODE -> NULL
	delete ptr;
	ptr = NULL;
}*/
void PromotionEvent::Execute() {
	
}
PromotionEvent::PromotionEvent(Time ET, int ID, int exm) :Event(ET, ID) {
	setExtraMoney(exm);
}
