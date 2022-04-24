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

}
VIPCargo* PromotionEvent::getVIPCargo() {
	return VCargoptr;
}
NormalCargo* PromotionEvent::getNormalCargo() {
	return NCargoptr;
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
PromotionEvent::PromotionEvent(Time ET, int ID, int exm, NormalCargo* NC) :Event(ET, ID) {
	setExtraMoney(exm);
	NCargoptr = NC;
}
