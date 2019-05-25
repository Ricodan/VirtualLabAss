#include "tinyfsm.hpp"
#include "Protocol.h"
#include <iostream>

using namespace std;


 
class PreStreak; //Forward declaration
class PostStreak;

// ----------------------------------------------------------------------------
// Transition functions
//

static void CallMaintenance() {
	cout << "*** calling maintenance ***" << endl;
}

static void CallFirefighters() {
	cout << "*** calling firefighters ***" << endl;
}

// ----------------------------------------------------------------------------
// State: Clean
//

class Clean : public Protocol
{
	void react(LoopDippedInVial const & e) override {
		cout << "InoculationLoop id is now contaminated";

		transit<PreStreak>();
	}

	void myState()
	{
		cout << "Clean" << endl;
	}
};

class InterClean : public Protocol
{
	void react(LoopDippedInVial const & e) override {
		cout << "InoculationLoop id is now Contaminated ";
		cout << "Dipped in Vial";

		transit<PreStreak>();

	}

	void react(Stow const & e) override {
		cout << "Beginning with a new Petri dish";


		transit<Clean>();

	}

	void react(Streak const & e) override {
		cout << "InoculationLoop id is now Contaminated ";
		cout << "Streaked";

		transit<PostStreak>();

	}

	void myState()
	{
		cout << "InterClean" << endl;
	}

};

class PostStreak : public Protocol
{
	void react(LoopSterilize const & e) override {
		cout << "InoculationLoop id is now Sterilized ";
		cout << "Sterilization";

		transit<InterClean>();
	};
	void myState()
	{
		cout << "PostStreak" << endl;
	}
};

class PreStreak : public Protocol
{
	void react(Streak const & e) override {
		cout << "InoculationLoop id is now contaminated ";
		cout << "Streak";

		transit<PostStreak>();
	
	};

	void myState()
	{
		cout << "PreStreak" << endl;
	}
};


class Spoiled : public Protocol
{

};

//Some definitions are missing, especially for the non  virtual functions I think. 
// ----------------------------------------------------------------------------
// Base state: default implementations
//

void Protocol::react(LoopDippedInVial const &) {
	cout << "LoopDippedInVial " << endl;
}

void Protocol::react(Streak const &) {
	cout << "Streak" << endl;
}

void Protocol::react(LoopSterilize const &) {
	cout << "LoopSterilize" << endl;
}
void Protocol::react(Stow const &) {
	cout << "Stow " << endl;
}
 
void Protocol::myState()
{
	cout << "Base State" << endl;
}

Protocol::Protocol()
{
	cout << "Create a new State Machine" << endl;
}


FSM_INITIAL_STATE(Protocol, Clean)
//Protocol::~Protocol()
//{
//
//}
