#include <iostream>
#include <vector>
#include <memory>
#include <utility>
#include <string>  
#include <fstream>
using namespace std;

#define ERROR_MSG "error " 

class RCB;
class PCB;
class List_Type;


enum Status_Type { e_Ready, e_Running, e_Blocked, e_Dead };
enum Priority_List { e_init, e_user, e_system };

string output;
string series;
int counter = 0;

class PCB {
public:
	string PID;
	vector<pair<RCB *, int>> Other_resources;
	Status_Type Type;
	void * List = nullptr;
	PCB * Parent = nullptr;
	vector<PCB> Child;
	Priority_List Priority;

	void Create(string PID, int Priority);
	void Destroy(string PID);
	PCB * find(string PID);
	vector<PCB *>& findList(string PID);
	bool AncestorCheck(PCB * TBD, PCB * Deleter);
	void Kill_Tree(PCB * E);
	void Request(string RID, int n);
	void remove();
	void Release(string RID, int n);
	void ReleaseSpecific(string RID, int n, PCB * p);
	void Scheduler();
	void preempt(PCB * p);//, PCB * self);
	void Timeout();
};

class RCB {
public:
	string RID;
	int Total_Units;
	int Remaining_Units;
	vector<PCB *> Waiting_List;
	vector<int> req;

};

class Resources {
public:
	RCB R1 = { "R1", 1, 1 };
	RCB R2 = { "R2", 2, 2 };
	RCB R3 = { "R3", 3, 3 };
	RCB R4 = { "R4", 4, 4 };

	RCB * Get_RCP(string RID)
	{
		if (!RID.compare("R1")) return &R1;
		if (!RID.compare("R2")) return &R2;
		if (!RID.compare("R3")) return &R3;
		if (!RID.compare("R4")) return &R4;
		return nullptr;
	}

	void Clear_RCB()
	{
		R1 = { "R1", 1, 1 };
		R2 = { "R2", 2, 2 };
		R3 = { "R3", 3, 3 };
		R4 = { "R4", 4, 4 };
	}
};

Resources Master;


class List_Type {
public:
	vector<PCB *> first;
	vector<PCB *> second;
	vector<PCB *> third;
};

List_Type RL;

void PCB::Create(string PID, int Priority)
{
	// error checking
	if (Priority < 0 || Priority > 2)
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");
		return;
	}
	if (this->Type == e_Blocked)
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");;
		return;
	}
	if (find(PID))
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");;
		return;
	}
	//create PCB data structure
	PCB * temp(new PCB);

	//initialize PCB using parameters
	temp->PID = PID;
	temp->Priority = static_cast<Priority_List>(Priority);
	temp->List = this->List;
	temp->Type = e_Ready;

	//link PCB to creation tree
	temp->Parent = this;
	this->Child.push_back(*temp);

	//insert(RL, PCB)
	if (Priority == 1) RL.second.push_back(temp);
	else RL.third.push_back(temp);

	Scheduler();
}

void PCB::Destroy(string PID)
{
	if (!PID.compare("init"))
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");;
		return;
	}
	auto temp = find(PID);
	if (temp && AncestorCheck(temp, this))
	{
		Kill_Tree(temp);
	}
	else
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");;
		return;
	}
	this->Type = e_Dead;
	Scheduler();
}

PCB * PCB::find(string PID)
{
	if (!RL.third.empty()) for (auto E : RL.third) if (!E->PID.compare(PID)) return E;
	if (!RL.second.empty()) for (auto E : RL.second) if (!E->PID.compare(PID)) return E;
	if (!Master.R1.Waiting_List.empty()) for (auto E : Master.R1.Waiting_List) if (!E->PID.compare(PID)) return E;
	if (!Master.R2.Waiting_List.empty()) for (auto E : Master.R2.Waiting_List) if (!E->PID.compare(PID)) return E;
	if (!Master.R3.Waiting_List.empty()) for (auto E : Master.R3.Waiting_List) if (!E->PID.compare(PID)) return E;
	if (!Master.R4.Waiting_List.empty()) for (auto E : Master.R4.Waiting_List) if (!E->PID.compare(PID)) return E;
	return nullptr;
}

vector<PCB *>& PCB::findList(string PID)
{
	if (!RL.third.empty()) for (auto E : RL.third) if (!E->PID.compare(PID)) return RL.third;
	if (!RL.second.empty()) for (auto E : RL.second) if (!E->PID.compare(PID)) return RL.second;
	if (!Master.R1.Waiting_List.empty()) for (auto E : Master.R1.Waiting_List) if (!E->PID.compare(PID)) return Master.R1.Waiting_List;
	if (!Master.R2.Waiting_List.empty()) for (auto E : Master.R2.Waiting_List) if (!E->PID.compare(PID)) return Master.R2.Waiting_List;
	if (!Master.R3.Waiting_List.empty()) for (auto E : Master.R3.Waiting_List) if (!E->PID.compare(PID)) return Master.R3.Waiting_List;
	if (!Master.R4.Waiting_List.empty()) for (auto E : Master.R4.Waiting_List) if (!E->PID.compare(PID)) return Master.R4.Waiting_List;
	vector<PCB *> x = {};
	return x;
}

bool PCB::AncestorCheck(PCB * TBD, PCB * Deleter)
{
	if (!TBD->PID.compare(Deleter->PID))
		return true;
	if (TBD->Priority == e_init)
		return false;
	if (AncestorCheck(TBD->Parent, Deleter))
		return true;
	return false;
}

void PCB::Kill_Tree(PCB * E)
{
	string PID = E->PID;
	while (!E->Child.empty())
		Kill_Tree(find(E->Child.front().PID));
	for (auto i = 0; i < E->Other_resources.size();)
	{
		ReleaseSpecific(E->Other_resources.at(i).first->RID, E->Other_resources.at(i).second, E);
	}
	int i_FAM = 0;
	for (auto i = 0; i < E->Parent->Child.size();) // find where E is in parents child list
	{
		if (!E->PID.compare(E->Parent->Child.at(i).PID))
		{
			i_FAM = i;
			break;
		}
		i++;
	}
	auto temp = &findList(PID);
	for (auto i = 0; i < temp->size();)
	{
		if (!PID.compare(temp->at(i)->PID))
		{
			//delete *(temp->at(i)->Parent->Child.begin() + i_FAM);
			temp->at(i)->Parent->Child.erase(temp->at(i)->Parent->Child.begin() + i_FAM);
			delete *(temp->begin() + i);
			temp->erase(temp->begin() + i);
			continue;
		}
		i++;
	}
}

void PCB::Request(string RID, int n)
{
	if (this->Priority == e_init)
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");;
		return;
	}

	RCB * r = Master.Get_RCP(RID);
	if (r->Total_Units < n)
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");;
		return;
	}
	int sum = 0;
	for (auto E : this->Other_resources)
	{
		if (!E.first->RID.compare(r->RID))
		{
			sum += E.second;
		}
	}
	if (sum + n > r->Total_Units)
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");;
		return;
	}

	if (r->Remaining_Units >= n)
	{
		r->Remaining_Units -= n;

		this->Other_resources.push_back({ r, n });
	}
	else
	{
		this->Type = e_Blocked;
		this->List = r;
		remove();
		r->Waiting_List.push_back(static_cast<PCB *>(this));
		r->req.push_back(n);
	}
	Scheduler();
}

void PCB::remove()
{
	if (this->Priority == e_user)
	{
		for (auto it = RL.second.begin(); it != RL.second.end(); ++it)
		{
			if (!this->PID.compare((*it)->PID))
			{
				RL.second.erase(it);
				return;
			}
		}
	}
	if (this->Priority == e_system)
	{
		for (auto it = RL.third.begin(); it != RL.third.end(); ++it)
		{
			if (!this->PID.compare((*it)->PID))
			{
				RL.third.erase(it);
				return;
			}
		}
	}
}

void PCB::Release(string RID, int n)
{
	RCB * r = Master.Get_RCP(RID);
	if (!r)
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");;
		return;
	}
	if (r->Remaining_Units >= r->Total_Units || r->Remaining_Units + n > r->Total_Units)
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");;
		return;
	}
	r->Remaining_Units += n;
	for (int i = 0; i < this->Other_resources.size(); )
	{
		if (!this->Other_resources.at(i).first->RID.compare(r->RID))
		{
			this->Other_resources.erase(this->Other_resources.begin() + i);
			continue;
		}
		i++;
	}
	while (!r->Waiting_List.empty()) // add blocked stuff back on
	{
		int req = r->req.front();
		if (r->Remaining_Units >= req)
		{
			r->Remaining_Units -= req;
		}
		auto q = r->Waiting_List.front();
		r->Waiting_List.erase(r->Waiting_List.begin());
		r->req.erase(r->req.begin());
		q->Type = e_Ready;
		q->List = &RL;
		q->Other_resources.push_back({ r, req });
		if (q->Priority == e_user) RL.second.push_back(q);
		else RL.third.push_back(q);
	}
	Scheduler();
}

void PCB::ReleaseSpecific(string RID, int n, PCB * p)
{
	RCB * r = Master.Get_RCP(RID);
	if (r->Remaining_Units >= r->Total_Units || r->Remaining_Units + n > r->Total_Units)
	{
		cout << ERROR_MSG;
		output.append(ERROR_MSG); series.append(std::to_string(++counter) + " ");;
		return;
	}
	r->Remaining_Units += n;
	for (int i = 0; i < p->Other_resources.size(); )
	{
		if (!p->Other_resources.at(i).first->RID.compare(r->RID))
		{
			p->Other_resources.erase(p->Other_resources.begin() + i);
			continue;
		}
		i++;
	}
	while (!r->Waiting_List.empty()) // add blocked stuff back on
	{

		int req = r->req.front();
		if (r->Remaining_Units >= req)
		{
			r->Remaining_Units -= req;
		}
		auto q = r->Waiting_List.front();
		r->Waiting_List.erase(r->Waiting_List.begin());
		r->req.erase(r->req.begin());
		q->Type = e_Ready;
		q->List = &RL;
		q->Other_resources.push_back({ r, req });
		if (q->Priority == e_user) RL.second.push_back(q);
		else RL.third.push_back(q);
	}
}

void PCB::Scheduler()
{
	// find highest priority process p
	PCB * p;
	if (!RL.third.empty()) { p = RL.third.front(); }
	else if (!RL.second.empty()) { p = RL.second.front(); }
	else { p = RL.first.front(); }

	// if self priority is less than p priority, or self is not running or self is null, preempt
	if (static_cast<int>(this->Priority) < static_cast<int>(p->Priority) || Type != e_Running)// || this == nullptr)
	{
		preempt(p);
	}
	cout << p->PID << " ";
	output.append(p->PID + " ");  series.append(std::to_string(++counter) + " ");;
}

void PCB::preempt(PCB * p)//, PCB * self)
{
	if (this->Type != e_Blocked) this->Type = e_Ready;
	p->Type = e_Running;
}

void PCB::Timeout()
{
	if (!this->PID.compare("init"))
	{
		cout << "init ";
		output.append("init "); series.append(std::to_string(++counter) + " ");;
		return;
	}
	auto q = this;
	remove();
	q->Type = e_Ready;
	if (q->Priority == e_user) RL.second.push_back(q);
	else RL.third.push_back(q);
	Scheduler();
}

PCB * Curr_Proc()
{
	if (!RL.third.empty())
	{
		for (auto E : RL.third)
		{
			if (E->Type == e_Running)
			{
				return E;
			}
		}
	}
	if (!RL.second.empty())
	{
		for (auto E : RL.second)
		{
			if (E->Type == e_Running)
			{
				return E;
			}
		}
	}
	return RL.first.front();
}

int main(int argc, char* argv[])
{
	List_Type * ReadyList = &RL;

	ifstream inFile;
	string x;// = "H:\input.txt";
	cin >> x;
	inFile.open(x);
	if (!inFile) {
		cout << "Unable to open file";
		exit(1); // terminate with error
	}
	string cmd, name;
	int priority, units;

	PCB * temp = nullptr, *placeholder = nullptr;
	temp = (new PCB);
	placeholder = temp;
	temp->PID = "init";
	temp->Parent = nullptr;
	temp->Priority = e_init;
	temp->Type = e_Running;
	temp->List = ReadyList;
	ReadyList->first.push_back(temp);
// 	inFile >> cmd;
	cout << "init ";
	output.append("init ");
// 	if (cmd.at(0) != 'i') // checking if first instruction is init or not
// 	{
// 		cmd = "init";
// 		inFile.clear();
// 		inFile.seekg(0, ios::beg);
// 		goto firstinstruction;
// 	}
	while (inFile >> cmd)
	{
	firstinstruction:
		if (!cmd.compare("cr") || !cmd.compare("create"))
		{
			inFile >> name; inFile >> priority;
			temp->Create(name, priority);
		}
		else if (!cmd.compare("de") || !cmd.compare("delete"))
		{
			inFile >> name;
			temp->Destroy(name);
		}
		else if (!cmd.compare("req") || !cmd.compare("request"))
		{
			inFile >> name; inFile >> units;
			temp->Request(name, units);
		}
		else if (!cmd.compare("rel") || !cmd.compare("release"))
		{
			inFile >> name; inFile >> units;
			temp->Release(name, units);
		}
		else if (!cmd.compare("to") || !cmd.compare("timeout"))
		{
			temp->Timeout();
		}
		else if (!cmd.compare("init") || !cmd.compare("initialize"))
		{
			if (temp)
			{
				while (!placeholder->Child.empty())
				{
					placeholder->Kill_Tree(&placeholder->Child.front());
				}
				Master.Clear_RCB();
				cout << "\ninit ";
				output.append("\ninit ");
			}
		}
		else
		{
			cout << ERROR_MSG;
			output.append(ERROR_MSG);
		}
		temp = Curr_Proc();
	}
	inFile.close();



	cin >> x;
	//	x = "H:\output.txt";
	ofstream outfile{ x };
	outfile << output;
	outfile.close();

	if (placeholder)
	{
		while (!placeholder->Child.empty())
		{
			placeholder->Destroy(placeholder->Child.front().PID);
		}
		delete placeholder;
	}

	return 0;
}



