#include "tinyfsm.hpp"
#include "Protocol.h"
#include <iostream>

using namespace std;


 
class PreStreak; //Forward declaration
class PostStreak;
class Spoiled;

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
	void react(LoopDippedInVial const & e) override 
	{
		cout << "InoculationLoop id is now contaminated" <<endl;
		cout << "Transiting to PreStreak" << endl;
		transit<PreStreak>();
	}

	void react(Streak const & e) override
	{
		cout << "InoculationLoop id is now contaminated" << endl;
		cout << "Transiting to Soiled, you made a wrong move" << endl;
		transit<Spoiled>();
	}


	string myState()
	{
		cout << "Clean" << endl;
		return "Clean";
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

	string myState()
	{
		cout << "InterClean" << endl;
		return "InterClean";
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

	string myState()
	{
		cout << "PostStreak" << endl;
		return "PostStreak";
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

	void react(LoopSterilize const & e) override
	{
		cout << "InoculationLoop id is now sterilized " << endl;
		cout << "Sterilized Loop" << endl;
		transit<Clean>();
	}
		
	string myState()
	{
		cout << "PreStreak" << endl;
		return "PreStreak";
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
	string myState()
	{
		cout << "Spoiled" << endl;
		return "Spoiled";
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

string Protocol::myState()
{
	cout << "Base State" << endl;
	return "Base State";
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
