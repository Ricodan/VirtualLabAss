#include "tinyfsm.hpp"
#include "Protocol.h"
#include <iostream>

using namespace std;


 
class PreStreak; //Forward declaration
class PostStreak;

// ----------------------------------------------------------------------------
// Transition and condition functions
//

static void CallMaintenance() 
{
	cout << "*** calling maintenance ***" << endl;
}

static void CallFirefighters() 
{
	cout << "*** calling firefighters ***" << endl;
}
static bool loopIsSterilized()
{
	
	cout << "*** Checking if the loop is sterlized***" << endl;
}


// ----------------------------------------------------------------------------
// State: Clean
//

class Clean : public Protocol
{
	void react(LoopDippedInVial const & e) override {
		cout << "InoculationLoop id is now contaminated" <<endl;
		cout << "Transiting to PreStreak" << endl;
		transit<PreStreak>();
	}

	void myState()
	{
		cout << "Clean" << endl;
	}
};

// ----------------------------------------------------------------------------
// State: End
//

class End : public Protocol
{

};

// ----------------------------------------------------------------------------
// State: InterClean
//
class InterClean : public Protocol
{
	void react(LoopDippedInVial const & e) override 
	{
		cout << "InoculationLoop id is now Contaminated "<< endl;
		cout << "Dipped in Vial";
		cout << "Transiting to PreStreak" << endl;
		transit<PreStreak>();

	}

	void react(Stow const & e) override 
	{
		cout << "Beginning with a new Petri dish" << endl;
		cout << "Transiting to Clean" << endl;
		transit<Clean>();

	}

	void react(Streak const & e) override 
	{
		cout << "InoculationLoop id is now Contaminated " << endl;
		cout << "Streaked" << endl;

		transit<PostStreak>();

	}

	void myState()
	{
		cout << "InterClean" << endl;
	}

};

// ----------------------------------------------------------------------------
// State: PostStreak
//
class PostStreak : public Protocol
{
	void react(LoopSterilize const & e) override 
	{
		cout << "InoculationLoop id is now Sterilized "<< endl;
		cout << "Sterilization" << endl;

		transit<InterClean>();
	}

	void react(Streak const & e) override
	{
		cout << "You fucked up" << endl;
		transit<End>();
	}

	void myState()
	{
		cout << "PostStreak" << endl;
	}
};

// ----------------------------------------------------------------------------
// State: PreStreak
//
class PreStreak : public Protocol
{
	void react(Streak const & e) override
	{
		cout << "InoculationLoop id is now contaminated " << endl;
		cout << "Streak" << endl;

		transit<PostStreak>();
	
	}

	void myState()
	{
		cout << "PreStreak" << endl;
	}
};

// ----------------------------------------------------------------------------
// State: Spoiled
//
class Spoiled : public Protocol
{
	void react(LoopSterilize const & e) override 
	{
		cout << "InoculationLoop id is now sterilized " << endl;
		cout << "Sterilize" << endl;
		transit<Clean>();

	}
	void myState()
	{
		cout << "Spoiled" << endl;
	}

};




//Some definitions are missing, especially for the non  virtual functions I think. 
// ----------------------------------------------------------------------------
// Base state: default implementations
//

void Protocol::react(LoopDippedInVial const &) 
{
	cout << "LoopDippedInVial " << endl;
}

void Protocol::react(Streak const &) 
{
	cout << "Streak" << endl;
}

void Protocol::react(LoopSterilize const &) 
{
	cout << "LoopSterilize" << endl;
}
void Protocol::react(Stow const &) 
{
	cout << "Stow " << endl;
}
 
void Protocol::react(Soil const &) 
{
 		cout << "InoculationLoop id is now contaminated ";
		cout << "Streak";
		transit<Spoiled>();
		 
}

void Protocol::myState()
{
	cout << "Base State" << endl;
}

//void Protocol::react(tinyfsm::Event const &) 
//{
//	cout << "Illegal" << endl;
//
//}

Protocol::Protocol()
{
	cout << "Create a new State Machine" << endl;
}



FSM_INITIAL_STATE(Protocol, Clean)


//Protocol::~Protocol()
//{
//
//}
