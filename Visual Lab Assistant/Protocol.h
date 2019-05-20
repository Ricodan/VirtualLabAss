#pragma once
#include "tinyfsm.hpp"

#include <iostream>

struct LabEvent : tinyfsm::Event
{
	int id;
	//What could go in here?
	//Proximity
	//Bool haveMadeContact
	//

};

//the loop get's dipped in the ependorph tube with bacteria
struct LoopDippedInVial : LabEvent { };
//Made a simple streak on the petri dish.
struct Streak : LabEvent { };
//Put the loop over the Bunsen burner.
struct LoopSterilize : LabEvent { };
//Stow away petri dish
struct Stow : LabEvent { };
//An Invalidating action
struct Soil : LabEvent { };
//Keyboard Interrupt
struct Alarm : tinyfsm::Event { };




class Protocol : public tinyfsm::Fsm<Protocol>
{
public:
	void react(tinyfsm::Event const &) {};

	virtual void react(LoopDippedInVial const &);
	virtual void react(Streak const &);
	virtual void react(LoopSterilize const &);
	virtual void react(Stow const &);
	void         react(Soil        const &); //This last one is shared by all states, so it's not virtual.
	void		 react(Soil         const&);

	virtual void entry(void) {};
	void exit(void) {};

	
};



