// Fill out your copyright notice in the Description page of Project Settings.


#include "StateMachineComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UStateMachineComponent::UStateMachineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	//Enable by default the replication on the state machine
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// If the tick is enabled then execute the tick for the current state.
	// If it's not enabled, it's mean that the state machine is currently on a state transition
	if(bCanUseTick)
	{
		TickState(DeltaTime);
	}

	// If bUseDebug == true then execute the Debug function
	// But only when inside the editor.
#if WITH_EDITOR
	if(bUseDebug)
	{
		Debug();
	}
#endif
}

void UStateMachineComponent::SwitchState(FGameplayTag NewState)
{
	// Call the switch state on the server for replication purpose
	Server_SwitchState(NewState);
}

void UStateMachineComponent::Server_SwitchState_Implementation(FGameplayTag NewState)
{
	// Call the switch state on all of the connected clients.
	Client_SwitchState(NewState);
}

void UStateMachineComponent::Client_SwitchState_Implementation(FGameplayTag NewState)
{
	// Leave the function if the new state isn't an accepted one
	if(!AcceptedStates.Contains(NewState) && !InitialState.MatchesTagExact(NewState))
	{
		if(bUseDebug)
			UE_LOG(LogTemp, Warning, TEXT("Couldn't switch state for %s because the state %s isn't in accepted state array"), *GetOwner()->GetName(), *NewState.ToString());
		return;
	}

	// Leave the function if the new state the same as the current one
	if(CurrentState.MatchesTagExact(NewState))
	{
		if(bUseDebug)
			UE_LOG(LogTemp, Warning, TEXT("Couldn't switch state for %s because it is already in %s state"), *GetOwner()->GetName(), *NewState.ToString());
		return;
	}

	//Stop the tick when changing the state in case of the transition takes too much time
	bCanUseTick = false;
	//First, exit the current state
	ExitState();
	//Then, set the new state
	CurrentState = NewState;
	//Finally, enter the new state
	EnterState();
	//Reenable the tick 
	bCanUseTick = true;
}

void UStateMachineComponent::EnterState()
{
	// If the enter state delegate is bound to something, then broadcast the delegate
	if(StateMachine_EnterState.IsBound())
	{
		StateMachine_EnterState.Broadcast(CurrentState);
	}
}

void UStateMachineComponent::TickState(float DeltaTime)
{
	// If the tick state delegate is bound to something, then broadcast the delegate
	if(StateMachine_TickState.IsBound())
	{
		StateMachine_TickState.Broadcast(DeltaTime, CurrentState);
	}
}

void UStateMachineComponent::ExitState()
{
	// If the exit state delegate is bound to something, then broadcast the delegate
	if(StateMachine_ExitState.IsBound())
	{
		StateMachine_ExitState.Broadcast(CurrentState);
	}
}

void UStateMachineComponent::Debug()
{
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(DebugKey, 0.0f, FColor::Cyan, FString::Printf(TEXT("Current State for %s: %s"), *GetOwner()->GetName(), *CurrentState.ToString()));
	}
}



