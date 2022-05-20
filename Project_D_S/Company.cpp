#include "Company.h"
#include"VIPCargo.h"
#include "SpecialCargo.h"
#include "NormalCargo.h"
#include <chrono>
#include <thread>
#include "Event.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

Company::Company()
{
	pUI = new UIClass();
	CurrentTime.sethour(1);
	CurrentTime.setDAY(1);
	NTruckCapacity = VTruckCapacity = STruckCapacity = 1;
	TruckCount = 0;
	TSM = 0;
	this->NumberOfAutoPromotedCargos = 0;		//ismail
	this->N_Cargo_Count = 0;	//delivered
	this->S_Cargo_Count = 0;
	this->VIP_Cargo_Count = 0;
}

void Company::LoadCargos() {
	CheckforCargosExceededMaxW();
	AssignExceeded();
	AssignVIP();
	AssignSpecial();
	AssignNormal();
}

void Company::setcurtime(Time time)
{
	this->CurrentTime.sethour(time.gethour());
	this->CurrentTime.setDAY(time.getDAY());
}

Time Company::getcurtime()
{
	return this->CurrentTime;
}

void Company::StepbyStepSimulation()
{
	int cnt = 10;
	while (cnt++) {
		TSM++;
		ExecuteEvents();
		CheckforTrucks();
		LoadCargos();
		PrintConsole();
		Sleep(1500);
		++CurrentTime;
		incrementWHs();
		this->OutputFile();
		
	}
}
void Company::PrintConsole() {
	pUI->PrintCurrentTime(this->CurrentTime);
	pUI->PrintWaitingCargos(this);
	pUI->PrintLine();
	pUI->PrintLoadingTrucks(this);
	pUI->PrintLine();
	pUI->PrintEmptytrucks(this);
	pUI->PrintLine();
	pUI->PrintMovingCargos(this);
	pUI->PrintLine();
	pUI->PrintIn_CheckupTrucks(this);
	pUI->PrintLine();
	pUI->PrintDeliveredCargo(this);
	pUI->PrintLine();
}

void Company::InteractiveSimulation() {
	int cnt = 10;
	while (cnt++) {
		ExecuteEvents();
		LoadCargos();
		incrementWHs();
		checkforAutop();
		CheckforTrucks();
		PrintConsole();
		++CurrentTime;
		pUI->getKey();	
	}
}

void Company::checkforAutop() {
	Cargo* cptr;
	Queue<Cargo*>q;
	while (WaitingNormalCargos.DeleteFirst(cptr))
	{
		
		if (cptr->GetWaitingHours() > AutoP) {
			if (cptr) {
				VIPCargo* temp = new VIPCargo();
				temp->SetCost(cptr->getCost());
				temp->SetDdes(cptr->getDis());
				temp->SetID(cptr->GetID());
				temp->SetLT(cptr->getLT());
				temp->setPT(cptr->getPT().getDAY(), cptr->getPT().gethour());
				AddCargotoVIPWaiting(temp);
			}
			this->set_NumberOfAutoPromotedCargos(this->get_NumberOfAutoPromotedCargos() + 1);
		}
		else q.enqueue(cptr);
		
	}
	while (q.dequeue(cptr))
		WaitingNormalCargos.InsertEnd(cptr);
}

void Company::OutputFile() {			//ismail
	ofstream Lfile;
	Lfile.open("Output.txt", ios::binary);
	Lfile << "CDT\t\tID\t\tWT\t\tTID\n";
	Cargo* cargo;
	bool bo;
	this->Calc_Delivered_Cargos_count();
	for (int i = 0; i < this->DeliveredCargos.GetCount(); i++)
	{

		bo = DeliveredCargos.dequeue(cargo);
		if (dynamic_cast<NormalCargo*> (cargo))
			Lfile << cargo->getDel_T().getDAY() <<':'<< cargo->getDel_T().gethour() <<"\t\t" << cargo->GetID() << "\t\t" << cargo->getPT().getDAY() << ':' << cargo->getPT().gethour() << cargo->GetWaitingHours() << "\t\t" << cargo->get_TrkId() << endl;
		bo = DeliveredCargos.enqueue(cargo);
	}
	for (int i = 0; i < this->DeliveredCargos.GetCount(); i++)
	{

		bo = DeliveredCargos.dequeue(cargo);
		if (dynamic_cast<SpecialCargo*> (cargo))
			Lfile << cargo->getDel_T().getDAY() <<':'<<cargo->getDel_T().gethour()<< "\t\t" << cargo->GetID() << "\t\t" << cargo->getPT().getDAY() << ':' << cargo->getPT().gethour() << cargo->GetWaitingHours() << "\t\t" << cargo->get_TrkId() << endl;
		bo = DeliveredCargos.enqueue(cargo);
	}
	for (int i = 0; i < this->DeliveredCargos.GetCount(); i++)
	{
		bo = DeliveredCargos.dequeue(cargo);
		if (dynamic_cast<VIPCargo*> (cargo))
			Lfile << cargo->getDel_T().getDAY()<<':'<<cargo->getDel_T().gethour()<<"\t\t" << cargo->GetID() << "\t\t" << cargo->getPT().getDAY() << ':' << cargo->getPT().gethour() << cargo->GetWaitingHours() << "\t\t" << cargo->get_TrkId() << endl;
		bo = DeliveredCargos.enqueue(cargo);
	}


	Lfile << ".....................................................\n";
	Lfile << ".....................................................\n";
	Lfile << "Cargos: " << GetnumOfDeliv();
	Lfile << '[' << "N: " << this->get_N_Cargo_Count() << ", " << "S: " << this->get_S_Cargo_Count() << ", " << "V: " << this->get_VIP_Cargo_Count() << "]\n";
	Lfile << "Cargo Avg Wait = " << AverageWaitingTime_DeliveredNormalCargos().getDAY() + AverageWaitingTime_DeliveredSpecialCargos().getDAY() + AverageWaitingTime_DeliveredVIPCargos().getDAY();
	Lfile << ':' << AverageWaitingTime_DeliveredNormalCargos().gethour() + AverageWaitingTime_DeliveredSpecialCargos().gethour() + AverageWaitingTime_DeliveredVIPCargos().gethour() << endl;
	Lfile << "Auto-promoted Caros:" << int((float(this->get_NumberOfAutoPromotedCargos()) / this->get_N_Cargo_Count()) * 100) << "%\n";
	Lfile << "Trucks: " << get_numOf_N_Truck() + get_numOf_S_Truck() + get_numOf_VIP_Truck();
	Lfile << '[' << "N: " << get_numOf_N_Truck() << ", " << "S: " << get_numOf_S_Truck() << ", " << "V: " << get_numOf_VIP_Truck() << "]\n";
	Lfile << "Avg Active time = " << NormalTrucks_ActiveTime().gethour() + NormalTrucks_ActiveTime().getDAY() * 24 + SpecialTrucks_ActiveTime().gethour() + SpecialTrucks_ActiveTime().getDAY() * 24 + VIPTrucks_ActiveTime().gethour() + VIPTrucks_ActiveTime().getDAY() * 24 << "%\n";
	Lfile << "Avg utilization = " << NormalTrucks_Utilization() + SpecialTrucks_Utilization() + VIPTrucks_Utilization() << "%\n";
	Lfile.close();
}
Time Company::AverageWaitingTime()
{
	Cargo* cargo;
	Time time;
	bool bo;
	for (int i = 0; i < DeliveredCargos.GetCount(); i++)
	{
		bo = DeliveredCargos.dequeue(cargo);
		time = time + cargo->GetWaitingHours();
		bo = DeliveredCargos.enqueue(cargo);
	}

	return time;
}
//Time Company::AverageWaitingTime_DeliveredSpecialCargos()
//{
//	Cargo* cargo;
//	Time time;
//	bool bo;
//	for (int i = 0; i < DeliveredSpCargo.GetCount(); i++)
//	{
//		bo = DeliveredSpCargo.dequeue(cargo);
//		time = time + cargo->GetWaitingHours();
//		bo = DeliveredSpCargo.enqueue(cargo);
//	}
//	return time;
//}
//Time Company::AverageWaitingTime_DeliveredVIPCargos()
//{
//	Cargo* cargo;
//	Time time;
//	bool bo;
//	for (int i = 0; i < DeliveredVIPCargo.GetCount(); i++)
//	{
//		bo = DeliveredVIPCargo.dequeue(cargo);
//		time = time + cargo->GetWaitingHours();
//		bo = DeliveredVIPCargo.enqueue(cargo);
//	}
//	return time;
//}
void Company::set_NumberOfAutoPromotedCargos(int i)
{
	this->NumberOfAutoPromotedCargos = i;
}
int Company::get_NumberOfAutoPromotedCargos()
{
	return this->NumberOfAutoPromotedCargos;
}

int Company::get_numOf_N_Truck()
{
	return this->N;
}

int Company::get_numOf_S_Truck()
{
	return this->S;
}

int Company::get_numOf_VIP_Truck()
{
	return this->V;
}

Time Company::NormalTrucks_ActiveTime()
{
	Time time;
	Truck* truck;
	bool bo;
	for (int i = 0; i < NormalTrucks.GetCount(); i++)
	{
		bo = NormalTrucks.dequeue(truck);
		time.sethour(time.gethour() + truck->get_ActiveTime().gethour() + 24 * truck->get_ActiveTime().getDAY());
		bo = NormalTrucks.enqueue(truck);
	}
	return time;
}

Time Company::SpecialTrucks_ActiveTime()
{
	Time time;
	Truck* truck;
	bool bo;
	for (int i = 0; i < SpecialTrucks.GetCount(); i++)
	{
		bo = SpecialTrucks.dequeue(truck);
		time.sethour(time.gethour() + truck->get_ActiveTime().gethour() + 24 * truck->get_ActiveTime().getDAY());
		bo = SpecialTrucks.enqueue(truck);
	}
	return time;
}

Time Company::VIPTrucks_ActiveTime()
{
	Time time;
	Truck* truck;
	bool bo;
	for (int i = 0; i < VIPTrucks.GetCount(); i++)
	{
		bo = VIPTrucks.dequeue(truck);
		time.sethour(time.gethour() + truck->get_ActiveTime().gethour() + 24 * truck->get_ActiveTime().getDAY());
		bo = VIPTrucks.enqueue(truck);
	}
	return time;
}

int Company::NormalTrucks_Utilization()
{
	Truck* truck;
	bool bo;
	int count = 0;
	for (int i = 0; i < NormalTrucks.GetCount(); i++)
	{
		bo = NormalTrucks.dequeue(truck);
		count = count + truck->Truck_utilization(TSM);
		bo = NormalTrucks.enqueue(truck);
	}
	return count;
}

int Company::SpecialTrucks_Utilization()
{
	Truck* truck;
	bool bo;
	int count = 0;
	for (int i = 0; i < SpecialTrucks.GetCount(); i++)
	{
		bo = SpecialTrucks.dequeue(truck);
		count = count + truck->Truck_utilization(TSM);
		bo = SpecialTrucks.enqueue(truck);
	}
	return count;
}

int Company::VIPTrucks_Utilization()
{
	Truck* truck;
	bool bo;
	int count = 0;
	for (int i = 0; i < VIPTrucks.GetCount(); i++)
	{
		bo = VIPTrucks.dequeue(truck);
		count = count + truck->Truck_utilization(TSM);
		bo = VIPTrucks.enqueue(truck);
	}
	return count;
}

void Company::incrementWHs() {
	WaitingNormalCargos.incrementWH();//List
	//===
	Queue<Cargo*>tq;
	Cargo* ct;
	while (WaitingSpecialCargos.dequeue(ct)) {
		ct->IncrementWaitingHours();
		tq.enqueue(ct);
	}
	while (tq.dequeue(ct))
		WaitingSpecialCargos.enqueue(ct);
	//===
	while (WaitingVIPCargos.dequeue(ct)) {
		ct->IncrementWaitingHours();
		tq.enqueue(ct);
	}
	while (tq.dequeue(ct))
		WaitingVIPCargos.enqueue(ct, ct->Getpriority());
}

void Company::GeneralSimulate() {
	switch (pUI->SelectMode()) {
	case interactive:
	{	
		InteractiveSimulation();
		break;	
	}
	case step_by_step: {
		StepbyStepSimulation();
		break;
	}
	case silent:
	{
		pUI->PrintSilentMode(this);
		break;
	}
	}
}

void Company::Loading_File()
{
	int temp;
	ifstream Lfile;
	Lfile.open("Text.txt");      //start from here to read 			 
	/*Lfile >> NTruckSpeed >> STruckSpeed >> VTruckSpeed;
	Lfile >> NTruckCapacity >> STruckCapacity >> VTruckCapacity;*/
	Lfile >> N;
	int i = N, j = N;
	while (i--) {
		Lfile >> temp;
		NSpeeds.enqueue(temp);
	}
	while (j--) {
		Lfile >> temp;
		NTCs.enqueue(temp);
	}
	/// </summary>
	Lfile >> S;
	 i =  j = S;
	while (i--) {
		Lfile >> temp;
		SSpeeds.enqueue(temp);
	}
	while (j--) {
		Lfile >> temp;
		STCs.enqueue(temp);
	}
	////////////
	Lfile >> V;
	 i =j = V;
	while (i--) {
		Lfile >> temp;
		VSpeeds.enqueue(temp);
	}
	while (j--) {
		Lfile >> temp;
		VTCs.enqueue(temp);
	}
	Lfile >> JourneyCount>> NTruckCheckupDuration>>STruckCheckupDuration>>VTruckChekcupDuration;
	Lfile >> AutoP >> MaxW;
	Lfile >> EventCount;
	char E;
	for (int i = 0; i < EventCount; i++)
	{
		Lfile >> E;
		switch (E)
		{
		case 'R': {
			char TYP;
			int H, D;
			int ID;
			int DIST;
			double LT;
			int Cost;
			char colon;
			Lfile >> TYP >> D >> colon >> H >> ID >> DIST >> LT >> Cost;
			Time ET(D, H);
			Cargo* C;
			PreparationEvent* PE = new PreparationEvent(TYP, DIST, LT, Cost, ET, ID, this);
			Event* E = PE;
			double priority = -((D - 1) * 24 + H);
			EventsPQ.enqueue(E,priority);
			break; }
		case 'X': {
			int H, D;
			int ID;
			char colon;
			Lfile >> D >> colon >> H >> ID;
			Time ET(D, H);
			CancellationEvent* CE = new CancellationEvent(ET, ID, this);
			Event* E = CE;
			double priority = -((D - 1) * 24 + H);
			EventsPQ.enqueue(E,priority);
			break;
		}
		case 'P': {
			int H, D;
			int ID;
			char colon;
			int ExtraMoney;
			Lfile >> D >> colon >> H >> ID >> ExtraMoney;
			Time ET(D, H);
			PromotionEvent* PRE = new PromotionEvent(ET, ID, ExtraMoney, this );
			double priority = -((D - 1) * 24 + H);
			Event* E = PRE;
			EventsPQ.enqueue(E,priority);
			break;
		}
		default:
			break;
		}
	}
	Lfile.close();
}
void Company::setMaxW(int M) {
	if (M > 0) MaxW = M;
}
bool Company::isClosed() {
	int H = CurrentTime.gethour();
	if (H >= 5 && H <= 23) return false;
	return true;
}
void Company:: ExecuteEvents() {
	//if (isClosed()) {
	//	return;
	//}
	Event* Eptr;
	EventsPQ.peak(Eptr);
	if (!Eptr ||EventsPQ.GetCount()==0) return;
	Time X = Eptr->getEventTime();
	if (!(X==CurrentTime)) return;
	while (EventsPQ.GetCount()>0 &&  Eptr->getEventTime()== CurrentTime)
		{
		cout << "Current EventCount: " << EventsPQ.GetCount() << endl;
		EventsPQ.dequeue(Eptr);
		Eptr->Execute();
		EventsPQ.peak(Eptr);
	}
}

void Company::AddCargotoVIPWaiting(VIPCargo* C) {
	WaitingVIPCargos.enqueue(C, C->Getpriority());
}

void Company::AddCargotoNormalWaiting(Cargo* n) {
	WaitingNormalCargos.InsertEnd(n);
}
void Company::AddCargotoSpWaiting(SpecialCargo* n) {
	WaitingSpecialCargos.enqueue(n);
}
void Company::PrintEvents() {
	//Event* E;
	//int cnt = 0;
	//EventsPQ.peak(E);
	//while (EventsPQ.GetCount()>0) {
	//	cnt++;
	//	if (dynamic_cast<PromotionEvent*> (E)) cout << "PRE";
	//	if (dynamic_cast<CancellationEvent*> (E)) cout << "CE";
	//	if (dynamic_cast<PreparationEvent*> (E)) cout << "PE";
	//	cout << cnt << endl;
	//	EventsPQ.dequeue(E);
	//	cout << "ID: " << E->getID() << endl;
	//	//EventsPQ.peak(E);
	//}
	cout << N << " " << S << " " << V << endl;;
	cout << NormalEmptyTrucks.GetCount() << " " << SpecialEmptyTrucks.GetCount() << " " << VIPEmptyTrucks.GetCount() << endl;
}

void Company::CancellationID(int id)
{
	WaitingNormalCargos.Remove(id);
}

NormalCargo* Company::GetNormalCargo(int id) {
	if (WaitingNormalCargos.PointerToNormalCRGO(id))
		return dynamic_cast<NormalCargo*>(WaitingNormalCargos.PointerToNormalCRGO(id)->getitem());
	else return nullptr;
}
void Company::printWaitingVIP(UIClass* pUI) {
	if (WaitingVIPCargos.GetCount() > 0) {
		pUI->openbraceforVIP();
		WaitingVIPCargos.PrintQ(pUI);
		pUI->closebraceforVIP();
	}
}
void Company::printWaitingNormal(UIClass* pUI) {
	if (WaitingNormalCargos.GetCount() > 0){
	   pUI->openbraceforNormal();
	   WaitingNormalCargos.PrintL(pUI);
	   pUI->closebraceforNormal();
    }
}

void Company::printWaitingSP(UIClass* pUI) {
	if (WaitingSpecialCargos.GetCount() > 0) {
		pUI->openbraceforSP();
		WaitingSpecialCargos.PrintQ(pUI);
		pUI->closebraceforSP();
	}
}

int Company::GetnumMoving()
{
	int num = 0;
	Queue<Truck*>q;
	Truck* t;
	while (MovingTrucks.dequeue(t)) {
		q.enqueue(t);
		num += t->GetnumofCRGS();
	}
	while (q.dequeue(t))
		MovingTrucks.enqueue(t,t->getPriority());
	return num;
}

void Company::PrintMoving(UIClass* pUI) {
	Queue<Truck*>q;
	Truck* tptr;

	while (MovingTrucks.dequeue(tptr)) {
		cout << tptr->getTimeforDelivery().getDAY() << ":" << tptr->getTimeforDelivery().gethour();
		cout << tptr->getTimeforReturn().getDAY() << ":" << tptr->getTimeforReturn().gethour();
		cout << " ";
		q.enqueue(tptr);
		if (tptr->GetCountOFCargosInTRK() > 0) {
			pUI->Print(tptr);
			if (dynamic_cast<VIPTruck*>(tptr))
			{
				pUI->openbraceforVIP();
				tptr->Print(pUI);
				pUI->closebraceforVIP();
			}
			else if (dynamic_cast<NormalTruck*>(tptr))
			{
				pUI->openbraceforNormal();
				tptr->Print(pUI);
				pUI->closebraceforNormal();
			}
			else if (dynamic_cast<SpecialTruck*>(tptr))
			{
				pUI->openbraceforSP();
				tptr->Print(pUI);
				pUI->closebraceforSP();
			}
		}
	}
	while (q.dequeue(tptr))
		MovingTrucks.enqueue(tptr, tptr->getPriority());
}
void Company::PrintNInCheckupTRKs(UIClass* pUI) {
	if (NInCheckupTrucks.GetCount() > 0) {
		pUI->openbraceforNormal();
		NInCheckupTrucks.PrintQ(pUI);
		pUI->closebraceforNormal();
	}
}
void Company::PrintSInCheckupTRKs(UIClass* pUI) {
	if (SInCheckupTrucks.GetCount() > 0) {
		pUI->openbraceforSP();
		SInCheckupTrucks.PrintQ(pUI);
		pUI->closebraceforSP();
	}
}
void Company::PrintVInCheckupTRKs(UIClass* pUI) {
	if (VInCheckupTrucks.GetCount() > 0) {
		pUI->openbraceforVIP();
		VInCheckupTrucks.PrintQ(pUI);
		pUI->closebraceforVIP();
	}
}
int Company::GetNumOfLaoding() {
	return LoadingTrucks.GetCount();
}
void Company:: PrintLoading(UIClass* pUI) {
	Queue<Truck*>q;
	Truck* ptr;
	while (LoadingTrucks.dequeue(ptr)) {
		q.enqueue(ptr);
		pUI->Print(ptr);
		if (dynamic_cast<NormalTruck*>(ptr))
		{
			pUI->openbraceforNormal();
			ptr->Print(pUI);
			pUI->closebraceforNormal();
		}
		else if (dynamic_cast<VIPTruck*>(ptr)) {
			pUI->openbraceforVIP();
			ptr->Print(pUI);
			pUI->closebraceforVIP();

		}
		else if (dynamic_cast<SpecialTruck*>(ptr)) {
			pUI->openbraceforSP();
			ptr->Print(pUI);
			pUI->closebraceforSP();
		}

	}
	while (q.dequeue(ptr))
		LoadingTrucks.enqueue(ptr);
}
int Company::GetCountTRUCKSincheckup() {
	return NInCheckupTrucks.GetCount() + SInCheckupTrucks.GetCount() + VInCheckupTrucks.GetCount();
}
int Company::Getcountall_waiting() {
	return WaitingVIPCargos.GetCount() + WaitingNormalCargos.GetCount() + WaitingSpecialCargos.GetCount();
}

int Company::GetnumOfDeliv() {
	return DeliveredCargos.GetCount();
}
void Company::printDelivered(UIClass* pUI) {
	Cargo* c;
	Queue<Cargo*>q;
	while (DeliveredCargos.GetCount() > 0) {
		DeliveredCargos.dequeue(c);
		q.enqueue(c);
			if (dynamic_cast<VIPCargo*>(c)) {
				pUI->openbraceforVIP();
				pUI->Print(c);
				pUI->closebraceforVIP();
			}
			else if(dynamic_cast<NormalCargo*>(c)) {
				pUI->openbraceforNormal();
				pUI->Print(c);
				pUI->closebraceforNormal();
			}
			else  {
				pUI->openbraceforSP();
				pUI->Print(c);
				pUI->closebraceforSP();
			}
	}
	while (q.dequeue(c))
		DeliveredCargos.enqueue(c);
}
//void Company::printDeliveredNormal(UIClass* pUI) {
//	if (DeliveredNormalCargo.GetCount() > 0) {
//		pUI->openbraceforNormal();
//		DeliveredNormalCargo.PrintQ(pUI);
//		pUI->closebraceforNormal();
//	}
//}
//void Company::printDeliveredSP(UIClass* pUI) {
//	if (DeliveredSpCargo.GetCount() > 0) {
//		pUI->openbraceforSP();
//		DeliveredSpCargo.PrintQ(pUI);
//		pUI->closebraceforSP();
//	}
//}
void Company::printEmptyTrucks(UIClass* pUI) {
	if (VIPEmptyTrucks.GetCount() > 0) {
		pUI->openbraceforVIP();
		VIPEmptyTrucks.PrintQ(pUI);
		pUI->closebraceforVIP();
	}
	if (NormalEmptyTrucks.GetCount() > 0) {
		pUI->openbraceforNormal();
		NormalEmptyTrucks.PrintQ(pUI);
		pUI->closebraceforNormal();
	}
	if (SpecialEmptyTrucks.GetCount() > 0) {
		pUI->openbraceforSP();
		SpecialEmptyTrucks.PrintQ(pUI);
		pUI->closebraceforSP();
	}
}
int Company::GetNumOfEmptyTrcs() {
	return VIPEmptyTrucks.GetCount() + NormalEmptyTrucks.GetCount() + SpecialEmptyTrucks.GetCount();
}
void Company::CheckforCargosExceededMaxW() {
	// Normal Check 
	Cargo* C = nullptr;
	if (WaitingNormalCargos.GetCount() > 0) {
		C = WaitingNormalCargos.getHead()->getitem();
		while (C&&C->GetWaitingHours() >= MaxW) {
			WaitingNormalCargos.DeleteFirst(C);
			NCargosExceededMaxW.enqueue(C);
			C = nullptr;
			if(WaitingNormalCargos.getHead())
			C = WaitingNormalCargos.getHead()->getitem();
		}
	}
	//Special Check
	if (WaitingSpecialCargos.GetCount() > 0) {
		WaitingSpecialCargos.peak(C);
		while (C&&C->GetWaitingHours() >= MaxW) {
			WaitingSpecialCargos.dequeue(C);
			SCargosExceededMaxW.enqueue(C);
			C = nullptr;
			WaitingSpecialCargos.peak(C);
		}
	}
	//VIP check 
	if (WaitingVIPCargos.GetCount() > 0) {
		WaitingVIPCargos.peak(C);
		while (WaitingVIPCargos.GetCount()>0&&C->GetWaitingHours() >= MaxW) {
			WaitingVIPCargos.dequeue(C);
			VCargosExceededMaxW.enqueue(C);
			WaitingVIPCargos.peak(C);
		}
	}

}
bool Company::AssignVIP() {
	Cargo* VC;
	if (VIPEmptyTrucks.GetCount() > 0) {
		AssignVIPTruck(0);
		return true;
	}
	else if (NormalEmptyTrucks.GetCount() > 0) {
		AssignNormalTruck(0);
		return true;
		
	}
	else if (SpecialEmptyTrucks.GetCount() > 0) {
		AssignSpecialTruck(0);
		return true;
	}
	else {
		return false;
	}
}
bool Company::AssignSpecial() {
	Cargo* SC;
	if (SpecialEmptyTrucks.GetCount() > 0) {
		AssignSpecialTruck(1);
		return true;
	}
	return false;
}
bool Company::AssignNormal() {
	if (NormalEmptyTrucks.GetCount() > 0) {
		AssignNormalTruck(1);
		return true;
	}
	else if (VIPEmptyTrucks.GetCount() > 0) {
		AssignVIPTruck(1);
		return true;
	}
	return false;
}
void Company::AssignVIPTruck(int T) {
		Cargo* C = nullptr;
		Truck* VT;
		bool CangoNow = false;
		if (VIPEmptyTrucks.GetCount() == 0) return;
		VIPEmptyTrucks.peak(VT);
		if (T == 0 && VT->getTC() > WaitingVIPCargos.GetCount()) return;
		if (T == 1 && VT->getTC() > WaitingNormalCargos.GetCount()) return;
		if (T == 2 && VT->getTC() > VCargosExceededMaxW.GetCount() < 1) return;
		if (T == 3 && VT->getTC() > NCargosExceededMaxW.GetCount() < 1) return;
		VIPEmptyTrucks.dequeue(VT);
		if (T == 0) {
			while (WaitingVIPCargos.dequeue(C) && !VT->isFull()) {
				C->setDel_T(CurrentTime);
				VT->AddCargo(C);
			}
		}
		else if (T == 1) {
			while (WaitingNormalCargos.DeleteFirst(C) && !VT->isFull()) {
				C->setDel_T(CurrentTime);
				VT->AddCargo(C);
			}
		}
		else if (T == 2 && VCargosExceededMaxW.GetCount() > 0) {
			while (VCargosExceededMaxW.dequeue(C) && !VT->isFull()) {
				C->setDel_T(CurrentTime);
				VT->AddCargo(C);
			}
		}
		else if (T == 3 && NCargosExceededMaxW.GetCount() > 0) {
			while (NCargosExceededMaxW.dequeue(C) && !VT->isFull()) {
				C->setDel_T(CurrentTime);
				VT->AddCargo(C);
			}
		}
		VT->incrementJC();
		VT->updateDI();
		VT->setTimeforDelivery(this->CurrentTime );
		VT->setTimeforReturn(this->CurrentTime);
		VT->setTimeforLoading(this->CurrentTime);
		VT->updateCargosDT();
		VT->ResetCargoCount();
		LoadingTrucks.enqueue(VT);
}
void Company::AssignNormalTruck(int T) {
		Cargo* C = nullptr;
		Truck* NT = nullptr;
		if (NormalEmptyTrucks.GetCount() == 0) return;
		NormalEmptyTrucks.peak(NT); 
		if (T == 0 && NT->getTC() > WaitingVIPCargos.GetCount()) return;
		if (T == 1 && NT->getTC() > WaitingNormalCargos.GetCount()) return;
		if (T == 2 && NT->getTC() > VCargosExceededMaxW.GetCount() < 1) return;
		if (T == 3 && NT->getTC() > NCargosExceededMaxW.GetCount() < 1) return;
		NormalEmptyTrucks.dequeue(NT);
		cout << NT->GetID() << " " << NT->getTC() << endl;
		bool CangoNow = false; 
			if (T == 0) {
				while (WaitingVIPCargos.dequeue(C) && !NT->isFull()) {
					C->setDel_T(CurrentTime);
					NT->AddCargo(C);
				}
			}
			else if (T == 1) {
				while (WaitingNormalCargos.DeleteFirst(C) && !NT->isFull()) {
					C->setDel_T(CurrentTime);
					NT->AddCargo(C);
				}
			}
			else if (T == 2) {
				while (VCargosExceededMaxW.dequeue(C) && !NT->isFull()) {
					C->setDel_T(CurrentTime);
					NT->AddCargo(C);
				}
			}
			else if (T == 3) {
				while (NCargosExceededMaxW.dequeue(C) && !NT->isFull()) {
					cout << "Normal Cargo Exceeded with ID " << C->GetID() << endl;
					C->setDel_T(CurrentTime);
					NT->AddCargo(C);
				}
			}			
			//if (!C) return; 
		NT->incrementJC();
		NT->updateDI();
		NT->setTimeforDelivery(this->CurrentTime);
		NT->setTimeforReturn(this->CurrentTime);
		NT->setTimeforLoading(this->CurrentTime);
		NT->updateCargosDT();
		NT->ResetCargoCount();
		LoadingTrucks.enqueue(NT);
}
void Company::AssignSpecialTruck(int T) {
	Cargo* C = nullptr;
	Truck* ST;
	bool CangoNow = false;
	if (SpecialEmptyTrucks.GetCount() == 0) return;
	SpecialEmptyTrucks.peak(ST);
	if (T == 0 && ST->getTC() > WaitingVIPCargos.GetCount()) return;
	if (T == 1 && ST->getTC() > WaitingSpecialCargos.GetCount()) return;
	if (T == 2 && ST->getTC() > VCargosExceededMaxW.GetCount() < 1) return;
	if (T == 3 && ST->getTC() > SCargosExceededMaxW.GetCount() < 1) return;
	SpecialEmptyTrucks.dequeue(ST);
	if (T == 0 && WaitingVIPCargos.GetCount() > 0 && WaitingVIPCargos.GetCount() >= ST->getTC()) {
		while (WaitingVIPCargos.dequeue(C) && !ST->isFull()) {
			C->setDel_T(CurrentTime);
			ST->AddCargo(C);
		}
	}
	else if (T == 1 && WaitingSpecialCargos.GetCount() > 0 && WaitingSpecialCargos.GetCount() >= ST->getTC()) {
		while (WaitingSpecialCargos.dequeue(C) && !ST->isFull()) {
			C->setDel_T(CurrentTime);
			ST->AddCargo(C);
		}
	}
	else if (T == 2 && VCargosExceededMaxW.GetCount() > 0) {
		while (VCargosExceededMaxW.dequeue(C) && !ST->isFull()) {
			C->setDel_T(CurrentTime);
			ST->AddCargo(C);
		}
	}
	else if (T == 3 && SCargosExceededMaxW.GetCount() > 0) {
		while (SCargosExceededMaxW.dequeue(C) && !ST->isFull()) {
			C->setDel_T(CurrentTime);
			ST->AddCargo(C);
		}
	}
	ST->incrementJC();
	ST->updateDI();
	ST->setTimeforDelivery(this->CurrentTime);
	ST->setTimeforReturn(this->CurrentTime);
	ST->setTimeforLoading(this->CurrentTime);
	ST->updateCargosDT();
	ST->ResetCargoCount();
	LoadingTrucks.enqueue(ST);
}
void Company::AssignExceeded() {
	Cargo* Cargoptr;
	if (VCargosExceededMaxW.GetCount() > 0) {
		if (VIPEmptyTrucks.GetCount() > 0) {
			AssignVIPTruck(2);
		}
		else if (NormalEmptyTrucks.GetCount() > 0) {
			AssignNormalTruck(2);
		}
		else if (SpecialEmptyTrucks.GetCount() > 0) {
			AssignSpecialTruck(2);
		}
	}
	if (SCargosExceededMaxW.GetCount() > 0) {
		if (SpecialEmptyTrucks.GetCount() > 0) {
			AssignSpecialTruck(3);
		}
	}
	if (NCargosExceededMaxW.GetCount() > 0) {
		if (NormalEmptyTrucks.GetCount() > 0) {
			AssignNormalTruck(3);
		}
		else if (VIPEmptyTrucks.GetCount() > 0) {
			AssignVIPTruck(3);
		}
	}
}
void Company::CreateTrucks() {
		int cnt = N;
		int s=0, c=0;
		while (cnt--) {
			NSpeeds.dequeue(s);
			NTCs.dequeue(c);
			NormalTruck* N = new NormalTruck(++TruckCount,c,s);
			NormalEmptyTrucks.enqueue(N,N->getprio_s_c());
		}
		cnt = S;
		while (cnt--) {
			SSpeeds.dequeue(s);
			STCs.dequeue(c);
			SpecialTruck* N = new SpecialTruck(++TruckCount, c, s);
			SpecialEmptyTrucks.enqueue(N,N->getprio_s_c());
		}
		cnt = V;
		while (cnt--) {
			VSpeeds.dequeue(s);
			VTCs.dequeue(c);
			VIPTruck* N = new VIPTruck(++TruckCount, c, s);
			VIPEmptyTrucks.enqueue(N,N->getprio_s_c());
		}
}
void Company::CheckforCheckupTrucks() {
	Truck* T; 
	Queue<Truck* >temp;
	// Check for normal
	while (NormalEmptyTrucks.GetCount() > 0 && NormalEmptyTrucks.dequeue(T)) {
		if (T->getJC() >= JourneyCount)NInCheckupTrucks.enqueue(T);
		else temp.enqueue(T);
		}
	while (temp.GetCount() > 0 && temp.dequeue(T))NormalEmptyTrucks.enqueue(T,T->getprio_s_c());

	// Check for VIP

	while (NormalEmptyTrucks.GetCount() > 0 && NormalEmptyTrucks.dequeue(T)) {
		if (T->getJC() >= JourneyCount)NInCheckupTrucks.enqueue(T);
		else temp.enqueue(T);
	}
	while (temp.GetCount() > 0 && temp.dequeue(T))NormalEmptyTrucks.enqueue(T,T->getprio_s_c());
	
}
void Company::CheckforTrucks() {
	Truck* T;
	bool bo;
	Queue<Truck* > temp;
	while (LoadingTrucks.GetCount() > 0) {
		LoadingTrucks.dequeue(T);
		if (T->getTimeforLoading() == CurrentTime) {
			T->updatePriority();
			MovingTrucks.enqueue(T, T->getPriority());
		}
		else {
			temp.enqueue(T);
		}
	}
	while (temp.dequeue(T)) 
		LoadingTrucks.enqueue(T);
	
	Queue<Truck* > temp2;
	while (MovingTrucks.dequeue(T)) {
		Queue<Cargo* > Cargos = T->getDelivered(CurrentTime);
		cout << T->getTimeforReturn().getDAY() << ":" << T->getTimeforReturn().gethour() << endl;
		if (Cargos.GetCount()>0) {
			Cargo* C;
			while (Cargos.GetCount() > 0) {
				Cargos.dequeue(C);
				DeliveredCargos.enqueue(C);
			}
			temp2.enqueue(T);
			T->ResetDeliveryTime();
		}
		else if (T->getTimeforReturn() == CurrentTime) {
			if (T->getJC() == this->Get_JourneyCount())	//ismail
			{
				if (dynamic_cast<NormalTruck*> (T))  bo = NInCheckupTrucks.enqueue(T);
				else if (dynamic_cast<SpecialTruck*> (T))  bo = SInCheckupTrucks.enqueue(T);
				else  bo = VInCheckupTrucks.enqueue(T);
				Time t;
				t.setDAY(CurrentTime.getDAY());
				t.sethour(CurrentTime.gethour() + T->get_MaintenanceTime());
				T->set_putInMaintenanceTime(t);
			}
			else if (dynamic_cast<NormalTruck*> (T)) NormalEmptyTrucks.enqueue(T,T->getprio_s_c());
			else if (dynamic_cast<SpecialTruck*> (T)) SpecialEmptyTrucks.enqueue(T,T->getprio_s_c());
			else VIPEmptyTrucks.enqueue(T,T->getprio_s_c());
		}
		else {
			temp2.enqueue(T);
		}
		cout << T->getTimeforReturn().getDAY() << ":" << T->getTimeforReturn().gethour() << endl;
	}

	while (temp2.GetCount() > 0) {
		temp2.dequeue(T);
		MovingTrucks.enqueue(T,T->getPriority());
	}
	cout << MovingTrucks.GetCount() << endl;
	
	for (int i = 0; i < NInCheckupTrucks.GetCount(); i++)	//ismail for N
	{
		bo = NInCheckupTrucks.dequeue(T);
		if (T->get_putInMaintenanceTime() == CurrentTime)
			bo = NormalEmptyTrucks.enqueue(T,T->getprio_s_c());
		else
			bo = NInCheckupTrucks.enqueue(T);
	}
	for (int i = 0; i < SInCheckupTrucks.GetCount(); i++)	//ismail for S
	{
		bo = SInCheckupTrucks.dequeue(T);
		if (T->get_putInMaintenanceTime() == CurrentTime)
			bo = SpecialEmptyTrucks.enqueue(T,T->getprio_s_c());
		else
			bo = SInCheckupTrucks.enqueue(T);
	}
	for (int i = 0; i < VInCheckupTrucks.GetCount(); i++)	//ismail for VIP
	{
		bo = VInCheckupTrucks.dequeue(T);
		if (T->get_putInMaintenanceTime() == CurrentTime)
			bo = VIPEmptyTrucks.enqueue(T,T->getprio_s_c());
		else
			bo = VInCheckupTrucks.enqueue(T);
	}
}

void Company::Deliver(Truck * &T) {
	Cargo* C;
	while (T->RemoveCargo(C)&&C) {
		DeliveredCargos.enqueue(C);
	}
	//MovingTrucks.enqueue(T,T->getPriority());
}

int Company::Get_JourneyCount()		//ismail
{
	return this->JourneyCount;
}

void Company::Calc_Delivered_Cargos_count()
{
	Cargo* c;
	bool bo;
	for (int i = 0; i < this->DeliveredCargos.GetCount(); i++)
	{
		bo = DeliveredCargos.dequeue(c);
		if (dynamic_cast<NormalCargo*> (c))	N_Cargo_Count++;
		else if(dynamic_cast<SpecialCargo*> (c)) S_Cargo_Count++;
		else VIP_Cargo_Count++;
	}
}

int Company::get_N_Cargo_Count()
{
	return N_Cargo_Count;
}

int Company::get_S_Cargo_Count()
{
	return S_Cargo_Count;
}

int Company::get_VIP_Cargo_Count()
{
	return VIP_Cargo_Count;
}






Time Company::AverageWaitingTime_DeliveredNormalCargos()
{
	Cargo* c;
	Time time;
	bool bo;
	for (int i = 0; i < DeliveredCargos.GetCount(); i++)
	{
		bo = DeliveredCargos.dequeue(c);
		if (dynamic_cast<NormalCargo*> (c))
		{
			time = time + c->GetWaitingHours();
		}
		bo = DeliveredCargos.enqueue(c);
	}

	return time;
}
Time Company::AverageWaitingTime_DeliveredSpecialCargos()
{
	Cargo* c;
	Time time;
	bool bo;
	for (int i = 0; i < DeliveredCargos.GetCount(); i++)
	{
		bo = DeliveredCargos.dequeue(c);
		if (dynamic_cast<SpecialCargo*> (c))
		{
			time = time + c->GetWaitingHours();
		}
		bo = DeliveredCargos.enqueue(c);
	}
	return time;
}
Time Company::AverageWaitingTime_DeliveredVIPCargos()
{
	Cargo* c;
	Time time;
	bool bo;
	for (int i = 0; i < DeliveredCargos.GetCount(); i++)
	{
		bo = DeliveredCargos.dequeue(c);
		if (dynamic_cast<VIPCargo*> (c))
		{
			time = time + c->GetWaitingHours();
		}
		bo = DeliveredCargos.enqueue(c);
	}
	return time;
}
//void Company::set_NumberOfAutoPromotedCargos(int i)
//{
//	this->NumberOfAutoPromotedCargos = i;
//}
//int Company::get_NumberOfAutoPromotedCargos()
//{
//	return this->NumberOfAutoPromotedCargos;
//}

//int Company::get_numOf_N_Truck()
//{
//	return this->N;
//}
//
//int Company::get_numOf_S_Truck()
//{
//	return this->S;
//}
//
//int Company::get_numOf_VIP_Truck()
//{
//	return this->V;
//}

//Time Company::NormalTrucks_ActiveTime()
//{
//	Time time;
//	Truck* truck;
//	bool bo;
//	for (int i = 0; i < NormalTrucks.GetCount(); i++)
//	{
//		bo = NormalTrucks.dequeue(truck);
//		time.sethour(time.gethour() + truck->get_ActiveTime().gethour() + 24 * truck->get_ActiveTime().getDAY());
//		bo = NormalTrucks.enqueue(truck);
//	}
//	return time;
//}
//
//Time Company::SpecialTrucks_ActiveTime()
//{
//	Time time;
//	Truck* truck;
//	bool bo;
//	for (int i = 0; i < SpecialTrucks.GetCount(); i++)
//	{
//		bo = SpecialTrucks.dequeue(truck);
//		time.sethour(time.gethour() + truck->get_ActiveTime().gethour() + 24 * truck->get_ActiveTime().getDAY());
//		bo = SpecialTrucks.enqueue(truck);
//	}
//	return time;
//}
//
//Time Company::VIPTrucks_ActiveTime()
//{
//	Time time;
//	Truck* truck;
//	bool bo;
//	for (int i = 0; i < VIPTrucks.GetCount(); i++)
//	{
//		bo = VIPTrucks.dequeue(truck);
//		time.sethour(time.gethour() + truck->get_ActiveTime().gethour() + 24 * truck->get_ActiveTime().getDAY());
//		bo = VIPTrucks.enqueue(truck);
//	}
//	return time;
//}

//int Company::NormalTrucks_Utilization()
//{
//	Truck* truck;
//	bool bo;
//	int count = 0;
//	for (int i = 0; i < NormalTrucks.GetCount(); i++)
//	{
//		bo = NormalTrucks.dequeue(truck);
//		count = count + truck->Truck_utilization(this->TSM);
//		bo = NormalTrucks.enqueue(truck);
//	}
//	return count;
//}
//
//int Company::SpecialTrucks_Utilization()
//{
//	Truck* truck;
//	bool bo;
//	int count = 0;
//	for (int i = 0; i < SpecialTrucks.GetCount(); i++)
//	{
//		bo = SpecialTrucks.dequeue(truck);
//		count = count + truck->Truck_utilization(this->TSM);
//		bo = SpecialTrucks.enqueue(truck);
//	}
//	return count;
//}
//
//int Company::VIPTrucks_Utilization()
//{
//	Truck* truck;
//	bool bo;
//	int count = 0;
//	for (int i = 0; i < VIPTrucks.GetCount(); i++)
//	{
//		bo = VIPTrucks.dequeue(truck);
//		count = count + truck->Truck_utilization(this->TSM);
//		bo = VIPTrucks.enqueue(truck);
//	}
//	return count;
//}







//
//void Company::cancellID(int id)
//{
//	Queue<Cargo*>Q;
//	Cargo* Cptr;
//	while (WaitingCargos.dequeue(Cptr)) {
//		if (Cptr->GetID() != id) {
//			Q.enqueue(Cptr);
//		}
//	}
//	while (Q.dequeue(Cptr))
//		WaitingCargos.enqueue(Cptr, Cptr->Getpriority());
//}
//void Company::PrintDelivered(UIClass* pUI) {
//	if(Deli)
//	DeliveredCargos.PrintQ(pUI);
//}
	/*Queue<Cargo*>Q;
	NormalCargo* temp=nullptr;
	Cargo* Cptr;
	while (WaitingCargos.dequeue(Cptr)) {
		if (Cptr->GetID() == id) {
			temp=dynamic_cast<NormalCargo*> (Cptr);
		}
		else Q.enqueue(Cptr);
	}
	while (Q.dequeue(Cptr))
		WaitingCargos.enqueue(Cptr, Cptr->Getpriority());*/

		//bool Company::UpdatetoVIP(int ID) {                //notworking, see getnormalCargo func and its excute 
		//	Cargo* C = NULL;
		//	Cargo* Search = NULL;
		//	WaitingCargos.peak(C);
		//	PriorityQueue<Cargo* > temp;
		//	while(WaitingCargos.GetCount()>0) {
		//		if (C->GetID() == ID) {
		//			Search = C;
		//		}
		//		else temp.enqueue(C, C->Getpriority());
		//		WaitingCargos.dequeue(C);
		//		WaitingCargos.peak(C);
		//	}
		//	temp.peak(C);
		//	while (temp.GetCount()>0) {
		//		WaitingCargos.enqueue(C, C->Getpriority());
		//		temp.dequeue(C);
		//		temp.peak(C);
		//	}
		//	if (Search) {
		//		int id = Search->GetID();
		//		int h = Search->getPT().gethour();
		//		int d = Search->getPT().getDAY();
		//		int lt = Search->getLT();
		//	//	double p = Search->Getpriority();  //M
		//		double dis = Search->getDdes();
		//		double c = Search->getCost();
		//		VIPCargo* VC = new VIPCargo(id/*, p*/, d, h, lt, c, dis);
		//		WaitingCargos.enqueue(VC, VC->Getpriority());
		//		return true;
		//	}
		//	return false;
		//}